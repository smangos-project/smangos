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
  Every worker walks the whole bot map on a jittered `RealBots.TickIntervalMs` interval (plus or
  minus 20%, so many workers don't all wake up in lockstep) and ticks only the bots pinned to it --
  and also wakes early to tick a bot the moment something is posted to it (see "Workers wake on
  work").
- **`RealBot` itself is touched only from its own worker thread** -- stage, identity, connections,
  brain, the command/inbox queues. The only thread-safe surfaces are `Post` (queue a callback to
  run on the bot's own next `Tick`), `QueueIncoming`, and `GetPublishedInfo()` (a mutex-guarded
  snapshot, `RealBotInfo`, refreshed once per `Tick` -- what `list`/`info`/`status` and
  `tests/conformance`'s `TestBot` all read from another thread). Anything that needs to *mutate* a
  bot from outside its own thread -- `RealBotMgr::RunTask`, `TestBot::Send`/`Disconnect`/`PushTask`
  -- does it by `Post`ing a closure, never by calling a mutator directly.
- **`RealBot::Tick`** order: drain posted commands, advance the clock, drain incoming packets (each
  dispatched through the hooks, then the task stack top-down, then default handling, with a
  zero-time brain update between packets -- see "Ordering guarantee"), check the logout deadline
  and the stage watchdog, maybe send a keepalive ping (only `InWorld`), update the brain (ticks only
  the top of the stack), publish the info snapshot.

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

- `ctx.Send(packet[, connection])` / `ctx.RawPacket(opcode, timeoutMs)` -- send/await one packet.
  Every world connection (`BotConnectionKind::Realm` / `Instance`) delivers into the bot's inbox, so
  a `BotPacket` carries the connection it arrived on or is bound for. `Send` picks the connection
  from the opcode by default (the handshake and character-screen opcodes go to Realm, everything
  else to Instance once there is one) unless told which. `ctx.RawPacket({opcodes...}, timeoutMs)`
  waits for the first of several.
- `ctx.Connect(kind, address, port, timeoutMs)` -> `BotConnectResult` -- opens a world connection the
  bot owns from the first byte. `ctx.SendAndArmEncryption(packet, kind, key)` sends the
  `CMSG_ENTER_ENCRYPTED_MODE_ACK` and arms that connection's encryption as it goes out (only if it
  really goes out: a hook that drops the ack leaves the connection unarmed).
- `ctx.Packet<T>(timeoutMs[, predicate])` -> `std::optional<T>` -- **observes**, never consumes;
  the bot's own default handling (and any other task's hook) still sees the same packet unless the
  awaiting/hooking task returns `BotPacketAction::Handled`.
- `ctx.Stage(stage, timeoutMs)` -> `bool`, `ctx.Delay(ms)`.
- `ctx.Run("name", args)` / `ctx.Run(routine)` -> `BotTaskResult` -- run another task to completion
  on top of this one, like calling a function.
- `ctx.Hold()` -- suspend until popped (removal, or the whole brain cleared) or disconnected.
- `ctx.Login(options)` -> `BotLogin::Result` -- the Battle.net half of the built-in `login` task
  (portal, RPC logon, realm join); the world half is `BotWorldLoginRoutine`, run with `ctx.Run`. See
  "What vibemangos needs during world entry" below for what they are actually doing.

`TestRoutine`'s free functions (`WhenAll`/`WhenAny`, over a `std::vector<Awaiter>` or over a range
with a function producing one awaiter per element -- for the latter, the function's parameter type
must match the range's own element type exactly, e.g. `std::unique_ptr<TestBot>&` for a
`std::vector<std::unique_ptr<TestBot>>`, not `TestBot&`) let a scenario coordinate several bots
without threads or polling in the test body itself.

**Ordering guarantee.** A packet-driven wait resumes its routine *inside* the dispatch that satisfied
it, and `RealBot::Tick` gives the brain a zero-time update between one incoming packet and the next
(and before the first, after posted completions such as a finished connect). So a routine, or a
routine's `ctx.Run` parent, has registered its next `co_await` before the packet after the one that
woke it is dispatched -- a server sending `SMSG_RESUME_COMMS` and `SMSG_LOGIN_VERIFY_WORLD` in one
write is not a race. It orders packets against awaits; it is not a buffer, so a packet nobody is
awaiting at that moment is gone (hooks and the packet log still saw it).
`BotBrain::Clear()` called from inside such a resume (a routine disconnecting or failing its own
bot) waits until the dispatch or update returns, so a routine is never destroyed while it runs.

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

Add it with `ctx.AddHook(std::make_unique<IgnoreTimeSyncTask>())` from inside a routine, or
`TestBot::AddHook` from a conformance scenario. A **hook** lives beside the brain's task stack, not
on it: it sees every packet (before the stack's tasks, in insertion order) and every stage change,
but is never `Update()`d and is never "on top", so it can't bury the routine that's running. It's
safe at any stage, including mid-login. Hooks go when the bot disconnects unless added with
`persistent=true`; `RemoveHook(handle)` removes one earlier, even from inside its own `OnPacket`.

`ctx.PushTask` / `TestBot::PushTask` still mean "a task that runs": **`BotBrain` only ticks the top of
the stack**, so a pushed task that never finishes stops whatever was ticking underneath it from
making progress. That is right for a task that does the bot's work, and wrong for something that only
watches or filters packets: use a hook for those.

## Stages and `stop_at`

```
Disconnected -> LoginPortal -> LoginRpc -> RealmSelect -> WorldConnecting -> WorldHandshake
             -> CharSelect -> EnteringWorld -> InWorld -> (LoggingOut ->) Disconnected
                                                        \-> Failed
```
(`Core/BotStage.h`.) A bot's stage is **the step in progress**, and `RealBotLoginOptions::StopAt`
(default `BotStage::InWorld`, meaning "don't stop early") holds the `login` routine as that stage
begins, *before* any of that stage's own work: `StopAt(LoginRpc)` has a portal login ticket and has
not touched the RPC port; `StopAt(WorldConnecting)` has joined a realm and opened no world socket;
`StopAt(WorldHandshake)` has a connected socket and has answered nothing (the server's
`SMSG_AUTH_CHALLENGE` reaches hooks and the packet log, unanswered); `StopAt(CharSelect)` has a
successful `SMSG_AUTH_RESPONSE` and an encrypted connection; `StopAt(EnteringWorld)` has a resolved
character and has not sent `CMSG_PLAYER_LOGIN`. Each stoppable value is checked where its stage
begins (`BotLogin.cpp`, `BotBnetLogin.cpp`, `Net/BotWorldLogin.cpp`: grep `StopAt`);
`tests/conformance/support/TestBot.h`'s `TestBotOptions().StopAt(...)` is how a conformance scenario
uses it (`RealBotMgr::AddWithOverrides` is the entry point behind it -- the plain runner/.realbot
`add <target>` still goes through the unmodified `Add`). A held bot keeps its connections open, so a
scenario can drive the rest of the login by hand with `TestBot::Send`.

A world connection closing while the login is running (`WorldConnecting` through `EnteringWorld`)
fails the bot at that stage (`Failed`, `FailedStage` set, `LastError` "realm connection closed
during WorldHandshake"); after `InWorld` it is a `Disconnected`/`ConnectionLost`.

A bot's `RealBotInfo::FailedStage` records what stage it was at right before `Failed`
("`info`" prints "`Failed (was CharSelect)`"), and `BotDisconnectReason` (`None` / `Requested` /
`ConnectionLost`) records whether `Disconnect()` was called first or the connection just dropped.

**Sessions.** `Disconnect(true)` is the clean one (`CMSG_LOGOUT_REQUEST`, wait for
`SMSG_LOGOUT_COMPLETE`, only from `InWorld`); `Disconnect(false)` is the abrupt "the client vanished":
the sockets close with no logout request and `Requested` is still what is recorded.
`RealBot::Reconnect()` (also `TestBot::Reconnect()`, `RealBotMgr::Reconnect(name)`, the `relog <name>`
console verb) drops whatever the bot is doing the same abrupt way -- connections closed, tasks and
non-persistent hooks gone -- clears the last session's error/failed stage/disconnect reason, and starts
a fresh `login` with the same account and options, from any stage. There is no automatic reconnect
policy; that is behavior. `RealBotMgr::Update` reaps a bot that has sat `Disconnected`/`Failed` for
`RealBots.ReapAfterMs`, unless it was added with `keep` (`add ... keep=1`, `RealBotAddOverrides::Keep`;
`TestBot` always does), which is what makes reconnecting a finished bot possible at all. Two sessions
on one account, for the duplicate-login scenarios, are `TestBotOptions().ShareAccountWith(otherBot)`
(`RealBotAddOverrides::ShareAccountWith`): it takes the other bot's credential without a second lease,
and the account only goes back to the pool when the last bot using it is removed. Bot names are
unique: a second bot on the same account is named `<account>#2`.

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
   original Realm connection stays open afterward (`SMSG_LOGOUT_COMPLETE` still routes through it,
   per vibemangos's `Opcodes.cpp` `PacketRoute::Realm` table; a ping is answered on the socket it
   arrived on -- `WorldSocket::_HandlePing` writes the pong itself, whatever route the table lists
   for `SMSG_PONG`); `SMSG_UPDATE_OBJECT` is
   received on the Instance connection and skipped, unparsed.

**A failed Instance connect is retried**, as both reference servers do it: the failure (a refused or
dropped connection, a handshake that times out) is reported on the Realm connection with
`CMSG_CONNECT_TO_FAILED`, echoing the failed attempt's serial (17, 35, 53, 71, 89) and connection
type, and the server answers with the next `SMSG_CONNECT_TO`; the fifth failure aborts the login
(`Net/BotWorldLogin.cpp`, `MaxInstanceConnectAttempts`). A hook can force a failure without
touching the network: `BotTask::OnConnect` runs before every world connect and refusing it
(`Drop`/`Handled`) makes that connect fail at once -- the way to test the retry against a live
server. An Instance connection closing during `EnteringWorld` does not fail the bot by itself (the
routine is woken and decides); every other world connection closing during the login does.

**Timeouts are the target's** (`RealBots.Target.X.ConnectTimeoutMs`, default 10000, bounds every
connect: portal, RPC, world; `StageTimeoutMs`, default 30000, is the per-stage watchdog in
`RealBot::Tick`): a login stage held longer fails the bot with `FailedStage` set and `LastError`
"stage X timed out after Nms", except `InWorld`, `LoggingOut` (own timeout), `Disconnected`, and a
stage held on purpose by `StopAt`. Every individual packet wait inside the world routines is also
bounded (`RealBotLoginOptions::PacketTimeoutMs`, 10000), so the watchdog is the backstop, not the
usual failure. Stages are reported as they begin, including `LoginRpc` and `RealmSelect` from the
Battle.net side, so `FailedStage` says where a login really died ("forged ticket rejected at
`LoginRpc`").

**Workers wake on work.** `RealBotMgr`'s worker threads tick every bot on `RealBots.TickIntervalMs`
(100 ms, jittered) but also wake as soon as any bot pinned to them has something to do (a packet, a
finished connect, another thread's command; `RealBot::SetWakeCallback`). That matters for protocol
timing, not just latency: vibemangos sends `SMSG_CONNECT_TO` in parallel with loading the
character, and a client that opens the Instance connection late loses the race --
`SMSG_LOGIN_VERIFY_WORLD` is then never delivered on it. A bot ticking only every 100 ms did exactly
that (found by running the conformance suite live), a real client never would.

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

`vibemangos` is the target every scenario must pass on; `trinitycore` is a backup reference, used by
hand only when vibemangos itself is in doubt (not yet stood up, see below); `vmangos` (this tree's own
root server) is defined and expected to fail everything until the port lands (that's the whole point
of `tests/conformance/expectations/vmangos.txt` -- it's the porting scoreboard, and it shrinks as
`port-plan.md` lands; `expectations/vibemangos.txt` is the same file format for what the *reference*
does not pass). `Tags` is what `tests-conformance --target <name>` uses to exclude
`[protocol]`/`[gameplay]`/`[slow]`/`[load]`-tagged scenarios the target structurally can't run (see
below) -- accounts are provisioned out-of-band with `contrib/realbotrunner/provision-accounts.ps1`, and
`tests/conformance/run-all.ps1` prints one scoreboard per target --
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
  `ASAN_OPTIONS=detect_leaks=0` (or `ctest -C RelWithDebInfo -L unit --repeat until-fail:50` there: the
  whole unit suite, 159 tests, passed 50 consecutive times under ASan on 2026-09-24, 22 minutes). The
  intermittent `BotBnetConnectionTests` crash from Phase 1 never reproduced (50 whole-binary ASan runs,
  100 rapid Release runs, and that repeat), so nothing was changed for it and it is not known to be
  fixed. Note the suite uses fixed loopback ports: never run two copies at once.
- The world half of the login has its own scripted fake, `tests/support/FakeWorldServer.h`: one
  straight-line script per accepted connection over a blocking `FakeWorldPeer` (V2 banner, framing,
  `PacketCrypt`, typed `Send`/`Expect`), with the Realm and Instance handshakes as ready-made
  functions. It only knows the wire protocol, so the same scripts test the handshake however the bot
  is built. `TestBotDriver` runs a real `RealBot` on a ticking thread against it
  (`BotWorldLoginTests.cpp`: every handshake packet is checked to reach hooks, in order, on the right
  connection; `BotLoginTests.cpp` runs the whole built-in `login` task against fake Battle.net
  servers plus this).
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

  **The scenarios** (all `[protocol]`, one directory per `port-plan.md` step, each with a line in
  `expectations/vmangos.txt` naming the step that will make it pass): `login/` (step 2: ticket, wrong
  password, unknown account, RPC logon, forged ticket, realm list, realm join, unlisted realm),
  `world_socket/` (step 3: encrypted mode, signature, wrong digest, foreign ticket, never-ack, unknown
  and truncated packets, compression, time sync, pong routing), `char_screen/` (step 4: enum, create,
  delete, duplicate and invalid names, a foreign guid), `world_entry/` (step 5: the redirect and its
  retry, `SMSG_LOGIN_VERIFY_WORLD`, logout), `session/` (reconnect after a clean and an abrupt exit, a
  second login, idling) and `load/` (`[load]`: 10, 50 and 100 bots). `[slow]` scenarios wait on
  server-side timeouts or long idles (minutes); `[load]` ones need a big account pool and SKIP,
  saying how many accounts they wanted, when it is smaller. Both run only if the target's `Tags`
  list them, so a plain `--target vibemangos` stays quick. `tests/conformance/run-all.ps1` runs every
  configured target and prints a scoreboard for each (with `-Strict`: which expected-fail scenarios
  now pass). Against the dev vibemangos, with `slow` in its tags: 35 passed, 0 failed, 2 skipped (the
  create/delete scenarios: the test account is at the realm's character limit), 1 listed in
  `expectations/vibemangos.txt`; the `[load]` scenarios were only ever seen to skip (one account).
- **`contrib/realbotrunner`** (`BUILD_REALBOT_RUNNER`): a standalone console,
  `realbotrunner -c realbotrunner.conf`. `targets`, `add [target] [count] [keep=1]`, `remove <name|all>`,
  `relog <name>`, `list`, `info <name>`, `task <name> <task> [args]`, `tasks <name>`, `status`, `help`, plus its own
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

## vibemangos vs TrinityCore

Where the two references disagree, the bots follow the one named here and say why (`port-plan.md`:
wire format favors TrinityCore unless our client build says otherwise; gameplay favors vibemangos).
None of these is proven against a real 1.15.6 client yet.

- **`CMSG_CONNECT_TO_FAILED` field order.** vibemangos reads `Serial` (u32) then `Con` (u8)
  (`Character.cpp`, `ConnectToFailed::ReadFromWorldPacket`, "confirmed against jimsproxy");
  TrinityCore cata_classic reads `Con` (u8) then `Serial` (u32) (`AuthenticationPackets.cpp`,
  `ConnectToFailed::Read`). The escalation itself (a failure of attempt N asks for N+1, five in all,
  serials 17/35/53/71/89, connection type 1 for the Instance) is identical. **Followed: vibemangos**,
  since it is the server the bots are proven against first and the tree the port lands in; a 4.4.x
  layout difference or a bug in either is equally possible. `SMSG_CONNECT_TO` itself is identical in
  both (`... port, serial, con, key`). Resolve with a captured real-client session
  (`bot-phase-2.md`, "Open questions").

- **Login tickets are not single-use.** `bot-phase-2.md` listed "a reused login ticket is rejected at
  LoginRpc"; neither reference does that. TrinityCore's bnetserver checks `LoginTicketExpiry`
  (`AuthenticationService.cpp`) and vibemangos's `LoginTicketStore` is likewise a TTL store
  (`refreshLoginTicket` extends it): a ticket is *expired*, never *consumed*, so a used one that
  has not expired logs in again (confirmed live against vibemangos). The scenario was dropped rather
  than written to encode a rule no server has; a forged ticket is rejected and is covered.

- **No timeout for a connection stuck before the encrypted-mode ack.** A bot that never sends
  `CMSG_ENTER_ENCRYPTED_MODE_ACK` (`world_socket: never acking ...`, `[slow]`) leaves vibemangos's
  Realm connection open for good: still connected after 420 s. The scenario stays (a server must
  eventually close a connection that never finishes its handshake) and is listed in
  `expectations/vibemangos.txt`, the file for what the *reference* does not pass, so the port is
  held to it while the reference run stays green. `ServerCloseTimeoutMs` (default 30000) is the
  bound a target grants.

- **The create/delete scenarios need a free character slot.** `char_screen: a created character ...`
  and `... name already in use` create (and delete) a character with a random name; on an account
  already at the realm's character limit (`CHAR_CREATE_SERVER_LIMIT`, 30) they `SKIP` with that
  reason instead of failing, and never touch the characters that are there. The dev vibemangos this
  was developed against caps accounts at three, so they only run on a target whose pool accounts
  have room (create-and-delete leaves the account as it found it).

- **A logout answers on the Realm connection and closes the Instance one.** vibemangos sends
  `SMSG_LOGOUT_COMPLETE` on the Realm socket and closes the Instance socket right after
  (`WorldSession::LogoutPlayer`, matching TrinityCore). A bot waiting on a clean logout therefore
  ignores the Instance connection closing (handling it as a disconnect tore the session down and
  discarded the very packet still in flight -- found by the logout scenario) and ends on
  `SMSG_LOGOUT_COMPLETE`, the Realm connection closing, or `LogoutTimeoutMs`. The server's own
  countdown (about 20 s in the open world, instant when resting) is why that scenario is `[slow]` and
  takes `TestBotOptions().LogoutTimeoutMs(...)`.

## TrinityCore as a cross-check (spike outcome: not run)

`bot-phase-2.md` planned TrinityCore `cata_classic` as a routine second target, starting with a spike:
build it, add build 58797, run the scenarios, see how far a 1.15.6 bot gets. **The spike was not run
here, and no `trinitycore` target has been proven**, for reasons that are about this environment, not
about the bots:

- A run needs the whole server stack: bnetserver plus worldserver built (Boost is in the tree, but
  the MySQL client and a full build are not something to do to a shared dev machine unasked), a MySQL
  server with TrinityCore's own `auth` database, and, past the login server, the `world`/`characters`
  databases with the multi-gigabyte TDB world data. Creating databases on the MySQL the vibemangos dev
  server uses was not something to do without being asked, and the login server alone (`login/`) is
  only the first quarter of the suite.
- Static reading of its source found what a run would need first: the auth database's `build_info` and
  `build_auth_key` tables (`sql/base/auth_database.sql`) have **no row for build 58797**, so
  `AuthenticationService.cpp`'s `HandleLogon` build check rejects the bot until
  `INSERT INTO build_info VALUES (58797, 1, 15, 6, NULL)` and the matching `build_auth_key` rows exist
  (the auth key is client key material the bots do not have, so the world-socket half may need
  TrinityCore-side changes too). Wire-format differences already found by reading both trees are in
  "vibemangos vs TrinityCore" above (`CMSG_CONNECT_TO_FAILED`'s field order); the handshake and
  character-screen opcode values are identical in both.

So the by-hand reference TrinityCore already was stays that way: `expectations/trinitycore.txt` stays
empty (a line there would be a guess without a run), the `trinitycore` block in the conf.dist is the
starting point for whoever stands one up, and `tests/conformance/run-all.ps1` will run it in turn with
the others the moment it is configured. Outcome 3's second half of bot-phase-2.md's "Done when" is
therefore **not met**; the recorded reason above is what the spike's own stop condition asks for.

## Where to look next

`bot-phase-1.md`'s "Done when" bar is now fully met except live-verifying `.realbot` against a real
game client (blocked on having one available) and `tests-conformance`'s "ten bots log in at once"
scenario (now `load: ten bots log in at once`, tagged `[load]`, needing ten free accounts on the live
target; this dev environment has one, so the `[load]` scenarios are only ever seen to SKIP here). Concrete gaps,
tracked and scheduled in `bot-phase-2.md`:

- Phase 2 (`bot-plan.md`): more `[protocol]` scenarios (reconnect, timeouts under load, many bots
  at once for real), TrinityCore as a routine cross-check rather than an emergency-only one, and
  starting to port login/world-entry backwards into this tree so `vmangos.txt`'s scoreboard starts
  shrinking.
