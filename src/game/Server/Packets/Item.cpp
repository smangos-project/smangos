#include "Item.h"
#include "ItemDefines.h"
#include "SpellEntry.h"

void WorldPackets::Item::QueryItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> itemEntry;
    recv_data >> itemGuid;
}

void WorldPackets::Item::AutoBankItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> srcbag;
    recv_data >> srcslot;
}

void WorldPackets::Item::AutoStoreBankItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> srcbag;
    recv_data >> srcslot;
}

void WorldPackets::Item::SetAmmo::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> item;
}

void WorldPackets::Item::SellItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> vendorGuid;
    recv_data >> itemGuid;
    recv_data >> count;
}

void WorldPackets::Item::ListInventory::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Item::BuyBankSlot::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Item::BuyItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> vendorGuid;
    recv_data >> item;
    recv_data >> count;
    recv_data >> unk1;
}

void WorldPackets::Item::BuyItemInSlot::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> vendorGuid;
    recv_data >> item;
    recv_data >> bagGuid;
    recv_data >> bagslot;
    recv_data >> count;
}

void WorldPackets::Item::WrapItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> giftBag;
    recv_data >> giftSlot;
    recv_data >> itemBag;
    recv_data >> itemSlot;
}

void WorldPackets::Item::BuybackItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> vendorGuid;
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_7_1
    recv_data >> slot;
#endif
}

size_t WorldPackets::Item::BuyBankSlotResult::EstimateFinalSize() const
{
    return sizeof(result);
}

void WorldPackets::Item::BuyBankSlotResult::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << result;
}

size_t WorldPackets::Item::ItemNameQueryResponse::EstimateFinalSize() const
{
    return sizeof(itemId) +
           name.size() + sizeof(char) /*null terminator*/;
}

void WorldPackets::Item::ItemNameQueryResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << itemId;
    buffer << name;
}

size_t WorldPackets::Item::BuyFailed::EstimateFinalSize() const
{
    return sizeof(vendorGuid) +
           sizeof(itemEntry) +
           sizeof(reason);
}

void WorldPackets::Item::BuyFailed::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << vendorGuid;
    buffer << itemEntry;
    buffer << reason;
}

size_t WorldPackets::Item::SellItemResponse::EstimateFinalSize() const
{
    return sizeof(vendorGuid) +
           sizeof(itemGuid) +
           sizeof(reason);
}

void WorldPackets::Item::SellItemResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << vendorGuid;
    buffer << itemGuid;
    buffer << reason;
}

size_t WorldPackets::Item::BuyItemResponse::EstimateFinalSize() const
{
    return sizeof(vendorGuid) +
           sizeof(vendorSlot) +
           sizeof(newCount) +
           sizeof(purchaseCount);
}

void WorldPackets::Item::BuyItemResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << vendorGuid;
    buffer << vendorSlot;
    buffer << newCount;
    buffer << purchaseCount;
}

