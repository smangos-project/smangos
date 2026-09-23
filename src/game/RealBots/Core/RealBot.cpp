#include "RealBot.h"
#include "BotContext.h"

RealBot::RealBot()
    : m_context(std::make_unique<BotContext>(*this))
{
}

void RealBot::SetStage(BotStage stage)
{
    if (stage == m_stage)
        return;

    BotStage from = m_stage;
    m_stage = stage;

    if (BotTask* top = m_brain.Top())
        top->OnStageChanged(*m_context, from, stage);

    if (stage == BotStage::Disconnected)
        m_brain.Clear();
}

void RealBot::QueueIncoming(BotPacket packet)
{
    m_incoming.add(std::move(packet));
}

void RealBot::SendRaw(BotPacket packet)
{
    BotPacketAction action = m_brain.DispatchPacket(*m_context, packet, BotPacketDir::Outgoing);
    if (action != BotPacketAction::Drop)
        m_sentPackets.push_back(std::move(packet));
}

void RealBot::ProcessIncoming(uint32 diff)
{
    BotPacket packet;
    while (m_incoming.next(packet))
        m_brain.DispatchPacket(*m_context, packet, BotPacketDir::Incoming);
}

void RealBot::Tick(uint32 diff)
{
    m_elapsedMs += diff;
    ProcessIncoming(diff);
    m_brain.Update(*m_context, diff);
}
