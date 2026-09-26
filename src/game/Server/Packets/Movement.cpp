#include "Movement.h"
#include "World.h"

size_t WorldPackets::Movement::MountSpecialAnim::EstimateFinalSize() const
{
    return sizeof(mountedUnitGuid);
}

void WorldPackets::Movement::MountSpecialAnim::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << mountedUnitGuid;
}
