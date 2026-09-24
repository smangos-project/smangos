#ifndef REALBOTS_TESTBOT_H
#define REALBOTS_TESTBOT_H

#include "BotPacket.h"
#include "BotStage.h"
#include "ByteBuffer.h"
#include "RealBotMgr.h"
#include "TestPacketRecorder.h"
#include "TestRoutine.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

class BotTask;

// TestBotOptions().StopAt(...).Password(...) -- bot-examples.md's builder for a conformance
// bot's login, a thin fluent wrapper over RealBotAddOverrides.
class TestBotOptions
{
    public:
        TestBotOptions& StopAt(BotStage stage) { m_overrides.StopAt = stage; return *this; }
        TestBotOptions& Password(std::string password) { m_overrides.Password = std::move(password); return *this; }
        TestBotOptions& CharacterName(std::string name) { m_overrides.CharacterName = std::move(name); return *this; }
        TestBotOptions& CreateIfNone(bool value) { m_overrides.CreateIfNone = value; return *this; }
        TestBotOptions& Race(uint8_t race) { m_overrides.Race = race; return *this; }
        TestBotOptions& Class(uint8_t cls) { m_overrides.Class = cls; return *this; }
        // Log in as this account name instead of the leased one (an account that does not exist).
        TestBotOptions& AccountName(std::string name) { m_overrides.AccountName = std::move(name); return *this; }
        // Join this realm by id (clearing the target's RealmName, which would win): one that is not
        // listed, say. RealmName joins by name instead.
        TestBotOptions& RealmId(uint32_t id) { m_overrides.RealmId = id; m_overrides.RealmName = std::string(); return *this; }
        TestBotOptions& RealmName(std::string name) { m_overrides.RealmName = std::move(name); return *this; }
        TestBotOptions& VerifyServerSignature(bool verify) { m_overrides.VerifyServerSignature = verify; return *this; }
        // Presents this login ticket at the RPC step instead of the one the portal issued.
        TestBotOptions& LoginTicket(std::string ticket) { m_overrides.LoginTicket = std::move(ticket); return *this; }
        // How long each wait inside the world login may take; a scenario checking that the SERVER
        // gives up on a bot sets it (and StageTimeoutMs 0) far beyond the server's own timeout, so
        // the bot cannot be the one to fail first.
        TestBotOptions& PacketTimeoutMs(uint32_t ms) { m_overrides.PacketTimeoutMs = ms; return *this; }
        TestBotOptions& StageTimeoutMs(uint32_t ms) { m_overrides.StageTimeoutMs = ms; return *this; }
        // A hook added before the bot's first tick, so it sees the very first packet and every connect
        // the login makes (a later TestBot::AddHook can race a fast login). `make` runs once.
        TestBotOptions& Hook(std::function<std::unique_ptr<BotTask>()> make, bool persistent = false)
        {
            m_overrides.Hooks.push_back(RealBotAddOverrides::InitialHook{ std::move(make), persistent });
            return *this;
        }
        // Leases from a different target than ConformanceRun::Target() (the run's --target) --
        // only "framework: connecting to a dead address fails" needs this, to reach a target
        // that's deliberately unreachable rather than the one under test.
        TestBotOptions& TargetOverride(std::string name) { m_targetOverride = std::move(name); return *this; }
        // Logs in on the same account as `other` (a TestBot) without a second lease, for the
        // duplicate-login scenarios: two sessions on one account, on purpose.
        template<typename Bot>
        TestBotOptions& ShareAccountWith(Bot const& other) { m_overrides.ShareAccountWith = other.GetName(); return *this; }

        RealBotAddOverrides const& Overrides() const { return m_overrides; }
        std::string const& TargetOverride() const { return m_targetOverride; }

    private:
        RealBotAddOverrides m_overrides;
        std::string m_targetOverride;
};

// co_await bot.Stage(stage, timeoutMs) -> bool. Polls RealBot::GetPublishedInfo() (already
// thread-safe, published once per Tick) from a background thread rather than pushing through
// TestPacketRecorder -- a stage is a level, not an event, so there's no "missed it" problem the
// way there is for a packet, and this needs no cooperation from the bot's own task stack.
class TestStageAwaiter
{
    public:
        // requireHeld: not just in the stage but held there by StopAt, i.e. the stage's results (the
        // session info, a resolved character, ...) are in place and nothing further will happen.
        TestStageAwaiter(std::shared_ptr<RealBot> bot, BotStage target, uint32_t timeoutMs, bool requireHeld = false)
            : m_bot(std::move(bot)), m_target(target), m_timeoutMs(timeoutMs), m_requireHeld(requireHeld) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            Subscribe([executor, handle]() { executor->Post(handle); });
        }

        bool await_resume() const { return m_reached; }

        void Subscribe(std::function<void()> onReady);

    private:
        std::shared_ptr<RealBot> m_bot;
        BotStage m_target;
        uint32_t m_timeoutMs;
        bool m_requireHeld;
        bool m_reached = false;
};

// co_await bot.RawPacket(opcode, timeoutMs) -> std::optional<BotPacket>.
class TestRawPacketAwaiter
{
    public:
        TestRawPacketAwaiter(std::shared_ptr<TestPacketRecorder> recorder, uint32_t opcode, uint32_t timeoutMs)
            : m_recorder(std::move(recorder)), m_opcode(opcode), m_timeoutMs(timeoutMs) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            Subscribe([executor, handle]() { executor->Post(handle); });
        }

        std::optional<BotPacket> await_resume() { return std::move(m_result); }

        void Subscribe(std::function<void()> onReady);

    private:
        std::shared_ptr<TestPacketRecorder> m_recorder;
        uint32_t m_opcode;
        uint32_t m_timeoutMs;
        std::optional<BotPacket> m_result;
};

// co_await bot.NoPacket(opcode, timeoutMs) -> bool, true if opcode never arrived within timeoutMs
// (bot-phase-1.md's "framework: Drop keeps a CMSG_PING off the wire" -- verifying the *absence*
// of a packet needs its own awaiter, not just Packet<T> with a longer timeout, since Packet<T>
// resolving empty already means "timed out", indistinguishable from "arrived but didn't match").
class TestNoPacketAwaiter
{
    public:
        TestNoPacketAwaiter(std::shared_ptr<TestPacketRecorder> recorder, uint32_t opcode, uint32_t timeoutMs)
            : m_recorder(std::move(recorder)), m_opcode(opcode), m_timeoutMs(timeoutMs) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            Subscribe([executor, handle]() { executor->Post(handle); });
        }

        bool await_resume() const { return m_neverArrived; }

        void Subscribe(std::function<void()> onReady);

    private:
        std::shared_ptr<TestPacketRecorder> m_recorder;
        uint32_t m_opcode;
        uint32_t m_timeoutMs;
        bool m_neverArrived = false;
};

// co_await bot.Packet<T>(timeoutMs[, predicate]) -> std::optional<T>. Mirrors BotContext's
// PacketAwaiter (Coroutines/BotAwaitables.h) one for one, just bridged across threads through
// TestPacketRecorder instead of BotBrain's own dispatch.
template<typename T>
class TestPacketAwaiter
{
    public:
        TestPacketAwaiter(std::shared_ptr<TestPacketRecorder> recorder, uint32_t timeoutMs, std::function<bool(T const&)> predicate)
            : m_recorder(std::move(recorder)), m_timeoutMs(timeoutMs), m_predicate(std::move(predicate)) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            Subscribe([executor, handle]() { executor->Post(handle); });
        }

        std::optional<T> await_resume() { return std::move(m_result); }

        void Subscribe(std::function<void()> onReady)
        {
            auto fired = std::make_shared<std::atomic<bool>>(false);
            auto decoded = std::make_shared<std::optional<T>>();
            std::shared_ptr<TestPacketRecorder> recorder = m_recorder;

            uint64_t const watchId = recorder->Watch(
                [decoded, predicate = m_predicate](BotPacket const& packet) -> bool
                {
                    if (packet.Opcode != T::Opcode)
                        return false;
                    ByteBuffer body = packet.Data;
                    body.rpos(0);
                    T value;
                    if (!value.Read(body))
                        return false;
                    if (predicate && !predicate(value))
                        return false;
                    *decoded = std::move(value);
                    return true;
                },
                [this, fired, decoded, onReady](BotPacket const&)
                {
                    bool expected = false;
                    if (!fired->compare_exchange_strong(expected, true))
                        return;
                    m_result = std::move(*decoded);
                    onReady();
                });

            std::thread([fired, recorder, watchId, timeoutMs = m_timeoutMs, onReady]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
                bool expected = false;
                if (fired->compare_exchange_strong(expected, true))
                {
                    recorder->Unwatch(watchId);
                    onReady();
                }
            }).detach();
        }

    private:
        std::shared_ptr<TestPacketRecorder> m_recorder;
        uint32_t m_timeoutMs;
        std::function<bool(T const&)> m_predicate;
        std::optional<T> m_result;
};

// One bot, driven against ConformanceRun::Target() through the shared sRealBotMgr
// (bot-phase-1.md, "TestBot"). Constructing one leases an account from the target and starts
// logging in immediately; destroying one removes the bot, releasing its lease for the next
// TEST_CASE. Every mutating call marshals onto the bot's own worker thread via RealBot::Post,
// the same way RealBotMgr::RunTask already does from the runner/.realbot's console thread --
// TestBot is just another thread reaching into a RealBot from the outside.
class TestBot
{
    public:
        explicit TestBot(TestBotOptions options = {});
        ~TestBot();

        TestBot(TestBot const&) = delete;
        TestBot& operator=(TestBot const&) = delete;
        TestBot(TestBot&&) = delete;
        TestBot& operator=(TestBot&&) = delete;

        // Resolves true once the bot is in `stage`; false on the timeout, or as soon as the bot has
        // Failed (unless that is the stage asked for), since it will never get there then.
        TestStageAwaiter Stage(BotStage stage, uint32_t timeoutMs) const { return TestStageAwaiter(m_bot, stage, timeoutMs); }
        // Like Stage, but only once the login has stopped there (StopAt): everything that stage
        // produces is in place, and the bot does nothing more on its own.
        TestStageAwaiter Held(BotStage stage, uint32_t timeoutMs) const { return TestStageAwaiter(m_bot, stage, timeoutMs, true); }
        TestStageAwaiter Disconnected(uint32_t timeoutMs) const { return Stage(BotStage::Disconnected, timeoutMs); }

        template<typename T>
        TestPacketAwaiter<T> Packet(uint32_t timeoutMs, std::function<bool(T const&)> predicate = nullptr) const
        {
            return TestPacketAwaiter<T>(m_recorder, timeoutMs, std::move(predicate));
        }

        TestRawPacketAwaiter RawPacket(uint32_t opcode, uint32_t timeoutMs) const { return TestRawPacketAwaiter(m_recorder, opcode, timeoutMs); }
        TestNoPacketAwaiter NoPacket(uint32_t opcode, uint32_t timeoutMs) const { return TestNoPacketAwaiter(m_recorder, opcode, timeoutMs); }

        // The connection is `connection` if given, else the opcode's default route (RealBot::Send):
        // pass one to send on the "wrong" connection on purpose.
        template<typename T>
        void Send(T const& packet, std::optional<BotConnectionKind> connection = std::nullopt) const
        {
            BotPacket pkt;
            pkt.Opcode = T::Opcode;
            packet.Write(pkt.Data);
            SendRawPacket(std::move(pkt), connection);
        }

        void SendRaw(uint32_t opcode, ByteBuffer const& body, std::optional<BotConnectionKind> connection = std::nullopt) const
        {
            BotPacket pkt;
            pkt.Opcode = opcode;
            pkt.Data = ByteBuffer(body); // ByteBuffer only has move assignment, not copy
            SendRawPacket(std::move(pkt), connection);
        }

        // clean: a CMSG_LOGOUT_REQUEST and the wait for SMSG_LOGOUT_COMPLETE; not clean is the abrupt
        // one, "the client vanished": the sockets close with no logout request, and the reason
        // recorded is still Requested.
        void Disconnect(bool clean) const;
        // Logs in again on the same account and options (RealBot::Reconnect), from any stage.
        // A TestBot is never reaped while Disconnected or Failed, so this always has a bot to act on.
        void Reconnect() const;
        void PushTask(std::unique_ptr<BotTask> task) const;

        // Hooks live beside the brain's stack (bot-phase-2.md, "Hooks off the stack"): they see
        // packets and stage changes but never bury the running routine, so unlike PushTask they're
        // safe to add at any stage, including mid-login. The returned handle is only ever compared,
        // never dereferenced here, so it's safe to hold after the hook is gone; RemoveHook with it
        // is then a no-op. Non-persistent hooks go when the bot disconnects.
        BotTask* AddHook(std::unique_ptr<BotTask> hook, bool persistent = false) const;
        void RemoveHook(BotTask* hook) const;

        // What the bot's packet observer has recorded since it started (the bot side of
        // Packet<T>/RawPacket, but looking back instead of waiting): every incoming packet, in order.
        std::vector<BotPacket> Received() const { return m_recorder->History(); }
        std::optional<BotPacket> LastReceived(uint32_t opcode) const { return m_recorder->Last(opcode); }
        size_t CountReceived(uint32_t opcode) const { return m_recorder->Count(opcode); }
        // The most recent packet of type T received, parsed; nothing if none arrived or it did not parse.
        template<typename T>
        std::optional<T> LastReceived() const
        {
            std::optional<BotPacket> packet = m_recorder->Last(T::Opcode);
            if (!packet)
                return std::nullopt;
            packet->Data.rpos(0);
            T value;
            if (!value.Read(packet->Data))
                return std::nullopt;
            return value;
        }

        // The bot's published state (stage, session info from the Battle.net half, character guid, ...).
        RealBotInfo Info() const { return m_bot->GetPublishedInfo(); }

        std::string GetLastError() const;
        BotStage GetFailedStage() const;
        BotDisconnectReason GetDisconnectReason() const;
        std::string GetCharacterName() const;
        BotStage GetStage() const;
        uint32_t GetLatencyMs() const;
        std::string const& GetName() const { return m_name; }

    private:
        void SendRawPacket(BotPacket packet, std::optional<BotConnectionKind> connection) const;

        std::string m_name;
        std::shared_ptr<RealBot> m_bot;
        std::shared_ptr<TestPacketRecorder> m_recorder;
};

#endif
