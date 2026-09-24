// Phase 1 "world_entry/" scenarios (bot-phase-1.md's Tests section), against
// ConformanceRun::Target(). Character creation itself isn't covered here yet -- every target's
// configured accounts already carry a character from earlier live runs, so CreateIfNone never
// actually triggers with the account pools available; BotCharSelectTests.cpp (unit, fake server)
// is what actually exercises that path today.
#include "ClassicPackets.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("world_entry: an existing character resolves at CharSelect", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // A bot's stage is the step in progress, and StopAt holds as that step begins: the
        // character is resolved (enumerated, or created) by the time it enters EnteringWorld, and
        // CMSG_PLAYER_LOGIN has not been sent.
        TestBot bot(TestBotOptions().StopAt(BotStage::EnteringWorld));
        REQUIRE(co_await bot.Stage(BotStage::EnteringWorld, 30000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("world_entry: ping gets a pong with the same serial", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        ClassicPing ping;
        ping.Serial = 0x50000001;
        bot.Send(ping);

        auto pong = co_await bot.Packet<ClassicPong>(5000, [](ClassicPong const& p) { return p.Serial == 0x50000001; });
        REQUIRE(pong);
    });
}

TEST_CASE("world_entry: the framework's own keepalive ping updates latency", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // RealBots.PingIntervalMs defaults to 5000ms (RealBot::UpdateKeepalive) -- give it two
        // cycles' worth of margin rather than trying to synchronize with the exact timer.
        co_await TestDelay(12000);
        CHECK(bot.GetLatencyMs() > 0);
    });
}
