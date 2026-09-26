#include "Packets/Classic/InventorySlots.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Equipment keeps its slot numbers on the wire", "[inventoryslots]")
{
    for (uint8 slot = 0; slot < 19; ++slot)
    {
        CHECK(ClassicSlotFromServer(slot) == slot);
        CHECK(ClassicSlotToServer(slot) == slot);
    }
}

TEST_CASE("Bags, backpack, bank, buyback and keyring move to the modern numbers", "[inventoryslots]")
{
    CHECK(ClassicSlotFromServer(19) == 30);   // first equipped bag
    CHECK(ClassicSlotFromServer(22) == 33);   // last equipped bag
    CHECK(ClassicSlotFromServer(23) == 35);   // first backpack slot
    CHECK(ClassicSlotFromServer(38) == 50);   // last backpack slot
    CHECK(ClassicSlotFromServer(39) == 59);   // first bank slot
    CHECK(ClassicSlotFromServer(62) == 82);
    CHECK(ClassicSlotFromServer(63) == 87);   // first bank bag
    CHECK(ClassicSlotFromServer(68) == 92);
    CHECK(ClassicSlotFromServer(69) == 94);   // first buyback slot
    CHECK(ClassicSlotFromServer(80) == 105);
    CHECK(ClassicSlotFromServer(81) == 106);  // first keyring slot
    CHECK(ClassicSlotFromServer(96) == 121);
}

TEST_CASE("Every server slot survives a round trip and slots outside the layout map nowhere", "[inventoryslots]")
{
    for (uint32 slot = 0; slot < 97; ++slot)
    {
        auto const modern = ClassicSlotFromServer(uint8(slot));
        REQUIRE(modern.has_value());
        CHECK(*modern < CLASSIC_INVSLOT_COUNT);
        CHECK(ClassicSlotToServer(*modern) == slot);
    }

    CHECK_FALSE(ClassicSlotFromServer(97).has_value());
    CHECK_FALSE(ClassicSlotFromServer(CLASSIC_INVENTORY_SLOT_BAG_0).has_value());

    // profession slots, the reagent bag and everything past the keyring have no server place
    CHECK_FALSE(ClassicSlotToServer(19).has_value());
    CHECK_FALSE(ClassicSlotToServer(29).has_value());
    CHECK_FALSE(ClassicSlotToServer(34).has_value());
    CHECK_FALSE(ClassicSlotToServer(51).has_value());
    CHECK_FALSE(ClassicSlotToServer(122).has_value());
}

TEST_CASE("A container is a slot or the character itself", "[inventoryslots]")
{
    CHECK(ClassicContainerFromServer(CLASSIC_INVENTORY_SLOT_BAG_0) == CLASSIC_INVENTORY_SLOT_BAG_0);
    CHECK(ClassicContainerToServer(CLASSIC_INVENTORY_SLOT_BAG_0) == CLASSIC_INVENTORY_SLOT_BAG_0);
    CHECK(ClassicContainerFromServer(20) == 31);
    CHECK(ClassicContainerToServer(31) == 20);
    CHECK_FALSE(ClassicContainerToServer(200).has_value());
}

TEST_CASE("A position translates the slot only inside the character's own inventory", "[inventoryslots]")
{
    auto const own = ClassicPosToServer(CLASSIC_INVENTORY_SLOT_BAG_0, 36);
    REQUIRE(own.has_value());
    CHECK(own->Bag == CLASSIC_INVENTORY_SLOT_BAG_0);
    CHECK(own->Slot == 24);

    auto const inBag = ClassicPosToServer(31, 7);
    REQUIRE(inBag.has_value());
    CHECK(inBag->Bag == 20);
    CHECK(inBag->Slot == 7);

    auto const autoSlot = ClassicPosToServer(31, CLASSIC_NULL_SLOT);
    REQUIRE(autoSlot.has_value());
    CHECK(autoSlot->Slot == CLASSIC_NULL_SLOT);

    CHECK_FALSE(ClassicPosToServer(CLASSIC_INVENTORY_SLOT_BAG_0, 20).has_value());
    CHECK_FALSE(ClassicPosToServer(20, 0).has_value());

    auto const back = ClassicPosFromServer(20, 7);
    REQUIRE(back.has_value());
    CHECK(back->Bag == 31);
    CHECK(back->Slot == 7);
    auto const ownBack = ClassicPosFromServer(CLASSIC_INVENTORY_SLOT_BAG_0, 24);
    REQUIRE(ownBack.has_value());
    CHECK(ownBack->Slot == 36);
}
