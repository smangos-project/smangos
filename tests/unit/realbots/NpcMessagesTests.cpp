// Round trips for the modern NPC gossip wire.
#include "Packets/NpcMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("GossipHello and GossipSelectOption round-trip", "[packets]")
{
    GossipHello hello;
    hello.Unit = { 5, 0x2000000000000000ULL };
    RoundTrip(hello, [](GossipHello const& a, GossipHello const& b) { CHECK(a.Unit == b.Unit); });
    RejectsEveryTruncation(hello);

    GossipSelectOption select;
    select.GossipUnit = { 5, 0x2000000000000000ULL };
    select.GossipId = 5111;
    select.GossipOptionId = 3;
    select.PromotionCode = "secret";
    RoundTrip(select, [](GossipSelectOption const& a, GossipSelectOption const& b)
    {
        CHECK(a.GossipUnit == b.GossipUnit);
        CHECK(a.GossipId == b.GossipId);
        CHECK(a.GossipOptionId == b.GossipOptionId);
        CHECK(a.PromotionCode == b.PromotionCode);
    });
    RejectsEveryTruncation(select);
}

TEST_CASE("GossipMessage round-trips options, a text id and quests", "[packets]")
{
    GossipMessage message;
    message.GossipGuid = { 5, 0x2000000000000000ULL };
    message.GossipId = 5111;
    message.TextId = 4700;

    GossipOptionData& home = message.Options.emplace_back();
    home.GossipOptionId = 0;
    home.OptionNpc = 5;
    home.Text = "Make this inn your home.";
    home.Confirm = "Make this inn your home?";
    home.OrderIndex = 0;

    GossipOptionData& coded = message.Options.emplace_back();
    coded.GossipOptionId = 1;
    coded.OptionFlags = 1;
    coded.OptionCost = 300;
    coded.Text = "Enter the code";
    coded.OrderIndex = 1;

    GossipQuestEntry& quest = message.Quests.emplace_back();
    quest.QuestId = 783;
    quest.QuestType = 2;
    quest.QuestLevel = 1;
    quest.QuestFlags[0] = 8;
    quest.Repeatable = true;
    quest.Title = "A Threat Within";

    RoundTrip(message, [](GossipMessage const& a, GossipMessage const& b)
    {
        CHECK(a.GossipGuid == b.GossipGuid);
        CHECK(a.GossipId == b.GossipId);
        CHECK(a.TextId == b.TextId);
        REQUIRE(a.Options.size() == b.Options.size());
        for (size_t i = 0; i < a.Options.size(); ++i)
        {
            CHECK(a.Options[i].GossipOptionId == b.Options[i].GossipOptionId);
            CHECK(a.Options[i].OptionNpc == b.Options[i].OptionNpc);
            CHECK(a.Options[i].OptionFlags == b.Options[i].OptionFlags);
            CHECK(a.Options[i].OptionCost == b.Options[i].OptionCost);
            CHECK(a.Options[i].OrderIndex == b.Options[i].OrderIndex);
            CHECK(a.Options[i].Text == b.Options[i].Text);
            CHECK(a.Options[i].Confirm == b.Options[i].Confirm);
        }
        REQUIRE(a.Quests.size() == b.Quests.size());
        CHECK(a.Quests[0].QuestId == b.Quests[0].QuestId);
        CHECK(a.Quests[0].QuestType == b.Quests[0].QuestType);
        CHECK(a.Quests[0].QuestLevel == b.Quests[0].QuestLevel);
        CHECK(a.Quests[0].QuestFlags[0] == b.Quests[0].QuestFlags[0]);
        CHECK(b.Quests[0].Repeatable);
        CHECK(a.Quests[0].Title == b.Quests[0].Title);
    });
    RejectsEveryTruncation(message);

    GossipMessage bare;
    bare.GossipGuid = { 6, 0x2000000000000000ULL };
    RoundTrip(bare, [](GossipMessage const& a, GossipMessage const& b)
    {
        CHECK_FALSE(b.TextId.has_value());
        CHECK(b.Options.empty());
        CHECK(b.Quests.empty());
    });
    RejectsEveryTruncation(bare);
}

TEST_CASE("GossipComplete and GossipPoi round-trip", "[packets]")
{
    GossipComplete complete;
    complete.SuppressSound = true;
    RoundTrip(complete, [](GossipComplete const&, GossipComplete const& b) { CHECK(b.SuppressSound); });
    RejectsEveryTruncation(complete);

    GossipPoi poi;
    poi.Id = 12;
    poi.Flags = 99;
    poi.X = -8913.0f;
    poi.Y = -134.0f;
    poi.Z = 81.0f;
    poi.Icon = 7;
    poi.Importance = 3;
    poi.Name = "Stormwind Keep";
    RoundTrip(poi, [](GossipPoi const& a, GossipPoi const& b)
    {
        CHECK(a.Id == b.Id);
        CHECK(a.Flags == b.Flags);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.Icon == b.Icon);
        CHECK(a.Importance == b.Importance);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(poi);
}
