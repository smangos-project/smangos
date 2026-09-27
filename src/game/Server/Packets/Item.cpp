#include "Item.h"
#include "ItemDefines.h"
#include "SpellEntry.h"

void WorldPackets::Item::QueryItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> itemEntry;
    recv_data >> itemGuid;
}

void WorldPackets::Item::WrapItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> giftBag;
    recv_data >> giftSlot;
    recv_data >> itemBag;
    recv_data >> itemSlot;
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

