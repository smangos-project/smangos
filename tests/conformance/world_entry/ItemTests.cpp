// Items in the inventory: getting them, moving them, refusing bad moves, using them.
#include "Packets/ItemMessages.h"
#include "Packets/SpellCast.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t LINEN_CLOTH = 2589;          // stacks to 20
constexpr int32_t MINOR_HEALING_POTION = 118;  // uses spell 439
constexpr int32_t MINOR_HEALING_SPELL = 439;
constexpr uint8_t BAG_0 = 255;
constexpr uint8_t LAST_BACKPACK_SLOT = 50;
constexpr int32_t ERR_ITEM_NOT_FOUND = 23;

std::vector<ClassicGuid> KnownItems(TestBot const& bot)
{
    std::vector<ClassicGuid> items;
    for (BotKnownObject const& object : bot.GetKnownObjects())
        if (object.ObjectType == BOT_TYPEID_ITEM)
            items.push_back(object.Guid);
    return items;
}

bool Contains(std::vector<ClassicGuid> const& guids, ClassicGuid const& guid)
{
    return std::find(guids.begin(), guids.end(), guid) != guids.end();
}

// Pooled characters keep their bags between runs, so what an earlier run left behind is cleared first.
void ClearLeftovers(TestBot const& gm, int32_t itemId)
{
    for (int count : { 20, 15, 10, 5, 1 })
        SayCommand(gm, ".additem " + std::to_string(itemId) + " -" + std::to_string(count));
}

void RequireGmTarget()
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");
}
}

TEST_CASE("items: a GM bot gets a stack, splits it, moves it and destroys the part", "[gameplay]")
{
    RequireGmTarget();

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        ClearLeftovers(gm, LINEN_CLOTH);
        co_await TestDelay(1500);

        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " 20");
        auto push = co_await gm.Packet<ItemPushResult>(10000, [](ItemPushResult const& p) { return p.Item.ItemId == LINEN_CLOTH; });
        REQUIRE(push);
        CHECK(push->Quantity == 20);
        CHECK(push->Player == gm.GetCharacterGuid());
        REQUIRE(push->Slot == BAG_0);
        REQUIRE(push->SlotInBag >= 35);   // the backpack, in the modern numbering
        REQUIRE(push->SlotInBag <= LAST_BACKPACK_SLOT);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(push->ItemGuid).has_value(); }, 10000));

        std::vector<ClassicGuid> const before = KnownItems(gm);

        SplitItem split;
        split.FromPackSlot = BAG_0;
        split.FromSlot = uint8_t(push->SlotInBag);
        split.ToPackSlot = BAG_0;
        split.ToSlot = LAST_BACKPACK_SLOT;
        split.Quantity = 5;
        gm.Send(split);

        std::optional<ClassicGuid> part;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            for (ClassicGuid const& guid : KnownItems(gm))
                if (!Contains(before, guid))
                    part = guid;
            return part.has_value();
        }, 10000));

        // put it back into a neighbouring slot and then take it away
        SwapInvItem swap;
        swap.Source = LAST_BACKPACK_SLOT;
        swap.Destination = LAST_BACKPACK_SLOT - 1;
        gm.Send(swap);
        co_await TestDelay(1000);

        DestroyItem destroy;
        destroy.ContainerId = BAG_0;
        destroy.SlotNum = LAST_BACKPACK_SLOT - 1;
        destroy.Count = 0;
        gm.Send(destroy);

        REQUIRE(co_await TestWaitUntil([&]() { return !gm.GetKnownObject(*part).has_value(); }, 10000));
        CHECK(gm.GetKnownObject(push->ItemGuid).has_value());

        auto failure = co_await gm.Packet<InventoryChangeFailure>(1500);
        CHECK_FALSE(failure);

        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " -15");
        co_await TestDelay(1000);
    });
}

TEST_CASE("items: a move from a slot with no place on the server is refused", "[gameplay]")
{
    RequireGmTarget();

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        SwapInvItem swap;
        swap.Source = 19; // a profession slot: the modern client has it, this server does not
        swap.Destination = 35;
        gm.Send(swap);

        auto failure = co_await gm.Packet<InventoryChangeFailure>(10000);
        REQUIRE(failure);
        CHECK(failure->BagResult == ERR_ITEM_NOT_FOUND);
    });
}

TEST_CASE("items: using a potion starts its cast under the request's cast id", "[gameplay]")
{
    RequireGmTarget();

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        ClearLeftovers(gm, MINOR_HEALING_POTION);
        co_await TestDelay(1500);

        SayCommand(gm, ".additem " + std::to_string(MINOR_HEALING_POTION) + " 1");
        auto push = co_await gm.Packet<ItemPushResult>(10000, [](ItemPushResult const& p) { return p.Item.ItemId == MINOR_HEALING_POTION; });
        REQUIRE(push);
        REQUIRE(push->Slot == BAG_0);
        REQUIRE(push->SlotInBag >= 35);

        ClassicGuid const castId = { 7, (uint64_t(47) << 58) | (uint64_t(MINOR_HEALING_SPELL) << 6) };
        UseItem use;
        use.PackSlot = BAG_0;
        use.Slot = uint8_t(push->SlotInBag);
        use.CastItem = push->ItemGuid;
        use.Cast.CastId = castId;
        use.Cast.SpellId = MINOR_HEALING_SPELL;
        gm.Send(use);

        auto prepare = co_await gm.Packet<SpellPrepare>(10000, [&](SpellPrepare const& p) { return p.ClientCastId == castId; });
        REQUIRE(prepare);
        CHECK_FALSE(prepare->ServerCastId.IsEmpty());

        // a potion the character cannot drink refuses under the same id
        UseItem wrong = use;
        wrong.Cast.SpellId = 133;
        wrong.Cast.CastId = { 8, (uint64_t(47) << 58) | (uint64_t(133) << 6) };
        gm.Send(wrong);
        auto refused = co_await gm.Packet<InventoryChangeFailure>(10000);
        REQUIRE(refused);
        CHECK(refused->BagResult == ERR_ITEM_NOT_FOUND);

        co_await TestDelay(1000);
        ClearLeftovers(gm, MINOR_HEALING_POTION);
        co_await TestDelay(1000);
    });
}
