#ifndef MANGOS_PACKETS_VENDORMESSAGES_H
#define MANGOS_PACKETS_VENDORMESSAGES_H

// The modern vendor wire: browsing a vendor's stock, buying, selling, buying back and repairing.
// Light like Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <vector>

constexpr uint32 CLASSIC_MAX_VENDOR_ITEMS = 300;
constexpr int32 CLASSIC_ITEM_VENDOR_TYPE_ITEM = 1;

// Opens the vendor window.
struct ListInventory
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LIST_INVENTORY;

    ClassicGuid Vendor;

    void Write(ByteBuffer& data) const { Vendor.Write(data); }
    bool Read(ByteBuffer& data) { return Vendor.Read(data); }
};

struct VendorListItem
{
    uint64 Price = 0;
    uint32 MuId = 0;                                        // the row, counted from 1
    int32 Type = CLASSIC_ITEM_VENDOR_TYPE_ITEM;
    int32 StackCount = 1;
    int32 Quantity = -1;                                    // what is left, -1 without limit
    ClassicItemInstance Item;

    void Write(ByteBuffer& data) const
    {
        data << Price << MuId << Type << StackCount << Quantity;
        data << int32(0);  // extended cost
        data << int32(0);  // player condition failed
        data.WriteBit(false);  // locked
        data.WriteBit(false);  // do not filter on vendor
        data.WriteBit(false);  // refundable
        data.FlushBits();
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64) + sizeof(uint32) + 3 * sizeof(int32) + 2 * sizeof(int32) + 1)
            return false;
        int32 extendedCost, conditionFailed;
        data >> Price >> MuId >> Type >> StackCount >> Quantity >> extendedCost >> conditionFailed;
        bool const locked = data.ReadBit();
        bool const noFilter = data.ReadBit();
        bool const refundable = data.ReadBit();
        data.ResetBitPos();
        if (extendedCost || conditionFailed || locked || noFilter || refundable)
            return false;
        return Item.Read(data);
    }
};

// A vendor's stock as one player sees it.
struct VendorInventory
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_VENDOR_INVENTORY;

    ClassicGuid Vendor;
    int32 Reason = 0;
    std::vector<VendorListItem> Items;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << Reason << uint32(Items.size());
        for (VendorListItem const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint32))
            return false;
        uint32 count;
        data >> Reason >> count;
        // Every row is at least 47 bytes.
        if (count > CLASSIC_MAX_VENDOR_ITEMS || count > (data.size() - data.rpos()) / 47)
            return false;
        Items.assign(count, VendorListItem());
        for (VendorListItem& item : Items)
            if (!item.Read(data))
                return false;
        return true;
    }
};

// Buys a vendor row, into a bag if the container guid names one.
struct BuyItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BUY_ITEM;

    ClassicGuid Vendor;
    ClassicGuid Container;
    int32 Quantity = 0;
    uint32 MuId = 0;
    uint32 Slot = 0;
    int32 ItemType = CLASSIC_ITEM_VENDOR_TYPE_ITEM;
    ClassicItemInstance Item;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        Container.Write(data);
        data << Quantity << MuId << Slot << ItemType;
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data) || !Container.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(int32) + sizeof(uint32))
            return false;
        data >> Quantity >> MuId >> Slot >> ItemType;
        return Item.Read(data);
    }
};

// Sells an item, or part of a stack, to a vendor.
struct SellItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SELL_ITEM;

    ClassicGuid Vendor;
    ClassicGuid ItemGuid;
    uint32 Amount = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        ItemGuid.Write(data);
        data << Amount;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data) || !ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Amount;
        return true;
    }
};

// Buys back something sold to a vendor. The slot is a buyback slot.
struct BuyBackItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BUY_BACK_ITEM;

    ClassicGuid Vendor;
    uint32 Slot = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Slot;
        return true;
    }
};

// Repairs one item, or everything when no item is named.
struct RepairItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REPAIR_ITEM;

    ClassicGuid Npc;
    ClassicGuid ItemGuid;
    bool UseGuildBank = false;

    void Write(ByteBuffer& data) const
    {
        Npc.Write(data);
        ItemGuid.Write(data);
        data.WriteBit(UseGuildBank);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Npc.Read(data) || !ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        UseGuildBank = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// A purchase went through: what is left of the row and how many were bought.
struct BuySucceeded
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BUY_SUCCEEDED;

    ClassicGuid Vendor;
    uint32 MuId = 0;
    int32 NewQuantity = -1;
    uint32 QuantityBought = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << MuId << NewQuantity << QuantityBought;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> MuId >> NewQuantity >> QuantityBought;
        return true;
    }
};

// A purchase was refused.
struct BuyFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BUY_FAILED;

    ClassicGuid Vendor;
    uint32 MuId = 0;
    int32 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << MuId << Reason;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> MuId >> Reason;
        return true;
    }
};

// A sale was refused.
struct SellResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SELL_RESPONSE;

    ClassicGuid Vendor;
    std::vector<ClassicGuid> ItemGuids;
    int32 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << uint32(ItemGuids.size()) << Reason;
        for (ClassicGuid const& guid : ItemGuids)
            guid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        uint32 count;
        data >> count >> Reason;
        if (count > (data.size() - data.rpos()) / 2)
            return false;
        ItemGuids.assign(count, ClassicGuid());
        for (ClassicGuid& guid : ItemGuids)
            if (!guid.Read(data))
                return false;
        return true;
    }
};

#endif
