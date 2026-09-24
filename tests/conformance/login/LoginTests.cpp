// Phase 1 "login/" scenarios (bot-phase-1.md's Tests section), against ConformanceRun::Target().
#include "ClassicPackets.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("login: a bot reaches the world", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        bot.Disconnect(true);
        REQUIRE(co_await bot.Stage(BotStage::Disconnected, 10000));
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);
    });
}

TEST_CASE("login: wrong password fails at the login portal", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().Password("definitely-wrong"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 10000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
        INFO("server said: " << bot.GetLastError());
    });
}
