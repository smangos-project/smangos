// Round trips for the modern auction house wire.
#include "Packets/AuctionHouseMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("AuctionHelloRequest round-trips", "[packets]")
{
    AuctionHelloRequest packet;
    packet.AuctioneerGuid = { 1, 0 };
    RoundTrip(packet, [](AuctionHelloRequest const& a, AuctionHelloRequest const& b) { CHECK(a.AuctioneerGuid == b.AuctioneerGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionSellItem round-trips", "[packets]")
{
    AuctionSellItem packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.Items.push_back({ { 99, 0 }, 3 });
    packet.Bid = 1000;
    packet.Buyout = 5000;
    packet.Etime = 720;
    RoundTrip(packet, [](AuctionSellItem const& a, AuctionSellItem const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        REQUIRE(b.Items.size() == 1);
        CHECK(a.Items[0].ItemGuid == b.Items[0].ItemGuid);
        CHECK(a.Items[0].UseCount == b.Items[0].UseCount);
        CHECK(a.Bid == b.Bid);
        CHECK(a.Buyout == b.Buyout);
        CHECK(a.Etime == b.Etime);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionSellItem reads a real 1.15 client post", "[packets]")
{
    // 2 Ruined Pelts, 1 silver starting bid, no buyout, 8 hours
    std::string const hex = "03a79831c08308042064000000000000000000000000000000e00100000401a01d040c02000000";
    ByteBuffer data;
    for (size_t i = 0; i < hex.size(); i += 2)
        data << uint8(std::stoul(hex.substr(i, 2), nullptr, 16));
    AuctionSellItem packet;
    REQUIRE(packet.Read(data));
    CHECK(data.rpos() == data.size());
    CHECK(packet.Bid == 100);
    CHECK(packet.Buyout == 0);
    CHECK(packet.Etime == 480);
    REQUIRE(packet.Items.size() == 1);
    CHECK(packet.Items[0].UseCount == 2);
}

TEST_CASE("AuctionRemoveItem round-trips", "[packets]")
{
    AuctionRemoveItem packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.AuctionId = 42;
    RoundTrip(packet, [](AuctionRemoveItem const& a, AuctionRemoveItem const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.AuctionId == b.AuctionId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionListOwnerItems round-trips", "[packets]")
{
    AuctionListOwnerItems packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.ListFrom = 50;
    RoundTrip(packet, [](AuctionListOwnerItems const& a, AuctionListOwnerItems const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.ListFrom == b.ListFrom);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionPlaceBid round-trips", "[packets]")
{
    AuctionPlaceBid packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.AuctionId = 42;
    packet.Price = 12345;
    RoundTrip(packet, [](AuctionPlaceBid const& a, AuctionPlaceBid const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.Price == b.Price);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionListBidderItems round-trips with and without refresh ids", "[packets]")
{
    AuctionListBidderItems packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.Offset = 0;
    RoundTrip(packet, [](AuctionListBidderItems const& a, AuctionListBidderItems const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.Offset == b.Offset);
        CHECK(a.AuctionIdsToRefresh.empty());
        CHECK(b.AuctionIdsToRefresh.empty());
    });
    RejectsEveryTruncation(packet);

    packet.AuctionIdsToRefresh = { 7, 42, 100 };
    RoundTrip(packet, [](AuctionListBidderItems const& a, AuctionListBidderItems const& b)
    {
        CHECK(a.AuctionIdsToRefresh == b.AuctionIdsToRefresh);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionListItems round-trips with no filter and with class/subclass filters", "[packets]")
{
    AuctionListItems packet;
    packet.AuctioneerGuid = { 1, 0 };
    RoundTrip(packet, [](AuctionListItems const& a, AuctionListItems const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.ListFrom == b.ListFrom);
        CHECK(a.SearchedName == b.SearchedName);
        CHECK(a.Quality == b.Quality);
        CHECK(b.ClassFilters.empty());
    });
    RejectsEveryTruncation(packet);

    packet.SearchedName = "Cloth";
    packet.LevelMin = 1;
    packet.LevelMax = 60;
    packet.OnlyUsable = true;
    packet.ClassFilters = { { 4, { { 1, 0xFFFFFFFF } } }, { 2, {} } };
    RoundTrip(packet, [](AuctionListItems const& a, AuctionListItems const& b)
    {
        CHECK(a.SearchedName == b.SearchedName);
        CHECK(a.LevelMin == b.LevelMin);
        CHECK(a.LevelMax == b.LevelMax);
        CHECK(b.OnlyUsable);
        REQUIRE(b.ClassFilters.size() == 2);
        CHECK(b.ClassFilters[0].ItemClass == 4);
        REQUIRE(b.ClassFilters[0].SubClassFilters.size() == 1);
        CHECK(b.ClassFilters[0].SubClassFilters[0].ItemSubclass == 1);
        CHECK(b.ClassFilters[0].SubClassFilters[0].InvTypeMask == 0xFFFFFFFF);
        CHECK(b.ClassFilters[1].SubClassFilters.empty());
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionListItems reads a real 1.15 client search", "[packets]")
{
    // name "abc", usable only, Weapons / One-Handed Axes, 7 sort columns
    std::string const hex =
        "03a79831c083080420000000000000ffffffff070100000001000180616263300200000008"
        "ffffffff00000000000000000e0000000100000105000600090108000300";
    ByteBuffer data;
    for (size_t i = 0; i < hex.size(); i += 2)
        data << uint8(std::stoul(hex.substr(i, 2), nullptr, 16));
    AuctionListItems packet;
    REQUIRE(packet.Read(data));
    CHECK(data.rpos() == data.size());
    CHECK(packet.SearchedName == "abc");
    CHECK(packet.OnlyUsable);
    CHECK(packet.Quality == 0xFFFFFFFF);
    REQUIRE(packet.ClassFilters.size() == 1);
    CHECK(packet.ClassFilters[0].ItemClass == 2);
    REQUIRE(packet.ClassFilters[0].SubClassFilters.size() == 1);
    CHECK(packet.ClassFilters[0].SubClassFilters[0].ItemSubclass == 0);
}

TEST_CASE("AuctionListItems reads a tainted 1.15 client search", "[packets]")
{
    // empty name and no filters, started from /run so the addon taint info is attached
    std::string const hex =
        "03a79831c083080420000000000000ffffffff070000000001800000064008756e6b6e6f776e206f72206578706c6f6974206164646f6e00"
        "0e0000000100000105000600090108000300";
    ByteBuffer data;
    for (size_t i = 0; i < hex.size(); i += 2)
        data << uint8(std::stoul(hex.substr(i, 2), nullptr, 16));
    AuctionListItems packet;
    REQUIRE(packet.Read(data));
    CHECK(data.rpos() == data.size());
    CHECK(packet.SearchedName.empty());
    CHECK(packet.ClassFilters.empty());
}

TEST_CASE("AuctionListPendingSales round-trips", "[packets]")
{
    RoundTrip(AuctionListPendingSales(), [](AuctionListPendingSales const&, AuctionListPendingSales const&) {});
    RejectsEveryTruncation(AuctionListPendingSales());
}

TEST_CASE("AuctionHelloResponse round-trips", "[packets]")
{
    AuctionHelloResponse packet;
    packet.AuctioneerGuid = { 1, 0 };
    packet.HouseId = 7;
    RoundTrip(packet, [](AuctionHelloResponse const& a, AuctionHelloResponse const& b)
    {
        CHECK(a.AuctioneerGuid == b.AuctioneerGuid);
        CHECK(a.HouseId == b.HouseId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionCommandResult round-trips ok and higher-bid", "[packets]")
{
    AuctionCommandResult packet;
    packet.AuctionId = 42;
    packet.Action = 2; // AUCTION_BID_PLACED
    packet.ErrorCode = 0; // AUCTION_OK
    packet.OutBid = 10;
    RoundTrip(packet, [](AuctionCommandResult const& a, AuctionCommandResult const& b)
    {
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.Action == b.Action);
        CHECK(a.ErrorCode == b.ErrorCode);
        CHECK(a.OutBid == b.OutBid);
    });
    RejectsEveryTruncation(packet);

    AuctionCommandResult higherBid;
    higherBid.AuctionId = 42;
    higherBid.Action = 2;
    higherBid.ErrorCode = 5; // AUCTION_ERR_HIGHER_BID
    higherBid.NewBidderGuid = { 55, 0 };
    higherBid.NewBid = 2000;
    higherBid.OutBid = 100;
    RoundTrip(higherBid, [](AuctionCommandResult const& a, AuctionCommandResult const& b)
    {
        CHECK(a.NewBidderGuid == b.NewBidderGuid);
        CHECK(a.NewBid == b.NewBid);
        CHECK(a.OutBid == b.OutBid);
    });
    RejectsEveryTruncation(higherBid);
}

TEST_CASE("AuctionItem round-trips with no bidder, a bidder, and an enchant", "[packets]")
{
    AuctionItem item;
    item.Item.ItemId = 2589; // Linen Cloth
    item.Count = 5;
    item.AuctionId = 42;
    item.Owner = { 7, 0 };
    item.DurationLeftMs = 3600000;
    item.MinBid = 100;
    item.BuyoutPrice = 500;
    item.ItemGuid = { 99, 0 };
    item.OwnerAccountGuid = { 12345, 0 };
    item.EndTime = 1000000;

    AuctionListItemsResult packet;
    packet.Items.push_back(item);
    packet.TotalCount = 1;
    RoundTrip(packet, [](AuctionListItemsResult const& a, AuctionListItemsResult const& b)
    {
        REQUIRE(a.Items.size() == b.Items.size());
        CHECK(a.Items[0].Item.ItemId == b.Items[0].Item.ItemId);
        CHECK(a.Items[0].Count == b.Items[0].Count);
        CHECK(a.Items[0].AuctionId == b.Items[0].AuctionId);
        CHECK(a.Items[0].Owner == b.Items[0].Owner);
        CHECK(a.Items[0].MinBid == b.Items[0].MinBid);
        CHECK(a.Items[0].BuyoutPrice == b.Items[0].BuyoutPrice);
        CHECK(a.Items[0].ItemGuid == b.Items[0].ItemGuid);
        CHECK(a.Items[0].OwnerAccountGuid == b.Items[0].OwnerAccountGuid);
        CHECK(a.Items[0].EndTime == b.Items[0].EndTime);
        CHECK(a.Items[0].Bidder == b.Items[0].Bidder);
        CHECK(a.Items[0].BidAmount == b.Items[0].BidAmount);
        CHECK(a.TotalCount == b.TotalCount);
    });
    RejectsEveryTruncation(packet);

    item.Bidder = { 55, 0 };
    item.BidAmount = 150;
    item.MinIncrement = 5;
    item.PermanentEnchantId = 29;

    AuctionListItemsResult withBidder;
    withBidder.Items.push_back(item);
    withBidder.TotalCount = 1;
    RoundTrip(withBidder, [](AuctionListItemsResult const& a, AuctionListItemsResult const& b)
    {
        REQUIRE(a.Items.size() == b.Items.size());
        CHECK(a.Items[0].Bidder == b.Items[0].Bidder);
        CHECK(a.Items[0].BidAmount == b.Items[0].BidAmount);
        CHECK(a.Items[0].MinIncrement == b.Items[0].MinIncrement);
        CHECK(a.Items[0].PermanentEnchantId == b.Items[0].PermanentEnchantId);
    });
    RejectsEveryTruncation(withBidder);
}

TEST_CASE("AuctionListBiddedItemsResult round-trips empty", "[packets]")
{
    AuctionListBiddedItemsResult packet;
    packet.TotalCount = 0;
    RoundTrip(packet, [](AuctionListBiddedItemsResult const& a, AuctionListBiddedItemsResult const& b)
    {
        CHECK(a.Items.empty());
        CHECK(b.Items.empty());
        CHECK(a.TotalCount == b.TotalCount);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionListOwnedItemsResult round-trips one posting", "[packets]")
{
    AuctionItem item;
    item.Item.ItemId = 2589;
    item.Count = 1;
    item.AuctionId = 7;
    item.Owner = { 7, 0 };
    item.MinBid = 100;
    item.ItemGuid = { 88, 0 };
    item.OwnerAccountGuid = { 1, 0 };
    item.EndTime = 5000;

    AuctionListOwnedItemsResult packet;
    packet.Items.push_back(item);
    packet.TotalCount = 1;
    RoundTrip(packet, [](AuctionListOwnedItemsResult const& a, AuctionListOwnedItemsResult const& b)
    {
        REQUIRE(a.Items.size() == b.Items.size());
        CHECK(a.Items[0].AuctionId == b.Items[0].AuctionId);
        CHECK(a.TotalCount == b.TotalCount);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionOwnerBidNotification round-trips", "[packets]")
{
    AuctionOwnerBidNotification packet;
    packet.AuctionId = 42;
    packet.BidAmount = 150;
    packet.Item.ItemId = 2589;
    packet.MinIncrement = 5;
    packet.Bidder = { 55, 0 };
    RoundTrip(packet, [](AuctionOwnerBidNotification const& a, AuctionOwnerBidNotification const& b)
    {
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.BidAmount == b.BidAmount);
        CHECK(a.Item.ItemId == b.Item.ItemId);
        CHECK(a.MinIncrement == b.MinIncrement);
        CHECK(a.Bidder == b.Bidder);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionClosedNotification round-trips sold and unsold", "[packets]")
{
    AuctionClosedNotification packet;
    packet.AuctionId = 42;
    packet.BidAmount = 500;
    packet.Item.ItemId = 2589;
    packet.Sold = true;
    RoundTrip(packet, [](AuctionClosedNotification const& a, AuctionClosedNotification const& b)
    {
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.BidAmount == b.BidAmount);
        CHECK(a.Item.ItemId == b.Item.ItemId);
        CHECK(a.Sold == b.Sold);
    });
    RejectsEveryTruncation(packet);

    packet.Sold = false;
    RoundTrip(packet, [](AuctionClosedNotification const& a, AuctionClosedNotification const& b) { CHECK(a.Sold == b.Sold); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionWonNotification round-trips", "[packets]")
{
    AuctionWonNotification packet;
    packet.AuctionId = 42;
    packet.BidderGuid = { 55, 0 };
    packet.Item.ItemId = 2589;
    RoundTrip(packet, [](AuctionWonNotification const& a, AuctionWonNotification const& b)
    {
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.BidderGuid == b.BidderGuid);
        CHECK(a.Item.ItemId == b.Item.ItemId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuctionOutbidNotification round-trips", "[packets]")
{
    AuctionOutbidNotification packet;
    packet.AuctionId = 42;
    packet.BidderGuid = { 55, 0 };
    packet.Item.ItemId = 2589;
    packet.BidAmount = 150;
    packet.MinIncrement = 5;
    RoundTrip(packet, [](AuctionOutbidNotification const& a, AuctionOutbidNotification const& b)
    {
        CHECK(a.AuctionId == b.AuctionId);
        CHECK(a.BidderGuid == b.BidderGuid);
        CHECK(a.Item.ItemId == b.Item.ItemId);
        CHECK(a.BidAmount == b.BidAmount);
        CHECK(a.MinIncrement == b.MinIncrement);
    });
    RejectsEveryTruncation(packet);
}
