#ifndef MANGOS_PACKETS_ITEM_H
#define MANGOS_PACKETS_ITEM_H

#include "Packet.h"
#include "ObjectGuid.h"
#include <string>
#include <vector>

namespace WorldPackets { namespace Item
{
    class QueryItem final : public ClientPacket
    {
    public:
        uint32 itemEntry; // db entry id
        ObjectGuid itemGuid;

        explicit QueryItem() : ClientPacket(CMSG_ITEM_QUERY_SINGLE), itemEntry(0) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class WrapItem final : public ClientPacket
    {
    public:
        uint8 giftBag = 0;
        uint8 giftSlot = 0;
        uint8 itemBag = 0;
        uint8 itemSlot = 0;

        explicit WrapItem() : ClientPacket(CMSG_WRAP_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    // --- Server Packets ---

    class ItemNameQueryResponse final : public ServerPacket
    {
    public:
        uint32 itemId = 0;
        std::string name;

        explicit ItemNameQueryResponse() : ServerPacket(SMSG_ITEM_NAME_QUERY_RESPONSE) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Item

#endif // MANGOS_PACKETS_ITEM_H
