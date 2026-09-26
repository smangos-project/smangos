#ifndef MANGOS_PACKETS_ITEMMESSAGES_H
#define MANGOS_PACKETS_ITEMMESSAGES_H

// The modern item wire: moving, splitting, equipping and destroying items, and what the server says
// back. Slots are the modern client's numbers (see Packets/Classic/InventorySlots.h). Light like
// Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/SpellCast.h"
#include <vector>

// The slots a move request says the client already changed on its own. The server ignores them and
// answers with the real result.
struct ClassicInvUpdate
{
    struct Item
    {
        uint8 ContainerSlot = 0;
        uint8 Slot = 0;
    };

    std::vector<Item> Items;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Items.size()), 2);
        data.FlushBits();
        for (Item const& item : Items)
            data << item.ContainerSlot << item.Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const count = data.ReadBits(2);
        data.ResetBitPos();
        if (data.size() - data.rpos() < count * 2)
            return false;
        Items.assign(count, Item());
        for (Item& item : Items)
            data >> item.ContainerSlot >> item.Slot;
        return true;
    }
};

// Moves an item between two inventory slots of the character's own.
struct SwapInvItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SWAP_INV_ITEM;

    ClassicInvUpdate Inv;
    uint8 Destination = 0;
    uint8 Source = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << Destination << Source;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        data >> Destination >> Source;
        return true;
    }
};

// Moves an item between two positions, either of which may be inside a bag.
struct SwapItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SWAP_ITEM;

    ClassicInvUpdate Inv;
    uint8 DestinationBag = 0;
    uint8 SourceBag = 0;
    uint8 DestinationSlot = 0;
    uint8 SourceSlot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << DestinationBag << SourceBag << DestinationSlot << SourceSlot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 4)
            return false;
        data >> DestinationBag >> SourceBag >> DestinationSlot >> SourceSlot;
        return true;
    }
};

// Equips the item at a position in the slot the server picks.
struct AutoEquipItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTO_EQUIP_ITEM;

    ClassicInvUpdate Inv;
    uint8 PackSlot = 0;
    uint8 Slot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << PackSlot << Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        data >> PackSlot >> Slot;
        return true;
    }
};

// Equips an item, named by guid, in a chosen equipment slot.
struct AutoEquipItemSlot
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTO_EQUIP_ITEM_SLOT;

    ClassicInvUpdate Inv;
    ClassicGuid Item;
    uint8 Slot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        Item.Write(data);
        data << Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data) || !Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Slot;
        return true;
    }
};

// Moves an item into a bag, wherever it fits.
struct AutoStoreBagItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTO_STORE_BAG_ITEM;

    ClassicInvUpdate Inv;
    uint8 DestinationBag = 0;
    uint8 SourceBag = 0;
    uint8 SourceSlot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << DestinationBag << SourceBag << SourceSlot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 3)
            return false;
        data >> DestinationBag >> SourceBag >> SourceSlot;
        return true;
    }
};

// Splits part of a stack into another position.
struct SplitItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SPLIT_ITEM;

    ClassicInvUpdate Inv;
    uint8 FromPackSlot = 0;
    uint8 FromSlot = 0;
    uint8 ToPackSlot = 0;
    uint8 ToSlot = 0;
    int32 Quantity = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << FromPackSlot << FromSlot << ToPackSlot << ToSlot << Quantity;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 + sizeof(int32))
            return false;
        data >> FromPackSlot >> FromSlot >> ToPackSlot >> ToSlot >> Quantity;
        return true;
    }
};

// Destroys an item, or some of a stack. A count of 0 destroys the lot.
struct DestroyItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DESTROY_ITEM;

    uint32 Count = 0;
    uint8 ContainerId = 0;
    uint8 SlotNum = 0;

    void Write(ByteBuffer& data) const { data << Count << ContainerId << SlotNum; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + 2)
            return false;
        data >> Count >> ContainerId >> SlotNum;
        return true;
    }
};

// Reads a readable item (a book or a letter).
struct ReadItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_READ_ITEM;

    uint8 PackSlot = 0;
    uint8 Slot = 0;

    void Write(ByteBuffer& data) const { data << PackSlot << Slot; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        data >> PackSlot >> Slot;
        return true;
    }
};

// Uses an item: the cast the item starts is named by spell id, not by the item's spell slot.
struct UseItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_USE_ITEM;

    uint8 PackSlot = 0;
    uint8 Slot = 0;
    ClassicGuid CastItem;
    SpellCastRequest Cast;

    void Write(ByteBuffer& data) const
    {
        data << PackSlot << Slot;
        CastItem.Write(data);
        Cast.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        data >> PackSlot >> Slot;
        return CastItem.Read(data) && Cast.Read(data);
    }
};

// The item behind an inventory result, or a spell-cast item.
struct ClassicItemInstance
{
    int32 ItemId = 0;
    int32 RandomPropertiesSeed = 0;
    int32 RandomPropertiesId = 0;

    // Bonus lists and modifiers are never sent, and a body that carries one is rejected on read.
    void Write(ByteBuffer& data) const
    {
        data << ItemId << RandomPropertiesSeed << RandomPropertiesId;
        data.WriteBit(false);
        data.FlushBits();
        data.WriteBits(0, 6);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3 * sizeof(int32) + 2)
            return false;
        data >> ItemId >> RandomPropertiesSeed >> RandomPropertiesId;
        bool const hasBonus = data.ReadBit();
        data.ResetBitPos();
        uint32 const modifierCount = data.ReadBits(6);
        data.ResetBitPos();
        return !hasBonus && modifierCount == 0;
    }
};

// Why an inventory move, equip, use or purchase was refused. The result is the modern number
// (Packets/ClassicInventoryResult.h); only the level requirement carries more.
struct InventoryChangeFailure
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_INVENTORY_CHANGE_FAILURE;

    static constexpr int32 CantEquipLevel = 1;

    int32 BagResult = 0;
    ClassicGuid Item[2];
    uint8 ContainerBSlot = 0;
    int32 Level = 0;

    void Write(ByteBuffer& data) const
    {
        data << BagResult;
        Item[0].Write(data);
        Item[1].Write(data);
        data << ContainerBSlot;
        if (BagResult == CantEquipLevel)
            data << Level;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> BagResult;
        if (!Item[0].Read(data) || !Item[1].Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> ContainerBSlot;
        if (BagResult == CantEquipLevel)
        {
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            data >> Level;
        }
        return true;
    }
};

// An item arrived in the inventory: looted, bought, created or received.
struct ItemPushResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ITEM_PUSH_RESULT;

    // The slot in the bag when the item made a new stack, -1 when it joined one.
    static constexpr int32 StackedSlot = -1;

    ClassicGuid Player;
    uint8 Slot = 0;
    int32 SlotInBag = 0;
    int32 QuestLogItemId = 0;
    int32 Quantity = 0;
    int32 QuantityInInventory = 0;
    ClassicGuid ItemGuid;
    bool Pushed = false;
    bool Created = false;
    uint8 DisplayText = 0;
    ClassicItemInstance Item;

    void Write(ByteBuffer& data) const
    {
        Player.Write(data);
        data << Slot << SlotInBag << QuestLogItemId << Quantity << QuantityInInventory;
        data << int32(0);  // dungeon encounter
        data << int32(0);  // battle pet species
        data << int32(0);  // battle pet breed
        data << uint8(0);  // battle pet breed quality
        data << int32(0);  // battle pet level
        ItemGuid.Write(data);
        data.WriteBit(Pushed);
        data.WriteBit(Created);
        data.WriteBit(false);  // unused
        data.WriteBits(DisplayText, 3);
        data.WriteBit(false);  // bonus roll
        data.WriteBit(false);  // encounter loot
        data.FlushBits();
        Item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Player.Read(data))
            return false;
        if (data.size() - data.rpos() < 1 + 4 * sizeof(int32) + 3 * sizeof(int32) + 1 + sizeof(int32))
            return false;
        int32 encounter, species, breed, level;
        uint8 quality;
        data >> Slot >> SlotInBag >> QuestLogItemId >> Quantity >> QuantityInInventory;
        data >> encounter >> species >> breed >> quality >> level;
        if (encounter || species || breed || quality || level)
            return false;
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Pushed = data.ReadBit();
        Created = data.ReadBit();
        bool const unused = data.ReadBit();
        DisplayText = uint8(data.ReadBits(3));
        bool const bonusRoll = data.ReadBit();
        bool const encounterLoot = data.ReadBit();
        data.ResetBitPos();
        return !unused && !bonusRoll && !encounterLoot && Item.Read(data);
    }
};

// A book or letter opened.
struct ReadItemResultOk
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_READ_ITEM_RESULT_OK;

    ClassicGuid Item;

    void Write(ByteBuffer& data) const { Item.Write(data); }
    bool Read(ByteBuffer& data) { return Item.Read(data); }
};

// A book or letter that could not be opened, with the wait before it can be tried again.
struct ReadItemResultFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_READ_ITEM_RESULT_FAILED;

    ClassicGuid Item;
    uint32 Delay = 0;
    uint8 Subcode = 0;

    void Write(ByteBuffer& data) const
    {
        Item.Write(data);
        data << Delay;
        data.WriteBits(Subcode, 2);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> Delay;
        Subcode = uint8(data.ReadBits(2));
        data.ResetBitPos();
        return true;
    }
};

// Time left on an item with a duration.
struct ItemTimeUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ITEM_TIME_UPDATE;

    ClassicGuid ItemGuid;
    uint32 DurationLeft = 0;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data << DurationLeft;
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> DurationLeft;
        return true;
    }
};

// An item's use spell went on cooldown.
struct ItemCooldown
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ITEM_COOLDOWN;

    ClassicGuid ItemGuid;
    uint32 SpellId = 0;
    uint32 Cooldown = 0;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data << SpellId << Cooldown;
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> SpellId >> Cooldown;
        return true;
    }
};

// An enchantment applied to an item.
struct EnchantmentLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENCHANTMENT_LOG;

    ClassicGuid Owner;
    ClassicGuid Caster;
    ClassicGuid ItemGuid;
    int32 ItemId = 0;
    int32 Enchantment = 0;
    int32 EnchantSlot = 0;

    void Write(ByteBuffer& data) const
    {
        Owner.Write(data);
        Caster.Write(data);
        ItemGuid.Write(data);
        data << ItemId << Enchantment << EnchantSlot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Owner.Read(data) || !Caster.Read(data) || !ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(int32))
            return false;
        data >> ItemId >> Enchantment >> EnchantSlot;
        return true;
    }
};

// Moves an item from a bag slot into the bank, wherever it fits. This engine has only the
// character-personal bank; a request naming any other bank type is refused.
struct AutoBankItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTOBANK_ITEM;

    static constexpr int8 BankTypeCharacter = 0;

    ClassicInvUpdate Inv;
    int8 BankType = BankTypeCharacter;
    uint8 Bag = 0;
    uint8 Slot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << BankType << Bag << Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 3)
            return false;
        data >> BankType >> Bag >> Slot;
        return true;
    }
};

// Moves an item between the bank and the inventory, whichever side the source slot is on.
struct AutoStoreBankItem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTOSTORE_BANK_ITEM;

    ClassicInvUpdate Inv;
    uint8 Bag = 0;
    uint8 Slot = 0;

    void Write(ByteBuffer& data) const
    {
        Inv.Write(data);
        data << Bag << Slot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Inv.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        data >> Bag >> Slot;
        return true;
    }
};

// Buys the next bank bag slot. No success ack: the client infers it from the gold change.
struct BuyBankSlot
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BUY_BANK_SLOT;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// Time left on a temporary enchantment.
struct ItemEnchantTimeUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ITEM_ENCHANT_TIME_UPDATE;

    ClassicGuid ItemGuid;
    uint32 DurationLeft = 0;
    uint32 Slot = 0;
    ClassicGuid Owner;

    void Write(ByteBuffer& data) const
    {
        ItemGuid.Write(data);
        data << DurationLeft << Slot;
        Owner.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!ItemGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> DurationLeft >> Slot;
        return Owner.Read(data);
    }
};

#endif
