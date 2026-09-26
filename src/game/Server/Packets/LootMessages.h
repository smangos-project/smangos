#ifndef MANGOS_PACKETS_LOOTMESSAGES_H
#define MANGOS_PACKETS_LOOTMESSAGES_H

// The modern loot wire: opening a loot window, taking items and money, rolling for items, and the
// server's answers. Loot list ids are 1-based on the wire. Light like Packets/CombatMessages.h, so
// RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <vector>

// The most slots one loot request may name.
constexpr uint32 CLASSIC_MAX_LOOT_REQUESTS = 100;
constexpr uint32 CLASSIC_LOOT_ROLL_INELIGIBLE_REASONS = 5;

// A loot window a player asks to open.
struct LootUnit
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOOT_UNIT;

    ClassicGuid Unit;

    void Write(ByteBuffer& data) const { Unit.Write(data); }
    bool Read(ByteBuffer& data) { return Unit.Read(data); }
};

struct LootRequestEntry
{
    ClassicGuid Object;
    uint8 LootListId = 0;
};

inline void ClassicWriteLootRequests(ByteBuffer& data, std::vector<LootRequestEntry> const& entries)
{
    for (LootRequestEntry const& entry : entries)
    {
        entry.Object.Write(data);
        data << entry.LootListId;
    }
}

inline bool ClassicReadLootRequests(ByteBuffer& data, uint32 count, std::vector<LootRequestEntry>& entries)
{
    if (count > CLASSIC_MAX_LOOT_REQUESTS)
        return false;
    entries.assign(count, LootRequestEntry());
    for (LootRequestEntry& entry : entries)
    {
        if (!entry.Object.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> entry.LootListId;
    }
    return true;
}

// Takes the named slots out of the open loot window, into the bags.
struct LootItemRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOOT_ITEM;

    std::vector<LootRequestEntry> Loot;
    bool IsSoftInteract = false;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Loot.size());
        ClassicWriteLootRequests(data, Loot);
        data.WriteBit(IsSoftInteract);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        if (!ClassicReadLootRequests(data, count, Loot))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        IsSoftInteract = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The master looter giving items to a player.
struct MasterLootItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MASTER_LOOT_ITEM;

    std::vector<LootRequestEntry> Loot;
    ClassicGuid Target;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Loot.size());
        Target.Write(data);
        ClassicWriteLootRequests(data, Loot);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        if (count > CLASSIC_MAX_LOOT_REQUESTS || !Target.Read(data))
            return false;
        return ClassicReadLootRequests(data, count, Loot);
    }
};

struct LootRelease
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOOT_RELEASE;

    ClassicGuid Object;

    void Write(ByteBuffer& data) const { Object.Write(data); }
    bool Read(ByteBuffer& data) { return Object.Read(data); }
};

// Takes the money out of the open loot window.
struct LootMoney
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOOT_MONEY;

    bool IsSoftInteract = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(IsSoftInteract);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        IsSoftInteract = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// A vote in a group roll: 0 pass, 1 need, 2 greed.
struct LootRoll
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOOT_ROLL;

    ClassicGuid LootObj;
    uint8 LootListId = 0;
    uint8 RollType = 0;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        data << LootListId << RollType;
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        data >> LootListId >> RollType;
        return true;
    }
};

// One slot of a loot window, or the item a roll message is about.
struct LootItemData
{
    // What the slot looks like: 0 lootable, 1 rolled for, 2 master looter only, 3 locked, 4 owner.
    uint8 UIType = 0;
    bool CanTradeToTapList = false;
    ClassicItemInstance Item;
    uint32 Quantity = 0;
    uint8 LootListId = 0;

    // Only plain items are sent: the item kind and the loot item kind are always 0.
    void Write(ByteBuffer& data) const
    {
        data.WriteBits(0, 2);
        data.WriteBits(UIType, 3);
        data.WriteBit(CanTradeToTapList);
        data.FlushBits();
        Item.Write(data);
        data << Quantity;
        data << uint8(0);
        data << LootListId;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const type = data.ReadBits(2);
        UIType = uint8(data.ReadBits(3));
        CanTradeToTapList = data.ReadBit();
        data.ResetBitPos();
        if (type != 0 || !Item.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 2)
            return false;
        uint8 lootItemType;
        data >> Quantity >> lootItemType >> LootListId;
        return lootItemType == 0;
    }
};

// The loot window: what is in it and why it did or did not open.
struct LootResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_RESPONSE;

    // The reason when the window did not open, or "no loot" when it did.
    static constexpr uint8 NoFailure = 17;

    ClassicGuid Owner;
    ClassicGuid LootObj;
    uint8 FailureReason = NoFailure;
    uint8 AcquireReason = 0;
    uint8 LootMethod = 0;
    uint8 Threshold = 2;
    uint32 Coins = 0;
    std::vector<LootItemData> Items;
    bool Acquired = false;

    void Write(ByteBuffer& data) const
    {
        Owner.Write(data);
        LootObj.Write(data);
        data << FailureReason << AcquireReason << LootMethod << Threshold;
        data << Coins;
        data << uint32(Items.size());
        data << uint32(0);  // currencies
        data.WriteBit(Acquired);
        data.WriteBit(false);  // area loot
        data.WriteBit(false);  // unused
        data.FlushBits();
        for (LootItemData const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Owner.Read(data) || !LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 + 3 * sizeof(uint32) + 1)
            return false;
        data >> FailureReason >> AcquireReason >> LootMethod >> Threshold;
        uint32 itemCount, currencyCount;
        data >> Coins >> itemCount >> currencyCount;
        Acquired = data.ReadBit();
        bool const areaLoot = data.ReadBit();
        bool const unused = data.ReadBit();
        data.ResetBitPos();
        if (currencyCount || areaLoot || unused)
            return false;
        // Every item is at least 21 bytes.
        if (itemCount > (data.size() - data.rpos()) / 21)
            return false;
        Items.assign(itemCount, LootItemData());
        for (LootItemData& item : Items)
            if (!item.Read(data))
                return false;
        return true;
    }
};

// A slot left the loot window because someone took it.
struct LootRemoved
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_REMOVED;

    ClassicGuid Owner;
    ClassicGuid LootObj;
    uint8 LootListId = 0;

    void Write(ByteBuffer& data) const
    {
        Owner.Write(data);
        LootObj.Write(data);
        data << LootListId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Owner.Read(data) || !LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> LootListId;
        return true;
    }
};

// The loot window closed.
struct LootReleaseResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_RELEASE;

    ClassicGuid LootObj;
    ClassicGuid Owner;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        Owner.Write(data);
    }

    bool Read(ByteBuffer& data) { return LootObj.Read(data) && Owner.Read(data); }
};

// Someone took the money out of the loot window.
struct CoinRemoved
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_COIN_REMOVED;

    ClassicGuid LootObj;

    void Write(ByteBuffer& data) const { LootObj.Write(data); }
    bool Read(ByteBuffer& data) { return LootObj.Read(data); }
};

// The player's share of the money, for the "you loot" message.
struct LootMoneyNotify
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_MONEY_NOTIFY;

    uint64 Money = 0;
    uint64 MoneyMod = 0;
    bool SoleLooter = false;

    void Write(ByteBuffer& data) const
    {
        data << Money << MoneyMod;
        data.WriteBit(SoleLooter);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(uint64) + 1)
            return false;
        data >> Money >> MoneyMod;
        SoleLooter = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Which rolls a voter may make on a rolled item.
constexpr uint8 CLASSIC_ROLL_MASK_PASS = 0x01;
constexpr uint8 CLASSIC_ROLL_MASK_NEED = 0x02;
constexpr uint8 CLASSIC_ROLL_MASK_GREED = 0x04;

// A group roll starts: the item, how long there is to vote, and the votes allowed.
struct LootRollStart
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_START_LOOT_ROLL;

    ClassicGuid LootObj;
    int32 MapId = 0;
    uint32 RollTime = 0;
    uint8 ValidRolls = CLASSIC_ROLL_MASK_PASS | CLASSIC_ROLL_MASK_NEED | CLASSIC_ROLL_MASK_GREED;
    uint8 Method = 0;
    LootItemData Item;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        data << MapId << RollTime << ValidRolls;
        for (uint32 i = 0; i < CLASSIC_LOOT_ROLL_INELIGIBLE_REASONS; ++i)
            data << uint32(0);
        data << Method;
        data << int32(0);  // dungeon encounter
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + 1 + CLASSIC_LOOT_ROLL_INELIGIBLE_REASONS * sizeof(uint32) + 1 + sizeof(int32))
            return false;
        data >> MapId >> RollTime >> ValidRolls;
        for (uint32 i = 0; i < CLASSIC_LOOT_ROLL_INELIGIBLE_REASONS; ++i)
        {
            uint32 reason;
            data >> reason;
            if (reason)
                return false;
        }
        int32 encounter;
        data >> Method >> encounter;
        return encounter == 0 && Item.Read(data);
    }
};

// A voter's roll, told to everyone in the roll.
struct LootRollBroadcast
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_ROLL;

    ClassicGuid LootObj;
    ClassicGuid Player;
    int32 Roll = 0;
    uint8 RollType = 0;
    LootItemData Item;
    bool Autopassed = false;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        Player.Write(data);
        data << Roll << RollType;
        data << int32(0);  // dungeon encounter
        Item.Write(data);
        data.WriteBit(Autopassed);
        data.WriteBit(false);  // off spec
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data) || !Player.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 1 + sizeof(int32))
            return false;
        int32 encounter;
        data >> Roll >> RollType >> encounter;
        if (encounter != 0 || !Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Autopassed = data.ReadBit();
        bool const offSpec = data.ReadBit();
        data.ResetBitPos();
        return !offSpec;
    }
};

// The roll's winner.
struct LootRollWon
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_ROLL_WON;

    ClassicGuid LootObj;
    ClassicGuid Winner;
    int32 Roll = 0;
    uint8 RollType = 0;
    LootItemData Item;
    bool MainSpec = true;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        Winner.Write(data);
        data << Roll << RollType;
        data << int32(0);  // dungeon encounter
        Item.Write(data);
        data.WriteBit(MainSpec);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data) || !Winner.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 1 + sizeof(int32))
            return false;
        int32 encounter;
        data >> Roll >> RollType >> encounter;
        if (encounter != 0 || !Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        MainSpec = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Everyone passed.
struct LootAllPassed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_ALL_PASSED;

    ClassicGuid LootObj;
    LootItemData Item;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        data << int32(0);  // dungeon encounter
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        int32 encounter;
        data >> encounter;
        return encounter == 0 && Item.Read(data);
    }
};

// The roll is over: the client drops its roll frame.
struct LootRollsComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOOT_ROLLS_COMPLETE;

    ClassicGuid LootObj;
    uint8 LootListId = 0;

    void Write(ByteBuffer& data) const
    {
        LootObj.Write(data);
        data << LootListId;
        data << int32(0);  // dungeon encounter
    }

    bool Read(ByteBuffer& data)
    {
        if (!LootObj.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + sizeof(int32))
            return false;
        int32 encounter;
        data >> LootListId >> encounter;
        return encounter == 0;
    }
};

#endif
