// A far teleport (map change) and what a GM bot sees of it.
#include "Packets/ChatMessages.h"
#include "Packets/WorldTransfer.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <cmath>
#include <string>

namespace
{
void SayCommand(TestBot const& bot, std::string const& text)
{
    ChatMessageSay say;
    say.Language = 7; // LANG_COMMON
    say.Message = text;
    bot.Send(say);
}
}

TEST_CASE("world transfer: a far teleport lands the bot on the destination map", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        ClassicGuid const guid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(guid).has_value(); }, 10000));

        BotKnownObject const start = *gm.GetKnownObject(guid);

        // Orgrimmar, a location guaranteed loaded on any standard database.
        SayCommand(gm, ".go xyz 1633.75 -4373.39 31.0 1");

        auto pending = co_await gm.Packet<TransferPending>(10000);
        REQUIRE(pending);
        CHECK(pending->MapId == 1);

        auto newWorld = co_await gm.Packet<NewWorld>(20000);
        REQUIRE(newWorld);
        CHECK(newWorld->MapId == 1);
        CHECK(newWorld->X == Catch::Approx(1633.75f).margin(1.0f));

        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = gm.GetKnownObject(guid);
            return seen && std::fabs(seen->X - 1633.75f) < 5.0f;
        }, 20000));

        // Back to the starting map and position, so the account is where the next scenario
        // to use it expects to find it.
        SayCommand(gm, ".go xyz " + std::to_string(start.X) + " " + std::to_string(start.Y) + " " +
                       std::to_string(start.Z) + " 0");
        REQUIRE(co_await gm.Packet<NewWorld>(20000));
        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = gm.GetKnownObject(guid);
            return seen && std::fabs(seen->X - start.X) < 5.0f && std::fabs(seen->Y - start.Y) < 5.0f;
        }, 20000));
    });
}

TEST_CASE("world transfer: a bystander already on the destination map sees the arriving bot", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot traveler;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await traveler.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        ClassicGuid const travelerGuid = traveler.GetCharacterGuid();
        std::string const travelerName = traveler.GetCharacterName();

        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(gmGuid).has_value(); }, 10000));
        BotKnownObject const gmStart = *gm.GetKnownObject(gmGuid);

        // The bystander (the GM) arrives at the destination map first.
        SayCommand(gm, ".go xyz 1633.75 -4373.39 31.0 1");
        REQUIRE(co_await gm.Packet<NewWorld>(20000));

        // Summons the traveler to the bystander's new location: a far teleport from the
        // traveler's point of view, landing right where the bystander already stands.
        SayCommand(gm, ".namego " + travelerName);

        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = gm.GetKnownObject(travelerGuid);
            return seen && std::fabs(seen->X - 1633.75f) < 5.0f;
        }, 20000));

        // Both back where they started, so the accounts are where the next scenario expects to
        // find them.
        SayCommand(gm, ".recall " + travelerName);
        SayCommand(gm, ".go xyz " + std::to_string(gmStart.X) + " " + std::to_string(gmStart.Y) + " " +
                       std::to_string(gmStart.Z) + " 0");
        REQUIRE(co_await gm.Packet<NewWorld>(20000));
    });
}
