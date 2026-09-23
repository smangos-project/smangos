#ifndef REALBOTS_BOTCONTEXT_H
#define REALBOTS_BOTCONTEXT_H

#include "BotIdentity.h"
#include "BotStage.h"
#include "BotTaskArgs.h"
#include <functional>
#include <memory>
#include <string>

class RealBot;
class BotTask;
struct BotRoutine;

template<typename T> class PacketAwaiter;
class RawPacketAwaiter;
class StageAwaiter;
class DelayAwaiter;
class RunAwaiter;
class HoldAwaiter;

// The API a BotRoutine or a hook-style BotTask sees. A thin, stateless facade over
// RealBot: it does not track "which task is asking" itself -- awaiters find their
// own owning BotRoutineTask through the coroutine handle they're given
// (bot-phase-1.md, "Talking to the server").
class BotContext
{
    public:
        explicit BotContext(RealBot& bot) : m_bot(bot) {}

        RealBot& GetBot() const { return m_bot; }

        BotStage GetStage() const;
        BotIdentity const& GetIdentity() const;
        uint32 GetLatencyMs() const;
        uint32 GetTimeMs() const;

        void Disconnect(bool clean);
        void Log(char const* fmt, ...);

        // Pushes a task without waiting for it.
        void PushTask(std::unique_ptr<BotTask> task);

        template<typename T>
        void Send(T const& packet);

        template<typename T>
        PacketAwaiter<T> Packet(uint32 timeoutMs, std::function<bool(T const&)> predicate = nullptr);

        RawPacketAwaiter RawPacket(uint32 opcode, uint32 timeoutMs);
        StageAwaiter Stage(BotStage stage, uint32 timeoutMs);
        DelayAwaiter Delay(uint32 ms);

        // Runs another task to completion on top of this one and hands back its result.
        RunAwaiter Run(std::string taskName, BotTaskArgs args);
        RunAwaiter Run(BotRoutine routine);

        // Stays suspended until popped (removal, or the whole brain being cleared) or disconnected.
        HoldAwaiter Hold();

    private:
        RealBot& m_bot;
};

#endif
