#ifndef MANGOS_PACKETS_MOVEMENT_H
#define MANGOS_PACKETS_MOVEMENT_H

#include "Packet.h"
#include "MovementInfo.h"

namespace WorldPackets { namespace Movement
{
    class MountSpecialAnim final : public ServerPacket
    {
    public:
        ObjectGuid mountedUnitGuid;

        MountSpecialAnim() : ServerPacket(SMSG_MOUNTSPECIAL_ANIM) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };
}} // namespace WorldPackets::Movement

#endif // MANGOS_PACKETS_MOVEMENT_H
