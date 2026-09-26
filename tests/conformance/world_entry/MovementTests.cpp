// Movement a bot sends and what the bots around it see of it.
#include "Packets/ChatMessages.h"
#include "Packets/MountSpecialAnim.h"
#include "Packets/MovementMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <cmath>

static float FlatDistance(BotKnownObject const& a, BotKnownObject const& b)
{
    return std::hypot(a.X - b.X, a.Y - b.Y);
}

TEST_CASE("movement: a bot that walks is seen moving by another bot", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot walker;
        TestBot watcher;
        REQUIRE(co_await walker.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));

        ClassicGuid const guid = walker.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&watcher, guid]() { return watcher.GetKnownObject(guid).has_value(); }, 10000));
        REQUIRE(co_await TestWaitUntil([&walker, guid]() { return walker.GetKnownObject(guid).has_value(); }, 10000));

        BotKnownObject const start = *watcher.GetKnownObject(guid);
        BotTaskArgs args;
        args.Set("x", std::to_string(start.X + 8.0f));
        args.Set("y", std::to_string(start.Y));
        args.Set("z", std::to_string(start.Z));
        walker.RunTask("move_to", args);

        // The watcher hears about the walk as it happens, and ends up where the walker stopped.
        bool arrived = false;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = watcher.GetKnownObject(guid);
            arrived = seen && seen->MoveUpdates >= 2 && FlatDistance(*seen, start) > 7.0f;
            return arrived;
        }, 15000));

        BotKnownObject const end = *watcher.GetKnownObject(guid);
        CHECK(FlatDistance(end, start) == Catch::Approx(8.0f).margin(1.5f));

        // Back to where the character started: it is saved with its position, and the next
        // scenario to use this account must still find it next to everyone else.
        BotTaskArgs back;
        back.Set("x", std::to_string(start.X));
        back.Set("y", std::to_string(start.Y));
        back.Set("z", std::to_string(start.Z));
        walker.RunTask("move_to", back);
        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const seen = watcher.GetKnownObject(guid);
            return seen && FlatDistance(*seen, start) < 1.0f;
        }, 15000));
    });
}

static bool WaitForKnown(TestBot const& observer, ClassicGuid guid)
{
    return observer.GetKnownObject(guid).has_value();
}

TEST_CASE("movement: a speed change reaches the mover and is relayed to other bots once acknowledged", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot watcher;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));
        ClassicGuid const guid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return WaitForKnown(watcher, guid); }, 10000));

        SayCommand(gm, ".modify speed 2");

        auto asked = co_await gm.Packet<MoveSetRunSpeed>(10000, [](MoveSetRunSpeed const& p) { return p.Speed > 13.9f; });
        REQUIRE(asked);
        CHECK(asked->Mover == guid);
        CHECK(asked->Speed == Catch::Approx(14.0f));

        auto relayed = co_await watcher.Packet<MoveUpdateRunSpeed>(10000, [](MoveUpdateRunSpeed const& p) { return p.Speed > 13.9f; });
        REQUIRE(relayed);
        CHECK(relayed->Status.Guid == guid);

        SayCommand(gm, ".modify speed 1");
        REQUIRE(co_await watcher.Packet<MoveUpdateRunSpeed>(10000, [](MoveUpdateRunSpeed const& p) { return p.Speed < 7.1f; }));
    });
}

TEST_CASE("movement: a root is acknowledged and other bots see the rooted flag", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot watcher;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));
        ClassicGuid const guid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return WaitForKnown(watcher, guid); }, 10000));

        SayCommand(gm, ".aura 122");
        REQUIRE(co_await gm.Packet<MoveRoot>(10000));
        auto rooted = co_await watcher.Packet<MoveUpdate>(10000, [](MoveUpdate const& p) { return (p.Status.Flags & CLASSIC_MOVEFLAG_ROOT) != 0; });
        REQUIRE(rooted);
        CHECK(rooted->Status.Guid == guid);

        SayCommand(gm, ".unaura 122");
        REQUIRE(co_await gm.Packet<MoveUnroot>(10000));
        REQUIRE(co_await watcher.Packet<MoveUpdate>(10000, [](MoveUpdate const& p) { return (p.Status.Flags & CLASSIC_MOVEFLAG_ROOT) == 0; }));
    });
}

TEST_CASE("movement: a teleport within the map reaches the mover and is relayed to other bots", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot watcher;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));
        ClassicGuid const guid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return WaitForKnown(watcher, guid); }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return WaitForKnown(gm, guid); }, 10000));

        BotKnownObject const start = *gm.GetKnownObject(guid);
        float const x = start.X + 6.0f;
        SayCommand(gm, ".go xyz " + std::to_string(x) + " " + std::to_string(start.Y) + " " + std::to_string(start.Z));

        auto moved = co_await gm.Packet<MoveTeleport>(10000);
        REQUIRE(moved);
        CHECK(moved->Mover == guid);
        CHECK(moved->X == Catch::Approx(x).margin(0.5f));

        auto relayed = co_await watcher.Packet<MoveUpdateTeleport>(10000, [x](MoveUpdateTeleport const& p) { return std::fabs(p.Status.X - x) < 0.5f; });
        REQUIRE(relayed);
        CHECK(relayed->Status.Guid == guid);

        // Back where it started: the character is saved with its position, and the next scenario to
        // use this account must still find it next to everyone else.
        SayCommand(gm, ".go xyz " + std::to_string(start.X) + " " + std::to_string(start.Y) + " " + std::to_string(start.Z));
        REQUIRE(co_await watcher.Packet<MoveUpdateTeleport>(10000, [&](MoveUpdateTeleport const& p) { return std::fabs(p.Status.X - start.X) < 0.5f; }));
    });
}

TEST_CASE("movement: a mount special anim request is relayed to a nearby bot", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot caster;
        TestBot watcher;
        REQUIRE(co_await caster.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));

        ClassicGuid const guid = caster.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return WaitForKnown(watcher, guid); }, 10000));

        MountSpecial request;
        request.SpellVisualKitIDs = { 321, 654 };
        request.SequenceVariation = 2;
        caster.Send(request);

        auto relayed = co_await watcher.Packet<SpecialMountAnim>(10000);
        REQUIRE(relayed);
        CHECK(relayed->UnitGuid == guid);
        CHECK(relayed->SpellVisualKitIDs == std::vector<int32>{ 321, 654 });
        CHECK(relayed->SequenceVariation == 2);
    });
}

TEST_CASE("movement: a bot sees creatures around it move", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // Creatures wander on their own; one of the nearby ones sets out within a minute.
        REQUIRE(co_await TestWaitUntil([&bot]()
        {
            for (BotKnownObject const& object : bot.GetKnownObjects())
            {
                if (object.ObjectType == BOT_TYPEID_UNIT && object.MonsterMoves > 0)
                    return true;
            }
            return false;
        }, 60000));
    });
}
