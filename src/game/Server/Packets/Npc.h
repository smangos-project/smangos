#ifndef MANGOS_PACKETS_NPC_H
#define MANGOS_PACKETS_NPC_H

#include "Packet.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

namespace WorldPackets { namespace Npc
{
    class ListStabledPets final : public ClientPacket
    {
    public:
        ObjectGuid npcGuid;

        explicit ListStabledPets() : ClientPacket(MSG_LIST_STABLED_PETS) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class StablePet final : public ClientPacket
    {
    public:
        ObjectGuid npcGuid;

        explicit StablePet() : ClientPacket(CMSG_STABLE_PET) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class UnstablePet final : public ClientPacket
    {
    public:
        ObjectGuid npcGuid;
        uint32 petNumber = 0;

        explicit UnstablePet() : ClientPacket(CMSG_UNSTABLE_PET) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class BuyStableSlot final : public ClientPacket
    {
    public:
        ObjectGuid npcGuid;

        explicit BuyStableSlot() : ClientPacket(CMSG_BUY_STABLE_SLOT) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class StableSwapPet final : public ClientPacket
    {
    public:
        ObjectGuid npcGuid;
        uint32 petNumber = 0;

        explicit StableSwapPet() : ClientPacket(CMSG_STABLE_SWAP_PET) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    // --- Server Packets ---

    class StableResult final : public ServerPacket
    {
    public:
        uint8 result = 0;

        explicit StableResult() : ServerPacket(SMSG_STABLE_RESULT) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class BinderConfirm final : public ServerPacket
    {
    public:
        ObjectGuid binderGuid; // guid of the innkeeper or binder

        explicit BinderConfirm() : ServerPacket(SMSG_BINDER_CONFIRM) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Npc

#endif // MANGOS_PACKETS_NPC_H
