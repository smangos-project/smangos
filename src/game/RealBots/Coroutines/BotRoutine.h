#ifndef REALBOTS_BOTROUTINE_H
#define REALBOTS_BOTROUTINE_H

#include "BotTask.h"
#include <coroutine>
#include <string>
#include <utility>

class RealBot;
class BotRoutineTask;

// What a suspended BotRoutine is currently waiting on. Every awaiter in
// BotAwaitables.h implements this so BotRoutineTask can drive resumption purely
// from RealBot::Tick, without any I/O callback ever touching the coroutine.
class IBotAwaiter
{
    public:
        virtual ~IBotAwaiter() = default;

        // Time-driven readiness check; diffMs is 0 except on the first call of a tick.
        virtual bool Poll(RealBot& bot, uint32 diffMs) = 0;

        // Packet-driven readiness check, called as packets are dispatched. Returning
        // true resolves the wait; it does not by itself change the packet's routing.
        virtual bool OnPacket(RealBot& bot, BotPacket const& packet, BotPacketDir dir) { return false; }

        // Fired when a task pushed by this awaiter (ctx.Run) finishes.
        virtual bool OnChildComplete(BotTaskResult result) { return false; }
};

// A bot procedure: a coroutine resumed only from a bot's own worker thread, via
// RealBot::Tick (bot-phase-1.md, "BotRoutine: procedures, on the bot's thread").
struct BotRoutine
{
    struct promise_type
    {
        BotTaskResult Result = BotTaskResult::Running;
        std::string ErrorMessage;
        BotRoutineTask* Owner = nullptr;

        BotRoutine get_return_object();
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(BotTaskResult result) { Result = result; }
        void unhandled_exception();
    };

    using HandleType = std::coroutine_handle<promise_type>;

    explicit BotRoutine(HandleType handle) : m_handle(handle) {}
    BotRoutine(BotRoutine const&) = delete;
    BotRoutine& operator=(BotRoutine const&) = delete;
    BotRoutine(BotRoutine&& other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
    BotRoutine& operator=(BotRoutine&& other) noexcept;
    ~BotRoutine() { Destroy(); }

    void Resume() { if (m_handle && !m_handle.done()) m_handle.resume(); }
    bool IsDone() const { return !m_handle || m_handle.done(); }
    BotTaskResult GetResult() const { return m_handle ? m_handle.promise().Result : BotTaskResult::Failed; }
    std::string const& GetErrorMessage() const { return m_handle.promise().ErrorMessage; }
    HandleType GetHandle() const { return m_handle; }

    private:
        void Destroy() { if (m_handle) { m_handle.destroy(); m_handle = {}; } }

        HandleType m_handle;
};

inline BotRoutine BotRoutine::promise_type::get_return_object()
{
    return BotRoutine{ HandleType::from_promise(*this) };
}

inline BotRoutine& BotRoutine::operator=(BotRoutine&& other) noexcept
{
    if (this != &other)
    {
        Destroy();
        m_handle = std::exchange(other.m_handle, {});
    }
    return *this;
}

// Wraps a BotRoutine as a BotTask so it can live on a BotBrain stack next to
// hook-style tasks (bot-phase-1.md, "Coroutines/BotRoutine.h/.cpp").
class BotRoutineTask : public BotTask
{
    public:
        BotRoutineTask(std::string name, BotRoutine routine);

        char const* GetName() const override { return m_name.c_str(); }
        void OnStart(BotContext& ctx) override;
        BotTaskResult Update(BotContext& ctx, uint32 diff) override;
        BotPacketAction OnPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir) override;
        void OnChildComplete(BotContext& ctx, BotTaskResult result) override;

        void SetPendingAwaiter(IBotAwaiter* awaiter) { m_pendingAwaiter = awaiter; }
        BotRoutine::HandleType GetHandle() const { return m_routine.GetHandle(); }

    private:
        void ResumeAndCheckException();

        std::string m_name;
        BotRoutine m_routine;
        IBotAwaiter* m_pendingAwaiter = nullptr;
};

#endif
