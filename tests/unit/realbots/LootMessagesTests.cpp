// Round trips for the modern loot wire.
#include "Packets/LootMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
LootItemData SomeItem()
{
    LootItemData item;
    item.UIType = 1;
    item.CanTradeToTapList = true;
    item.Item.ItemId = 2589;
    item.Item.RandomPropertiesSeed = 3;
    item.Item.RandomPropertiesId = -4;
    item.Quantity = 6;
    item.LootListId = 2;
    return item;
}

void CheckItem(LootItemData const& a, LootItemData const& b)
{
    CHECK(a.UIType == b.UIType);
    CHECK(a.CanTradeToTapList == b.CanTradeToTapList);
    CHECK(a.Item.ItemId == b.Item.ItemId);
    CHECK(a.Item.RandomPropertiesSeed == b.Item.RandomPropertiesSeed);
    CHECK(a.Item.RandomPropertiesId == b.Item.RandomPropertiesId);
    CHECK(a.Quantity == b.Quantity);
    CHECK(a.LootListId == b.LootListId);
}

ClassicGuid Corpse() { return { 41, 0x2000000000000000ULL }; }
}

TEST_CASE("LootUnit, LootRelease and LootMoney round-trip", "[packets]")
{
    LootUnit unit;
    unit.Unit = Corpse();
    RoundTrip(unit, [](LootUnit const& a, LootUnit const& b) { CHECK(a.Unit == b.Unit); });
    RejectsEveryTruncation(unit);

    LootRelease release;
    release.Object = Corpse();
    RoundTrip(release, [](LootRelease const& a, LootRelease const& b) { CHECK(a.Object == b.Object); });
    RejectsEveryTruncation(release);

    LootMoney money;
    money.IsSoftInteract = true;
    RoundTrip(money, [](LootMoney const& a, LootMoney const& b) { CHECK(a.IsSoftInteract == b.IsSoftInteract); });
    RejectsEveryTruncation(money);
}

TEST_CASE("LootItemRequest and MasterLootItem round-trip and cap their entries", "[packets]")
{
    LootItemRequest request;
    request.Loot = { { Corpse(), 1 }, { Corpse(), 3 } };
    request.IsSoftInteract = true;
    RoundTrip(request, [](LootItemRequest const& a, LootItemRequest const& b)
    {
        REQUIRE(a.Loot.size() == b.Loot.size());
        for (size_t i = 0; i < a.Loot.size(); ++i)
        {
            CHECK(a.Loot[i].Object == b.Loot[i].Object);
            CHECK(a.Loot[i].LootListId == b.Loot[i].LootListId);
        }
        CHECK(b.IsSoftInteract);
    });
    RejectsEveryTruncation(request);

    MasterLootItem give;
    give.Loot = { { Corpse(), 2 } };
    give.Target = { 9, 0x0800000000000000ULL };
    RoundTrip(give, [](MasterLootItem const& a, MasterLootItem const& b)
    {
        CHECK(a.Target == b.Target);
        REQUIRE(b.Loot.size() == 1);
        CHECK(a.Loot[0].LootListId == b.Loot[0].LootListId);
    });
    RejectsEveryTruncation(give);

    ByteBuffer tooMany;
    tooMany << uint32(CLASSIC_MAX_LOOT_REQUESTS + 1);
    LootItemRequest in;
    CHECK_FALSE(in.Read(tooMany));
}

TEST_CASE("LootRoll round-trips", "[packets]")
{
    LootRoll roll;
    roll.LootObj = Corpse();
    roll.LootListId = 4;
    roll.RollType = 2;
    RoundTrip(roll, [](LootRoll const& a, LootRoll const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.LootListId == b.LootListId);
        CHECK(a.RollType == b.RollType);
    });
    RejectsEveryTruncation(roll);
}

TEST_CASE("LootResponse round-trips an open window and a refusal", "[packets]")
{
    LootResponse open;
    open.Owner = Corpse();
    open.LootObj = Corpse();
    open.AcquireReason = 1;
    open.LootMethod = 3;
    open.Threshold = 2;
    open.Coins = 123;
    open.Acquired = true;
    open.Items = { SomeItem(), SomeItem() };
    open.Items[1].LootListId = 3;
    open.Items[1].UIType = 0;
    RoundTrip(open, [](LootResponse const& a, LootResponse const& b)
    {
        CHECK(a.Owner == b.Owner);
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.FailureReason == b.FailureReason);
        CHECK(a.AcquireReason == b.AcquireReason);
        CHECK(a.LootMethod == b.LootMethod);
        CHECK(a.Threshold == b.Threshold);
        CHECK(a.Coins == b.Coins);
        CHECK(b.Acquired);
        REQUIRE(a.Items.size() == b.Items.size());
        for (size_t i = 0; i < a.Items.size(); ++i)
            CheckItem(a.Items[i], b.Items[i]);
    });
    RejectsEveryTruncation(open);

    LootResponse refused;
    refused.Owner = Corpse();
    refused.LootObj = Corpse();
    refused.FailureReason = 0;
    RoundTrip(refused, [](LootResponse const& a, LootResponse const& b)
    {
        CHECK(b.FailureReason == 0);
        CHECK_FALSE(b.Acquired);
        CHECK(b.Items.empty());
    });
    RejectsEveryTruncation(refused);
}

TEST_CASE("Loot removal, release, coin and money messages round-trip", "[packets]")
{
    LootRemoved removed;
    removed.Owner = Corpse();
    removed.LootObj = Corpse();
    removed.LootListId = 5;
    RoundTrip(removed, [](LootRemoved const& a, LootRemoved const& b)
    {
        CHECK(a.Owner == b.Owner);
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.LootListId == b.LootListId);
    });
    RejectsEveryTruncation(removed);

    LootReleaseResponse release;
    release.LootObj = Corpse();
    release.Owner = Corpse();
    RoundTrip(release, [](LootReleaseResponse const& a, LootReleaseResponse const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.Owner == b.Owner);
    });
    RejectsEveryTruncation(release);

    CoinRemoved coin;
    coin.LootObj = Corpse();
    RoundTrip(coin, [](CoinRemoved const& a, CoinRemoved const& b) { CHECK(a.LootObj == b.LootObj); });
    RejectsEveryTruncation(coin);

    LootMoneyNotify notify;
    notify.Money = 4'000'000'000ULL;
    notify.MoneyMod = 7;
    notify.SoleLooter = true;
    RoundTrip(notify, [](LootMoneyNotify const& a, LootMoneyNotify const& b)
    {
        CHECK(a.Money == b.Money);
        CHECK(a.MoneyMod == b.MoneyMod);
        CHECK(b.SoleLooter);
    });
    RejectsEveryTruncation(notify);
}

TEST_CASE("Roll messages round-trip", "[packets]")
{
    LootRollStart start;
    start.LootObj = Corpse();
    start.MapId = 0;
    start.RollTime = 60000;
    start.Method = 3;
    start.Item = SomeItem();
    RoundTrip(start, [](LootRollStart const& a, LootRollStart const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.MapId == b.MapId);
        CHECK(a.RollTime == b.RollTime);
        CHECK(a.ValidRolls == b.ValidRolls);
        CHECK(a.Method == b.Method);
        CheckItem(a.Item, b.Item);
    });
    RejectsEveryTruncation(start);

    LootRollBroadcast broadcast;
    broadcast.LootObj = Corpse();
    broadcast.Player = { 9, 0x0800000000000000ULL };
    broadcast.Roll = -1;
    broadcast.RollType = 2;
    broadcast.Item = SomeItem();
    broadcast.Autopassed = true;
    RoundTrip(broadcast, [](LootRollBroadcast const& a, LootRollBroadcast const& b)
    {
        CHECK(a.Player == b.Player);
        CHECK(a.Roll == b.Roll);
        CHECK(a.RollType == b.RollType);
        CheckItem(a.Item, b.Item);
        CHECK(b.Autopassed);
    });
    RejectsEveryTruncation(broadcast);

    LootRollWon won;
    won.LootObj = Corpse();
    won.Winner = { 9, 0x0800000000000000ULL };
    won.Roll = 87;
    won.RollType = 1;
    won.Item = SomeItem();
    RoundTrip(won, [](LootRollWon const& a, LootRollWon const& b)
    {
        CHECK(a.Winner == b.Winner);
        CHECK(a.Roll == b.Roll);
        CHECK(a.RollType == b.RollType);
        CheckItem(a.Item, b.Item);
        CHECK(b.MainSpec);
    });
    RejectsEveryTruncation(won);

    LootAllPassed passed;
    passed.LootObj = Corpse();
    passed.Item = SomeItem();
    RoundTrip(passed, [](LootAllPassed const& a, LootAllPassed const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CheckItem(a.Item, b.Item);
    });
    RejectsEveryTruncation(passed);

    LootRollsComplete complete;
    complete.LootObj = Corpse();
    complete.LootListId = 2;
    RoundTrip(complete, [](LootRollsComplete const& a, LootRollsComplete const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.LootListId == b.LootListId);
    });
    RejectsEveryTruncation(complete);
}
