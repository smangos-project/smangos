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

BotBrain::StackScope::StackScope(BotBrain& brain) : m_brain(brain)
{
    ++m_brain.m_stackBusy;
}

BotBrain::StackScope::~StackScope()
{
    if (--m_brain.m_stackBusy == 0 && m_brain.m_clearPending)
    {
        m_brain.m_clearPending = false;
        m_brain.m_stack.clear();
    }
}

BotTaskResult BotBrain::Update(BotContext& ctx, uint32 diff)
{
    if (m_stack.empty())
        return BotTaskResult::Done;

    StackScope scope(*this);
    uint32 appliedDiff = diff;

    while (!m_stack.empty() && !m_clearPending)
    {
        BotTaskResult result = m_stack.back()->Update(ctx, appliedDiff);
        appliedDiff = 0; // only the tick's real elapsed time counts once; cascaded completions below happen at the same instant

        if (m_clearPending)
            return BotTaskResult::Running; // a task cleared the brain (a disconnect) from inside its own Update; ~StackScope finishes it

        if (result == BotTaskResult::Running)
            return BotTaskResult::Running;

        m_stack.pop_back();

        if (m_stack.empty())
            return result;

        m_stack.back()->OnChildComplete(ctx, result);
    }

    return BotTaskResult::Done;
}

template<typename Call>
BotPacketAction BotBrain::DispatchToAll(Call const& call)
{
    BotPacketAction hookAction = BotPacketAction::Continue;

    // Index over a size read up front: a hook added mid-dispatch (appended) waits for the next
    // one, and one removed mid-dispatch is only marked, so nothing here dangles.
    ++m_hookDispatchDepth;
    for (size_t i = 0, n = m_hooks.size(); i < n && hookAction == BotPacketAction::Continue; ++i)
        if (!m_hooks[i].Removed)
            hookAction = call(*m_hooks[i].Task);
    if (--m_hookDispatchDepth == 0)
        SweepRemovedHooks();

    if (hookAction != BotPacketAction::Continue)
        return hookAction;

    // By index, from the top, over the size read up front: a task's OnPacket can resume a routine
    // that pushes a child (ctx.Run), which grows m_stack mid-loop. Nothing below the current
    // index ever moves, and a Clear() from inside a hook is deferred by the scope until the loop is
    // done, so the task being called is never destroyed under its own call.
    StackScope scope(*this);
    for (size_t i = m_stack.size(); i > 0; --i)
    {
        BotPacketAction action = call(*m_stack[i - 1]);
        if (action != BotPacketAction::Continue)
            return action;
    }

    return BotPacketAction::Continue;
}

BotPacketAction BotBrain::DispatchPacket(BotContext& ctx, BotPacket& packet, BotPacketDir dir)
{
    return DispatchToAll([&](BotTask& task) { return task.OnPacket(ctx, packet, dir); });
}

void BotBrain::DispatchConnectionLost(BotContext& ctx, BotConnectionKind kind)
{
    DispatchToAll([&](BotTask& task)
    {
        task.OnConnectionLost(ctx, kind);
        return BotPacketAction::Continue;
    });
}

BotPacketAction BotBrain::DispatchConnect(BotContext& ctx, BotConnectionKind kind, std::string const& address, uint16_t port)
{
    return DispatchToAll([&](BotTask& task) { return task.OnConnect(ctx, kind, address, port); });
}

void BotBrain::Clear()
{
    // A stack task can trigger this itself (a routine calling Disconnect, a packet hook seeing a
    // disconnect): destroying the coroutine frame that is executing would be use-after-free, so
    // inside an Update or a dispatch the clear is deferred to the end of the outermost one.
    if (m_stackBusy > 0)
        m_clearPending = true;
    else
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
