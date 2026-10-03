// A GM bot sells an item at a spawned auctioneer over the live wire, lists it back, and a plain
// partner bot buys it out -- resolving instantly (a real listing's minimum duration is 2 hours,
// far too long for a scenario to wait on). Both bots end up near the auctioneer the same way
// MailTests.cpp gets both bots near its mailbox: the GM warps to the partner first, then spawns
// the interactive object where they both already are.
#include "Packets/AuctionHouseMessages.h"
#include "Packets/CombatMessages.h"
#include "Packets/ItemMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t LINEN_CLOTH = 2589;
constexpr int32_t TEST_AUCTIONEER = 7938; // "Test Auctioneer"
constexpr uint32_t SHORTEST_AUCTION_MINUTES = 120; // the client's shortest duration choice, 2 hours
}

TEST_CASE("auction house: an item is sold, listed and bought out live", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot partner;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await partner.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        ClassicGuid const partnerGuid = partner.GetCharacterGuid();

        SayCommand(gm, ".namego " + partner.GetCharacterName());
        co_await TestDelay(2500);

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add " + std::to_string(TEST_AUCTIONEER));
        std::optional<ClassicGuid> auctioneer;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            auctioneer = FindNewUnit(gm, before);
            return auctioneer.has_value();
        }, 10000));
        co_await TestDelay(500);

        SetSelection selectSelf;
        selectSelf.Target = gmGuid;
        gm.Send(selectSelf);
        co_await TestDelay(500);
        SayCommand(gm, ".modify money 100000");
        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " 5");
        auto push = co_await gm.Packet<ItemPushResult>(10000, [](ItemPushResult const& p) { return p.Item.ItemId == LINEN_CLOTH; });
        REQUIRE(push);

        AuctionHelloRequest hello;
        hello.AuctioneerGuid = *auctioneer;
        gm.Send(hello);
        auto helloResponse = co_await gm.Packet<AuctionHelloResponse>(5000);
        REQUIRE(helloResponse);
        CHECK(helloResponse->AuctioneerGuid == *auctioneer);

        AuctionSellItem sell;
        sell.AuctioneerGuid = *auctioneer;
        sell.Items.push_back({ push->ItemGuid, uint32(push->Quantity) });
        sell.Bid = 100;
        sell.Buyout = 500;
        sell.Etime = SHORTEST_AUCTION_MINUTES;
        gm.Send(sell);
        auto sellResult = co_await gm.Packet<AuctionCommandResult>(5000);
        REQUIRE(sellResult);
        CHECK(sellResult->Action == 0);    // AUCTION_STARTED
        CHECK(sellResult->ErrorCode == 0); // AUCTION_OK

        AuctionListItems list;
        list.AuctioneerGuid = *auctioneer;
        gm.Send(list);
        auto listResult = co_await gm.Packet<AuctionListItemsResult>(5000);
        REQUIRE(listResult);

        AuctionItem const* posting = nullptr;
        for (AuctionItem const& item : listResult->Items)
            if (item.Item.ItemId == LINEN_CLOTH && item.Owner == gmGuid)
                posting = &item;
        REQUIRE(posting);
        CHECK(posting->MinBid == 100);
        CHECK(posting->BuyoutPrice == 500);
        uint32_t const auctionId = uint32_t(posting->AuctionId);

        SetSelection selectPartner;
        selectPartner.Target = partnerGuid;
        gm.Send(selectPartner);
        co_await TestDelay(500);
        SayCommand(gm, ".modify money 100000");
        co_await TestDelay(500);
        gm.Send(selectSelf);
        co_await TestDelay(500);

        AuctionPlaceBid buyout;
        buyout.AuctioneerGuid = *auctioneer;
        buyout.AuctionId = auctionId;
        buyout.Price = 500; // matches Buyout above -- resolves the auction immediately
        partner.Send(buyout);
        auto bidResult = co_await partner.Packet<AuctionCommandResult>(5000);
        REQUIRE(bidResult);
        CHECK(bidResult->Action == 2);    // AUCTION_BID_PLACED
        CHECK(bidResult->ErrorCode == 0); // AUCTION_OK

        // Both the owner's and the bidder's notifications can arrive before the routine gets back
        // around to awaiting them (they're a direct, near-instant consequence of the bid packet
        // partner just sent), so history-aware waits are used here, not Packet<T>.
        auto const closedFilter = [&](AuctionClosedNotification const& p) { return p.AuctionId == int32_t(auctionId); };
        REQUIRE(co_await gm.WaitReceived<AuctionClosedNotification>(5000, closedFilter));
        auto closed = gm.FindReceived<AuctionClosedNotification>(closedFilter);
        REQUIRE(closed);
        CHECK(closed->Sold);

        auto const wonFilter = [&](AuctionWonNotification const& p) { return p.AuctionId == int32_t(auctionId); };
        REQUIRE(co_await partner.WaitReceived<AuctionWonNotification>(5000, wonFilter));
        auto won = partner.FindReceived<AuctionWonNotification>(wonFilter);
        REQUIRE(won);
        CHECK(won->BidderGuid == partnerGuid);
        CHECK(won->Item.ItemId == LINEN_CLOTH);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("auction house: a bid notifies the owner and a higher bid notifies the outbid bidder", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot first;
        TestBot rival;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await first.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await rival.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        ClassicGuid const firstGuid = first.GetCharacterGuid();
        ClassicGuid const rivalGuid = rival.GetCharacterGuid();

        SayCommand(gm, ".namego " + first.GetCharacterName());
        SayCommand(gm, ".namego " + rival.GetCharacterName());
        co_await TestDelay(2500);

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add " + std::to_string(TEST_AUCTIONEER));
        std::optional<ClassicGuid> auctioneer;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            auctioneer = FindNewUnit(gm, before);
            return auctioneer.has_value();
        }, 10000));
        co_await TestDelay(500);

        for (ClassicGuid const& target : { firstGuid, rivalGuid, gmGuid })
        {
            SetSelection select;
            select.Target = target;
            gm.Send(select);
            co_await TestDelay(500);
            SayCommand(gm, ".modify money 100000");
        }
        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " 5");
        auto push = co_await gm.Packet<ItemPushResult>(10000, [](ItemPushResult const& p) { return p.Item.ItemId == LINEN_CLOTH; });
        REQUIRE(push);

        AuctionSellItem sell;
        sell.AuctioneerGuid = *auctioneer;
        sell.Items.push_back({ push->ItemGuid, uint32(push->Quantity) });
        sell.Bid = 100;
        sell.Buyout = 5000;
        sell.Etime = SHORTEST_AUCTION_MINUTES;
        gm.Send(sell);
        auto sellResult = co_await gm.Packet<AuctionCommandResult>(5000);
        REQUIRE(sellResult);
        REQUIRE(sellResult->ErrorCode == 0);

        AuctionListItems list;
        list.AuctioneerGuid = *auctioneer;
        gm.Send(list);
        auto listResult = co_await gm.Packet<AuctionListItemsResult>(5000);
        REQUIRE(listResult);
        uint32_t auctionId = 0;
        for (AuctionItem const& item : listResult->Items)
            if (item.Item.ItemId == LINEN_CLOTH && item.Owner == gmGuid)
                auctionId = uint32_t(item.AuctionId);
        REQUIRE(auctionId != 0);

        AuctionPlaceBid bid;
        bid.AuctioneerGuid = *auctioneer;
        bid.AuctionId = auctionId;
        bid.Price = 100;
        first.Send(bid);
        auto firstResult = co_await first.Packet<AuctionCommandResult>(5000);
        REQUIRE(firstResult);
        CHECK(firstResult->ErrorCode == 0);

        auto const ownerBidFilter = [&](AuctionOwnerBidNotification const& p) { return p.AuctionId == int32_t(auctionId); };
        REQUIRE(co_await gm.WaitReceived<AuctionOwnerBidNotification>(5000, ownerBidFilter));
        auto ownerBid = gm.FindReceived<AuctionOwnerBidNotification>(ownerBidFilter);
        REQUIRE(ownerBid);
        CHECK(ownerBid->BidAmount == 100);
        CHECK(ownerBid->Bidder == firstGuid);
        CHECK(ownerBid->Item.ItemId == LINEN_CLOTH);

        bid.Price = 200;
        rival.Send(bid);
        auto rivalResult = co_await rival.Packet<AuctionCommandResult>(5000);
        REQUIRE(rivalResult);
        CHECK(rivalResult->ErrorCode == 0);

        auto const outbidFilter = [&](AuctionOutbidNotification const& p) { return p.AuctionId == int32_t(auctionId); };
        REQUIRE(co_await first.WaitReceived<AuctionOutbidNotification>(5000, outbidFilter));
        auto outbid = first.FindReceived<AuctionOutbidNotification>(outbidFilter);
        REQUIRE(outbid);
        CHECK(outbid->BidAmount == 200);
        CHECK(outbid->BidderGuid == rivalGuid);
        CHECK(outbid->Item.ItemId == LINEN_CLOTH);

        AuctionRemoveItem remove;
        remove.AuctioneerGuid = *auctioneer;
        remove.AuctionId = auctionId;
        gm.Send(remove);
        co_await TestDelay(1000);

        SayCommand(gm, ".npc despawn");
    });
}
