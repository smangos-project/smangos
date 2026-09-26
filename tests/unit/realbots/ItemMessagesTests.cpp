// Round trips for the modern item wire.
#include "Packets/ItemMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicInvUpdate PredictedMoves()
{
    ClassicInvUpdate inv;
    inv.Items = { { 255, 35 }, { 31, 2 } };
    return inv;
}

void CheckInv(ClassicInvUpdate const& a, ClassicInvUpdate const& b)
{
    REQUIRE(a.Items.size() == b.Items.size());
    for (size_t i = 0; i < a.Items.size(); ++i)
    {
        CHECK(a.Items[i].ContainerSlot == b.Items[i].ContainerSlot);
        CHECK(a.Items[i].Slot == b.Items[i].Slot);
    }
}
}

TEST_CASE("SwapInvItem round-trips with and without predicted moves", "[packets]")
{
    SwapInvItem packet;
    packet.Inv = PredictedMoves();
    packet.Destination = 36;
    packet.Source = 35;
    RoundTrip(packet, [](SwapInvItem const& a, SwapInvItem const& b)
    {
        CheckInv(a.Inv, b.Inv);
        CHECK(a.Destination == b.Destination);
        CHECK(a.Source == b.Source);
    });
    RejectsEveryTruncation(packet);

    SwapInvItem plain;
    plain.Destination = 1;
    plain.Source = 2;
    RoundTrip(plain, [](SwapInvItem const& a, SwapInvItem const& b)
    {
        CHECK(b.Inv.Items.empty());
        CHECK(a.Source == b.Source);
    });
    RejectsEveryTruncation(plain);
}

TEST_CASE("SwapItem round-trips", "[packets]")
{
    SwapItem packet;
    packet.Inv = PredictedMoves();
    packet.DestinationBag = 31;
    packet.SourceBag = 255;
    packet.DestinationSlot = 4;
    packet.SourceSlot = 37;
    RoundTrip(packet, [](SwapItem const& a, SwapItem const& b)
    {
        CheckInv(a.Inv, b.Inv);
        CHECK(a.DestinationBag == b.DestinationBag);
        CHECK(a.SourceBag == b.SourceBag);
        CHECK(a.DestinationSlot == b.DestinationSlot);
        CHECK(a.SourceSlot == b.SourceSlot);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AutoEquipItem, AutoEquipItemSlot and AutoStoreBagItem round-trip", "[packets]")
{
    AutoEquipItem equip;
    equip.Inv = PredictedMoves();
    equip.PackSlot = 255;
    equip.Slot = 40;
    RoundTrip(equip, [](AutoEquipItem const& a, AutoEquipItem const& b)
    {
        CheckInv(a.Inv, b.Inv);
        CHECK(a.PackSlot == b.PackSlot);
        CHECK(a.Slot == b.Slot);
    });
    RejectsEveryTruncation(equip);

    AutoEquipItemSlot slot;
    slot.Item = { 77, 0x0C00000000000000ULL };
    slot.Slot = 15;
    RoundTrip(slot, [](AutoEquipItemSlot const& a, AutoEquipItemSlot const& b)
    {
        CHECK(a.Item == b.Item);
        CHECK(a.Slot == b.Slot);
    });
    RejectsEveryTruncation(slot);

    AutoStoreBagItem store;
    store.DestinationBag = 30;
    store.SourceBag = 255;
    store.SourceSlot = 44;
    RoundTrip(store, [](AutoStoreBagItem const& a, AutoStoreBagItem const& b)
    {
        CHECK(a.DestinationBag == b.DestinationBag);
        CHECK(a.SourceBag == b.SourceBag);
        CHECK(a.SourceSlot == b.SourceSlot);
    });
    RejectsEveryTruncation(store);
}

TEST_CASE("SplitItem, DestroyItem and ReadItem round-trip", "[packets]")
{
    SplitItem split;
    split.Inv = PredictedMoves();
    split.FromPackSlot = 255;
    split.FromSlot = 35;
    split.ToPackSlot = 31;
    split.ToSlot = 3;
    split.Quantity = 7;
    RoundTrip(split, [](SplitItem const& a, SplitItem const& b)
    {
        CheckInv(a.Inv, b.Inv);
        CHECK(a.FromPackSlot == b.FromPackSlot);
        CHECK(a.FromSlot == b.FromSlot);
        CHECK(a.ToPackSlot == b.ToPackSlot);
        CHECK(a.ToSlot == b.ToSlot);
        CHECK(a.Quantity == b.Quantity);
    });
    RejectsEveryTruncation(split);

    DestroyItem destroy;
    destroy.Count = 5;
    destroy.ContainerId = 255;
    destroy.SlotNum = 41;
    RoundTrip(destroy, [](DestroyItem const& a, DestroyItem const& b)
    {
        CHECK(a.Count == b.Count);
        CHECK(a.ContainerId == b.ContainerId);
        CHECK(a.SlotNum == b.SlotNum);
    });
    RejectsEveryTruncation(destroy);

    ReadItem read;
    read.PackSlot = 255;
    read.Slot = 36;
    RoundTrip(read, [](ReadItem const& a, ReadItem const& b)
    {
        CHECK(a.PackSlot == b.PackSlot);
        CHECK(a.Slot == b.Slot);
    });
    RejectsEveryTruncation(read);
}

TEST_CASE("UseItem round-trips the item position, item guid and cast request", "[packets]")
{
    UseItem use;
    use.PackSlot = 255;
    use.Slot = 38;
    use.CastItem = { 12, 0x0C00000000000000ULL };
    use.Cast.CastId = { 9, 0xBC00000000000000ULL };
    use.Cast.SpellId = 2050;
    use.Cast.Visual.SpellXSpellVisualId = 11;
    RoundTrip(use, [](UseItem const& a, UseItem const& b)
    {
        CHECK(a.PackSlot == b.PackSlot);
        CHECK(a.Slot == b.Slot);
        CHECK(a.CastItem == b.CastItem);
        CHECK(a.Cast.CastId == b.Cast.CastId);
        CHECK(a.Cast.SpellId == b.Cast.SpellId);
    });
    RejectsEveryTruncation(use);
}

TEST_CASE("InventoryChangeFailure round-trips, carrying a level only for a level refusal", "[packets]")
{
    InventoryChangeFailure failure;
    failure.BagResult = 23;
    failure.Item[0] = { 5, 0x0C00000000000000ULL };
    failure.ContainerBSlot = 4;
    RoundTrip(failure, [](InventoryChangeFailure const& a, InventoryChangeFailure const& b)
    {
        CHECK(a.BagResult == b.BagResult);
        CHECK(a.Item[0] == b.Item[0]);
        CHECK(b.Item[1].IsEmpty());
        CHECK(a.ContainerBSlot == b.ContainerBSlot);
    });
    RejectsEveryTruncation(failure);

    InventoryChangeFailure level;
    level.BagResult = InventoryChangeFailure::CantEquipLevel;
    level.Level = 40;
    RoundTrip(level, [](InventoryChangeFailure const& a, InventoryChangeFailure const& b)
    {
        CHECK(b.Level == a.Level);
    });
    RejectsEveryTruncation(level);
}

TEST_CASE("ItemPushResult round-trips a new stack and a joined stack", "[packets]")
{
    ItemPushResult push;
    push.Player = { 3, 0x0800000000000000ULL };
    push.Slot = 255;
    push.SlotInBag = 36;
    push.Quantity = 10;
    push.QuantityInInventory = 10;
    push.ItemGuid = { 44, 0x0C00000000000000ULL };
    push.Pushed = true;
    push.DisplayText = 1;
    push.Item.ItemId = 2589;
    push.Item.RandomPropertiesSeed = 5;
    push.Item.RandomPropertiesId = -6;
    RoundTrip(push, [](ItemPushResult const& a, ItemPushResult const& b)
    {
        CHECK(a.Player == b.Player);
        CHECK(a.Slot == b.Slot);
        CHECK(a.SlotInBag == b.SlotInBag);
        CHECK(a.Quantity == b.Quantity);
        CHECK(a.QuantityInInventory == b.QuantityInInventory);
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(b.Pushed);
        CHECK_FALSE(b.Created);
        CHECK(a.DisplayText == b.DisplayText);
        CHECK(a.Item.ItemId == b.Item.ItemId);
        CHECK(a.Item.RandomPropertiesSeed == b.Item.RandomPropertiesSeed);
        CHECK(a.Item.RandomPropertiesId == b.Item.RandomPropertiesId);
    });
    RejectsEveryTruncation(push);

    ItemPushResult joined = push;
    joined.SlotInBag = ItemPushResult::StackedSlot;
    joined.Created = true;
    RoundTrip(joined, [](ItemPushResult const& a, ItemPushResult const& b)
    {
        CHECK(b.SlotInBag == ItemPushResult::StackedSlot);
        CHECK(b.Created);
    });
}

TEST_CASE("Read results, times, cooldowns and enchantments round-trip", "[packets]")
{
    ReadItemResultOk ok;
    ok.Item = { 8, 0x0C00000000000000ULL };
    RoundTrip(ok, [](ReadItemResultOk const& a, ReadItemResultOk const& b) { CHECK(a.Item == b.Item); });
    RejectsEveryTruncation(ok);

    ReadItemResultFailed failed;
    failed.Item = { 8, 0x0C00000000000000ULL };
    failed.Delay = 2000;
    failed.Subcode = 2;
    RoundTrip(failed, [](ReadItemResultFailed const& a, ReadItemResultFailed const& b)
    {
        CHECK(a.Item == b.Item);
        CHECK(a.Delay == b.Delay);
        CHECK(a.Subcode == b.Subcode);
    });
    RejectsEveryTruncation(failed);

    ItemTimeUpdate time;
    time.ItemGuid = { 9, 0x0C00000000000000ULL };
    time.DurationLeft = 3600;
    RoundTrip(time, [](ItemTimeUpdate const& a, ItemTimeUpdate const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.DurationLeft == b.DurationLeft);
    });
    RejectsEveryTruncation(time);

    ItemCooldown cooldown;
    cooldown.ItemGuid = { 9, 0x0C00000000000000ULL };
    cooldown.SpellId = 2050;
    cooldown.Cooldown = 30000;
    RoundTrip(cooldown, [](ItemCooldown const& a, ItemCooldown const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Cooldown == b.Cooldown);
    });
    RejectsEveryTruncation(cooldown);

    EnchantmentLog log;
    log.Owner = { 1, 0x0800000000000000ULL };
    log.Caster = { 2, 0x0800000000000000ULL };
    log.ItemGuid = { 3, 0x0C00000000000000ULL };
    log.ItemId = 100;
    log.Enchantment = 249;
    log.EnchantSlot = 1;
    RoundTrip(log, [](EnchantmentLog const& a, EnchantmentLog const& b)
    {
        CHECK(a.Owner == b.Owner);
        CHECK(a.Caster == b.Caster);
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.ItemId == b.ItemId);
        CHECK(a.Enchantment == b.Enchantment);
        CHECK(a.EnchantSlot == b.EnchantSlot);
    });
    RejectsEveryTruncation(log);

    ItemEnchantTimeUpdate update;
    update.ItemGuid = { 3, 0x0C00000000000000ULL };
    update.DurationLeft = 900;
    update.Slot = 1;
    update.Owner = { 1, 0x0800000000000000ULL };
    RoundTrip(update, [](ItemEnchantTimeUpdate const& a, ItemEnchantTimeUpdate const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.DurationLeft == b.DurationLeft);
        CHECK(a.Slot == b.Slot);
        CHECK(a.Owner == b.Owner);
    });
    RejectsEveryTruncation(update);
}
