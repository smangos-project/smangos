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

void WorldPackets::Item::BuyBankSlot::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> guid;
}

void WorldPackets::Item::WrapItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> giftBag;
    recv_data >> giftSlot;
    recv_data >> itemBag;
    recv_data >> itemSlot;
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

