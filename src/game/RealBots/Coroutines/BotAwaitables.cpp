#include "BotAwaitables.h"

RawPacketAwaiter::RawPacketAwaiter(RealBot&, uint32 opcode, uint32 timeoutMs)
    : m_opcode(opcode), m_timeoutMs(timeoutMs) {}

bool RawPacketAwaiter::Poll(RealBot&, uint32 diffMs)
{
    return TimedOut(diffMs, m_timeoutMs);
}

bool RawPacketAwaiter::OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir)
{
    if (dir != BotPacketDir::Incoming || packet.Opcode != m_opcode)
        return false;

    m_result = packet;
    return true;
}

StageAwaiter::StageAwaiter(RealBot&, BotStage stage, uint32 timeoutMs)
    : m_target(stage), m_timeoutMs(timeoutMs) {}

bool StageAwaiter::Poll(RealBot& bot, uint32 diffMs)
{
    if (bot.GetStage() == m_target)
    {
        m_reached = true;
        return true;
    }

    return TimedOut(diffMs, m_timeoutMs);
}
