#include "BotAwaitables.h"
#include <algorithm>

RawPacketAwaiter::RawPacketAwaiter(RealBot&, uint32 opcode, uint32 timeoutMs)
    : m_opcodes{ opcode }, m_timeoutMs(timeoutMs) {}

RawPacketAwaiter::RawPacketAwaiter(RealBot&, std::vector<uint32> opcodes, uint32 timeoutMs)
    : m_opcodes(std::move(opcodes)), m_timeoutMs(timeoutMs) {}

bool RawPacketAwaiter::Poll(RealBot&, uint32 diffMs)
{
    return TimedOut(diffMs, m_timeoutMs);
}

bool RawPacketAwaiter::OnPacket(RealBot&, BotPacket const& packet, BotPacketDir dir)
{
    if (dir != BotPacketDir::Incoming || std::find(m_opcodes.begin(), m_opcodes.end(), packet.Opcode) == m_opcodes.end())
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
