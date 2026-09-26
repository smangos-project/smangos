// Looting a killed creature: opening the window, taking the money and the items, closing it.
#include "Packets/CombatLog.h"
#include "Packets/CombatMessages.h"
#include "Packets/ItemMessages.h"
#include "Packets/LootMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr uint8_t LOOT_ERROR_DIDNT_KILL = 0;
}

TEST_CASE("loot: a GM bot loots a killed creature's money and items", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 6"); // Kobold Vermin: always drops a few coins
        std::optional<ClassicGuid> victim;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            victim = FindNewUnit(gm, before);
            return victim.has_value();
        }, 10000));

        SetSelection select;
        select.Target = *victim;
        gm.Send(select);
        co_await TestDelay(500);
        SayCommand(gm, ".damage 1000");
        REQUIRE(co_await gm.WaitReceived<PartyKillLog>(10000, [&](PartyKillLog const& p) { return p.Victim == *victim; }));
        co_await TestDelay(1000);

        LootUnit open;
        open.Unit = *victim;
        gm.Send(open);

        auto window = co_await gm.Packet<LootResponse>(10000, [&](LootResponse const& p) { return p.Owner == *victim; });
        REQUIRE(window);
        REQUIRE(window->Acquired);
        CHECK(window->LootObj == *victim);
        CHECK(window->AcquireReason == 1); // LOOT_CORPSE
        CHECK(window->Coins > 0);
        for (LootItemData const& item : window->Items)
        {
            CHECK(item.LootListId >= 1);
            CHECK(item.Item.ItemId != 0);
            CHECK(item.Quantity >= 1);
        }

        LootMoney money;
        gm.Send(money);
        // the coin removal goes out before the money notice, so both are read from what was received
        REQUIRE(co_await gm.WaitReceived<LootMoneyNotify>(10000, [](LootMoneyNotify const&) { return true; }));
        auto notify = gm.FindReceived<LootMoneyNotify>([](LootMoneyNotify const&) { return true; });
        REQUIRE(notify);
        CHECK(notify->Money == window->Coins);
        CHECK(notify->SoleLooter);
        auto removed = gm.FindReceived<CoinRemoved>([&](CoinRemoved const& p) { return p.LootObj == *victim; });
        REQUIRE(removed);

        if (!window->Items.empty())
        {
            LootItemData const& first = window->Items.front();
            LootItemRequest take;
            take.Loot = { { *victim, first.LootListId } };
            gm.Send(take);

            auto gone = co_await gm.Packet<LootRemoved>(10000, [&](LootRemoved const& p) { return p.LootObj == *victim; });
            REQUIRE(gone);
            CHECK(gone->LootListId == first.LootListId);

            auto push = co_await gm.Packet<ItemPushResult>(10000, [&](ItemPushResult const& p) { return p.Item.ItemId == first.Item.ItemId; });
            if (push)
            {
                CHECK(push->Quantity == int32_t(first.Quantity));
                SayCommand(gm, ".additem " + std::to_string(first.Item.ItemId) + " -" + std::to_string(first.Quantity));
            }
        }

        LootRelease release;
        release.Object = *victim;
        gm.Send(release);
        auto closed = co_await gm.Packet<LootReleaseResponse>(10000, [&](LootReleaseResponse const& p) { return p.LootObj == *victim; });
        REQUIRE(closed);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("loot: looting something that was never killed is refused", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 6");
        std::optional<ClassicGuid> alive;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            alive = FindNewUnit(gm, before);
            return alive.has_value();
        }, 10000));

        LootUnit open;
        open.Unit = *alive;
        gm.Send(open);

        auto refusal = co_await gm.Packet<LootResponse>(10000, [&](LootResponse const& p) { return p.Owner == *alive; });
        REQUIRE(refusal);
        CHECK_FALSE(refusal->Acquired);
        CHECK(refusal->FailureReason == LOOT_ERROR_DIDNT_KILL);
        CHECK(refusal->Items.empty());

        SayCommand(gm, ".npc despawn");
    });
}
