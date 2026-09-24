#include "BotBrain.h"
#include <algorithm>

void BotBrain::PushTask(std::unique_ptr<BotTask> task, BotContext& ctx)
{
    m_stack.push_back(std::move(task));
    m_stack.back()->OnStart(ctx);
}

BotTask* BotBrain::AddHook(std::unique_ptr<BotTask> hook, BotContext& ctx, bool persistent)
{
    BotTask* raw = hook.get();

    // Hooks can be added from inside another hook's dispatch, which is walking m_hooks by index
    // over a size it read up front, so appending is safe there.
    m_hooks.push_back(Hook{std::move(hook), persistent, false});
    raw->OnStart(ctx);
    return raw;
}

void BotBrain::RemoveHook(BotTask* hook)
{
    for (auto it = m_hooks.begin(); it != m_hooks.end(); ++it)
    {
        if (it->Task.get() != hook || it->Removed)
            continue;

        if (m_hookDispatchDepth > 0)
            it->Removed = true;
        else
            m_hooks.erase(it);
        return;
    }
}

size_t BotBrain::HookCount() const
{
    size_t count = 0;
    for (Hook const& hook : m_hooks)
        if (!hook.Removed)
            ++count;
    return count;
}

void BotBrain::SweepRemovedHooks()
{
    m_hooks.erase(std::remove_if(m_hooks.begin(), m_hooks.end(), [](Hook const& hook) { return hook.Removed; }), m_hooks.end());
}

void BotBrain::DispatchStageChanged(BotContext& ctx, BotStage from, BotStage to)
{
    ++m_hookDispatchDepth;
    for (size_t i = 0, n = m_hooks.size(); i < n; ++i)
        if (!m_hooks[i].Removed)
            m_hooks[i].Task->OnStageChanged(ctx, from, to);
    if (--m_hookDispatchDepth == 0)
        SweepRemovedHooks();

    if (BotTask* top = Top())
        top->OnStageChanged(ctx, from, to);
}

BotTaskResult BotBrain::Update(BotContext& ctx, uint32 diff)
{
    if (m_stack.empty())
        return BotTaskResult::Done;

    uint32 appliedDiff = diff;

    while (!m_stack.empty())
    {
        BotTaskResult result = m_stack.back()->Update(ctx, appliedDiff);
        appliedDiff = 0; // only the tick's real elapsed time counts once; cascaded completions below happen at the same instant

        if (result == BotTaskResult::Running)
            return BotTaskResult::Running;

        m_stack.pop_back();

        if (m_stack.empty())
            return result;

        m_stack.back()->OnChildComplete(ctx, result);
    }

    return BotTaskResult::Done;
}

BotPacketAction BotBrain::DispatchPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir)
{
    BotPacketAction hookAction = BotPacketAction::Continue;

    // Index over a size read up front: a hook added mid-dispatch (appended) waits for the next
    // packet, and one removed mid-dispatch is only marked, so nothing here dangles.
    ++m_hookDispatchDepth;
    for (size_t i = 0, n = m_hooks.size(); i < n && hookAction == BotPacketAction::Continue; ++i)
        if (!m_hooks[i].Removed)
            hookAction = m_hooks[i].Task->OnPacket(ctx, packet, dir);
    if (--m_hookDispatchDepth == 0)
        SweepRemovedHooks();

    if (hookAction != BotPacketAction::Continue)
        return hookAction;

    for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
    {
        BotPacketAction action = (*it)->OnPacket(ctx, packet, dir);
        if (action != BotPacketAction::Continue)
            return action;
    }

    return BotPacketAction::Continue;
}

void BotBrain::Clear()
{
    m_stack.clear();

    // A hook can't be destroyed out from under a dispatch that's running its own OnPacket (Clear
    // runs from SetStage(Disconnected), which a hook can trigger); mark instead, the sweep follows.
    for (Hook& hook : m_hooks)
        if (!hook.Persistent)
            hook.Removed = true;

    if (m_hookDispatchDepth == 0)
        SweepRemovedHooks();
}

std::vector<std::string> BotBrain::TaskNames() const
{
    std::vector<std::string> names;
    names.reserve(m_stack.size());
    for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
        names.push_back((*it)->GetName());
    return names;
}
