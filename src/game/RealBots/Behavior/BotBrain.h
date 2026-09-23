#ifndef REALBOTS_BOTBRAIN_H
#define REALBOTS_BOTBRAIN_H

#include "BotTask.h"
#include <memory>
#include <vector>

// A bot's task stack. The top task is ticked; every task sees packet hooks,
// top-down. Pushing a task (BotContext::Run/PushTask) puts it above whatever
// pushed it, so the pusher stops being ticked until it's popped. OnStart runs
// synchronously inside PushTask, before it returns, so a task's own push always
// lands on the stack before anything it pushes in turn during that same OnStart.
class BotBrain
{
    public:
        void PushTask(std::unique_ptr<BotTask> task, BotContext& ctx);
        BotTaskResult Update(BotContext& ctx, uint32 diff);
        BotPacketAction DispatchPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir);

        // Destroys every task on the stack without resuming any of them
        // (bot-phase-1.md: "Removing a bot destroys its tasks ... never resumed").
        void Clear();

        bool Empty() const { return m_stack.empty(); }
        size_t Size() const { return m_stack.size(); }
        BotTask* Top() const { return m_stack.empty() ? nullptr : m_stack.back().get(); }

    private:
        std::vector<std::unique_ptr<BotTask>> m_stack;
};

#endif
