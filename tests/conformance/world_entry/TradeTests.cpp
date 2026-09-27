// Two bots trading gold and an item, and calling a trade off, with a GM bot providing the goods.
#include "Packets/CombatMessages.h"
#include "Packets/ItemMessages.h"
#include "Packets/TradeMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t LINEN_CLOTH = 2589;
constexpr uint8_t BAG_0 = 255;
constexpr uint8_t STATUS_CANCELLED = 3;
constexpr uint8_t STATUS_ACCEPTED = 4;
constexpr uint8_t STATUS_COMPLETE = 8;
constexpr uint8_t STATUS_STATE_CHANGED = 9;

bool HasStatus(TestBot const& bot, uint8_t status)
{
    return bot.FindReceived<TradeStatusUpdate>([&](TradeStatusUpdate const& p) { return p.Status == status; }).has_value();
}

void ClearCloth(TestBot const& gm)
{
    for (int count : { 20, 15, 10, 5, 1 })
        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " -" + std::to_string(count));
}

TestWaitUntil WaitStatus(TestBot const& bot, uint8_t status)
{
    return bot.WaitReceived<TradeStatusUpdate>(10000, [status](TradeStatusUpdate const& p) { return p.Status == status; });
}
}

TEST_CASE("trade: a proposed trade is opened, filled, accepted and completed", "[gameplay]")
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
        // Pooled characters are saved as they were left, so the partner may start dead or far away.
        SayCommand(gm, ".revive " + partner.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".namego " + partner.GetCharacterName());
        co_await TestDelay(2500);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(partnerGuid).has_value(); }, 10000));

        // A partner still holding cloth would merge the stack instead of receiving the item.
        SetSelection select;
        select.Target = partnerGuid;
        gm.Send(select);
        co_await TestDelay(500);
        ClearCloth(gm);
        co_await TestDelay(1500);
        select.Target = gmGuid;
        gm.Send(select);
        co_await TestDelay(500);
        ClearCloth(gm);
        co_await TestDelay(1500);
        SayCommand(gm, ".additem " + std::to_string(LINEN_CLOTH) + " 5");
        SayCommand(gm, ".modify money 1000");
        auto push = co_await gm.Packet<ItemPushResult>(10000, [](ItemPushResult const& p) { return p.Item.ItemId == LINEN_CLOTH; });
        REQUIRE(push);

        InitiateTrade initiate;
        initiate.Guid = partnerGuid;
        gm.Send(initiate);
        REQUIRE(co_await partner.WaitReceived<TradeStatusUpdate>(10000, [&](TradeStatusUpdate const& p)
        {
            return p.Status == TradeStatusUpdate::Proposed && p.Partner == gmGuid;
        }));

        partner.Send(BeginTrade());
        REQUIRE(co_await WaitStatus(gm, TradeStatusUpdate::Initiated));
        REQUIRE(co_await WaitStatus(partner, TradeStatusUpdate::Initiated));

        SetTradeItem setItem;
        setItem.TradeSlot = 0;
        setItem.PackSlot = BAG_0;
        setItem.ItemSlotInPack = uint8_t(push->SlotInBag);
        gm.Send(setItem);
        SetTradeGold setGold;
        setGold.Coinage = 50;
        gm.Send(setGold);

        REQUIRE(co_await partner.WaitReceived<TradeUpdated>(10000, [](TradeUpdated const& p)
        {
            return p.WhichPlayer == 1 && p.Gold == 50 && p.Items.size() == 1;
        }));
        std::optional<TradeUpdated> const offered = partner.LastReceived<TradeUpdated>();
        REQUIRE(offered);
        REQUIRE(offered->Items.size() == 1);
        CHECK(offered->Items[0].Slot == 0);
        CHECK(offered->Items[0].StackCount == 5);
        CHECK(offered->Items[0].Item.ItemId == LINEN_CLOTH);
        CHECK(offered->ClientStateIndex > 1);

        // An accept that echoes an index the window never had is stale.
        co_await TestDelay(1500);
        AcceptTrade stale;
        stale.StateIndex = offered->CurrentStateIndex ^ 0x5a5a5a5a;
        partner.Send(stale);
        REQUIRE(co_await WaitStatus(partner, STATUS_STATE_CHANGED));
        CHECK_FALSE(HasStatus(gm, STATUS_ACCEPTED));

        co_await TestDelay(1500);
        AcceptTrade accept;
        accept.StateIndex = offered->CurrentStateIndex;
        partner.Send(accept);
        REQUIRE(co_await WaitStatus(gm, STATUS_ACCEPTED));

        // The partner made no change, so the index its side started with is still current.
        AcceptTrade gmAccept;
        gmAccept.StateIndex = 1;
        gm.Send(gmAccept);
        REQUIRE(co_await WaitStatus(gm, STATUS_COMPLETE));
        REQUIRE(co_await WaitStatus(partner, STATUS_COMPLETE));

        // The item itself changes hands, so the partner gets a create block for the same guid.
        REQUIRE(co_await TestWaitUntil([&]() { return partner.GetKnownObject(push->ItemGuid).has_value(); }, 10000));

        // Take the cloth back from the pooled partner.
        select.Target = partnerGuid;
        gm.Send(select);
        co_await TestDelay(500);
        ClearCloth(gm);
        co_await TestDelay(1000);
    });
}

TEST_CASE("trade: a trade is called off by the partner", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot partner;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await partner.Stage(BotStage::InWorld, 30000));

        ClassicGuid const partnerGuid = partner.GetCharacterGuid();
        SayCommand(gm, ".revive " + partner.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".namego " + partner.GetCharacterName());
        co_await TestDelay(2500);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(partnerGuid).has_value(); }, 10000));

        InitiateTrade initiate;
        initiate.Guid = partnerGuid;
        gm.Send(initiate);
        REQUIRE(co_await WaitStatus(partner, TradeStatusUpdate::Proposed));

        partner.Send(CancelTrade());
        REQUIRE(co_await WaitStatus(gm, STATUS_CANCELLED));
        REQUIRE(co_await WaitStatus(partner, STATUS_CANCELLED));
    });
}
