// Movement a bot sends and what the bots around it see of it.
#include "TestBot.h"
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
    });
}
