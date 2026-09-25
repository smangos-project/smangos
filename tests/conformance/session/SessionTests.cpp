// "session/" scenarios (port-plan.md steps 3-5: what a server does with a session's whole life:
// leaving cleanly, vanishing, a second login, sitting idle), against ConformanceRun::Target().
// Reconnect() logs the same bot in again on the same account and options (README, "Sessions").

#include "ConformanceRun.h"
#include "RealBotTarget.h"
#include "Packets/Auth.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <memory>
#include <vector>

namespace
{
uint32_t ServerCloseTimeoutMs()
{
    return ConformanceRun::TargetConfig().ServerCloseTimeoutMs;
}
}

TEST_CASE("session: logging out cleanly, then reconnecting, reaches the world again", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().LogoutTimeoutMs(45000));
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        bot.Disconnect(true);
        REQUIRE(co_await bot.Stage(BotStage::Disconnected, 50000));
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);

        bot.Reconnect();
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        CHECK(bot.GetLastError().empty());
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::None); // a new session
    });
}

TEST_CASE("session: vanishing abruptly, then reconnecting, reaches the world again within the target's bound", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // "The client vanished": both sockets close with no CMSG_LOGOUT_REQUEST. The server still
        // has a session for the account and the character in the world when the next login comes.
        bot.Disconnect(false);
        REQUIRE(co_await bot.Stage(BotStage::Disconnected, 5000));
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);

        bot.Reconnect();
        REQUIRE(co_await bot.Stage(BotStage::InWorld, ServerCloseTimeoutMs()));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("session: a second login on an account already in the world takes the session over", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot first;
        REQUIRE(co_await first.Stage(BotStage::InWorld, 30000));

        // Two sessions on one account, on purpose (a pool of one account is enough).
        TestBot second(TestBotOptions().ShareAccountWith(first));

        // The account has one session: the newcomer gets it and the first bot loses it. Which of the
        // two a server keeps is the rule under test (both references keep the newcomer); how the first
        // one hears of it is not: a disconnect, or a failed login step if it was cut off mid-way.
        REQUIRE(co_await second.Stage(BotStage::InWorld, ServerCloseTimeoutMs()));
        bool const firstLostIt = co_await TestWaitUntil([&first]()
        {
            BotStage const stage = first.GetStage();
            return stage == BotStage::Disconnected || stage == BotStage::Failed;
        }, ServerCloseTimeoutMs());
        CHECK(firstLostIt);
        CHECK(second.GetStage() == BotStage::InWorld);
        INFO("first bot ended: " << BotStageName(first.GetStage()) << ", " << BotDisconnectReasonName(first.GetDisconnectReason()));
    });
}

TEST_CASE("session: an idle bot with only default keepalives is not kicked", "[protocol][slow]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // The bot answers pings and time syncs and sends its own keepalives, nothing else, for the
        // target's IdleSessionMs (default two minutes); a server that drops idle sessions sooner than
        // that would drop every real client standing still.
        uint32_t const idleMs = ConformanceRun::TargetConfig().IdleSessionMs;
        bool const leftWorld = co_await TestWaitUntil([&bot]() { return bot.GetStage() != BotStage::InWorld; }, idleMs);
        CHECK_FALSE(leftWorld);
        CHECK(bot.GetStage() == BotStage::InWorld);
    });
}

TEST_CASE("session: a queued session is told its wait-queue position, then that the queue cleared", "[protocol][slow]")
{
    // Needs PlayerLimit forced low on the target; skips otherwise. Doesn't drive the queued bot to
    // InWorld (queued sessions drop CLASSIC_CMSG_ENUM_CHARACTERS until dequeued, and the bot has no
    // retry for that) -- just checks WaitQueueUpdate then WaitQueueFinish arrive.
    RunTest([]() -> TestRoutine
    {
        uint32_t const limit = ConformanceRun::TargetConfig().PlayerLimit;
        if (limit == 0)
            SKIP("target's PlayerLimit isn't configured for this scenario (RealBots.Target.<name>.PlayerLimit)");

        std::vector<std::unique_ptr<TestBot>> bots;
        for (uint32_t i = 0; i < limit; ++i)
        {
            bots.push_back(std::make_unique<TestBot>());
            REQUIRE(co_await bots.back()->Stage(BotStage::InWorld, 30000));
        }

        TestBot queued;
        auto update = co_await queued.Packet<WaitQueueUpdate>(15000);
        REQUIRE(update);
        CHECK(update->WaitCount == 1);

        bots.front()->Disconnect(true);
        REQUIRE(co_await bots.front()->Stage(BotStage::Disconnected, 50000));

        auto finish = co_await queued.Packet<WaitQueueFinish>(15000);
        REQUIRE(finish);
    });
}
