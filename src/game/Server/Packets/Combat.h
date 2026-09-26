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

    class PartyKillLog final : public ServerPacket
    {
    public:
        ObjectGuid killerGuid; // player with the killing blow
        ObjectGuid victimGuid;

        explicit PartyKillLog() : ServerPacket(SMSG_PARTYKILLLOG) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class EnvironmentalDamageLog final : public ServerPacket
    {
    public:
        ObjectGuid victimGuid;
        uint8 damageType = 0; // type of environmental damage
        uint32 damage = 0;
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1
        uint32 absorb = 0;
        int32 resist = 0;
#endif

        explicit EnvironmentalDamageLog() : ServerPacket(SMSG_ENVIRONMENTALDAMAGELOG) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    // SMSG_FEIGN_DEATH_RESISTED: empty body; sent when feign death is resisted
    class FeignDeathResisted final : public ServerPacket
    {
    public:
        explicit FeignDeathResisted() : ServerPacket(SMSG_FEIGN_DEATH_RESISTED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class SpellDamageShield final : public ServerPacket
    {
    public:
        ObjectGuid victimGuid;  // unit that has the shield aura
        ObjectGuid attackerGuid; // unit that attacked the shield bearer
        uint32 damage = 0;
        uint32 school = 0;  // spell school of the shield damage

        explicit SpellDamageShield() : ServerPacket(SMSG_SPELLDAMAGESHIELD) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Combat

#endif // MANGOS_PACKETS_COMBAT_H
