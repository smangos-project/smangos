#ifndef REALBOTS_BOTTASK_H
#define REALBOTS_BOTTASK_H

#include "BotPacket.h"
#include "BotStage.h"

class BotContext;

// One entry on a bot's BotBrain stack: either a plain hook-driven task, or a
// BotRoutineTask wrapping a coroutine. Only the top of the stack is ticked; every
// task on the stack sees packet hooks, top-down (bot-phase-1.md, "Framework").
class BotTask
{
    public:
        virtual ~BotTask() = default;

        virtual char const* GetName() const = 0;

        // Called once, the first time this task becomes the top of the stack.
        virtual void OnStart(BotContext& ctx) {}

        // Called every tick while this task is the top of the stack.
        virtual BotTaskResult Update(BotContext& ctx, uint32 diff) = 0;

        virtual void OnStageChanged(BotContext& ctx, BotStage from, BotStage to) {}

        // Called for every packet, for every task on the stack, top-down, regardless
        // of which one is ticking.
        virtual BotPacketAction OnPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir) { return BotPacketAction::Continue; }

        // Called on the new top of the stack right after the task above it finished
        // (BotBrain pops the finished task before calling this).
        virtual void OnChildComplete(BotContext& ctx, BotTaskResult result) {}
};

#endif
