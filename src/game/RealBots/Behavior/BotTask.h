#ifndef REALBOTS_BOTTASK_H
#define REALBOTS_BOTTASK_H

#include "BotPacket.h"
#include "BotStage.h"
#include <cstdint>
#include <string>

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

        // Called before a world connection is opened (ctx.Connect), for every hook and every task on
        // the stack. Returning Drop or Handled refuses the connect: it resolves as failed at once,
        // without touching the network -- how a hook forces a connection failure (e.g. to exercise
        // the CMSG_CONNECT_TO_FAILED retry against a live server).
        virtual BotPacketAction OnConnect(BotContext& ctx, BotConnectionKind kind, std::string const& address, uint16_t port) { return BotPacketAction::Continue; }

        // Called when a world connection closes on its own (the server hung up, a socket error),
        // for every hook and task on the stack, before the bot decides what that means for its
        // stage. A routine's pending wait resolves as failed (BotRoutineTask).
        virtual void OnConnectionLost(BotContext& ctx, BotConnectionKind kind) {}

        // Called on the new top of the stack right after the task above it finished
        // (BotBrain pops the finished task before calling this).
        virtual void OnChildComplete(BotContext& ctx, BotTaskResult result) {}
};

#endif
