#include "Combat.h"

void WorldPackets::Combat::SetSheathed::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> sheathed;
}

// --- Server Packets ---

size_t WorldPackets::Combat::FeignDeathResisted::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Combat::FeignDeathResisted::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}
