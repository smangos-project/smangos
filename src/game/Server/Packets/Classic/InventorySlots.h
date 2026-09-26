#ifndef MANGOS_CLASSIC_INVENTORYSLOTS_H
#define MANGOS_CLASSIC_INVENTORYSLOTS_H

#include "Common.h"
#include <optional>

// Inventory positions on the modern wire. The server numbers them as 1.12 does (equipment 0-18, bags
// 19-22, backpack 23-38, ...); the modern client numbers the same places differently, so every slot
// that crosses the wire goes through these.
//
// A container is named by the slot it sits in, or CLASSIC_INVENTORY_SLOT_BAG_0 for the character's own
// equipment, backpack and bank.

constexpr uint8 CLASSIC_INVENTORY_SLOT_BAG_0 = 255;

// The modern layout: equipment, professions, bags, backpack, bank, bank bags, buyback, keyring.
constexpr uint8 CLASSIC_INVSLOT_EQUIPMENT_END = 19;
constexpr uint8 CLASSIC_INVSLOT_BAG_START = 30;
constexpr uint8 CLASSIC_INVSLOT_BACKPACK_START = 35;
constexpr uint8 CLASSIC_INVSLOT_BANK_START = 59;
constexpr uint8 CLASSIC_INVSLOT_BANK_BAG_START = 87;
constexpr uint8 CLASSIC_INVSLOT_BUYBACK_START = 94;
constexpr uint8 CLASSIC_INVSLOT_KEYRING_START = 106;
constexpr uint32 CLASSIC_INVSLOT_COUNT = 146;

// The server's own layout, one run per group of places: {first server slot, count, first modern slot}.
struct ClassicInventorySlotRun
{
    uint8 ServerStart;
    uint8 Count;
    uint8 ModernStart;
};

constexpr ClassicInventorySlotRun CLASSIC_INVENTORY_SLOT_RUNS[] =
{
    { 0,  19, 0 },                                  // equipment
    { 19, 4,  CLASSIC_INVSLOT_BAG_START },          // equipped bags
    { 23, 16, CLASSIC_INVSLOT_BACKPACK_START },     // backpack
    { 39, 24, CLASSIC_INVSLOT_BANK_START },         // bank
    { 63, 6,  CLASSIC_INVSLOT_BANK_BAG_START },     // bank bags
    { 69, 12, CLASSIC_INVSLOT_BUYBACK_START },      // buyback
    { 81, 16, CLASSIC_INVSLOT_KEYRING_START },      // keyring
};

inline std::optional<uint8> ClassicSlotFromServer(uint8 serverSlot)
{
    for (ClassicInventorySlotRun const& run : CLASSIC_INVENTORY_SLOT_RUNS)
        if (serverSlot >= run.ServerStart && serverSlot < run.ServerStart + run.Count)
            return uint8(run.ModernStart + (serverSlot - run.ServerStart));
    return std::nullopt;
}

inline std::optional<uint8> ClassicSlotToServer(uint8 modernSlot)
{
    for (ClassicInventorySlotRun const& run : CLASSIC_INVENTORY_SLOT_RUNS)
        if (modernSlot >= run.ModernStart && modernSlot < run.ModernStart + run.Count)
            return uint8(run.ServerStart + (modernSlot - run.ModernStart));
    return std::nullopt;
}

// A container position: the character itself, or the slot of an equipped or bank bag.
inline std::optional<uint8> ClassicContainerFromServer(uint8 serverBag)
{
    if (serverBag == CLASSIC_INVENTORY_SLOT_BAG_0)
        return serverBag;
    return ClassicSlotFromServer(serverBag);
}

inline std::optional<uint8> ClassicContainerToServer(uint8 modernBag)
{
    if (modernBag == CLASSIC_INVENTORY_SLOT_BAG_0)
        return modernBag;
    return ClassicSlotToServer(modernBag);
}

// An item position: a container, and a slot in it. Inside the character's own inventory the slot is
// an inventory slot; inside a bag it is a plain index and stays as it is. The empty slot (255) that
// asks the server to pick one stays as it is too.
struct ClassicItemPos
{
    uint8 Bag = CLASSIC_INVENTORY_SLOT_BAG_0;
    uint8 Slot = 0;
};

constexpr uint8 CLASSIC_NULL_SLOT = 255;

inline std::optional<ClassicItemPos> ClassicPosToServer(uint8 modernBag, uint8 modernSlot)
{
    auto const bag = ClassicContainerToServer(modernBag);
    if (!bag)
        return std::nullopt;
    if (modernBag != CLASSIC_INVENTORY_SLOT_BAG_0 || modernSlot == CLASSIC_NULL_SLOT)
        return ClassicItemPos{ *bag, modernSlot };
    auto const slot = ClassicSlotToServer(modernSlot);
    if (!slot)
        return std::nullopt;
    return ClassicItemPos{ *bag, *slot };
}

inline std::optional<ClassicItemPos> ClassicPosFromServer(uint8 serverBag, uint8 serverSlot)
{
    auto const bag = ClassicContainerFromServer(serverBag);
    if (!bag)
        return std::nullopt;
    if (serverBag != CLASSIC_INVENTORY_SLOT_BAG_0 || serverSlot == CLASSIC_NULL_SLOT)
        return ClassicItemPos{ *bag, serverSlot };
    auto const slot = ClassicSlotFromServer(serverSlot);
    if (!slot)
        return std::nullopt;
    return ClassicItemPos{ *bag, *slot };
}

#endif
