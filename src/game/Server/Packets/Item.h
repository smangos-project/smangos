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

    class AutoBankItem final : public ClientPacket
    {
    public:
        uint8 srcbag = 0;
        uint8 srcslot = 0;

        explicit AutoBankItem() : ClientPacket(CMSG_AUTOBANK_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class AutoStoreBankItem final : public ClientPacket
    {
    public:
        uint8 srcbag = 0;
        uint8 srcslot = 0;

        explicit AutoStoreBankItem() : ClientPacket(CMSG_AUTOSTORE_BANK_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class SetAmmo final : public ClientPacket
    {
    public:
        uint32 item = 0;

        explicit SetAmmo() : ClientPacket(CMSG_SET_AMMO) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class SellItem final : public ClientPacket
    {
    public:
        ObjectGuid vendorGuid;
        ObjectGuid itemGuid;
        uint8 count = 0;

        explicit SellItem() : ClientPacket(CMSG_SELL_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class ListInventory final : public ClientPacket
    {
    public:
        ObjectGuid guid;

        explicit ListInventory() : ClientPacket(CMSG_LIST_INVENTORY) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class BuyBankSlot final : public ClientPacket
    {
    public:
        ObjectGuid guid;

        explicit BuyBankSlot() : ClientPacket(CMSG_BUY_BANK_SLOT) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class BuyItem final : public ClientPacket
    {
    public:
        ObjectGuid vendorGuid;
        uint32 item = 0;
        uint8 count = 0;
        uint8 unk1 = 0;

        explicit BuyItem() : ClientPacket(CMSG_BUY_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class BuyItemInSlot final : public ClientPacket
    {
    public:
        ObjectGuid vendorGuid;
        ObjectGuid bagGuid;
        uint32 item = 0;
        uint8 bagslot = 0;
        uint8 count = 0;

        explicit BuyItemInSlot() : ClientPacket(CMSG_BUY_ITEM_IN_SLOT) {}
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

    class BuybackItem final : public ClientPacket
    {
    public:
        ObjectGuid vendorGuid;
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_7_1
        uint32 slot = 0;
#endif

        explicit BuybackItem() : ClientPacket(CMSG_BUYBACK_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };
    // --- Server Packets ---

    class BuyBankSlotResult final : public ServerPacket
    {
    public:
        uint32 result = 0;

        explicit BuyBankSlotResult() : ServerPacket(SMSG_BUY_BANK_SLOT_RESULT) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class ItemNameQueryResponse final : public ServerPacket
    {
    public:
        uint32 itemId = 0;
        std::string name;

        explicit ItemNameQueryResponse() : ServerPacket(SMSG_ITEM_NAME_QUERY_RESPONSE) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class BuyFailed final : public ServerPacket
    {
    public:
        ObjectGuid vendorGuid;
        uint32 itemEntry = 0;
        uint8 reason = 0; // BuyResult enum value

        explicit BuyFailed() : ServerPacket(SMSG_BUY_FAILED) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class SellItemResponse final : public ServerPacket
    {
    public:
        ObjectGuid vendorGuid;
        ObjectGuid itemGuid;
        uint8 reason = 0; // SellResult enum value

        explicit SellItemResponse() : ServerPacket(SMSG_SELL_ITEM) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

    class BuyItemResponse final : public ServerPacket
    {
    public:
        ObjectGuid vendorGuid;
        uint32 vendorSlot = 0;      // vendor slot, numbered from 1 at client
        uint32 newCount = 0;        // new stock count (0xFFFFFFFF if unlimited)
        uint32 purchaseCount = 0;   // number of items purchased

        explicit BuyItemResponse() : ServerPacket(SMSG_BUY_ITEM) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

}} // namespace WorldPackets::Item

#endif // MANGOS_PACKETS_ITEM_H
