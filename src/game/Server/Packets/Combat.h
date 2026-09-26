#ifndef MANGOS_PACKETS_COMBAT_H
#define MANGOS_PACKETS_COMBAT_H

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets { namespace Combat
{
    class SetSheathed final : public ClientPacket
    {
    public:
        uint32 sheathed = 0;

        explicit SetSheathed() : ClientPacket(CMSG_SETSHEATHED) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };
    // --- Server Packets ---

    // SMSG_FEIGN_DEATH_RESISTED: empty body; sent when feign death is resisted
    class FeignDeathResisted final : public ServerPacket
    {
    public:
        explicit FeignDeathResisted() : ServerPacket(SMSG_FEIGN_DEATH_RESISTED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Combat

#endif // MANGOS_PACKETS_COMBAT_H
