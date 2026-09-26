// Quest giver interaction: status, details, accept and turn-in (at a different NPC than the giver).
#include "Packets/NpcMessages.h"
#include "Packets/QuestMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("quest: a GM bot takes a quest from one NPC and turns it in at another", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        // "A Threat Within": Deputy Willem sends the player to talk to Marshal McBride, both in
        // Northshire Abbey. No item or kill objective (Objectives text is just "Speak with Marshal
        // McBride"), so it needs no GM setup once accepted -- a genuinely simple accept/turn-in pair.
        constexpr uint32 QuestId = 783;
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        // A pooled character may already have taken this quest in an earlier run: start clean.
        SayCommand(gm, ".quest remove 783");
        co_await TestDelay(500);

        std::vector<ClassicGuid> const beforeGiver = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 823"); // Deputy Willem
        std::optional<ClassicGuid> giver;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            giver = FindNewUnit(gm, beforeGiver);
            return giver.has_value();
        }, 10000));
        co_await TestDelay(500);

        QuestgiverStatusQuery statusQuery;
        statusQuery.Guid = *giver;
        gm.Send(statusQuery);
        auto status = co_await gm.Packet<QuestGiverStatus>(10000, [&](QuestGiverStatus const& p) { return p.NpcGuid == *giver; });
        REQUIRE(status);
        CHECK(status->Status != 0);

        QuestgiverQueryQuest query;
        query.Guid = *giver;
        query.QuestId = QuestId;
        gm.Send(query);
        auto details = co_await gm.Packet<QuestGiverQuestDetails>(10000, [&](QuestGiverQuestDetails const& p) { return p.NpcGuid == *giver; });
        REQUIRE(details);
        CHECK(details->QuestId == QuestId);
        CHECK(details->Title == "A Threat Within");
        CHECK(details->Objectives.empty());

        QuestgiverAcceptQuest accept;
        accept.Guid = *giver;
        accept.QuestId = QuestId;
        gm.Send(accept);
        REQUIRE(co_await gm.WaitReceived<GossipComplete>(10000, [](GossipComplete const&) { return true; }));

        // Belt-and-braces: this quest has no real objective to satisfy, but make sure regardless.
        SayCommand(gm, ".quest complete 783");
        co_await TestDelay(500);

        std::vector<ClassicGuid> const beforeEnder = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 197"); // Marshal McBride, the turn-in target
        std::optional<ClassicGuid> ender;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            ender = FindNewUnit(gm, beforeEnder);
            return ender.has_value();
        }, 10000));
        co_await TestDelay(500);

        QuestgiverCompleteQuest complete;
        complete.Guid = *ender;
        complete.QuestId = QuestId;
        gm.Send(complete);
        auto offer = co_await gm.Packet<QuestGiverOfferReward>(10000, [&](QuestGiverOfferReward const& p) { return p.NpcGuid == *ender; });
        REQUIRE(offer);
        CHECK(offer->QuestId == QuestId);
        CHECK(offer->Title == "A Threat Within");

        QuestgiverChooseReward choose;
        choose.Guid = *ender;
        choose.QuestId = QuestId;
        gm.Send(choose);
        auto complete2 = co_await gm.Packet<QuestGiverQuestComplete>(10000, [&](QuestGiverQuestComplete const& p) { return p.QuestId == QuestId; });
        REQUIRE(complete2);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("quest: the quest log lookup answers with the record or a denial", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        QueryQuestInfo known;
        known.QuestId = 783;
        bot.Send(known);
        auto info = co_await bot.Packet<QueryQuestInfoResponse>(10000, [](QueryQuestInfoResponse const& p) { return p.QuestId == 783; });
        REQUIRE(info);
        REQUIRE(info->Allow);
        CHECK(info->LogTitle == "A Threat Within");
        CHECK_FALSE(info->QuestDescription.empty());

        QueryQuestInfo unknown;
        unknown.QuestId = 99999999;
        bot.Send(unknown);
        auto denied = co_await bot.Packet<QueryQuestInfoResponse>(10000, [](QueryQuestInfoResponse const& p) { return p.QuestId == 99999999; });
        REQUIRE(denied);
        CHECK_FALSE(denied->Allow);
    });
}

TEST_CASE("quest: the area status query lists a nearby giver and a repeat accept is refused", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        constexpr uint32 QuestId = 783;
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        SayCommand(gm, ".quest remove 783");
        co_await TestDelay(500);

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 823"); // Deputy Willem
        std::optional<ClassicGuid> giver;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            giver = FindNewUnit(gm, before);
            return giver.has_value();
        }, 10000));
        co_await TestDelay(500);

        gm.Send(QuestGiverStatusMultipleQuery());
        auto multiple = co_await gm.Packet<QuestGiverStatusMultiple>(10000, [&](QuestGiverStatusMultiple const& p)
        {
            for (QuestGiverStatusEntry const& entry : p.Givers)
                if (entry.Guid == *giver)
                    return true;
            return false;
        });
        REQUIRE(multiple);
        for (QuestGiverStatusEntry const& entry : multiple->Givers)
            if (entry.Guid == *giver)
                CHECK(entry.Status != 0);

        QuestgiverAcceptQuest accept;
        accept.Guid = *giver;
        accept.QuestId = QuestId;
        gm.Send(accept);
        REQUIRE(co_await gm.WaitReceived<GossipComplete>(10000, [](GossipComplete const&) { return true; }));

        gm.Send(accept);
        auto invalid = co_await gm.Packet<QuestGiverInvalidQuest>(10000);
        REQUIRE(invalid);
        CHECK(invalid->Reason == 13);

        SayCommand(gm, ".quest remove 783");
        SayCommand(gm, ".npc despawn");
    });
}
