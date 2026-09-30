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
        Ping ping;
        ping.Serial = 0x40000000 + i;
        ctx.Send(ping);

        auto pong = co_await ctx.Packet<Pong>(5000, [&](Pong const& p) { return p.Serial == ping.Serial; });
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

## Running the port target

The root server (`bnetserver` + `mangosd`) runs beside the live vibemangos on its own database
instance, ports and data links; it never touches the live server or its databases. `contrib/vmport/`
holds the steps. From a fresh checkout, with a scratch MariaDB/MySQL server available for the login,
character and log databases (the world database is a DoltLite file):

1. Create the three schemas (`vmport_realmd`, `vmport_characters`, `vmport_logs`) and a
   user with all privileges on them. This setup used a second `mariadbd` on `127.0.0.1:3309` started
   from the installed binaries with its own data directory under `.local/vmport`.
2. `contrib/vmport/setup-databases.ps1` loads `sql/logon.sql`, `characters.sql`, `logs.sql`, then their
   migrations in `sql/migrations`. Build the world database with `database_tool create`, which writes
   `.world/world.doltlite`; `CheckRequiredMigrations` at startup checks it is current.
3. Build the `server_setup` target with `-DSERVER_MAP_DATA_DIR=<the maps folder>`. It creates `.server/`
   (configs from the `.dist` files, the development certificate, a link to the map data). Edit the
   database strings and ports (REST 8091, RPC 1129, world 8095, instance 8096) in
   `.server/config/mangosd.conf` and `bnetserver.conf` once; mangosd adds the realm row itself.
4. Run `bin/RelWithDebInfo/bnetserver.exe` and `mangosd.exe` with no arguments (both change into `.server`).
5. `account create <name> <password>` on the `mangosd` console provisions each bot account, including
   its Battle.net row. `contrib/realbotrunner/provision-accounts.ps1 -Flavor vmangos` prints the
   commands and the `Accounts` line.
6. `tests-conformance --target vmangos` with the `vmangos` block of `tests-conformance.conf.dist`.
   Scenarios that need a game master (a speed change, a root or a teleport, driven by chat
   commands) lease from a companion target named `<target>-gm` and skip when it is not configured:
   create one account, `account set gmlevel <name> 4 -1`, and list it as the `vmangos-gm` target's
   only account (same address and realm as `vmangos`, no `Tags`).
7. `debug unported` on the `mangosd` console (or in game, `.debug unported`) lists every vanilla SMSG a
   session tried to send and the legacy lane dropped, with counts. This is the server-side list of
   what Phase 4 still has to port. After one `world_entry:` login (`SMSG_LOGIN_VERIFY_WORLD carries
   the character's map...`) on a fresh server it reads:

   ```
   SMSG_COMPRESSED_UPDATE_OBJECT
   SMSG_AUTH_RESPONSE
   SMSG_FRIEND_LIST
   SMSG_IGNORE_LIST
   SMSG_MESSAGECHAT
   ```

   (Slice A of Phase 4 is done, so the per-character packets are gone from the list; the counts differ
   per process. Movement, spell and object-destroy packets show up once a session does more than
   log in. The reference for what is left is `bot-phase-4.md`.)

   The counter is process-wide and cumulative. `SMSG_AUTH_RESPONSE` here is the vanilla session-queue
   reply (`World::AddSession_`/`AddQueuedSession`, `WorldSession::SendAuthWaitQue`), still sent per
   session and dropped; a queued modern client needs its own queue packet instead.

## vibemangos vs TrinityCore

Where the two references disagree, the bots follow the one named here and say why (`port-plan.md`:
wire format favors TrinityCore unless our client build says otherwise; gameplay favors vibemangos).
None of these is proven against a real 1.15.6 client yet.

- **`CMSG_CONNECT_TO_FAILED` field order.** The two references disagree on the field order: one
  reads `Serial` (u32) then `Con` (u8), the other reads `Con` (u8) then `Serial` (u32). The
  escalation itself (a failure of attempt N asks for N+1, five in all, serials 17/35/53/71/89,
  connection type 1 for the Instance) is identical either way. **Followed: TrinityCore** (`Con` then
  `Serial`), per `bot-phase-3.md`'s "the bots follow TrinityCore's login flow": wire format goes to
  TrinityCore unless our client build or a live bot run says otherwise. vibemangos was tried first
  and may reflect a 4.4.x-vs-1.15 layout difference or a bug in either; it now fails
  `world_entry: a failed Instance connect is reported and the next SMSG_CONNECT_TO tried`
  (`vibemangos.txt`). `SMSG_CONNECT_TO` itself is identical in both (`... port, serial, con, key`).
  Resolve with a captured real-client session.

- **Values block "contents changed" byte.** A values block carries one byte after the entity-fragment
  flag: TrinityCore writes `0x02` (CGObject changed) for a normal change-mask update and `0x03` for a
  forced with-flags update; vibemangos writes `0x03` for both. **Followed: TrinityCore.** Unverified;
  resolve with a captured real-client session.

- **`SMSG_STAND_STATE_UPDATE` field order.** TrinityCore writes the stand state byte then the anim kit id; vibemangos writes the anim kit id (u32) then the stand state byte, noting the order was checked against a real client through a translating proxy. **Followed: vibemangos**, since that is live evidence and TrinityCore targets a different client. Unverified against our build.

- **`SMSG_SPELL_GO` / `SMSG_SPELL_START` body.** The two references disagree on where the per-target
  miss status sits (TrinityCore: after the target block, one byte per miss, a second byte for a reflect;
  vibemangos: before it, four bits per miss) and on the remaining-power entry (TrinityCore: `i8` type,
  `i32` cost; vibemangos: two `u32`) and the ammo fields (TrinityCore: `i32` display id only when set,
  the inventory type is never sent). **Followed: TrinityCore** for all three. Unverified; a cast with a
  miss or a reflect is where a real client would show it.
- **Cast failure reasons.** The server's `SpellCastResult` is the 1.12 numbering; the modern client's
  differs. vibemangos sends the server value as is, so the client shows the wrong message. Here
  `Packets/ClassicCastResult.h` maps each server reason to the modern one by name; a reason with no
  modern name goes out as the generic unknown reason. Unverified against a client.
- **Spell visuals.** Every cast message carries a `SpellXSpellVisual` id. The server has only the
  1.12 `SpellVisual` id, so it echoes the id a client cast request named and otherwise sends the raw 1.12
  id. Unverified; the real mapping is client data.
- **The cast id.** Cast messages pair by a 128-bit cast guid (type, realm, map, spell, counter). A client
  cast is answered with `SMSG_SPELL_PREPARE` naming the client's id and the server's, and everything
  after uses the server's id. Both references do this; a cast guid with no bits set crashes the client,
  so the server never sends one.
- **The combat log.** Every body follows TrinityCore, which vibemangos matches after its own fixes. Only the
  basic form is sent: the advanced-combat-log block and the content-tuning block are never written, and the
  reader rejects a body that sets either. A damage log carries the school mask (`1 << school`), not the
  1.12 school index, and the spell-cast id when a spell caused it (an empty one otherwise). The execute log
  sends heals, energizes and interrupts through their own messages, where vibemangos also lists their targets
  as generic victims. Unverified against our build.
- **Answering a resurrection offer.** `CMSG_RESURRECT_RESPONSE`'s `u32` is 0 for accept and non-zero for
  decline or timeout in TrinityCore, and the other way round (1 accepts) in vibemangos. **Followed:
  TrinityCore.** If a real client's accept never resurrects, this is the first thing to flip. The offer
  (`SMSG_RESURRECT_REQUEST`) is the same in both. `SMSG_PLAYER_SKINNED` is no longer sent: TrinityCore has no
  body for it and it only ever carried a constant zero.
- **Creature query response.** TrinityCore writes a quest-currency count after the quest-item count; vibemangos
  (built against an older client) does not. It also disagrees on an empty title: TrinityCore writes a length
  of 1 and no bytes, vibemangos a length of 0. **Followed: TrinityCore** for both (our build is the newer
  client), so the response carries the extra count and an empty title is length 1. If a real client drops
  creature names, these two are the first suspects. Only the first name slot is filled; the other three and
  every alternate are empty with length 1.
- **Game object query response.** Both write 35 data words; TrinityCore ends with a content tuning id, vibemangos
  with a required level (both `u32`, always zero here). **Followed: TrinityCore.**
- **NPC text.** Both send only probabilities and broadcast text ids, since the client reads the text from its own
  data; the server's broadcast text ids only match if the client's data does (`port-plan.md`, step 7).
- **Inventory slot numbers.** Both references number slots as the modern client does (bags from 30, the backpack
  from 35, the bank from 59, keyring from 106) and vibemangos moved its own constants to match. This tree keeps the
  1.12 numbers inside the server (bags 19, backpack 23, ...) and translates at the wire
  (`Packets/Classic/InventorySlots.h`), in the active player's `InvSlots` fill and in every message that names a
  slot, since the saved characters and every `Player` slot constant use the old ones. **Removal point:** when the
  flat update-field array goes, the constants move to the modern numbers with a `character_inventory` migration
  and the translation goes. Profession, reagent-bag and child-equipment slots exist in the modern layout only;
  a request naming one is refused with "item not found". The backpack is 16 slots (35 to 50) in this layout, not
  the 24 the references reserve.
- **Item push result.** TrinityCore writes an unused bit between "created" and the display text; vibemangos does not.
  **Followed: TrinityCore.** Bonus lists and modifiers are never sent (and a body with any is rejected on read).
- **Inventory change failure.** TrinityCore writes the result, both item guids and the container slot, then the level
  for a level refusal; vibemangos writes the level before the guids. **Followed: TrinityCore.** The server's
  `InventoryResult` values are mapped by the client's own error name (`Packets/ClassicInventoryResult.h`); the
  server's numbered duplicates collapse onto one reason, and "no error, free the pending item" is `EQUIP_ERR_OK`.
- **Item cooldown and enchantment log.** TrinityCore's item cooldown ends with the cooldown in milliseconds and its
  enchantment log names owner, caster, item guid, item id, enchantment and slot; vibemangos's are older shapes
  (no cooldown, a spell id, no slot). **Followed: TrinityCore.**
- **Using an item.** The modern request names the spell to cast, not the item's spell slot; the server accepts it
  only if it is one of the item's on-use spells. `SMSG_OPEN_CONTAINER` has no modern message (TrinityCore leaves it
  unhandled) and is no longer sent.
- **Loot object guid.** A modern loot window is named by a loot object guid that TrinityCore mints separately from the
  looted unit's. vibemangos reuses the looted object's own guid (one open loot per player), and so does this tree:
  `LootResponse.Owner` and `.LootObj` are the same, and a request is checked against the loot the server has open
  for the player, never trusted. If a real client will not open the window, a real loot object guid is the first
  thing to try. Loot slots are 0-based inside the server and 1-based on the wire.
- **Roll messages.** TrinityCore's start/broadcast/won/all-passed bodies carry a dungeon encounter id, an off-spec
  bit and a "main spec" bit; vibemangos's are older and lack them. **Followed: TrinityCore.** A vote is broadcast
  with roll -1 and the number follows once the roll is counted, and `SMSG_LOOT_ROLLS_COMPLETE` closes the roll frame
  (vibemangos sends it too; the vanilla server never did). The master looter candidate list
  (`SMSG_MASTER_LOOT_CANDIDATE_LIST`) is not sent yet: it belongs with the groups slice.
- **Money.** `SMSG_LOOT_MONEY_NOTIFY` is now sent for solo looting as well (the modern client prints the "you loot"
  line from it), with the sole-looter flag set.
- **Gossip window.** TrinityCore's `SMSG_GOSSIP_MESSAGE` carries per-option ids, an order index, status, optional
  spell and icon and a text id the client queries; vibemangos's is an older, shorter shape (no option NPC id,
  no text-id bit). **Followed: TrinityCore.** The gossip id is the NPC's entry rather than the DB menu id:
  vibemangos diffed a real client's traffic and found the client only accepts the entry. An option id is the
  index the server gave the option in the menu it sent. Quests in the list are typed 2 available, 4 in the log
  and 0 ready to hand in, with vibemangos's flag default of 8. The vanilla server pushed NPC text
  (`SendTalking`) unasked; the client now queries it, so that code is gone.
- **Vendors.** A vendor row is TrinityCore's (price, 1-based row id, type, stack count, quantity left, extended
  cost, item). The buy request carries an item count where the vanilla one carried stacks, so the server
  divides by the item's buy count. TrinityCore reads an item-type integer before the item instance in `CMSG_BUY_ITEM`;
  vibemangos (from a proxy's traffic) puts three bits after it. **Followed: TrinityCore.** Buyback slots and every
  bag slot in a purchase go through the slot translation.
- **Trainers.** `SMSG_TRAINER_LIST`'s per-spell row is TrinityCore's (spell id, money cost, skill line and rank,
  three ability requirements, a usable enum, req level); vmangos's own shape carried two "primary profession"
  confirmation flags and a trailing always-zero field TrinityCore has no room for. **Followed: TrinityCore.**
  Neither reference sends a purchase-succeeded ack (the modern client infers it from the resulting spellbook and
  gold changes), so `SMSG_TRAINER_BUY_SUCCEEDED` is gone. `NpcInteractionOpenResult` (one packet, tagged by a
  `PlayerInteractionType`) replaces the separate `SMSG_SHOW_BANK` / `SMSG_PLAYER_TABARD_VENDOR_ACTIVATE` /
  binder-confirm opcodes for banker and tabard vendor; binder and spirit healer send no packet of their own
  (the existing bind-spell cast and resurrect-and-teleport paths are unchanged, only their CMSG moved to the
  classic table). Unverified against a real client.
- **Quest giver status.** TrinityCore's `SMSG_QUEST_GIVER_STATUS` sends a `u64` bitmask
  (`Future`/`Trivial`/`Reward`/`RepeatableTurnin`/`Quest`/`RewardCompleteNoPOI`/`RewardCompletePOI`);
  vibemangos sends the raw vanilla 0-7 `DIALOG_STATUS_*` value as a `u32` on the wire (a bug their own
  code comment calls out: `DIALOG_STATUS_AVAILABLE` (5, binary 101) becomes bits `0x1|0x4`, neither a
  real modern flag). **Followed: TrinityCore** for the shape (`u64` bitmask via `ToModernQuestGiverStatus`,
  GossipDef.cpp), with vibemangos's already-derived bit values, since the two are only a shape
  disagreement, not a value one. Unverified against a real client.
- **The quest-giver quest list.** TrinityCore's `SMSG_QUEST_GIVER_QUEST_LIST_MESSAGE` (`QuestPackets.cpp`)
  carries only greeting text variants (`NPC::ClientGossipText` entries) and no per-quest fields at all --
  the quest list itself appears to always ride the gossip window there (`SMSG_GOSSIP_MESSAGE`'s own
  `GossipQuestEntry` list, already used for every gossip-flagged NPC in this tree). vibemangos's version
  instead carries a real per-quest list (id, type, level, flags, repeatable, title), cross-checked against
  HermesProxy's translation of this exact opcode. **Followed: vibemangos**, kept for the "no gossip flag,
  several quests to hand out" case `Player::SendPreparedQuest` already special-cases (a single quest goes
  straight to `QuestGiverQuestDetails` either way). Not live-tested this slice -- the turn-in scenario uses
  single-quest questgivers throughout. If a real client rejects this body, the fallback is routing that case
  through `SendGossipMenu` instead, the way TrinityCore's shape implies.
- **Quest giver request items / offer reward / details.** Bit-packing in these three (and the nested
  `QuestRewards` block) follows TrinityCore's field order but flushes bits after each field group instead
  of packing every adjacent bit-field from a whole section into one region before a byte-aligned flush.
  Self-consistent (round-trip tested), not a byte-for-byte match to a real client. `QuestStartItemID` is
  always 0 (vibemangos's live finding: a nonzero value here made a real client believe the quest was
  auto-accepted via item use and broke its accept UI). `QuestGiverRequestItems.StatusFlags` is 223 complete
  / 219 not (vibemangos's finding, reverse-derived from a legacy-to-modern translator's own decode of this
  field). `QuestGiverQuestComplete` follows TrinityCore's shape (xp/money/skill fields plus one optional
  `ItemInstance`, not vibemangos's placeholder list-of-five-items body carried over from the vanilla
  struct); this engine has no notion of a single "highlighted" reward item distinct from the full list
  already shown in the offer window, so that field is always empty.
- **Quest failure toasts.** `SMSG_QUEST_GIVER_INVALID_QUEST` follows TrinityCore's body (reason, contribution
  reward id, error-message bit, reason text), and its reason is the modern `QuestFailedReason` numbering, not
  the engine's `INVALIDREASON_*`: low level 1, wrong race 6, one timed quest 12, already on 13, missing items
  21, not enough money 23; a requirement the client has no text for is 0. `SMSG_QUEST_GIVER_QUEST_FAILED` keeps
  the inventory-result reasons the engine already used (4 inventory full, 17 duplicate item).
  `SMSG_QUEST_UPDATE_FAILED` has no body in TrinityCore (unhandled there); vibemangos's single quest id is used.
  Unverified against a real client.
- **Quest info by id and area status.** `SMSG_QUERY_QUEST_INFO_RESPONSE` follows TrinityCore's field order,
  including its two treasure-picker counts, `QuestGiverCreatureID` and the two conditional-text counts;
  vibemangos writes a single treasure picker id and omits the rest, which does not line up with
  TrinityCore's serializer. A game object objective carries its plain entry with type 2 (TrinityCore); the
  `0x80000000` mark vibemangos puts on it belongs to `QuestUpdateAddCredit`. An unknown quest gets an
  `Allow = false` reply where the vanilla lookup stayed silent. `SMSG_QUEST_GIVER_STATUS_MULTIPLE` lists
  only the visible creatures and game objects whose status is not "none", so a marker the client already
  shows is cleared by the single-giver status the server sends on every change. Unverified against a real client.
- **Pets.** `SMSG_PET_SPELLS_MESSAGE` follows TrinityCore's body (family, specialization 0, time limit, command /
  flag / react bytes, ten action buttons, action and cooldown counts, an always-empty spell history), which
  vibemangos agrees with; it is used for the pet, a charmed unit and a possessed unit alike (the time limit carries
  the charm duration). Both references also agree that the flag byte set for a disabled bar (`0x8`, vanilla's
  "enabled flags") is never read back from the wire; it is still sent, in `SMSG_PET_SPELLS_MESSAGE` and
  `SMSG_PET_MODE`. The spell list only carries the learned spells of a warlock's own summon or a hunter pet
  (`IsPermanentPetFor`). vibemangos defers the message by one tick because the pet's create block may still sit
  in the same visibility batch and a real client drops the bar when it cannot resolve the guid yet; that
  deferral is not ported, so a real client may need it. `CMSG_PET_SET_ACTION` carries one button per message, not
  the vanilla batch of up to two: a command or reaction button is accepted only while the bar still holds it in
  another slot (a move is two messages), and is never cleared by an empty action. `CMSG_PET_CAST_SPELL` shares
  `SpellCastRequest` with the player cast, and the server answers it with `SMSG_SPELL_PREPARE` like one.
  `SMSG_PET_CAST_FAILED` carries the cast id (the request's own when there is one) and an int reason.
  `SMSG_PET_NAME_INVALID` has a plain-byte name length where its request has a bit-packed one (TrinityCore's own
  asymmetry). A pet name lookup for a guid the server cannot resolve answers `Allow = false` where the vanilla
  lookup stayed silent. Three vanilla sends have no modern message and are gone: `SMSG_PET_BROKEN` (the pet
  still runs away, silently), and the pet-skill wipe confirmation with its `CMSG_PET_UNLEARN` reply (the beast
  trainer's "unlearn pet skills" gossip option now only closes the window). The stable master stays on the legacy
  lane: its list rides update fields in the modern protocol. Unverified against a real client.
- **Groups.** Every client request follows TrinityCore: an optional party-index bit leads the body and the index
  byte trails it, where vibemangos reads a leading index byte before the bits (`CMSG_PARTY_INVITE`,
  `_INVITE_RESPONSE`, `_UNINVITE`, `LEAVE_GROUP`, `SET_PARTY_LEADER`, `SET_LOOT_METHOD`, the ping, roll, raid marker
  and ready check requests). The index is parsed and ignored: a player has one party (the battleground's shows as
  index 1). `CMSG_PARTY_INVITE` puts `ProposedRoles` after the two name lengths (vibemangos has a virtual realm
  address there) and `SMSG_PARTY_INVITE` writes it as one byte (vibemangos four). `SMSG_PARTY_UPDATE` lists every
  member, the recipient included, with `MyIndex` marking them; the party guid is the group's own id (vibemangos uses
  one constant per party index), and a party the client must drop is sent with the destroyed flag, no members and
  index -1 under the group's guid. The voice chat id is announced as length 1 with no bytes, as both references
  do. Only `SMSG_PARTY_MEMBER_FULL_STATE` is sent (TrinityCore never sends a partial state, vibemangos builds
  one): a change resends the whole state, auras included, to the members who cannot see the player. Its auras come
  from the visible aura slots in slot order with the flags of `SMSG_AURA_UPDATE`; phase and tuning options are
  always the defaults, the spec is 0. The vanilla party result numbers are the modern ones now (one reason, "not in
  your instance", sits at 3), and uninviting reports operation 1. A raid marker request with a symbol outside 0-7
  (other than -1, the list request) is dropped. Not done: `CMSG_SET_EVERYONE_IS_ASSISTANT`, roles and the role
  poll, dungeon finder blocks, `CMSG_OPT_OUT_OF_LOOT`, `SMSG_SET_LOOT_METHOD_FAILED`, world raid markers
  (`CMSG_CLEAR_RAID_MARKER`), turning a raid back into a party, `PlayerData.PartyType` in the player update, and a
  ready check timeout (it ends once every online member has answered). Unverified against a real client.
- **Duels.** Bodies follow TrinityCore: `SMSG_DUEL_COMPLETE` is one bit, and `SMSG_DUEL_WINNER` is two 6-bit name lengths, the fled bit, two realm addresses (0) and the beaten player's name before the winner's (vibemangos writes a byte, then the winner's name first). `SMSG_DUEL_REQUESTED` carries the arbiter, the challenger and the challenger's account guid; vibemangos leaves the account guid out. `CMSG_DUEL_RESPONSE` replaces accept and cancel, and an accept naming an arbiter other than the player's own is ignored. The wire struct for the completion message is `DuelCompleteNotice` because `Player::DuelComplete` hides the plain name. Unverified against a real client.
- **Trade.** Bodies follow TrinityCore, which vibemangos matches: `SMSG_TRADE_STATUS` is a bnet-account bit, a 5-bit status and a tail per status (`Proposed` carries the partner and the partner's account guid, `Initiated` a trade id, `Failed` a bag result); `SMSG_TRADE_UPDATED` carries the window's gold, item list and two state indexes, and `CMSG_ACCEPT_TRADE` echoes the partner side's `CurrentStateIndex`, so a stale accept gets status 9. The engine's trade status names keep their vanilla spelling (the values 0-22 match the modern ones) and the wire struct is `TradeStatusUpdate` because `TradeStatus` is that enum. `CMSG_SET_TRADE_GOLD` is a `u64`. Not sent: the trade id (always 0), gems, the "use" enchant and the taplist, currency and wrong-realm failures. The engine's own ignore/busy sends keep their vanilla status numbers (0 and 14), which TrinityCore swaps. Unverified against a real client.
- **GM tickets.** `CMSG_COMPLAINT`'s mail-spam id: TrinityCore reads it as a `u64`, vibemangos as a `u32`
  despite crediting the same TrinityCore body. **Followed: TrinityCore.** No external ticket portal exists
  here, so `SMSG_GM_TICKET_CASE_STATUS` always reports zero cases and the per-case fields are never modeled.
  The old create/get-text/update-text/delete-ticket SMSG family has no modern wire form at all; a GM's
  `.ticket` reply, escalation notice or forced close now reaches the player as plain system chat text
  instead of a packet. Unverified against a real client.
- **Petitions.** `CMSG_OFFER_PETITION`: vibemangos's body carries a leading unused id ahead of the two
  guids (its own comment says a live capture needed it), TrinityCore's does not. **Followed: vibemangos**,
  live evidence beating an older capture; an extra field a reader skips is cheaper to be wrong about than
  one it silently misparses. `CMSG_TURN_IN_PETITION` similarly gets a five-`u32` tabard-customization tail
  in vibemangos only, parsed when present and otherwise left at defaults (this engine's guild creation
  never reads them). `SMSG_PETITION_SHOW_LIST`: TrinityCore sends a count-prefixed array of
  `{index, cost, entry, unk, required signs}` rows (a guild-charter row plus, for a non-tabard-designer
  vendor, three arena-team-charter rows); vibemangos sends just the NPC guid and a `u32` price, no entries
  list. **Followed: vibemangos**, matching this engine's own charter model (one fixed guild-charter price;
  no arena-team system exists here to fill the other rows anyway). `SMSG_QUERY_PETITION_RESPONSE`'s
  choice-text tail: the client reads a fixed ten slots regardless of how many this server actually offers
  (a guild charter never offers any) -- writing fewer desyncs the bit stream the name and body text read
  from next, a bug vibemangos's own comment says it found live (a corrupted guild name in the petition
  window) and fixed by always writing all ten, empty or not. **Followed: vibemangos.** `SMSG_PETITION_SIGN_RESULTS`
  and `SMSG_TURN_IN_PETITION_RESULT` use their own 4-bit result codes (`PetitionSignResult`/
  `TurnInPetitionResult` in `PetitionMessages.h`), a different numbering from vmangos's own pre-port
  `PetitionSigns` enum despite sharing several values. `MSG_PETITION_DECLINE` sends nothing back to the
  owner: TrinityCore's own handler has the send commented out with a note that no modern client acts on
  it, and vibemangos matches. Unverified against a real client.
- **Channels.** The old vanilla `ChannelFlags` scheme packed its own bits (`CUSTOM`/`TRADE`/`GENERAL`/...)
  that collide with different modern meanings at the same positions (e.g. vanilla `TRADE` at 0x04 is
  `ReadOnly` on the modern wire, so the Trade channel would render read-only). **Followed: vibemangos**,
  which found this live and switched the wire `ChannelFlags` to just forward the ChatChannels.dbc `Flags`
  column bits directly -- there is no separate "wire" scheme any more. `SMSG_CHANNEL_NOTIFY`'s legacy
  `YOU_JOINED_NOTICE`/`YOU_LEFT_NOTICE` types no longer reach a real client at all: a real player gets the
  dedicated `SMSG_CHANNEL_NOTIFY_JOINED`/`SMSG_CHANNEL_NOTIFY_LEFT` instead (the generic notify's two types
  are a fallback for the internal `MasterPlayer` client only). Both the join notice and a `CHAT_MSG_CHANNEL`
  chat line carry a synthetic channel guid (map/zone/channel-id, the same map-specific layout this engine's
  own `ClassicGuidMapping` already uses for a creature or game object) the client keys its channel UI and
  incoming chat lines on -- this also fills the channel-guid gap slice B's chat left open
  (`ChatHandler::BuildChatPacket` always wrote the "no channel guid" bit). `CMSG_CHANNEL_MODERATE` (the
  whole-channel moderation toggle) has no modern opcode at all and is dropped outright, matching
  vibemangos's own finding; `SILENCE_ALL`/`UNSILENCE_ALL`/`DECLINE_INVITE` are true no-ops on a real
  reference server (TrinityCore's own `Channel::SilenceAll`/`UnsilenceAll`/`DeclineInvite` are empty
  bodies), so they are not wired to the old per-player mute. Unverified against a real client.

- **Inspect.** The 1.12 client reconstructed the inspect window from the target's visible-item update
  fields and a separate `MSG_INSPECT_HONOR_STATS` round trip; the modern client expects one
  `SMSG_INSPECT_RESULT` with equipment and honor both, confirmed against both TrinityCore's
  `InspectResult::Write` and vibemangos's own live-checked body -- the two agree on shape here.
  `MSG_INSPECT_HONOR_STATS` has no modern client request at all and is retired outright; vibemangos
  keeps an `SMSG_INSPECT_HONOR_STATS` reply class for wire completeness but never sends it (no send
  site, and neither reference confirms real content for it), so this port doesn't port it either. Guild
  data (`InspectResult`'s optional guild block) is filled from the guild slice (F8g): `HasGuildData` is
  true and `GuildData` carries the target's guild guid and member count when the target is in a guild,
  false/empty otherwise. `CMSG_INSPECT_PVP`/`SMSG_INSPECT_PVP` and `CMSG_QUERY_INSPECT_ACHIEVEMENTS` have
  no vanilla equivalent at all (no arenas, no achievements in 1.12); the PvP tab always answers with an
  empty bracket list and three empty arena-team slots, and the achievements-tab request is a consumed
  no-op, matching both references. Unverified against a real client.
- **Whois, emotes, tutorials and account data.** `CLASSIC_CMSG_WHO_IS`'s name is a 6-bit length-prefixed
  string and `CLASSIC_SMSG_WHO_IS`'s reply message an 11-bit one (TrinityCore's `WhoIsRequest`/
  `WhoIsResponse`); the legacy `CMSG_WHOIS`/`SMSG_WHOIS` carried a plain C-string either way. `CMSG_EMOTE`
  can arrive with an empty body at world entry (a login-time emote-state sync quirk observed by
  vibemangos); read as a no-op instead of throwing. `CMSG_SEND_TEXT_EMOTE`'s body gained a target guid,
  a sound index and a variable-length (usually empty) visual-kit list ahead of a trailing sequence
  variation, all discarded except the target and sound index; `SMSG_TEXT_EMOTE`'s reply carries no name
  string any more; the client resolves both source and target names from its own cache via the guids
  alone (TrinityCore's `CTextEmote`/`STextEmote`). The vanilla set/clear/reset tutorial-flag opcode
  triplet folds into one `CLASSIC_CMSG_TUTORIAL` with a 2-bit action field (TrinityCore's
  `TutorialSetFlag`); the login-time `SMSG_TUTORIAL_FLAGS` send (`Packets/PlayerSetup.h`) was already
  ported in an earlier slice and needed no change here. Account data gained a player guid, a timestamp
  and a real length prefix ahead of the compressed payload on both the request and the update/response
  bodies (TrinityCore's `ClientConfig::RequestAccountData`/`UpdateAccountData`); the legacy bodies had no
  guid, no timestamp and, on the response, no way for the client to know where the compressed payload
  ended. Unverified against a real client.
- **Guilds.** No modern `CMSG_GUILD_CREATE` exists at all (confirmed absent from both references' opcode
  dispatch); a real client only creates a guild through the petition/charter system (F8d,
  `PetitionsHandler.cpp`'s `HandleTurnInPetitionOpcode`) or, in this tree, the `.guild create` GM command
  -- `HandleGuildCreateOpcode` and the vanilla `GuildCreate` wire class are gone, not ported.
  `CMSG_GUILD_INVITE`/`PROMOTE`/`DEMOTE`/`REMOVE`/`LEADER` and the two note-setters all moved from a
  name-based body to a guid-based one, and the single vanilla `SMSG_GUILD_EVENT` (a numbered
  `GuildEvents` enum plus a string-params array) is replaced entirely by around a dozen typed
  `SMSG_GUILD_EVENT_*`/`SMSG_GUILD_SEND_RANK_CHANGE` messages -- `Guild::BroadcastEvent` is gone, replaced
  by one `Broadcast*Event` method per typed message plus a generic `BroadcastClassic<T>` guild-wide
  sender. `CMSG_GUILD_GET_ROSTER` (a real client's guild-roster open) replaces the old bodyless "guild
  info" request outright: `SMSG_GUILD_INFO` is gone, its created-date/account-count fields folded into
  `SMSG_GUILD_ROSTER`. **Followed TrinityCore, not vibemangos, for three wire bodies where vibemangos's
  own implementation doesn't match its own reference's source** (unlike its many other, explicitly
  live-verified fixes elsewhere in this section):
  - `SMSG_GUILD_ROSTER`'s per-member body. vibemangos's version omits `GuildClubMemberID` (`u64`) and
    `RaceID` (`u8`) entirely and adds a `SorEligible` bit with no counterpart in TrinityCore's real
    `GuildRosterMemberData` struct at all -- every one of those bytes shifts every field after it. Also
    missing: the trailing (always-empty, this engine has no mythic+) `DungeonScoreSummary` triplet
    (two `float`s and a run count). Followed TrinityCore's exact layout; `RaceID` is filled for an
    online member (`Player::GetRace()`), 0 for a cached offline one (not tracked in `MemberSlot`).
  - `CMSG_GUILD_SET_RANK_PERMISSIONS`'s trailing `OldFlags` field is read *before* the bit-packed rank
    name in TrinityCore's own `Read()` source, but vibemangos explicitly documents (and this port
    follows) reading it *before* the name instead, citing a live capture via HermesProxy and a past bug
    from trusting the source order. This is the one exception to "TrinityCore wins": vibemangos's claim
    here cites live evidence, TrinityCore's own source does not.
  - `SMSG_GUILD_PERMISSIONS_QUERY_RESULTS`'s field order: TrinityCore writes rank id, flags, withdraw
    gold limit, tab count; vibemangos swaps the middle two with no verification comment at all (unlike
    the `OldFlags` case above). Followed TrinityCore.
  `SMSG_GUILD_INVITE_DECLINED` (the inviter's "player declined" notification) has no TrinityCore body at
  all -- a real client never sends anything after a retail guild invite goes unanswered, but the opcode
  exists in the 1.15.6 table and vibemangos guesses a name-only shape for it; taken as given, unverified
  by either reference. Guild creation's `SMSG_QUERY_GUILD_INFO_RESPONSE` echoes the query's own guid at
  the top level and a freshly-built one (from the resolved guild's numeric id) inside, matching a real
  client capture vibemangos cites that the *response* carries no player guid at all, unlike the request.
  **Out of scope, matching vibemangos exactly** (its own `Opcodes.cpp` leaves every one of these
  `DEFINE_UNHANDLED`, several with an explicit "this core does not implement the feature yet" comment):
  the entire guild bank (`CMSG_GUILD_BANK_*`, `*_GUILD_BANK_ITEM`), achievements, news, challenges,
  recipes, the Battle.net-friend add, guild party state/XP, guild rename and
  `CMSG_GUILD_REPLACE_GUILD_MASTER` (an inactive-account leader-transfer flow this engine has no
  concept of). `CMSG_GUILD_SHIFT_RANK` (drag-reorder a rank in the ranks UI) is also left unbound: this
  engine's ranks have no separate id/order distinction, so a real reorder would mean reassigning every
  affected member's rank id live, which no reference here backs with verified behavior worth the risk.
  Unverified against a real client throughout.

- **Mail.** `CMSG_MAIL_RETURN_TO_SENDER`/`CMSG_MAIL_DELETE` carry no mailbox guid on the real wire,
  unlike every other mail opcode -- TrinityCore re-checks proximity through a generic "last
  interaction target" tracker (`PlayerTalkClass::GetInteractionData()`) this engine doesn't have, so
  `WorldSession::m_currentMailboxGuid` (set by a prior `CMSG_MAIL_GET_LIST`) stands in for it on the
  return-to-sender path; `MailDelete` gets no proximity check at all, matching TrinityCore's own
  handler exactly. `SMSG_MAIL_LIST_RESULT`'s per-mail body sends its item body inline (`Body`, a real
  string) where the old vanilla wire pointed at a separate `CMSG_ITEM_TEXT_QUERY` round trip; this
  engine still stores mail body text the old way (`Mail::itemTextId` into `ObjectMgr`'s item-text
  cache), so the handler fetches it by id and inlines it to match the modern shape without changing
  the storage architecture. **Followed TrinityCore, not vibemangos, for `SMSG_MAIL_LIST_RESULT` and
  `SMSG_MAIL_COMMAND_RESULT`, both cases where vibemangos's own implementation doesn't match its own
  credited TrinityCore source** (the same pattern the Guilds bullet above found twice):
  - `MailListEntry`'s field widths and layout. vibemangos writes `MailID` as `int32` and `SenderType`
    as `uint8`; TrinityCore's own body writes `uint64`/`uint32`. vibemangos also adds two bits
    (`hasSenderCharacter`, `hasAltSenderId`) with no counterpart in TrinityCore's `Write()` at all,
    and moves the sender-guid-or-alt-id block to *after* the attachment list instead of before it.
    Followed TrinityCore's exact layout and field widths throughout.
  - `SMSG_MAIL_COMMAND_RESULT` (`MailCommandResult` here, renamed from vanilla's `SendMailResult` to
    avoid colliding with the `WorldSession` method of that name once namespaces were dropped).
    vibemangos keeps every field `uint32`, matching the old vanilla shape; TrinityCore's own body
    widens `MailID` and `AttachID` to `uint64`. Followed TrinityCore. Confirmed at the same time: a
    real client identifies the taken attachment by the item's own guid (`Player::SendMailResult`'s
    `itemGuid` parameter, forwarded straight from `HandleMailTakeItem`'s `AttachID`), not by the
    item's template id the way the pre-port vanilla handler passed it -- fixed to pass the guid.
  Unverified against a real client throughout.

- **Auction house.** TrinityCore's `AuctionHousePackets.h` is the retail post-Legion commodity/bucket
  redesign (browse-by-key, batched commodity purchase, favorites) -- the 1.15 client has none of that
  UI and never sends those CMSGs, so none of it is implemented here; only vibemangos (cross-checked
  by its own comments against a real classic-client protocol proxy) covers the pre-bucket wire this
  client actually speaks. **Followed vibemangos** for the CMSG bodies (hello, sell, place bid, remove
  item, list items/bidder/owned, the empty pending-sales query) and the notification shapes
  (`AuctionCommandResult`'s unconditional fixed body; `AuctionClosedNotification` covering both the
  sold and unsold-expired cases via one bit; `AuctionOutbidNotification` doubling as the closest
  available signal for "owner cancelled, your bid is refunded", which has no modern equivalent).
  Two places this port does more than either reference:
  - **The list-result envelope and per-posting shape are this port's own design**, not carried over
    from either reference verbatim: vibemangos's own rewrite never defined structs for
    `SMSG_AUCTION_LIST_ITEMS_RESULT`/`_BIDDED_ITEMS_RESULT`/`_OWNED_ITEMS_RESULT` at all, still
    building them as raw, unstructured writes; TrinityCore's `AuctionItem`/`AuctionListItemsResult`
    are the commodity-era shape (a `BucketKey`, a `ListType`, a `TotalCount` outside the item array)
    this client doesn't use. The per-posting fields TrinityCore's `AuctionItem` struct does share with
    the pre-bucket wire (item instance, count, owner, min bid/increment/buyout, item and
    owner-account guid, end time, bidder and bid amount) were kept; the bucket-only fields
    (`AuctionBucketKey`, `Gems`, a `Creator` distinct from the owner, `UnitPrice`) are always
    absent, matching this engine's own data model. Unverified against a real client; this is the
    single largest guess in this bullet.
  - **`SMSG_AUCTION_OWNER_BID_NOTIFICATION` is wired up**, using TrinityCore's own body for it
    (vibemangos has no send site at all and folds "a new bid just came in, the auction is still
    open" into the same closed-auction notification the sold/expired cases use, which is wrong for
    that case specifically -- the auction isn't closed). This port keeps `AuctionClosedNotification`
    for the two true-close cases (sold, expired unsold) and sends the owner-bid notification
    separately from `HandleAuctionPlaceBid`'s non-buyout branch.
  Real player and NPC guids only (`ClassicGuidMapping::ToClassic`/`FromClassic`); no synthetic guid
  type needed, matching this engine's own single-stack-per-listing auction model (no commodities).
  `CLASSIC_CMSG_AUCTION_LIST_BIDDER_ITEMS` (a second, differently-numbered "list bidder items"
  opcode TrinityCore's own enum also carries with no packet class or handler behind it) is left
  unbound, matching how this port treats every other TrinityCore-enumerated-but-bodyless opcode.
  Live-tested: hello, sell, list (browse), and a bid that buys out immediately (owner and bidder both
  get their live notification). Not covered live: a plain (non-buyout) bid, the bidder-items/
  owned-items list queries, an owner cancelling an active auction, and a real expiry (this engine's
  shortest listing is a real 2 hours, far too long for a scenario to wait on) -- all wire-tested only
  via round trips.

- **Battlegrounds.** TrinityCore's `LFG::RideTicket`/`BattlegroundQueueTypeId` are reused as-is
  (`RideTicket`, `BattlefieldStatusHeader` in `Packets/BattlegroundMessages.h`): the queue ticket
  identifies a queue slot by (id, time) instead of the old mapId+action shape, and `BattlemasterJoin`'s
  `QueueId` packs the `BattlemasterListId` in its low 16 bits, numerically identical to this engine's
  own `BattleGroundTypeId` (AV=1, WS=2, AB=3), so no lookup table is needed either way. The one true
  ticket per queue slot is minted once (`BattleGroundMgr::GenerateNextTicketId`) and stashed on both
  `Player::BgBattleGroundQueueID_Rec` and `PlayerQueueInfo` so it survives the queued -> invited ->
  playing transition and every re-echoed status packet keeps handing back the same (id, time) pair --
  re-deriving `time` on every echo would make the client treat a repeat of the same invite as a new one.
  `SMSG_GROUP_JOINED_BATTLEGROUND` and the 1.9-retired `SMSG_BATTLEFIELD_WIN`/`_LOSE` have no modern
  equivalent at all (confirmed absent from both references): the deserter/join-failed cases now go out
  as `SMSG_BATTLEFIELD_STATUS_FAILED` (`GroupJoinBattlegroundResult`, TrinityCore's own wire-level
  codes, only the subset this engine can actually produce), and the win/lose toast is inferred by the
  client from `SMSG_PVP_LOG_DATA`'s own winner field. `SMSG_BATTLEGROUND_PLAYER_POSITIONS` is confirmed
  against TrinityCore to carry only WSG's flag-carrier minimap blips, not every teammate's position like
  the old 1.12 `MSG_BATTLEGROUND_PLAYER_POSITIONS` did (that behavior has no modern equivalent); this
  port pushes it on a plain 5-second timer (`BG_PLAYER_POSITIONS_UPDATE_INTERVAL`) since neither
  reference documents the real client-visible cadence. `PvpMatchPlayerStatistics` is a field-by-field
  match against TrinityCore's `BattlegroundScore::BuildPvPLogPlayerDataPacket` -- notably that
  production code itself never populates `PrimaryTalentTree`/`Sex`/`Race`/`Class`/`CreatureId`/
  `HonorLevel`/`Role`/`IsInWorld` from the player object either, so leaving them zeroed matches
  confirmed real behavior rather than guessing richer data would help. Per-BG scoreboard stat ids are
  client-hardcoded and never sent on the wire, only the values in insertion order (confirmed against
  TrinityCore: WSG 928/929, AB 926/927, AV 61/63/64/65); this engine's own `BattleGroundAVScore` also
  tracks `secondaryObjectives`/`lieutnantCount`/`secondaryNPC`, which have no known modern stat slot and
  are left out. `BattlefieldList`'s `verification` field is a plain HARDCODE of the client's own
  hardcoded value (its purpose is unclear even in the references). Retail dropped per-instance BG
  picking, but Classic Era preserves the original vanilla UX of listing/choosing among active
  instances, so `BattlefieldList.Battlefields` still carries the same per-instance ids the old
  pre-modern packet sent. `CMSG_BATTLEFIELD_JOIN` (the old raw mapId+action opcode) has no modern
  counterpart at all, replaced entirely by `BattlemasterJoin`/`BattleFieldPort`'s ticket-based flow.
  Wire-tested only via round trips (`BattlegroundMessagesTests.cpp`) and the existing login/queue
  conformance coverage; no live scenario exists for this slice at all (see the F8j entry,
  `cur-step.md`) -- actually running a battleground to completion needs more of the surrounding map/AI
  machinery than one scenario should pull in, so every queue/status/scoreboard/flag-position body here
  is unverified against a real client.

- **The opcode tables agree; the connection routes need TrinityCore.** All 1673 nonzero entries in
  vibemangos's 1.15.6 table match TrinityCore's enum by name and value, with no exceptions. vibemangos
  also lists 166 more names at value `0` (retail-only systems with no known classic value); they are
  left out of `ClassicOpcodes.h` rather than given invented numbers. vibemangos has no per-SMSG
  connection route (it serves one socket), so `ClassicSendRoute` comes from TrinityCore's server-opcode
  table: 964 of the 977 SMSGs have an entry there, 324 of them Instance. The other 13 (`SPECTATE_*`,
  `WOW_LABS_*`, `HARDCORE_*` and similar retail features) have none and default to Realm.
  **Followed: TrinityCore** for routes. The table is only a *default*: a packet can be constructed
  with an explicit connection that overrides it, and TrinityCore does so for `SMSG_RESUME_COMMS`
  (table says Realm, sent on Instance in `HandleContinuePlayerLogin`, matching what the bots see) and
  for its combat-log packets, nothing else. So a `SendPacket` that routes by the table needs a
  per-packet override. `SMSG_LOGIN_VERIFY_WORLD` and `SMSG_TIME_SYNC_REQUEST` default to Instance.

- **A transport guid carries its entry (vibemangos's layout, not TrinityCore's).** TrinityCore's
  transport guid is `type << 58 | counter << 38` with no entry; vibemangos also puts the 23-bit entry
  in the low bits of `high` (a live capture against a real client is what led it to give transports
  their own layout at all). `ClassicGuidMapping` follows vibemangos so the entry survives a round
  trip; with an entry of 0 the two are identical. Unverified against a real client. `HIGHGUID_MO_TRANSPORT`
  and `HIGHGUID_TRANSPORT` both map to the classic transport type and come back as the latter.

- **Login tickets are not single-use.** `bot-phase-2.md` listed "a reused login ticket is rejected at
  LoginRpc"; neither reference does that. TrinityCore's bnetserver checks `LoginTicketExpiry`
  (`AuthenticationService.cpp`) and vibemangos's `LoginTicketStore` is likewise a TTL store
  (`refreshLoginTicket` extends it): a ticket is *expired*, never *consumed*, so a used one that
  has not expired logs in again (confirmed live against vibemangos). The scenario was dropped rather
  than written to encode a rule no server has; a forged ticket is rejected and is covered.

- **`SMSG_INITIALIZE_FACTIONS` body.** TrinityCore: two counts, then `FactionID`, `u16 Flags`,
  `Standing` per faction, then a `FactionID` + bonus bit per faction (a bonus entry for every one).
  vibemangos: a fixed 400-slot array of `u8 flags` and `Standing` (2050 bytes), then 400 bonus bits.
  **Followed: TrinityCore.** Unverified. The scenario is listed in `vibemangos.txt`.

- **`SMSG_ACCOUNT_DATA_TIMES` length.** TrinityCore sends 17 account data times, vibemangos 13.
  **Followed: TrinityCore** (17; the server's own types fill the first eight, the rest are zero).
  Unverified. Listed in `vibemangos.txt`.

- **`SMSG_UPDATE_ACTION_BUTTONS`.** TrinityCore sends 180 `u64` buttons plus a reason byte at login.
  vibemangos never sends it: its buttons ride the active-player create block. **Followed:
  TrinityCore** for the standalone packet (the create-block copy comes with the object-update work).
  Listed in `vibemangos.txt`.

- **The client binary can't settle any of these.** `WowClassic.exe` 58797 is loaded in Ghidra, but its
  code is protected: the opcode constant `0x3B01A6` appears neither as an instruction operand nor as
  bytes in the analyzed image. A disputed body waits for a real-client session or a packet capture.

- **vibemangos's `account create` skips the Battle.net row.** `AccountMgr::CreateAccount` reads the new
  account back before its asynchronous INSERT has landed, so `ProvisionBattlenetAccount` never runs and
  the account can't log in through the portal. `account set password <name> <pw> <pw>` afterwards
  provisions it (`provision-accounts.ps1 -Flavor vibemangos` emits both). Running
  `contrib/realbotrunner/start-vibemangos.ps1` starts vibemangos with a console the script feeds from a
  file, so accounts are created by script without editing its live config.

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

- **The glue screen is five packets, not seven.** Captured live (packet log) on the Realm
  connection right after `SMSG_AUTH_RESPONSE`, before the client would reach the character screen:
  `SMSG_SET_TIME_ZONE_INFORMATION`, `SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN`,
  `SMSG_CACHE_VERSION`, `SMSG_AVAILABLE_HOTFIXES`, `SMSG_BATTLE_NET_CONNECTION_STATUS`, in that
  order. `SMSG_ACCOUNT_DATA_TIMES` and `SMSG_TUTORIAL_FLAGS` never arrived at that point in the
  same capture (through `SMSG_LOGIN_VERIFY_WORLD`); if vibemangos sends them at all it is later, in
  the unparsed flood after world entry. **Followed: vibemangos's observed order and set**, since it
  is a live capture rather than a read of either reference's source; the bots recognize these five
  opcodes (`ClassicOpcodes.h`) and a `world_socket:` scenario asserts they arrive, but none of the
  seven bodies are parsed yet (`ClassicPackets.h` has no structs for them).

- **bnetserver's RPC status codes are TrinityCore's.** vibemangos returns values of its own
  (`0x33000006` for a bad program, `5` for not-implemented, ...); TrinityCore's numbers are the
  real `BattlenetRpcErrorCodes` (`ERROR_BAD_PROGRAM` `0x4D`, `ERROR_RPC_NOT_IMPLEMENTED` `0xBC7`,
  `ERROR_DENIED` `3`, ...). **Followed: TrinityCore.** The bots only test for a non-zero status.

- **`Command_RealmListRequest_v1` with an empty sub-region lists every realm.** TrinityCore filters
  by the sub-region and an empty one matches nothing; the real client always asks for one it got from
  `GetAllValuesForAttribute`, the bots ask with none. **Followed: vibemangos.** A non-empty
  sub-region that is not `region-battlegroup-0` gets an empty list.

- **`Command_LastCharPlayedRequest_v1` answers OK with no attributes when the realm is offline or
  gone.** TrinityCore fails the call; the client takes `Param_RealmEntry` as "connect to this realm
  now", so an offline realm there would skip the realm list and then fail the join. **Followed:
  vibemangos.**

- **`ResourcesService.GetContentHandle` is answered with an empty OK.** TrinityCore leaves the
  service unimplemented; vibemangos found the client treats an error there as fatal on the character
  screen. **Followed: vibemangos.**

- **The logon announces the plain login form URL** (`https://host:port/bnetserver/login/`), as
  TrinityCore does; vibemangos adds platform, build and locale path segments and serves them.
  Game-account entity ids use TrinityCore's `0x200000200576F57` (vibemangos's ends in `51`).
  **Followed: TrinityCore.** Neither is proven against a real client here.

- **An unknown account gets a challenge like a known one.** TrinityCore answers the SRP challenge
  for an unknown account with a bare `DONE` and no challenge, which tells a scanner which accounts
  exist. bnetserver derives a stable fake salt from the login and a random verifier, so the failure
  only shows at the proof, the same as a wrong password.

- **Realm flags are vmangos's.** `realmlist.realmflags` uses vmangos's values (`0x20` new players,
  `0x40` recommended), not TrinityCore's (`0x20` recommended, `0x40` new), and every realm reports
  the one modern build, so the version-mismatch flag is never set.

- **Login tickets live in memory in bnetserver** rather than in a database column (TrinityCore) or a
  file-static map (vibemangos); a restart logs everyone out of the portal.

- **`Param_RealmJoinTicket` is the account id as a decimal string**, not TrinityCore's JSON
  (`gameAccount`/`platform`/`clientArch`/`type`) or a Battle.net-shaped identifier. The client treats
  the ticket as opaque either way, and this server has one game account per Battle.net account, so
  nothing else needs to travel in it; `WorldSocket::HandleAuthSession` (root `mangosd`) reads it
  straight as the account id, no lookup by name needed. **Followed: neither reference** -- this is a
  vmangos-side simplification (`bot-phase-3.md` step C), recorded here since it deviates from step
  B's original design. Not proven against a real client, which is expected to echo the ticket back
  verbatim regardless of its shape.

- **The world handshake has its own timeout on root `mangosd`.** Neither reference closes a
  connection that never finishes the V2 handshake (see "No timeout for a connection stuck before the
  encrypted-mode ack" above, about vibemangos); the port does, reusing `Network.TimeoutSecsIfNoAuth`
  (default 10s) to cover the whole handshake rather than just the pre-`CMSG_AUTH_SESSION` leg.
  `world_socket: never acking SMSG_ENTER_ENCRYPTED_MODE gets the connection closed` passes on vmangos
  for this reason and is `vibemangos.txt`'s only entry.

- **CMSG_AUTH_SESSION carries no OS/platform/build on the modern protocol** (unlike vanilla's), so
  root `mangosd`'s `WorldSession` never learns them at auth time; `SetGameBuild` is set to the known
  constant (58797) and `SetOS`/`SetPlatform` are left at their defaults. Nothing in this tree reads
  them yet. The vanilla "same IP as realmd" check is dropped entirely (there is no realmd in this
  session's path); the ordinary `account_banned`/`ip_banned` checks still apply.

- **`SMSG_AUTH_RESPONSE`'s `SuccessInfo` (race/class availability, realm data) isn't built.** The
  port sends `Result = Ok` with no `SuccessInfo`, which is enough for every bot scenario (none parse
  it) but not for a real client, which is expected to need it to reach the character screen for
  real. Left for whenever a real 58797 client is available to check against, or Step D needs the
  same race/class data for character creation.

- **Classic opcodes go through the handler table, split between socket and session.** The
  classic-keyed `uint32` table (`BuildClassicOpcodeList()` in `Opcodes.cpp`) and the
  `ClassicClient<T>` adapter (`Packets/ClassicClient.h`) carry the session-level opcodes, so they take
  the same `QueueBinaryPacket`/`PacketProcessing` path as every vanilla one. Seven are table-dispatched
  and run on the **world thread**: `CMSG_ENUM_CHARACTERS`, `CMSG_CREATE_CHARACTER`, `CMSG_CHAR_DELETE`,
  `CMSG_PLAYER_LOGIN`, `CMSG_CONNECT_TO_FAILED` (`STATUS_AUTHED`), `CMSG_LOGOUT_REQUEST`
  (`STATUS_LOGGEDIN`) and `CMSG_TIME_SYNC_RESPONSE` (`STATUS_AUTHED`). Four stay in `WorldSocket`
  because they belong to the connection, not the session: `CMSG_AUTH_SESSION`,
  `CMSG_AUTH_CONTINUED_SESSION`, `CMSG_ENTER_ENCRYPTED_MODE_ACK` and `CMSG_PING`. The table and
  dispatch mechanism are vmangos's; the split itself is a vmangos-side choice (an opcode is
  table-dispatched when it needs a session), not compared against either reference's own split. One
  consequence worth knowing: a
  `CMSG_CHAR_DELETE` sent right before the connection closes can now lose the race and never run, since
  it used to execute synchronously on the IO thread. The conformance suite's own cleanup no longer
  relies on that.

- **`SMSG_ENUM_CHARACTERS_RESULT` is built from `ObjectMgr`'s player cache, not a DB query.** A DB-backed
  enum run immediately after a create can race the create's own transaction commit and come back short
  a character -- found live (`char_screen: a created character shows up in the enum...` failed
  intermittently) before switching to the same in-memory cache `Player::SaveNewPlayer`/`DeleteFromDB`
  already update synchronously. The vanilla `SMSG_CHAR_ENUM` handler still queries the DB directly and
  is unaffected. The same fix uncovered that vanilla's own per-session `m_charactersCount` guard (starts
  at `UINT32_MAX` until a first enum runs, "to prevent character creation before amount is recalculated")
  assumes a client always enumerates before it creates; the classic create/two-side-account checks read
  the live cache instead, needing no prior enum.

- **A zero-customization create gets a random valid look, not a fixed one.** The bots
  never send `ChrCustomization` choices, so `Player::SelectRandomAppearance` picks one valid choice
  per option. A create with pairs is validated and completed instead.

- **A handful of `ResponseCodes` have no confirmed modern wire value.** `Packets/Classic/Character.h`
  defines the classic-numbered codes the bots and this tree's checks actually exercise
  (`CLASSIC_CHAR_CREATE_*`, `CLASSIC_CHAR_NAME_*`); a disabled-race create, a reserved name, and the
  PvP-teams-violation check all still block creation but answer with the closest generic failure code
  rather than a precise one, since TrinityCore's exact numbers for those specific results aren't
  confirmed here yet.

- **`CLASSIC_CMSG_SET_AMMO` has no reference body.** TrinityCore dropped ranged ammo/quivers after
  Classic (the opcode and its packet don't exist there at all), and vibemangos only knows the opcode
  *number* -- it never wires up a handler for it. This port keeps the vanilla shape unchanged (a
  single item id, 0 clears ammo, `Packets/ItemMessages.h`'s `SetAmmo`) as its own design; unverified
  against a real client.

- **The pet stable UI is not ported.** Vanilla's `CMSG_STABLE_PET`/`CMSG_UNSTABLE_PET`/
  `CMSG_BUY_STABLE_SLOT`/`CMSG_STABLE_SWAP_PET`/`MSG_LIST_STABLED_PETS` collapse into just three
  modern opcodes (`CLASSIC_CMSG_REQUEST_STABLED_PETS`, `CLASSIC_CMSG_STABLE_PET`,
  `CLASSIC_SMSG_PET_STABLE_RESULT`), but vibemangos -- the primary reference for this exact client
  build -- never solved the new shape either: it leaves `CLASSIC_CMSG_STABLE_PET` explicitly
  unhandled ("no cata counterpart") and notes that `PET_ADDED`/`PET_SLOT_UPDATED`/`PET_STABLE_LIST`
  have no modern opcodes at all. TrinityCore's own retail packets (`RequestStabledPets`/
  `SetPetSlot`, `NPCPackets.h`) are a different, per-slot redesign that doesn't obviously map onto
  the three-opcode classic-era shape either. Left unported rather than guessed, the same call as U6
  (player appearance): it needs a real classic-era client capture to see what the three opcodes
  actually carry.

- **Mirror timers and level-up info follow TrinityCore's field widths over vibemangos's.**
  vibemangos's `StartMirrorTimer`/`PauseMirrorTimer`/`StopMirrorTimer` keep vanilla's `uint32`
  timer-type field and write `Paused` as a full byte; TrinityCore's `MiscPackets.cpp` sends `Timer`
  as a single byte and bit-packs `Paused`, followed here. `LevelUpInfo` similarly follows
  TrinityCore's `MAX_POWERS_PER_CLASS` (10 power-gain slots) over vibemangos's own comment citing "7
  powers for the 1.14.1+ client (HermesProxy)"; this engine only ever fills slot 0 (mana), so the
  dispute costs no real gameplay data, but the wire shape itself is unverified against our specific
  58797 build.

- **`SMSG_SUMMON_REQUEST` drops the auto-decline timer.** vibemangos keeps vanilla's (summonerGuid,
  zoneId, autoDeclineDelay) body; TrinityCore's `SummonRequest::Write()` (`MovementPackets.cpp`)
  carries no delay field at all, adding instead a virtual-realm address (always 0, single-realm
  server), a `Reason` byte (always Spell -- this engine has no scenario-summon source) and a
  `SkipStartingArea` bit (always false). Followed TrinityCore; presumably the modern client's summon
  dialog times out on its own rather than being told a duration.

- **`PlaySpellVisual`'s travel/target fields are guessed.** TrinityCore's body is the full retail
  projectile-visual shape (source, target, transport, target position, travel speed, hit/miss/
  reflect reason, launch delay, min duration, speed-as-time) for spells that fly to a target. This
  engine's only two callers (`Unit::SendPlaySpellVisualKit`: food/drink kits and a channeled spell's
  beam refresh) have no target or travel data at all, so `Target` is set equal to `Source` and every
  field past `SpellVisualId` is a zero "detonates instantly at the caster" default. Unverified
  against a real client, same class of guess as S6's raw `SpellVisual.dbc` id.

- **`SMSG_GAMEOBJECT_SPAWN_ANIM`/`DESPAWN_ANIM` have no modern opcode.** The 1.15.6 opcode table has
  no replacement for either: a modern client animates a game object's appearance purely from its
  `UPDATE_OBJECT` create block. `GAME_OBJECT_CUSTOM_ANIM` gained a `PlayAsDespawn` bit for exactly
  this in retail, so `WorldObject::SendObjectSpawnAnim`/`SendObjectDeSpawnAnim` now delegate to
  `GameObject::SendGameObjectCustomAnim(0, playAsDespawn)` -- matching vibemangos's own fix. Units
  (e.g. `Totem`, which also calls these two) have no equivalent opcode at all, so the send is a
  silent no-op for anything that isn't a game object.

- **`SMSG_PLAY_SPELL_IMPACT`, `SMSG_SPELL_UPDATE_CHAIN_TARGETS` and `SMSG_SET_FORCED_REACTIONS` are
  not ported.** None of the three has a `CLASSIC_SMSG_*` counterpart anywhere in the 1.15.6 opcode
  table (checked against `ClassicOpcodes.h`'s own claim to be the full set the client can send or
  receive). Spell-impact and chain-target visuals are presumably computed client-side from
  `SpellXSpellVisual` data now; forced-reaction overrides, per vibemangos's own note, "ride player
  object-data update fields instead" -- a real update-field feature this port doesn't have yet, not
  a thin rebind. Left on their vanilla opcodes (silently dropped by the legacy lane) rather than
  guessed: `SetForcedReactions`'s only call site (`ReputationMgr::SendForceReactions`, a
  rep-forcing spell aura) and the debug-only `PlaySpellImpact`/channel chain-target sends are
  untouched.

- **`SMSG_SPELL_ABSORB_LOG` is not ported.** It exists in TrinityCore's `CombatLogPackets.h`, but
  vanilla 1.12 has no such opcode at all -- absorbed damage rides along inside the main damage log
  (`SpellNonMeleeDamageLog::Absorbed`, already sent) instead of getting its own toast, and this
  engine has no heal-absorb mechanic (a later-expansion shield type) to source `SpellHealLog::
  Absorbed` from either, so that field stays at its default 0. A new client feature, not a parity
  gap -- same class of decision as threat packets below, except that one had an owner (a real
  vmangos source to fill it from) and this one doesn't. `SpellHealLog::OverHeal`, which does have a
  source (`SpellCaster::DealHeal`'s own `ModifyHealth` clamp), is filled.

- **`SMSG_THREAT_UPDATE`/`HIGHEST_THREAT_UPDATE`/`THREAT_REMOVE`/`THREAT_CLEAR` (nameplate aggro
  coloring) are ported, own design throughout.** Vanilla 1.12 had no such feedback at all, so
  there's no wire shape to port -- structs follow TrinityCore's `CombatPackets.cpp` field order,
  but the send-site wiring is new: `ThreatManager` (`src/game/Threat`) has no periodic `Update`
  tick the way TrinityCore's does, so one was added (`ThreatManager::clientUpdate`, ticked from
  `Creature::Update` while `CSTATE_COMBAT` is set) using the same throttle TrinityCore's own
  `THREAT_UPDATE_INTERVAL` uses (1s) so a busy threat list (every melee swing touches it) doesn't
  spam a packet per hit -- unverified against a real client, since nothing pins this exact interval
  to the 1.15.6 wire itself, only to TrinityCore's server-side choice. `ThreatRemove` and the full
  list clear (`Unit::DeleteThreatList`) send immediately rather than waiting for the throttle,
  matching TrinityCore's own `UpdateOffline`/list-clear call sites. `HighestThreatUpdate` (vs. the
  plain list update) is sent when the current victim differs from the last one actually sent to
  clients, not on every internal victim reselection.

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
