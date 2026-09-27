#include "Skill.h"

void WorldPackets::Skill::TalentWipeConfirm::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> trainerGuid;
}

size_t WorldPackets::Skill::TalentWipeConfirmResponse::EstimateFinalSize() const
{
    return sizeof(trainerGuid) +
           sizeof(cost);
}

void WorldPackets::Skill::TalentWipeConfirmResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << trainerGuid;
    buffer << cost;
}
