#include "Npc.h"

void WorldPackets::Npc::ListStabledPets::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> npcGuid;
}

void WorldPackets::Npc::StablePet::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> npcGuid;
}

void WorldPackets::Npc::UnstablePet::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> npcGuid;
    recv_data >> petNumber;
}

void WorldPackets::Npc::BuyStableSlot::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> npcGuid;
}

void WorldPackets::Npc::StableSwapPet::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> npcGuid;
    recv_data >> petNumber;
}

// --- Server Packets ---

size_t WorldPackets::Npc::StableResult::EstimateFinalSize() const
{
    return sizeof(result);
}

void WorldPackets::Npc::StableResult::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << result;
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1
size_t WorldPackets::Npc::BinderConfirm::EstimateFinalSize() const
{
    return sizeof(binderGuid);
}

void WorldPackets::Npc::BinderConfirm::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << binderGuid;
}
#endif
