#ifndef MANGOS_PACKETS_TRADEMESSAGES_H
#define MANGOS_PACKETS_TRADEMESSAGES_H

// The modern player-to-player trade wire: the requests that drive a trade window, the status
// notices and the window contents. Light like Packets/DuelMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <optional>
#include <vector>

constexpr uint32 CLASSIC_TRADE_SLOT_COUNT = 7;

// Asks a player to trade.
struct InitiateTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_INITIATE_TRADE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// The answer to a trade offer that opens the window.
struct BeginTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BEGIN_TRADE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct BusyTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BUSY_TRADE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct IgnoreTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_IGNORE_TRADE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct UnacceptTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_UNACCEPT_TRADE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct CancelTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_TRADE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Accepts the trade as the partner's window last showed it: the state index echoes the
// partner side's CurrentStateIndex, so an accept made before a late change is refused.
struct AcceptTrade
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ACCEPT_TRADE;

    uint32 StateIndex = 0;

    void Write(ByteBuffer& data) const { data << StateIndex; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> StateIndex;
        return true;
    }
};

// Puts the item at a position in the bags into a trade slot. Slot 6 is the one that is not traded.
struct SetTradeItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_TRADE_ITEM;

    uint8 TradeSlot = 0;
    uint8 PackSlot = 0;
    uint8 ItemSlotInPack = 0;

    void Write(ByteBuffer& data) const { data << TradeSlot << PackSlot << ItemSlotInPack; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3)
            return false;
        data >> TradeSlot >> PackSlot >> ItemSlotInPack;
        return true;
    }
};

struct ClearTradeItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CLEAR_TRADE_ITEM;

    uint8 TradeSlot = 0;

    void Write(ByteBuffer& data) const { data << TradeSlot; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> TradeSlot;
        return true;
    }
};

struct SetTradeGold
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_TRADE_GOLD;

    uint64 Coinage = 0;

    void Write(ByteBuffer& data) const { data << Coinage; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64))
            return false;
        data >> Coinage;
        return true;
    }
};

// A trade notice. `Status` is the modern number; the tail depends on it.
struct TradeStatusUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRADE_STATUS;

    static constexpr uint8 Proposed = 1;
    static constexpr uint8 Initiated = 2;
    static constexpr uint8 Failed = 12;
    static constexpr uint8 WrongRealm = 22;
    static constexpr uint8 NotOnTaplist = 23;
    static constexpr uint8 CurrencyNotTradable = 24;
    static constexpr uint8 NotEnoughCurrency = 25;

    uint8 Status = 0;
    bool PartnerIsSameBnetAccount = false;
    bool FailureForYou = false;
    int32 BagResult = 0;
    int32 ItemId = 0;
    uint32 Id = 0;
    ClassicGuid Partner;
    ClassicGuid PartnerAccount;
    uint8 TradeSlot = 0;
    int32 CurrencyType = 0;
    int32 CurrencyQuantity = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(PartnerIsSameBnetAccount);
        data.WriteBits(Status, 5);
        switch (Status)
        {
            case Failed:
                data.WriteBit(FailureForYou);
                data << BagResult << ItemId;
                break;
            case Initiated:
                data << Id;
                break;
            case Proposed:
                data.FlushBits();
                Partner.Write(data);
                PartnerAccount.Write(data);
                break;
            case WrongRealm:
            case NotOnTaplist:
                data << TradeSlot;
                break;
            case NotEnoughCurrency:
            case CurrencyNotTradable:
                data << CurrencyType << CurrencyQuantity;
                break;
            default:
                data.FlushBits();
                break;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        PartnerIsSameBnetAccount = data.ReadBit();
        Status = uint8(data.ReadBits(5));
        if (Status == Failed)
            FailureForYou = data.ReadBit();
        data.ResetBitPos();

        size_t const remaining = data.size() - data.rpos();
        switch (Status)
        {
            case Failed:
                if (remaining < 2 * sizeof(int32))
                    return false;
                data >> BagResult >> ItemId;
                break;
            case Initiated:
                if (remaining < sizeof(uint32))
                    return false;
                data >> Id;
                break;
            case Proposed:
                return Partner.Read(data) && PartnerAccount.Read(data);
            case WrongRealm:
            case NotOnTaplist:
                if (remaining < 1)
                    return false;
                data >> TradeSlot;
                break;
            case NotEnoughCurrency:
            case CurrencyNotTradable:
                if (remaining < 2 * sizeof(int32))
                    return false;
                data >> CurrencyType >> CurrencyQuantity;
                break;
            default:
                break;
        }
        return true;
    }
};

// What an item shows once unwrapped: its enchant, maker, charges and durability.
struct TradeItemDetails
{
    int32 EnchantId = 0;
    int32 OnUseEnchantmentId = 0;
    ClassicGuid Creator;
    int32 Charges = 0;
    bool Lock = false;
    uint32 MaxDurability = 0;
    uint32 Durability = 0;

    // Gems are never sent, and a body that carries one is rejected on read.
    void Write(ByteBuffer& data) const
    {
        data << EnchantId << OnUseEnchantmentId;
        Creator.Write(data);
        data << Charges << MaxDurability << Durability;
        data.WriteBits(0, 2);
        data.WriteBit(Lock);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        data >> EnchantId >> OnUseEnchantmentId;
        if (!Creator.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32) + 1)
            return false;
        data >> Charges >> MaxDurability >> Durability;
        uint32 const gemCount = data.ReadBits(2);
        Lock = data.ReadBit();
        data.ResetBitPos();
        return gemCount == 0;
    }
};

// One filled trade slot. A wrapped item leaves `Details` out and shows only who wrapped it.
struct TradeItemEntry
{
    uint8 Slot = 0;
    int32 StackCount = 0;
    ClassicGuid GiftCreator;
    ClassicItemInstance Item;
    std::optional<TradeItemDetails> Details;

    void Write(ByteBuffer& data) const
    {
        data << Slot << StackCount;
        GiftCreator.Write(data);
        Item.Write(data);
        data.WriteBit(Details.has_value());
        data.FlushBits();
        if (Details)
            Details->Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + sizeof(int32))
            return false;
        data >> Slot >> StackCount;
        if (!GiftCreator.Read(data) || !Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasDetails = data.ReadBit();
        data.ResetBitPos();
        if (!hasDetails)
            return true;
        TradeItemDetails details;
        if (!details.Read(data))
            return false;
        Details = details;
        return true;
    }
};

// One side of the trade window. `WhichPlayer` is 1 for the partner's side, 0 for the receiver's own.
struct TradeUpdated
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRADE_UPDATED;

    uint8 WhichPlayer = 0;
    uint32 Id = 0;
    uint32 ClientStateIndex = 0;
    uint32 CurrentStateIndex = 0;
    uint64 Gold = 0;
    int32 CurrencyType = 0;
    int32 CurrencyQuantity = 0;
    int32 ProposedEnchantment = 0;
    std::vector<TradeItemEntry> Items;

    void Write(ByteBuffer& data) const
    {
        data << WhichPlayer << Id << ClientStateIndex << CurrentStateIndex << Gold;
        data << CurrencyType << CurrencyQuantity << ProposedEnchantment;
        data << uint32(Items.size());
        for (TradeItemEntry const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + 3 * sizeof(uint32) + sizeof(uint64) + 4 * sizeof(int32))
            return false;
        data >> WhichPlayer >> Id >> ClientStateIndex >> CurrentStateIndex >> Gold;
        data >> CurrencyType >> CurrencyQuantity >> ProposedEnchantment;
        uint32 count;
        data >> count;
        if (count > CLASSIC_TRADE_SLOT_COUNT)
            return false;
        Items.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            TradeItemEntry item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        return true;
    }
};

#endif
