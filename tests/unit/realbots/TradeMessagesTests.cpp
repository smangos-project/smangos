// Round trips for the modern trade wire.
#include "Packets/TradeMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Trade requests round-trip", "[packets]")
{
    InitiateTrade initiate;
    initiate.Guid = { 0x1234, 0x0400000000000000 };
    RoundTrip(initiate, [](InitiateTrade const& a, InitiateTrade const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(initiate);

    AcceptTrade accept;
    accept.StateIndex = 0x7654321;
    RoundTrip(accept, [](AcceptTrade const& a, AcceptTrade const& b) { CHECK(a.StateIndex == b.StateIndex); });
    RejectsEveryTruncation(accept);

    SetTradeItem setItem;
    setItem.TradeSlot = 6;
    setItem.PackSlot = 255;
    setItem.ItemSlotInPack = 35;
    RoundTrip(setItem, [](SetTradeItem const& a, SetTradeItem const& b)
    {
        CHECK(a.TradeSlot == b.TradeSlot);
        CHECK(a.PackSlot == b.PackSlot);
        CHECK(a.ItemSlotInPack == b.ItemSlotInPack);
    });
    RejectsEveryTruncation(setItem);

    ClearTradeItem clear;
    clear.TradeSlot = 3;
    RoundTrip(clear, [](ClearTradeItem const& a, ClearTradeItem const& b) { CHECK(a.TradeSlot == b.TradeSlot); });
    RejectsEveryTruncation(clear);

    SetTradeGold gold;
    gold.Coinage = 0x1122334455ull;
    RoundTrip(gold, [](SetTradeGold const& a, SetTradeGold const& b) { CHECK(a.Coinage == b.Coinage); });
    RejectsEveryTruncation(gold);
}

TEST_CASE("TradeStatusUpdate round-trips for every tail", "[packets]")
{
    TradeStatusUpdate proposed;
    proposed.Status = TradeStatusUpdate::Proposed;
    proposed.PartnerIsSameBnetAccount = true;
    proposed.Partner = { 0x99, 0x0400000000000000 };
    proposed.PartnerAccount = { 0x55, 0x0200000000000000 };
    RoundTrip(proposed, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b)
    {
        CHECK(a.Status == b.Status);
        CHECK(a.PartnerIsSameBnetAccount == b.PartnerIsSameBnetAccount);
        CHECK(a.Partner == b.Partner);
        CHECK(a.PartnerAccount == b.PartnerAccount);
    });
    RejectsEveryTruncation(proposed);

    TradeStatusUpdate initiated;
    initiated.Status = TradeStatusUpdate::Initiated;
    initiated.Id = 42;
    RoundTrip(initiated, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b) { CHECK(a.Id == b.Id); });
    RejectsEveryTruncation(initiated);

    TradeStatusUpdate failed;
    failed.Status = TradeStatusUpdate::Failed;
    failed.FailureForYou = true;
    failed.BagResult = 3;
    failed.ItemId = 2589;
    RoundTrip(failed, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b)
    {
        CHECK(b.FailureForYou);
        CHECK(a.BagResult == b.BagResult);
        CHECK(a.ItemId == b.ItemId);
    });
    RejectsEveryTruncation(failed);

    TradeStatusUpdate slotted;
    slotted.Status = TradeStatusUpdate::NotOnTaplist;
    slotted.TradeSlot = 4;
    RoundTrip(slotted, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b) { CHECK(a.TradeSlot == b.TradeSlot); });
    RejectsEveryTruncation(slotted);

    TradeStatusUpdate currency;
    currency.Status = TradeStatusUpdate::NotEnoughCurrency;
    currency.CurrencyType = 395;
    currency.CurrencyQuantity = 7;
    RoundTrip(currency, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b)
    {
        CHECK(a.CurrencyType == b.CurrencyType);
        CHECK(a.CurrencyQuantity == b.CurrencyQuantity);
    });
    RejectsEveryTruncation(currency);

    TradeStatusUpdate complete;
    complete.Status = 8;
    RoundTrip(complete, [](TradeStatusUpdate const& a, TradeStatusUpdate const& b) { CHECK(a.Status == b.Status); });
    RejectsEveryTruncation(complete);
}

TEST_CASE("TradeUpdated round-trips with a plain and a wrapped item", "[packets]")
{
    TradeUpdated update;
    update.WhichPlayer = 1;
    update.Id = 9;
    update.ClientStateIndex = 4;
    update.CurrentStateIndex = 0x5a5a5a;
    update.Gold = 12345;
    update.ProposedEnchantment = 7;

    TradeItemEntry plain;
    plain.Slot = 0;
    plain.StackCount = 5;
    plain.Item.ItemId = 2589;
    plain.Item.RandomPropertiesSeed = 11;
    plain.Item.RandomPropertiesId = -3;
    TradeItemDetails details;
    details.EnchantId = 2504;
    details.Creator = { 0x77, 0x0400000000000000 };
    details.Charges = 3;
    details.Lock = true;
    details.MaxDurability = 40;
    details.Durability = 31;
    plain.Details = details;
    update.Items.push_back(plain);

    TradeItemEntry wrapped;
    wrapped.Slot = 6;
    wrapped.StackCount = 1;
    wrapped.GiftCreator = { 0x88, 0x0400000000000000 };
    wrapped.Item.ItemId = 5042;
    update.Items.push_back(wrapped);

    RoundTrip(update, [](TradeUpdated const& a, TradeUpdated const& b)
    {
        CHECK(a.WhichPlayer == b.WhichPlayer);
        CHECK(a.Id == b.Id);
        CHECK(a.ClientStateIndex == b.ClientStateIndex);
        CHECK(a.CurrentStateIndex == b.CurrentStateIndex);
        CHECK(a.Gold == b.Gold);
        CHECK(a.ProposedEnchantment == b.ProposedEnchantment);
        REQUIRE(b.Items.size() == 2);

        TradeItemEntry const& first = b.Items[0];
        CHECK(first.Slot == 0);
        CHECK(first.StackCount == 5);
        CHECK(first.Item.ItemId == 2589);
        CHECK(first.Item.RandomPropertiesId == -3);
        REQUIRE(first.Details.has_value());
        CHECK(first.Details->EnchantId == 2504);
        CHECK(first.Details->Creator == a.Items[0].Details->Creator);
        CHECK(first.Details->Charges == 3);
        CHECK(first.Details->Lock);
        CHECK(first.Details->MaxDurability == 40);
        CHECK(first.Details->Durability == 31);

        TradeItemEntry const& second = b.Items[1];
        CHECK(second.Slot == 6);
        CHECK(second.GiftCreator == a.Items[1].GiftCreator);
        CHECK(!second.Details.has_value());
    });
    RejectsEveryTruncation(update);
}

TEST_CASE("TradeUpdated rejects more items than the window has slots", "[packets]")
{
    TradeUpdated update;
    for (uint32 i = 0; i <= CLASSIC_TRADE_SLOT_COUNT; ++i)
    {
        TradeItemEntry item;
        item.Slot = uint8(i);
        update.Items.push_back(item);
    }

    ByteBuffer data;
    update.Write(data);
    TradeUpdated in;
    CHECK_FALSE(in.Read(data));
}
