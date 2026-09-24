// The per-stage watchdog (bot-phase-2.md, "Timeouts, used"): a login stage held longer than
// RealBotLoginOptions::StageTimeoutMs fails the bot with FailedStage set, except InWorld, a stage
// held on purpose by StopAt, LoggingOut (which has its own timeout) and Disconnected.
#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>

namespace
{
BotRoutine HoldRoutine(BotContext& ctx)
{
    co_return co_await ctx.Hold();
}
}

TEST_CASE("stage watchdog: a stage held past the timeout fails the bot at that stage", "[watchdog]")
{
    TestFakeBot bot;
    bot.GetLoginOptions().StageTimeoutMs = 100;

    bot.SetStage(BotStage::WorldHandshake);
    bot.Tick(60);
    CHECK(bot.GetStage() == BotStage::WorldHandshake);

    bot.Tick(60);
    CHECK(bot.GetStage() == BotStage::Failed);
    CHECK(bot.GetFailedStage() == BotStage::WorldHandshake);
    CHECK(bot.GetLastError() == "stage WorldHandshake timed out after 100ms");
}

TEST_CASE("stage watchdog: the clock restarts on every stage change", "[watchdog]")
{
    TestFakeBot bot;
    bot.GetLoginOptions().StageTimeoutMs = 100;

    bot.SetStage(BotStage::LoginPortal);
    bot.Tick(80);
    bot.SetStage(BotStage::LoginRpc);
    bot.Tick(80);
    bot.SetStage(BotStage::RealmSelect);
    bot.Tick(80);

    CHECK(bot.GetStage() == BotStage::RealmSelect); // 240ms in all, never 100 in one stage
    bot.Tick(30);
    CHECK(bot.GetStage() == BotStage::Failed);
    CHECK(bot.GetFailedStage() == BotStage::RealmSelect);
}

TEST_CASE("stage watchdog: InWorld, Disconnected and LoggingOut are never timed out", "[watchdog]")
{
    for (BotStage stage : { BotStage::InWorld, BotStage::LoggingOut })
    {
        TestFakeBot bot;
        bot.GetLoginOptions().StageTimeoutMs = 50;
        bot.SetStage(stage);
        bot.Tick(10000);
        CHECK(bot.GetStage() == stage);
    }

    TestFakeBot idle; // Disconnected from the start
    idle.GetLoginOptions().StageTimeoutMs = 50;
    idle.Tick(10000);
    CHECK(idle.GetStage() == BotStage::Disconnected);
}

TEST_CASE("stage watchdog: a stage held on purpose by StopAt is exempt, any other is not", "[watchdog]")
{
    TestFakeBot held;
    held.GetLoginOptions().StageTimeoutMs = 100;
    held.GetLoginOptions().StopAt = BotStage::CharSelect;
    held.SetStage(BotStage::CharSelect);
    held.Tick(10000);
    CHECK(held.GetStage() == BotStage::CharSelect);

    TestFakeBot passingThrough;
    passingThrough.GetLoginOptions().StageTimeoutMs = 100;
    passingThrough.GetLoginOptions().StopAt = BotStage::CharSelect;
    passingThrough.SetStage(BotStage::WorldHandshake); // on the way there, not the stop
    passingThrough.Tick(10000);
    CHECK(passingThrough.GetStage() == BotStage::Failed);
    CHECK(passingThrough.GetFailedStage() == BotStage::WorldHandshake);
}

TEST_CASE("stage watchdog: a zero timeout turns it off", "[watchdog]")
{
    TestFakeBot bot;
    bot.GetLoginOptions().StageTimeoutMs = 0;
    bot.SetStage(BotStage::WorldHandshake);
    bot.Tick(1000000);
    CHECK(bot.GetStage() == BotStage::WorldHandshake);
}

TEST_CASE("stage watchdog: failing clears the brain, so a routine stuck in the stage is destroyed", "[watchdog]")
{
    TestFakeBot bot;
    bot.GetLoginOptions().StageTimeoutMs = 100;
    bot.SetStage(BotStage::EnteringWorld);
    bot.GetBrain().PushTask(std::make_unique<BotRoutineTask>("stuck", HoldRoutine(bot.GetContext())), bot.GetContext());
    REQUIRE(bot.GetBrain().Size() == 1);

    bot.Tick(150);
    CHECK(bot.GetStage() == BotStage::Failed);
    CHECK(bot.GetBrain().Empty());
}
