#include "BotBrain.h"

void BotBrain::PushTask(std::unique_ptr<BotTask> task, BotContext& ctx)
{
    m_stack.push_back(std::move(task));
    m_stack.back()->OnStart(ctx);
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
}
