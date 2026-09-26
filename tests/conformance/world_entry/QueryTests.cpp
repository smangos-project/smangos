// The static-data queries a client makes for everything it sees: names, templates, text, time.
#include "Packets/QueryMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("query: player names come back for a known bot and mark an unknown guid", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot asker;
        TestBot other;
        REQUIRE(co_await asker.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await other.Stage(BotStage::InWorld, 30000));

        ClassicGuid unknown = other.GetCharacterGuid();
        unknown.Low += 1000000;

        QueryPlayerNames request;
        request.Players = { other.GetCharacterGuid(), unknown };
        asker.Send(request);

        auto response = co_await asker.Packet<QueryPlayerNamesResponse>(10000);
        REQUIRE(response);
        REQUIRE(response->Players.size() == 2);

        PlayerNameLookup const& known = response->Players[0];
        CHECK(known.Result == 0);
        CHECK(known.Player == other.GetCharacterGuid());
        REQUIRE(known.Data.has_value());
        CHECK(known.Data->Name == other.GetCharacterName());
        CHECK(known.Data->PlayerGuid == other.GetCharacterGuid());
        CHECK(known.Data->Race != 0);
        CHECK(known.Data->ClassId != 0);
        CHECK(known.Data->Level >= 1);

        PlayerNameLookup const& missing = response->Players[1];
        CHECK(missing.Result != 0);
        CHECK(missing.Player == unknown);
        CHECK_FALSE(missing.Data.has_value());
    });
}

TEST_CASE("query: a creature template comes back, and a missing entry is refused", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        QueryCreature request;
        request.Entry = 6; // Kobold Vermin
        bot.Send(request);
        auto found = co_await bot.Packet<QueryCreatureResponse>(10000, [](QueryCreatureResponse const& p) { return p.Entry == 6; });
        REQUIRE(found);
        CHECK(found->Allow);
        CHECK(found->Name == "Kobold Vermin");
        CHECK(found->Title.empty());
        REQUIRE_FALSE(found->Displays.empty());
        CHECK(found->Displays.front().DisplayId != 0);

        request.Entry = 197; // Marshal McBride
        bot.Send(request);
        auto named = co_await bot.Packet<QueryCreatureResponse>(10000, [](QueryCreatureResponse const& p) { return p.Entry == 197; });
        REQUIRE(named);
        CHECK(named->Name == "Marshal McBride");

        request.Entry = 99999999;
        bot.Send(request);
        auto missing = co_await bot.Packet<QueryCreatureResponse>(10000, [](QueryCreatureResponse const& p) { return p.Entry == 99999999; });
        REQUIRE(missing);
        CHECK_FALSE(missing->Allow);
    });
}

TEST_CASE("query: a game object template comes back, and a missing entry is refused", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        QueryGameObject request;
        request.Entry = 1731; // Copper Vein
        bot.Send(request);
        auto found = co_await bot.Packet<QueryGameObjectResponse>(10000, [](QueryGameObjectResponse const& p) { return p.Entry == 1731; });
        REQUIRE(found);
        CHECK(found->Allow);
        CHECK(found->Name == "Copper Vein");
        CHECK(found->Type == 3); // GAMEOBJECT_TYPE_CHEST: mining nodes are chests
        CHECK(found->DisplayId != 0);

        request.Entry = 99999999;
        bot.Send(request);
        auto missing = co_await bot.Packet<QueryGameObjectResponse>(10000, [](QueryGameObjectResponse const& p) { return p.Entry == 99999999; });
        REQUIRE(missing);
        CHECK_FALSE(missing->Allow);
    });
}

TEST_CASE("query: NPC text answers with broadcast text ids", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        QueryNpcText request;
        request.TextId = 68;
        bot.Send(request);
        auto found = co_await bot.Packet<QueryNpcTextResponse>(10000, [](QueryNpcTextResponse const& p) { return p.TextId == 68; });
        REQUIRE(found);
        CHECK(found->Allow);
        CHECK(found->BroadcastTextIds[0] != 0);

        request.TextId = 99999999;
        bot.Send(request);
        auto missing = co_await bot.Packet<QueryNpcTextResponse>(10000, [](QueryNpcTextResponse const& p) { return p.TextId == 99999999; });
        REQUIRE(missing);
        CHECK_FALSE(missing->Allow);
    });
}

TEST_CASE("query: a page text request gets the whole chain of pages", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        QueryPageText request;
        request.PageTextId = 16; // links to page 17
        bot.Send(request);
        auto response = co_await bot.Packet<QueryPageTextResponse>(10000);
        REQUIRE(response);
        CHECK(response->Allow);
        REQUIRE(response->Pages.size() >= 2);
        CHECK(response->Pages.front().Id == 16);
        for (size_t i = 0; i < response->Pages.size(); ++i)
        {
            CHECK_FALSE(response->Pages[i].Text.empty());
            if (i + 1 < response->Pages.size())
                CHECK(response->Pages[i].NextPageId == response->Pages[i + 1].Id);
            else
                CHECK(response->Pages[i].NextPageId == 0);
        }
    });
}

TEST_CASE("query: the server time is close to now", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        bot.Send(QueryTime());
        auto response = co_await bot.Packet<QueryTimeResponse>(10000);
        REQUIRE(response);

        int64 const now = int64(time(nullptr));
        CHECK(std::abs(int64(response->CurrentTime) - now) < 60);
    });
}
