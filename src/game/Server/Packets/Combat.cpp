#include "Combat.h"

void WorldPackets::Combat::SetSheathed::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> sheathed;
}

// --- Server Packets ---

size_t WorldPackets::Combat::PartyKillLog::EstimateFinalSize() const
{
    return sizeof(killerGuid) +
           sizeof(victimGuid);
}

void WorldPackets::Combat::PartyKillLog::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << killerGuid;
    buffer << victimGuid;
}

size_t WorldPackets::Combat::EnvironmentalDamageLog::EstimateFinalSize() const
{
    return sizeof(victimGuid) +
           sizeof(damageType) +
           sizeof(damage)
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1
           + sizeof(absorb)
           + sizeof(resist)
#endif
           ;
}

void WorldPackets::Combat::EnvironmentalDamageLog::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << victimGuid;
    buffer << damageType;
    buffer << damage;
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1
    buffer << absorb;
    buffer << resist;
#endif
}

size_t WorldPackets::Combat::FeignDeathResisted::EstimateFinalSize() const
{
    return 0;
}

void WorldPackets::Combat::FeignDeathResisted::AppendBodyTo(ByteBuffer& /*buffer*/) const
{
}

size_t WorldPackets::Combat::SpellDamageShield::EstimateFinalSize() const
{
    return sizeof(victimGuid) +
           sizeof(attackerGuid) +
           sizeof(damage) +
           sizeof(school);
}

void WorldPackets::Combat::SpellDamageShield::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << victimGuid;
    buffer << attackerGuid;
    buffer << damage;
    buffer << school;
}

