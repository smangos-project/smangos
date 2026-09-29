// Round trips for the modern inspect-window wire.
#include "Packets/InspectMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Inspect round-trips", "[packets]")
{
    Inspect packet;
    packet.TargetGuid = { 0x1234, 0 };
    RoundTrip(packet, [](Inspect const& a, Inspect const& b) { CHECK(a.TargetGuid == b.TargetGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("InspectPvp round-trips", "[packets]")
{
    InspectPvp packet;
    packet.TargetGuid = { 0x4321, 0 };
    RoundTrip(packet, [](InspectPvp const& a, InspectPvp const& b) { CHECK(a.TargetGuid == b.TargetGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("QueryInspectAchievements round-trips", "[packets]")
{
    QueryInspectAchievements packet;
    packet.TargetGuid = { 0x5555, 0 };
    RoundTrip(packet, [](QueryInspectAchievements const& a, QueryInspectAchievements const& b) { CHECK(a.TargetGuid == b.TargetGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("InspectResult round-trips with no items and with an enchanted item", "[packets]")
{
    InspectResult bare;
    bare.DisplayInfo.PlayerGuid = { 0x6666, 0 };
    bare.DisplayInfo.Name = "Realbots";
    bare.DisplayInfo.Gender = 0;
    bare.DisplayInfo.Race = 1;
    bare.DisplayInfo.ClassId = 4;
    bare.ItemLevel = 0;
    bare.LifetimeHK = 12;
    bare.HonorLevel = 0;
    bare.TodayHK = 3;
    bare.YesterdayHK = 1;
    bare.LifetimeMaxRank = 2;
    RoundTrip(bare, [](InspectResult const& a, InspectResult const& b)
    {
        CHECK(a.DisplayInfo.PlayerGuid == b.DisplayInfo.PlayerGuid);
        CHECK(a.DisplayInfo.Name == b.DisplayInfo.Name);
        CHECK(a.DisplayInfo.Gender == b.DisplayInfo.Gender);
        CHECK(a.DisplayInfo.Race == b.DisplayInfo.Race);
        CHECK(a.DisplayInfo.ClassId == b.DisplayInfo.ClassId);
        CHECK(a.DisplayInfo.Items.size() == b.DisplayInfo.Items.size());
        CHECK(a.HasGuildData == b.HasGuildData);
        CHECK(a.ItemLevel == b.ItemLevel);
        CHECK(a.LifetimeHK == b.LifetimeHK);
        CHECK(a.HonorLevel == b.HonorLevel);
        CHECK(a.TodayHK == b.TodayHK);
        CHECK(a.YesterdayHK == b.YesterdayHK);
        CHECK(a.LifetimeMaxRank == b.LifetimeMaxRank);
    });
    RejectsEveryTruncation(bare);

    InspectItemEntry item;
    item.Index = 7;
    item.Item.ItemId = 25;
    item.Item.RandomPropertiesSeed = 0;
    item.Item.RandomPropertiesId = 0;
    item.CreatorGuid = { 0x7777, 0 };
    item.Enchants.push_back({ 3241, 0 });
    bare.DisplayInfo.Items.push_back(item);
    bare.HasGuildData = true;
    bare.GuildData.GuildGuid = { 0x8888, 0 };
    bare.GuildData.NumMembers = 5;
    RoundTrip(bare, [](InspectResult const& a, InspectResult const& b)
    {
        REQUIRE(a.DisplayInfo.Items.size() == b.DisplayInfo.Items.size());
        CHECK(a.DisplayInfo.Items[0].Index == b.DisplayInfo.Items[0].Index);
        CHECK(a.DisplayInfo.Items[0].Item.ItemId == b.DisplayInfo.Items[0].Item.ItemId);
        CHECK(a.DisplayInfo.Items[0].CreatorGuid == b.DisplayInfo.Items[0].CreatorGuid);
        REQUIRE(a.DisplayInfo.Items[0].Enchants.size() == b.DisplayInfo.Items[0].Enchants.size());
        CHECK(a.DisplayInfo.Items[0].Enchants[0].Id == b.DisplayInfo.Items[0].Enchants[0].Id);
        CHECK(a.HasGuildData == b.HasGuildData);
        CHECK(a.GuildData.GuildGuid == b.GuildData.GuildGuid);
        CHECK(a.GuildData.NumMembers == b.GuildData.NumMembers);
    });
    RejectsEveryTruncation(bare);
}

TEST_CASE("InspectResult round-trips customization choices", "[packets]")
{
    InspectResult result;
    result.DisplayInfo.PlayerGuid = { 0x6666, 0 };
    result.DisplayInfo.Name = "Looks";
    result.DisplayInfo.Customizations = { { 1, 10 }, { 2, 20 }, { 5, 50 } };
    RoundTrip(result, [](InspectResult const& a, InspectResult const& b)
    {
        CHECK(a.DisplayInfo.Customizations == b.DisplayInfo.Customizations);
    });
    RejectsEveryTruncation(result);
}

TEST_CASE("InspectPvpResult round-trips with empty and populated tail arrays", "[packets]")
{
    InspectPvpResult empty;
    empty.PlayerGuid = { 0x9999, 0 };
    RoundTrip(empty, [](InspectPvpResult const& a, InspectPvpResult const& b)
    {
        CHECK(a.PlayerGuid == b.PlayerGuid);
        CHECK(a.Brackets.size() == b.Brackets.size());
        CHECK(a.ArenaTeams.size() == b.ArenaTeams.size());
    });
    RejectsEveryTruncation(empty);

    InspectPvpResult populated;
    populated.PlayerGuid = { 0xaaaa, 0 };
    populated.ArenaTeams.resize(3);
    populated.ArenaTeams[0].TeamGuid = { 0xbbbb, 0 };
    populated.ArenaTeams[0].TeamRating = 1500;
    RoundTrip(populated, [](InspectPvpResult const& a, InspectPvpResult const& b)
    {
        REQUIRE(a.ArenaTeams.size() == b.ArenaTeams.size());
        CHECK(a.ArenaTeams[0].TeamGuid == b.ArenaTeams[0].TeamGuid);
        CHECK(a.ArenaTeams[0].TeamRating == b.ArenaTeams[0].TeamRating);
    });
    RejectsEveryTruncation(populated);
}
