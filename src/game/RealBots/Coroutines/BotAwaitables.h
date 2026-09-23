#ifndef REALBOTS_BOTAWAITABLES_H
#define REALBOTS_BOTAWAITABLES_H

#include "BotRoutine.h"
#include "BotContext.h"
#include "RealBot.h"
#include <coroutine>
#include <functional>
#include <optional>

// Common suspend/register bookkeeping every awaiter below needs: never skip
// suspension, and register with the coroutine's owning BotRoutineTask so it gets
// polled from RealBot::Tick.
class BotAwaiterBase : public IBotAwaiter
{
    public:
        bool await_ready() const noexcept { return false; }

        void await_suspend(BotRoutine::HandleType handle)
        {
            handle.promise().Owner->SetPendingAwaiter(this);
        }

    protected:
        bool TimedOut(uint32 diffMs, uint32 timeoutMs)
        {
            m_elapsedMs += diffMs;
            return m_elapsedMs >= timeoutMs;
        }

    private:
        uint32 m_elapsedMs = 0;
};

// ctx.Packet<T>(timeoutMs[, predicate]) -> std::optional<T>
template<typename T>
class PacketAwaiter : public BotAwaiterBase
{
    public:
        PacketAwaiter(RealBot& bot, uint32 timeoutMs, std::function<bool(T const&)> predicate)
            : m_bot(bot), m_timeoutMs(timeoutMs), m_predicate(std::move(predicate)) {}

        bool Poll(RealBot&, uint32 diffMs) override
        {
            return TimedOut(diffMs, m_timeoutMs);
        }

        bool OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir) override
        {
            if (dir != BotPacketDir::Incoming || packet.Opcode != T::Opcode)
                return false;

            ByteBuffer body = packet.Data;
            body.rpos(0);

            T value;
            if (!value.Read(body))
                return false;

            if (m_predicate && !m_predicate(value))
                return false;

            m_result = std::move(value);
            return true;
        }

        std::optional<T> await_resume() { return std::move(m_result); }

    private:
        RealBot& m_bot;
        uint32 m_timeoutMs;
        std::function<bool(T const&)> m_predicate;
        std::optional<T> m_result;
};

// ctx.RawPacket(opcode, timeoutMs) -> std::optional<BotPacket>
class RawPacketAwaiter : public BotAwaiterBase
{
    public:
        RawPacketAwaiter(RealBot& bot, uint32 opcode, uint32 timeoutMs);

        bool Poll(RealBot&, uint32 diffMs) override;
        bool OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir) override;
        std::optional<BotPacket> await_resume() { return std::move(m_result); }

    private:
        uint32 m_opcode;
        uint32 m_timeoutMs;
        std::optional<BotPacket> m_result;
};

// ctx.Stage(stage, timeoutMs) -> bool
class StageAwaiter : public BotAwaiterBase
{
    public:
        StageAwaiter(RealBot& bot, BotStage stage, uint32 timeoutMs);

        bool Poll(RealBot& bot, uint32 diffMs) override;
        bool await_resume() const { return m_reached; }

    private:
        BotStage m_target;
        uint32 m_timeoutMs;
        bool m_reached = false;
};

// ctx.Delay(ms)
class DelayAwaiter : public BotAwaiterBase
{
    public:
        explicit DelayAwaiter(uint32 ms) : m_ms(ms) {}

        bool Poll(RealBot&, uint32 diffMs) override { return TimedOut(diffMs, m_ms); }
        void await_resume() const {}

    private:
        uint32 m_ms;
};

// ctx.Run(...) -> BotTaskResult
class RunAwaiter : public BotAwaiterBase
{
    public:
        RunAwaiter(BotBrain& brain, BotContext& ctx, std::unique_ptr<BotTask> child)
            : m_brain(brain), m_ctx(ctx), m_child(std::move(child)) {}

        void await_suspend(BotRoutine::HandleType handle)
        {
            BotAwaiterBase::await_suspend(handle);
            m_brain.PushTask(std::move(m_child), m_ctx);
        }

        bool Poll(RealBot&, uint32) override { return false; }
        bool OnChildComplete(BotTaskResult result) override { m_result = result; return true; }
        BotTaskResult await_resume() const { return m_result; }

    private:
        BotBrain& m_brain;
        BotContext& m_ctx;
        std::unique_ptr<BotTask> m_child;
        BotTaskResult m_result = BotTaskResult::Failed;
};

// ctx.Hold(): never resolves on its own; the frame is destroyed, never resumed,
// when the brain is cleared (BotBrain::Clear).
class HoldAwaiter : public BotAwaiterBase
{
    public:
        bool Poll(RealBot&, uint32) override { return false; }
        BotTaskResult await_resume() const { return BotTaskResult::Done; }
};

// ctx.Login(options) -> BotLogin::Result. Kicks off BotLogin::Login on the bot's IoContext from
// await_suspend; the completion (on the IO thread) is marshalled onto the bot's own thread via
// RealBot::Post into a block shared with the coroutine frame, so Poll (only ever called from
// RealBot::Tick) never touches anything the IO thread might still be writing to. Uses a
// std::weak_ptr<RealBot> rather than the bot reference directly, exactly like
// RealBot::AttachConnections's own callbacks -- the bot can be removed while a login is in
// flight, and the completion must become a harmless no-op rather than reach into a dead object.
class LoginAwaiter : public BotAwaiterBase
{
    public:
        LoginAwaiter(RealBot& bot, BotLogin::Options options)
            : m_bot(bot), m_options(std::move(options)) {}

        void await_suspend(BotRoutine::HandleType handle)
        {
            BotAwaiterBase::await_suspend(handle);

            std::shared_ptr<Shared> shared = m_shared;
            std::weak_ptr<RealBot> weakBot = m_bot.weak_from_this();
            std::shared_ptr<BotLogin> login = BotLogin::Create(m_bot.GetIoContext());
            login->Login(m_options, [weakBot, shared](BotLogin::Result result)
            {
                if (std::shared_ptr<RealBot> bot = weakBot.lock())
                {
                    bot->Post([shared, result = std::move(result)]() mutable
                    {
                        shared->Result = std::move(result);
                        shared->Done = true;
                    });
                }
            });
        }

        bool Poll(RealBot&, uint32) override { return m_shared->Done; }
        BotLogin::Result await_resume() { return std::move(m_shared->Result); }

    private:
        struct Shared
        {
            bool Done = false;
            BotLogin::Result Result;
        };

        RealBot& m_bot;
        BotLogin::Options m_options;
        std::shared_ptr<Shared> m_shared = std::make_shared<Shared>();
};

template<typename T>
void BotContext::Send(T const& packet)
{
    m_bot.Send(packet);
}

template<typename T>
PacketAwaiter<T> BotContext::Packet(uint32 timeoutMs, std::function<bool(T const&)> predicate)
{
    return PacketAwaiter<T>(m_bot, timeoutMs, std::move(predicate));
}

inline LoginAwaiter BotContext::Login(BotLogin::Options options)
{
    return LoginAwaiter(m_bot, std::move(options));
}

#endif
