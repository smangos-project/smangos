// Round trips for the modern vendor wire.
#include "Packets/VendorMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("ListInventory, SellItem, BuyBackItem and RepairItem round-trip", "[packets]")
{
    ListInventory list;
    list.Vendor = { 5, 0x2000000000000000ULL };
    RoundTrip(list, [](ListInventory const& a, ListInventory const& b) { CHECK(a.Vendor == b.Vendor); });
    RejectsEveryTruncation(list);

    SellItem sell;
    sell.Vendor = { 5, 0x2000000000000000ULL };
    sell.ItemGuid = { 9, 0x0C00000000000000ULL };
    sell.Amount = 7;
    RoundTrip(sell, [](SellItem const& a, SellItem const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.Amount == b.Amount);
    });
    RejectsEveryTruncation(sell);

    BuyBackItem buyBack;
    buyBack.Vendor = { 5, 0x2000000000000000ULL };
    buyBack.Slot = 95;
    RoundTrip(buyBack, [](BuyBackItem const& a, BuyBackItem const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        CHECK(a.Slot == b.Slot);
    });
    RejectsEveryTruncation(buyBack);

    RepairItem repair;
    repair.Npc = { 5, 0x2000000000000000ULL };
    repair.ItemGuid = { 9, 0x0C00000000000000ULL };
    repair.UseGuildBank = true;
    RoundTrip(repair, [](RepairItem const& a, RepairItem const& b)
    {
        CHECK(a.Npc == b.Npc);
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(b.UseGuildBank);
    });
    RejectsEveryTruncation(repair);
}

TEST_CASE("BuyItem round-trips", "[packets]")
{
    BuyItem buy;
    buy.Vendor = { 5, 0x2000000000000000ULL };
    buy.Container = { 1, 0x0800000000000000ULL };
    buy.Quantity = 5;
    buy.MuId = 3;
    buy.Slot = 36;
    buy.Item.ItemId = 2512;
    RoundTrip(buy, [](BuyItem const& a, BuyItem const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        CHECK(a.Container == b.Container);
        CHECK(a.Quantity == b.Quantity);
        CHECK(a.MuId == b.MuId);
        CHECK(a.Slot == b.Slot);
        CHECK(a.ItemType == b.ItemType);
        CHECK(a.Item.ItemId == b.Item.ItemId);
    });
    RejectsEveryTruncation(buy);
}

TEST_CASE("VendorInventory round-trips a stocked and an empty vendor", "[packets]")
{
    VendorInventory stock;
    stock.Vendor = { 5, 0x2000000000000000ULL };
    VendorListItem& first = stock.Items.emplace_back();
    first.Price = 50;
    first.MuId = 1;
    first.StackCount = 20;
    first.Item.ItemId = 2512;
    VendorListItem& limited = stock.Items.emplace_back();
    limited.Price = 123456;
    limited.MuId = 2;
    limited.Quantity = 3;
    limited.Item.ItemId = 4540;
    RoundTrip(stock, [](VendorInventory const& a, VendorInventory const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        REQUIRE(a.Items.size() == b.Items.size());
        for (size_t i = 0; i < a.Items.size(); ++i)
        {
            CHECK(a.Items[i].Price == b.Items[i].Price);
            CHECK(a.Items[i].MuId == b.Items[i].MuId);
            CHECK(a.Items[i].Type == b.Items[i].Type);
            CHECK(a.Items[i].StackCount == b.Items[i].StackCount);
            CHECK(a.Items[i].Quantity == b.Items[i].Quantity);
            CHECK(a.Items[i].Item.ItemId == b.Items[i].Item.ItemId);
        }
    });
    RejectsEveryTruncation(stock);

    VendorInventory empty;
    empty.Vendor = { 6, 0x2000000000000000ULL };
    RoundTrip(empty, [](VendorInventory const&, VendorInventory const& b) { CHECK(b.Items.empty()); });
    RejectsEveryTruncation(empty);
}

TEST_CASE("Purchase and sale results round-trip", "[packets]")
{
    BuySucceeded succeeded;
    succeeded.Vendor = { 5, 0x2000000000000000ULL };
    succeeded.MuId = 2;
    succeeded.NewQuantity = -1;
    succeeded.QuantityBought = 5;
    RoundTrip(succeeded, [](BuySucceeded const& a, BuySucceeded const& b)
    {
        CHECK(a.MuId == b.MuId);
        CHECK(b.NewQuantity == -1);
        CHECK(a.QuantityBought == b.QuantityBought);
    });
    RejectsEveryTruncation(succeeded);

    BuyFailed failed;
    failed.Vendor = { 5, 0x2000000000000000ULL };
    failed.MuId = 2;
    failed.Reason = 2;
    RoundTrip(failed, [](BuyFailed const& a, BuyFailed const& b)
    {
        CHECK(a.MuId == b.MuId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(failed);

    SellResponse response;
    response.Vendor = { 5, 0x2000000000000000ULL };
    response.ItemGuids = { { 9, 0x0C00000000000000ULL }, { 10, 0x0C00000000000000ULL } };
    response.Reason = 2;
    RoundTrip(response, [](SellResponse const& a, SellResponse const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        CHECK(a.ItemGuids == b.ItemGuids);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(response);
}
