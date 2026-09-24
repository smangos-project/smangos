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
        // Leases from a different target than ConformanceRun::Target() (the run's --target) --
        // only "framework: connecting to a dead address fails" needs this, to reach a target
        // that's deliberately unreachable rather than the one under test.
        TestBotOptions& TargetOverride(std::string name) { m_targetOverride = std::move(name); return *this; }

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
        TestStageAwaiter(std::shared_ptr<RealBot> bot, BotStage target, uint32_t timeoutMs)
            : m_bot(std::move(bot)), m_target(target), m_timeoutMs(timeoutMs) {}

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

        TestStageAwaiter Stage(BotStage stage, uint32_t timeoutMs) const { return TestStageAwaiter(m_bot, stage, timeoutMs); }
        TestStageAwaiter Disconnected(uint32_t timeoutMs) const { return Stage(BotStage::Disconnected, timeoutMs); }

        template<typename T>
        TestPacketAwaiter<T> Packet(uint32_t timeoutMs, std::function<bool(T const&)> predicate = nullptr) const
        {
            return TestPacketAwaiter<T>(m_recorder, timeoutMs, std::move(predicate));
        }

        TestRawPacketAwaiter RawPacket(uint32_t opcode, uint32_t timeoutMs) const { return TestRawPacketAwaiter(m_recorder, opcode, timeoutMs); }
        TestNoPacketAwaiter NoPacket(uint32_t opcode, uint32_t timeoutMs) const { return TestNoPacketAwaiter(m_recorder, opcode, timeoutMs); }

        template<typename T>
        void Send(T const& packet) const
        {
            BotPacket pkt;
            pkt.Opcode = T::Opcode;
            packet.Write(pkt.Data);
            SendRawPacket(std::move(pkt));
        }

        void SendRaw(uint32_t opcode, ByteBuffer const& body) const
        {
            BotPacket pkt;
            pkt.Opcode = opcode;
            pkt.Data = ByteBuffer(body); // ByteBuffer only has move assignment, not copy
            SendRawPacket(std::move(pkt));
        }

        void Disconnect(bool clean) const;
        void PushTask(std::unique_ptr<BotTask> task) const;

        // Hooks live beside the brain's stack (bot-phase-2.md, "Hooks off the stack"): they see
        // packets and stage changes but never bury the running routine, so unlike PushTask they're
        // safe to add at any stage, including mid-login. The returned handle is only ever compared,
        // never dereferenced here, so it's safe to hold after the hook is gone; RemoveHook with it
        // is then a no-op. Non-persistent hooks go when the bot disconnects.
        BotTask* AddHook(std::unique_ptr<BotTask> hook, bool persistent = false) const;
        void RemoveHook(BotTask* hook) const;

        std::string GetLastError() const;
        BotStage GetFailedStage() const;
        BotDisconnectReason GetDisconnectReason() const;
        std::string GetCharacterName() const;
        BotStage GetStage() const;
        uint32_t GetLatencyMs() const;
        std::string const& GetName() const { return m_name; }

    private:
        void SendRawPacket(BotPacket packet) const;

        std::string m_name;
        std::shared_ptr<RealBot> m_bot;
        std::shared_ptr<TestPacketRecorder> m_recorder;
};

#endif
