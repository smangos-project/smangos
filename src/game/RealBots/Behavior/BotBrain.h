#ifndef REALBOTS_BOTBRAIN_H
#define REALBOTS_BOTBRAIN_H

#include "BotTask.h"
#include <memory>
#include <string>
#include <vector>

// A bot's task stack, plus a separate list of hooks. The top task is ticked; every task sees packet
// hooks, top-down. Pushing a task (BotContext::Run/PushTask) puts it above whatever pushed it, so
// the pusher stops being ticked until it's popped. OnStart runs synchronously inside PushTask,
// before it returns, so a task's own push always lands on the stack before anything it pushes in
// turn during that same OnStart.
//
// A hook (AddHook) is a BotTask that lives beside the stack instead of on it: it sees OnPacket
// (before any stack task, in insertion order) and OnStageChanged, is never Update()d, and is never
// "on top", so a hook that never finishes can't bury the routine that's actually running
// (bot-phase-2.md, "Hooks off the stack"). Its Update is never called, so it can just return Running.
class BotBrain
{
    public:
        void PushTask(std::unique_ptr<BotTask> task, BotContext& ctx);

        // Adds a hook and runs its OnStart. Hooks are dropped by Clear() (a disconnect) unless
        // persistent. Returns the hook, as the handle RemoveHook takes.
        BotTask* AddHook(std::unique_ptr<BotTask> hook, BotContext& ctx, bool persistent = false);

        // No-op if the hook is already gone (removed, or dropped by Clear()). Safe to call from
        // inside a hook's own OnPacket/OnStageChanged: the removal takes effect once dispatch ends.
        void RemoveHook(BotTask* hook);

        void DispatchStageChanged(BotContext& ctx, BotStage from, BotStage to);
        BotTaskResult Update(BotContext& ctx, uint32 diff);
        BotPacketAction DispatchPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir);

        // Destroys every task on the stack without resuming any of them
        // (bot-phase-1.md: "Removing a bot destroys its tasks ... never resumed"), and every
        // hook that wasn't added persistent.
        void Clear();

        bool Empty() const { return m_stack.empty(); }
        size_t Size() const { return m_stack.size(); }
        size_t HookCount() const;
        BotTask* Top() const { return m_stack.empty() ? nullptr : m_stack.back().get(); }

        // Task names, top of the stack first -- what the "tasks <name>" command shows
        // (bot-examples.md #6). Bot-thread only, like everything else here; RealBot::PublishInfo
        // is what makes this safe to read from another thread (a copy in RealBotInfo).
        std::vector<std::string> TaskNames() const;

    private:
        struct Hook
        {
            std::unique_ptr<BotTask> Task;
            bool Persistent = false;
            bool Removed = false; // removed while a dispatch was walking the list; swept when it ends
        };

        void SweepRemovedHooks();

        std::vector<std::unique_ptr<BotTask>> m_stack;
        std::vector<Hook> m_hooks;
        uint32 m_hookDispatchDepth = 0;
};

#endif
