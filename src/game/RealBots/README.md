# RealBots

A client bot framework, embedded in `mangosd` and in a standalone console host
(`contrib/realbotrunner`), that speaks the **modern** (1.15.x classic-era) WoW protocol as a real
client would. It drives the vmangos-to-modern-client port described in `.local/plan/bot/port-plan.md`
and its own conformance suite (`tests/conformance/`). Spec: `.local/plan/bot/bot-phase-1.md`;
design direction: `.local/plan/bot/bot-plan.md`; current spec: `.local/plan/bot/bot-phase-2.md`.

Everything here lives behind `ENABLE_REALBOTS` (`#ifdef REALBOTS` at every touch point outside this
directory: three lines in `World.cpp`, a chat command in `Chat.h`/`Chat.cpp`). Protocol code the
server itself also uses -- `src/shared/Crypto`, `bnetproto`, `nlohmann/json` -- is never behind the
switch, since the server-side port will need it unconditionally.

## Design principles

1. **Bots touch no server runtime state.** No `sWorld`, `Map`, `Player`, `WorldSession` or live
   `sObjectMgr` anywhere in this directory -- everything a bot knows arrives as a packet, exactly
   like a real client. `contrib/realbotrunner` is the proof: it never initializes the world at all,
   and links only `shared`/`framework`/`bnetproto`/`zlib`/OpenSSL, never `game`'s script loader or
   database layer.
2. **Built for the modern protocol, directly.** No protocol abstraction, no dummy protocol. Any
   task or test can send any packet (typed or raw, malformed on purpose), observe or intercept any
   packet, and drive the login sequence stage by stage.
3. **Coroutines for procedures and tests**, built and unit-tested before anything networked. A bot
   procedure (`BotRoutine`) and a test scenario (`TestRoutine`) share the same awaitable
   vocabulary and differ only in which thread resumes them.
4. **Threading built for vmangos's `IO` library** (no strands): IO threads only move bytes into a
   bot's inbox; each bot lives on exactly one worker thread and needs no locks of its own; bots
   never run on the world thread.

## Threading model

- **`RealBotMgr`** owns one `IO::IoContext`, `RealBots.IoThreads` IO threads, and
  `RealBots.WorkerThreads` worker threads. IO threads only decode bytes off the wire into a bot's
  inbox (`RealBot::QueueIncoming`, reached through `RealBot::Post` so it's never touched directly
  from another thread); they do nothing else.
- **Each bot is pinned to one worker** at `Add` time (`id % WorkerThreads`) and never rebalanced.
  Every worker walks the whole bot map on a jittered `RealBots.TickIntervalMs` interval (±20%, so
  many workers don't all wake up in lockstep) and ticks only the bots pinned to it.
- **`RealBot` itself is touched only from its own worker thread** -- stage, identity, connections,
  brain, the command/inbox queues. The only thread-safe surfaces are `Post` (queue a callback to
  run on the bot's own next `Tick`), `QueueIncoming`, and `GetPublishedInfo()` (a mutex-guarded
  snapshot, `RealBotInfo`, refreshed once per `Tick` -- what `list`/`info`/`status` and
  `tests/conformance`'s `TestBot` all read from another thread). Anything that needs to *mutate* a
  bot from outside its own thread -- `RealBotMgr::RunTask`, `TestBot::Send`/`Disconnect`/`PushTask`
  -- does it by `Post`ing a closure, never by calling a mutator directly.
- **`RealBot::Tick`** order: drain posted commands, advance the clock, drain incoming packets
  (dispatched top-down through the task stack, then default handling), check the logout deadline,
  maybe send a keepalive ping, update the brain (ticks only the top of the stack), publish the
  info snapshot.

## The coroutine layer

C++20 has no ready-made task type, so the framework carries two small ones, sharing one vocabulary,
differing only in which thread resumes them:

| | `BotRoutine` (`Coroutines/BotRoutine.h`) | `TestRoutine` (`tests/support/TestRoutine.h`) |
|---|---|---|
| Runs on | the bot's own worker thread, via `RealBot::Tick` -> `BotBrain::Update` | the test thread, via `RunTest`'s single-threaded executor |
| Awaits through | `BotContext` (`ctx.Packet`, `ctx.Stage`, ...) | `TestBot` (`bot.Packet`, `bot.Stage`, ...) in conformance scenarios, or `TestFakeBot`/`TestBot` (unit) for the coroutine layer itself |
| Used by | tasks registered in `BotTaskRegistry`, starting with the built-in `login` | `tests/unit/realbots/*Tests.cpp` and `tests/conformance/*/*.cpp` |

Every wait has a timeout; there is no way to wait forever except `ctx.Hold()` /
`TestBot`'s equivalent, which is exactly what a routine uses to stay parked once it has nothing
left to do (`login` holds at `InWorld`; `idle` holds immediately).

`BotContext` awaitables (`Behavior/BotContext.h`, `Coroutines/BotAwaitables.h`):

- `ctx.Send(packet)` / `ctx.RawPacket(opcode, timeoutMs)` -- send/await one packet.
- `ctx.Packet<T>(timeoutMs[, predicate])` -> `std::optional<T>` -- **observes**, never consumes;
  the bot's own default handling (and any other task's hook) still sees the same packet unless the
  awaiting/hooking task returns `BotPacketAction::Handled`.
- `ctx.Stage(stage, timeoutMs)` -> `bool`, `ctx.Delay(ms)`.
- `ctx.Run("name", args)` / `ctx.Run(routine)` -> `BotTaskResult` -- run another task to completion
  on top of this one, like calling a function.
- `ctx.Hold()` -- suspend until popped (removal, or the whole brain cleared) or disconnected.
- `ctx.Login(options)` -> `BotLogin::Result` -- what the built-in `login` task's whole body is; see
  "What vibemangos needs during world entry" below for what it's actually doing.

`TestRoutine`'s free functions (`WhenAll`/`WhenAny`, over a `std::vector<Awaiter>` or over a range
with a function producing one awaiter per element -- for the latter, the function's parameter type
must match the range's own element type exactly, e.g. `std::unique_ptr<TestBot>&` for a
`std::vector<std::unique_ptr<TestBot>>`, not `TestBot&`) let a scenario coordinate several bots
without threads or polling in the test body itself.

**Tested without a server** through `TestFakeBot` (`tests/support/TestFakeBot.h`): a `RealBot` with
no connections, ticked by hand, packets injected directly. Exercises resumption-only-from-`Tick`,
timeouts, `Run` composition, `Hold`, observe-vs-`Handled`, frames destroyed without resuming on
removal, exceptions becoming `Failed`, and cross-thread completions -- none of it needs a socket.

## Writing a task or routine

Register a coroutine-based task with `sBotTaskRegistry.RegisterRoutine`, called once from
`RegisterBuiltinBotTasks()` (`Behavior/Tasks/BuiltinBotTasks.cpp`) or your own registration point:

```c++
namespace
{
BotRoutine LatencyProbe(BotContext& ctx, uint32 intervalMs, uint32 count)
{
    if (!co_await ctx.Stage(BotStage::InWorld, 60000))
        co_return BotTaskResult::Failed;

    for (uint32 i = 0; i < count; ++i)
    {
        ClassicPing ping;
        ping.Serial = 0x40000000 + i;
        ctx.Send(ping);

        auto pong = co_await ctx.Packet<ClassicPong>(5000, [&](ClassicPong const& p) { return p.Serial == ping.Serial; });
        if (!pong)
            co_return BotTaskResult::Failed;

        co_await ctx.Delay(intervalMs);
    }
    co_return BotTaskResult::Done;
}
}

void RegisterLatencyProbeTask()
{
    sBotTaskRegistry.RegisterRoutine("latency_probe", [](BotContext& ctx, BotTaskArgs const& args)
    {
        return LatencyProbe(ctx, args.GetUInt32("interval", 2000), args.GetUInt32("count", 10));
    });
}
```

It reads top to bottom -- no state machine, no `Update(diff)` bookkeeping. Run it by hand with
`task <bot> latency_probe interval=1000,count=5` (the runner console or `.realbot`), or from another
routine with `co_await ctx.Run("latency_probe", args)`.

For something that's mostly packet hooks rather than a sequence, use the plain `BotTask` interface
instead (`Behavior/BotTask.h`):

```c++
class IgnoreTimeSyncTask : public BotTask
{
    public:
        char const* GetName() const override { return "ignore_time_sync"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            if (dir == BotPacketDir::Incoming && packet.Opcode == CLASSIC_SMSG_TIME_SYNC_REQUEST)
                return BotPacketAction::Handled; // swallow it: no CMSG_TIME_SYNC_RESPONSE goes out
            return BotPacketAction::Continue;
        }
};
```

Push it with `ctx.PushTask(std::make_unique<IgnoreTimeSyncTask>())` from inside a routine, or
`TestBot::PushTask` from a conformance scenario. **`BotBrain` only ticks the top of the stack** --
every task's `OnPacket` hook fires top-down regardless of what's ticking, but `Update()` does not.
Pushing a task that never finishes (like this one, or `tests/conformance/framework/
FrameworkTests.cpp`'s `DropPingTask`) permanently stops whatever was ticking underneath it from
making progress. That's harmless once the thing underneath has nothing left it needs ticking for
(an already-`Hold()`ing `login`, or `idle`) -- and a real bug, found by running a scenario live and
watching it hang, if pushed while `login` is still actively resolving its own awaiters (which *are*
polled from `Update()`, not pushed some other way). Push denylist-style tasks like this only once
the bot is already parked where you want it.

## Stages and `stop_at`

```
Disconnected -> LoginPortal -> LoginRpc -> RealmSelect -> WorldConnecting -> WorldHandshake
             -> CharSelect -> EnteringWorld -> InWorld -> (LoggingOut ->) Disconnected
                                                        \-> Failed
```
(`Core/BotStage.h`.) `RealBotLoginOptions::StopAt` (default `BotStage::InWorld`, meaning "don't
stop early") holds the `login` routine once that stage is reached, *before* that stage's own
network step runs -- e.g. `StopAt(CharSelect)` resolves a character but never sends
`CMSG_PLAYER_LOGIN`. Each stoppable value is checked at its own stage's step (`BotLogin.cpp`,
`BotBnetLogin.cpp`, `BotCharSelect.cpp`: grep `m_options.StopAt`);
`tests/conformance/support/TestBot.h`'s `TestBotOptions().StopAt(...)` is how a conformance scenario uses it (`RealBotMgr::AddWithOverrides` is the entry point behind
it -- the plain runner/.realbot `add <target>` still goes through the unmodified `Add`).

A bot's `RealBotInfo::FailedStage` records what stage it was at right before `Failed`
("`info`" prints "`Failed (was CharSelect)`"), and `BotDisconnectReason` (`None` / `Requested` /
`ConnectionLost`) records whether `Disconnect()` was called first or the connection just dropped.

## What vibemangos needs during world entry

Confirmed live against a running vibemangos, and (for the redirect) against TrinityCore
cata_classic's source directly, since it looked at first like it might be a vibemangos-only quirk:

1. **`LoginPortal`**: HTTPS to `LoginAddress:RestPort`; SRP6 login (`POST /bnetserver/login/srp/`
   for the challenge, `POST /bnetserver/login/` with the proof) -> a login ticket. The account name
   *and* password are uppercased (Latin-only) before any SRP math -- `AccountMgr::CreateAccount`
   normalizes both the same way when an account is provisioned, and a password that works from a
   real client is silently rejected if the bot skips this (`Net/BotRestClient.cpp`'s
   `NormalizeAccountString`).
2. **`LoginRpc`**: TLS to `LoginAddress:RpcPort`; `ConnectionService.Connect` ->
   `AuthenticationService.Logon` -> `AuthenticationService.VerifyWebCredentials` with the ticket ->
   wait for the `AuthenticationListener.OnLogonComplete` push (arrives independently of
   `VerifyWebCredentials`'s own RPC response; both must complete before moving on).
3. **`RealmSelect`**: `GameUtilitiesService.ProcessClientRequest` three times --
   `Command_RealmListTicketRequest_v1` (carries a 32-byte client secret, half of the eventual world
   session key), `Command_RealmListRequest_v1` (zlib-compressed JSON realm list), then
   `Command_RealmJoinRequest_v1` for the chosen realm -> world server address, a join ticket (the
   account id as a decimal string), and the server's half of the session key.
4. **`WorldConnecting`/`WorldHandshake`**: V2 banner strings, then `SMSG_AUTH_CHALLENGE` ->
   `CMSG_AUTH_SESSION` (an HMAC-SHA512 digest over the two challenges and the join session key) ->
   `SMSG_ENTER_ENCRYPTED_MODE` (an Ed25519 signature, checked if `VerifyServerSignature`) ->
   `CMSG_ENTER_ENCRYPTED_MODE_ACK` -> `PacketCrypt` armed on both sides -> `SMSG_AUTH_RESPONSE`.
5. **`CharSelect`**: `CMSG_ENUM_CHARACTERS`; create one (`CMSG_CREATE_CHARACTER`) if none exist and
   the caller asked for that.
6. **`EnteringWorld` -> `InWorld`**: `CMSG_PLAYER_LOGIN` -> the server *unconditionally* redirects
   to a second ("Instance") world connection via `SMSG_CONNECT_TO` -- this is not a vibemangos
   quirk, TrinityCore cata_classic's `WorldSession::HandlePlayerLoginOpcode` does the exact same
   thing, so it's mandatory, not optional, and was moved in-scope for Phase 1 after starting out
   explicitly excluded. The Instance connection repeats the V2-banner/`AUTH_CHALLENGE`/
   `ENTER_ENCRYPTED_MODE` dance, but with `CMSG_AUTH_CONTINUED_SESSION` in place of
   `CMSG_AUTH_SESSION` (reusing the Realm connection's already-established session key, no fresh
   exchange) and `SMSG_RESUME_COMMS` in place of `SMSG_AUTH_RESPONSE`. `SMSG_LOGIN_VERIFY_WORLD`
   only ever arrives on *this* connection -- that's the framework's own `InWorld` marker. The
   original Realm connection stays open afterward (`SMSG_PONG`/`SMSG_LOGOUT_COMPLETE` still route
   through it, per vibemangos's `Opcodes.cpp` `PacketRoute::Realm` table); `SMSG_UPDATE_OBJECT` is
   received on the Instance connection and skipped, unparsed.

Not yet ported: the up-to-5-attempt `CMSG_CONNECT_TO_FAILED` retry
(`Net/BotInstanceConnect.h`'s own `TODO(port)`) -- a failed Instance connect just fails the whole
login instead. `RealBotTarget::ConnectTimeoutMs`/`StageTimeoutMs` are read and stored but not
consumed anywhere yet; every real connect attempt hardcodes 10 seconds
(`Net/BotConnector.h`'s callers).

**Default packet handling** (only when no task's hook already handled the packet): `SMSG_PONG`
bookkeeping (updates latency), `CMSG_TIME_SYNC_RESPONSE` to a `SMSG_TIME_SYNC_REQUEST`, and
periodic `CMSG_PING`/`CMSG_KEEP_ALIVE` on `RealBots.PingIntervalMs` once `InWorld`
(`RealBot::UpdateKeepalive`/`ApplyDefaultHandling`).

## Adding a target

A target is a named server profile -- addresses, realm, client build, tags, and a pool of existing
accounts leased one per bot (`RealBotTarget`/`BotAccountLease`). Adding one needs no code:

```
RealBots.Targets = "vibemangos trinitycore vmangos"

RealBots.Target.vibemangos.LoginAddress          = "127.0.0.1"
RealBots.Target.vibemangos.RestPort              = 8081
RealBots.Target.vibemangos.RpcPort               = 1119
RealBots.Target.vibemangos.RealmName             = "VibeMangos"   # wins over RealmId when set
RealBots.Target.vibemangos.ClientBuild           = 58797
RealBots.Target.vibemangos.ClientVersion         = "1.15.6"
RealBots.Target.vibemangos.Tags                  = "protocol gameplay"
RealBots.Target.vibemangos.Accounts              = "bot1@bots:pass bot2@bots:pass"
RealBots.Target.vibemangos.VerifyServerSignature = 1
```

`vibemangos` is the only target Phase 1 must actually pass on; `trinitycore` is a backup reference,
used by hand only when vibemangos itself is in doubt; `vmangos` (this tree's own root server) is
defined and expected to fail everything until the port lands (that's the whole point of
`tests/conformance/expectations/vmangos.txt` -- it's the porting scoreboard, and it shrinks as
`port-plan.md` lands). `Tags` is what `tests-conformance --target <name>` uses to exclude
`[protocol]`/`[gameplay]`-tagged scenarios the target structurally can't run (see below) --
`contrib/realbotrunner/realbotrunner.conf.dist` and `tests/conformance/targets/
tests-conformance.conf.dist` both have the full shape, including every global `RealBots.*` key.

## Running it

- **`ctest -L unit`**: hermetic, no network, no live server -- `tests-realbots` (the framework
  itself: packets, framing, the coroutine layer, tasks, targets/leases) and `tests-shared`
  (crypto). `[live]`-tagged cases (`tests/unit/realbots/BotLoginLiveTests.cpp`) are excluded from
  this (`TEST_SPEC "~[live]"`) and only ever run by hand, against a real server, with
  `tests-realbots.exe "[live]" --success`.
  For memory errors, configure a separate tree with `-DENABLE_ASAN=ON` (clang or MSVC; output goes to
  `bin-asan/`, never over a normal build's `bin/`), build `tests-realbots` there and run it with
  `ASAN_OPTIONS=detect_leaks=0`. The intermittent `BotBnetConnectionTests` crash from Phase 1 did not
  reproduce in 50 whole-binary runs under ASan, 50 `ctest -L unit --repeat until-fail:50` passes, or
  100 rapid whole-binary Release runs (2026-09-24); nothing was changed for it.
- **`tests-conformance`** (`BUILD_TESTING`, `ENABLE_REALBOTS`): talks to a real target over the
  network, driven through `TestBot`/`TestBotOptions` (`tests/conformance/support/`), the same
  awaitable vocabulary as `BotContext` bridged across threads (`Stage`/`Disconnected` poll
  `RealBot::GetPublishedInfo()`; `Packet<T>`/`RawPacket`/`NoPacket` are event-driven, through a
  `RealBot::SetPacketObserver` hook that -- deliberately unlike a pushed `BotTask` -- survives
  `BotBrain::Clear()` on disconnect). Runs from any working directory (apart from the OpenSSL
  `legacy.dll` note below): `expectations/<target>.txt` is found relative to the config file (its
  directory, then its parent), falling back to the source tree the binary was built from:
  ```
  tests-conformance --target vibemangos --config targets/tests-conformance.conf
  ```
  `--target` (or `REALBOTS_TARGET`) is required; `--config`/`REALBOTS_CONFIG` defaults to
  `tests-conformance.conf` in the working directory, else next to the executable. It prints what it excluded and why before running -- every scenario
  tagged with something the target's own `Tags` don't list (always, `--strict` never overrides
  this, since it's a structural capability mismatch, not a "not yet ported" placeholder) plus every
  scenario named in `expectations/<target>.txt` (skipped under `--strict`, to find newly-passing
  ones). Any extra Catch2 filter still works on top, e.g.
  `tests-conformance --target vibemangos "[protocol]"`.
- **`contrib/realbotrunner`** (`BUILD_REALBOT_RUNNER`): a standalone console,
  `realbotrunner -c realbotrunner.conf`. `targets`, `add [target] [count]`, `remove <name|all>`,
  `list`, `info <name>`, `task <name> <task> [args]`, `tasks <name>`, `status`, `help`, plus its own
  `quit`/`exit` -- everything else goes through `RealBotCommands::Handle`
  (`RealBotCommands.h/.cpp`), shared verb-for-verb with:
- **`.realbot` in mangosd** (`RealBots.Enable = 1`, `SEC_ADMINISTRATOR`, console-allowed): the same
  verbs, e.g. `.realbot add vibemangos 5`. `World.cpp` starts `sRealBotMgr` at the end of
  `SetInitialWorldSettings`, ticks its lifecycle housekeeping (`Update`, not the per-bot ticking --
  that's the worker pool's own threads) next to `sPlayerBotMgr.Update`, and stops it in
  `World::Shutdown`. Never verified against a real, connected game client in this environment (no
  client available) -- only by code review and by `mangosd` linking successfully both with
  `ENABLE_REALBOTS` on and off.

**A local environment gotcha, not a RealBots bug**: if crypto init fails with "Failed to load
openssl legacy provider," it's about the process's **current working directory at launch**, not
where `legacy.dll` or the executable physically live -- `Crypto::InitializeCryptoAndPrintVersion`
(`src/shared/Crypto/InitializeCrypto.cpp`) retries `OSSL_PROVIDER_load` after pointing OpenSSL's
provider search path at `ToAbsolutePath(".")`. Run from the directory `legacy.dll` actually sits in
(the build's shared output directory, e.g. `bin/RelWithDebInfo/`), not from the repo root or
elsewhere, even with the DLL correctly copied there by `mangos_copy_openssl_dlls`.

## Where to look next

`bot-phase-1.md`'s "Done when" bar is now fully met except live-verifying `.realbot` against a real
game client (blocked on having one available) and `tests-conformance`'s "ten bots log in at once"
scenario (needs ten free accounts on the live target; this dev environment has one). Concrete gaps,
tracked and scheduled in `bot-phase-2.md`:

- Packets still aren't observable while a bot is mid-handshake (`WorldConnecting` through
  `CharSelect`) -- `BotWorldHandshake`/`BotCharSelect` still take a connection's packet callback
  over exclusively rather than routing through `RealBot`'s normal dispatch pipe the way everything
  from `InWorld` onward does. Rewriting those two as `ctx`-driven coroutines would fix it, but is a
  bigger, higher-risk change than anything attempted so far.
- `StopAt(EnteringWorld)`, and `BotInstanceConnect` (the class driving the `SMSG_CONNECT_TO`
  redirect) more generally, have zero test coverage anywhere -- unit or conformance. A fake
  "Instance" world socket (mirroring `tests/unit/realbots/BotWorldHandshakeTests.cpp`'s style, but
  continuing past the handshake into the redirect) would close both gaps at once.
- `RealBotTarget::ConnectTimeoutMs`/`StageTimeoutMs` are unconsumed (see above).
- Phase 2 (`bot-plan.md`): more `[protocol]` scenarios (reconnect, timeouts under load, many bots
  at once for real), TrinityCore as a routine cross-check rather than an emergency-only one, and
  starting to port login/world-entry backwards into this tree so `vmangos.txt`'s scoreboard starts
  shrinking.
