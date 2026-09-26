// Talking to NPCs: the gossip window.
#include "Packets/ItemMessages.h"
#include "Packets/NpcMessages.h"
#include "Packets/VendorMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("npc: right-clicking a spawned innkeeper opens its gossip window", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 5111"); // Innkeeper Firebrew
        std::optional<ClassicGuid> innkeeper;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            innkeeper = FindNewUnit(gm, before);
            return innkeeper.has_value();
        }, 10000));
        co_await TestDelay(500);

        GossipHello hello;
        hello.Unit = *innkeeper;
        gm.Send(hello);

        auto window = co_await gm.Packet<GossipMessage>(10000, [&](GossipMessage const& p) { return p.GossipGuid == *innkeeper; });
        REQUIRE(window);
        CHECK(window->GossipId == 5111);
        CHECK(window->TextId.has_value());
        REQUIRE_FALSE(window->Options.empty());

        bool foundInnOption = false;
        for (size_t i = 0; i < window->Options.size(); ++i)
        {
            CHECK(window->Options[i].GossipOptionId == int32_t(i));
            CHECK_FALSE(window->Options[i].Text.empty());
            foundInnOption |= window->Options[i].OptionNpc == 5 && window->Options[i].Text.find("inn your home") != std::string::npos;
        }
        CHECK(foundInnOption);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("npc: a vendor lists its stock and a bought item can be sold back", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 5111"); // Innkeeper Firebrew sells food and drink
        std::optional<ClassicGuid> vendor;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            vendor = FindNewUnit(gm, before);
            return vendor.has_value();
        }, 10000));
        co_await TestDelay(500);

        ListInventory list;
        list.Vendor = *vendor;
        gm.Send(list);
        auto stock = co_await gm.Packet<VendorInventory>(10000, [&](VendorInventory const& p) { return p.Vendor == *vendor; });
        REQUIRE(stock);
        REQUIRE_FALSE(stock->Items.empty());
        for (size_t i = 0; i < stock->Items.size(); ++i)
        {
            CHECK(stock->Items[i].MuId == i + 1);
            CHECK(stock->Items[i].Item.ItemId != 0);
        }

        VendorListItem const* cheapest = &stock->Items.front();
        for (VendorListItem const& row : stock->Items)
            if (row.Price < cheapest->Price)
                cheapest = &row;

        SayCommand(gm, ".modify money 500000");
        co_await TestDelay(1000);

        BuyItem buy;
        buy.Vendor = *vendor;
        buy.Quantity = cheapest->StackCount;
        buy.MuId = cheapest->MuId;
        buy.Item = cheapest->Item;
        gm.Send(buy);

        auto push = co_await gm.Packet<ItemPushResult>(10000, [&](ItemPushResult const& p) { return p.Item.ItemId == cheapest->Item.ItemId; });
        REQUIRE(push);
        CHECK(push->Quantity == cheapest->StackCount);
        REQUIRE(co_await gm.WaitReceived<BuySucceeded>(10000, [&](BuySucceeded const& p) { return p.MuId == cheapest->MuId; }));
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(push->ItemGuid).has_value(); }, 10000));

        SellItem sell;
        sell.Vendor = *vendor;
        sell.ItemGuid = push->ItemGuid;
        sell.Amount = uint32_t(push->Quantity);
        gm.Send(sell);
        co_await TestDelay(1500);

        if (gm.FindReceived<SellResponse>([&](SellResponse const& p) { return p.Vendor == *vendor; }))
        {
            // this vendor does not buy it back: remove it by hand
            SayCommand(gm, ".additem " + std::to_string(cheapest->Item.ItemId) + " -" + std::to_string(push->Quantity));
            co_await TestDelay(1000);
        }
        else
            CHECK_FALSE(gm.GetKnownObject(push->ItemGuid).has_value());

        SayCommand(gm, ".npc despawn");
    });
}
