// A far teleport (map change) and what a GM bot sees of it.
#include "Packets/ChatMessages.h"
#include "Packets/MiscMessages.h"
#include "Packets/WorldTransfer.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <cmath>
#include <string>

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
        auto newWorld = co_await gm.Packet<NewWorld>(20000);
        bool const arrived = co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = gm.GetKnownObject(guid);
            return seen && std::fabs(seen->X - 1633.75f) < 5.0f;
        }, 20000);

        // Back to the starting map and position before anything is asserted, so a failure here
        // does not leave the account where the next scenario to use it does not expect it.
        SayCommand(gm, ".go xyz " + std::to_string(start.X) + " " + std::to_string(start.Y) + " " +
                       std::to_string(start.Z) + " 0");
        bool const returned = static_cast<bool>(co_await gm.Packet<NewWorld>(20000)) &&
            co_await TestWaitUntil([&]()
            {
                std::optional<BotKnownObject> const seen = gm.GetKnownObject(guid);
                return seen && std::fabs(seen->X - start.X) < 5.0f && std::fabs(seen->Y - start.Y) < 5.0f;
            }, 20000);

        REQUIRE(pending);
        CHECK(pending->MapId == 1);
        REQUIRE(newWorld);
        CHECK(newWorld->MapId == 1);
        CHECK(newWorld->X == Catch::Approx(1633.75f).margin(1.0f));
        REQUIRE(arrived);
        REQUIRE(returned);
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
        bool const gmArrived = static_cast<bool>(co_await gm.Packet<NewWorld>(20000));

        // Summons the traveler to the bystander's new location: a far teleport from the
        // traveler's point of view, landing right where the bystander already stands.
        SayCommand(gm, ".namego " + travelerName);

        bool const seenArriving = co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = gm.GetKnownObject(travelerGuid);
            return seen && std::fabs(seen->X - 1633.75f) < 5.0f;
        }, 20000);

        // Both back where they started before anything is asserted, so a failure does not leave
        // the accounts where the next scenario does not expect them.
        SayCommand(gm, ".recall " + travelerName);
        SayCommand(gm, ".go xyz " + std::to_string(gmStart.X) + " " + std::to_string(gmStart.Y) + " " +
                       std::to_string(gmStart.Z) + " 0");
        bool const gmReturned = static_cast<bool>(co_await gm.Packet<NewWorld>(20000));

        REQUIRE(gmArrived);
        REQUIRE(seenArriving);
        REQUIRE(gmReturned);
    });
}

TEST_CASE("world transfer: a bot inside an instance leaves it through its exit trigger", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot traveler;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await traveler.Stage(BotStage::InWorld, 30000));
        std::string const travelerName = traveler.GetCharacterName();

        // The Stormwind Stockade's exit trigger (503), standing next to it.
        SayCommand(gm, ".go trigger 503");
        auto gmInside = co_await gm.Packet<NewWorld>(20000);
        REQUIRE(gmInside);
        SayCommand(gm, ".namego " + travelerName);
        auto inside = co_await traveler.Packet<NewWorld>(20000);

        std::optional<NewWorld> outside;
        if (inside)
        {
            BotTaskArgs args;
            args.Set("x", "42.0");
            args.Set("y", "0.8");
            args.Set("z", "-13.0");
            traveler.RunTask("move_to", args);
            co_await TestWaitUntil([]() { return false; }, 4000);
            for (int32 id : {503})
            {
                AreaTrigger trigger;
                trigger.TriggerId = id;
                traveler.Send(trigger);
            }
            outside = co_await traveler.Packet<NewWorld>(10000);
        }

        SayCommand(gm, ".recall " + travelerName);
        SayCommand(gm, ".go xyz -8949.95 -132.493 83.53 0");
        co_await gm.Packet<NewWorld>(20000);

        REQUIRE(inside);
        CHECK(inside->MapId == 34);
        REQUIRE(outside);
        CHECK(outside->MapId == 0);
    });
}

TEST_CASE("world transfer: a bot that entered through an instance trigger leaves through the exit trigger", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        SayCommand(gm, ".cheat triggerpass on");
        SayCommand(gm, ".go trigger 101");
        co_await TestWaitUntil([]() { return false; }, 3000);

        AreaTrigger enter;
        enter.TriggerId = 101;
        gm.Send(enter);
        auto inside = co_await gm.Packet<NewWorld>(20000);

        std::optional<NewWorld> outside;
        if (inside)
        {
            co_await TestWaitUntil([]() { return false; }, 3000);
            AreaTrigger leave;
            leave.TriggerId = 503;
            gm.Send(leave);
            outside = co_await gm.Packet<NewWorld>(10000);
        }

        if (!outside)
        {
            SayCommand(gm, ".go xyz -8949.95 -132.493 83.53 0");
            co_await gm.Packet<NewWorld>(20000);
        }

        REQUIRE(inside);
        CHECK(inside->MapId == 34);
        REQUIRE(outside);
        CHECK(outside->MapId == 0);
    });
}
