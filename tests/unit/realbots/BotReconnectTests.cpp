// RealBot::Reconnect and the abrupt Disconnect (bot-phase-2.md, "Reconnect"), on a bot with no
// connections and a stand-in "login" task that just counts how often it starts.
#include "BuiltinBotTasks.h"
#include "BotTaskRegistry.h"
#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>

namespace
{
int g_loginStarts = 0;

BotRoutine StubLogin(BotContext& ctx)
{
    ++g_loginStarts;
    ctx.SetStage(BotStage::LoginPortal);
    co_return co_await ctx.Hold();
}

// Swaps the built-in "login" for the stub for one test, and puts the real one back afterwards.
struct StubLoginTask
{
    StubLoginTask()
    {
        g_loginStarts = 0;
        sBotTaskRegistry.RegisterRoutine("login", [](BotContext& ctx, BotTaskArgs const&) { return StubLogin(ctx); });
    }

    ~StubLoginTask() { RegisterBuiltinBotTasks(); }
};

class NamedHook : public BotTask
{
    public:
        char const* GetName() const override { return "named_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }
};
}

TEST_CASE("Reconnect: from InWorld it drops the session and starts a fresh login", "[reconnect]")
{
    StubLoginTask const stub;
    TestFakeBot bot;

    bot.GetBrain().PushTask(sBotTaskRegistry.Create("login", bot.GetContext(), BotTaskArgs()), bot.GetContext());
    bot.SetStage(BotStage::InWorld);
    bot.SetLastError("old failure");
    bot.GetContext().AddHook(std::make_unique<NamedHook>());
    bot.GetContext().AddHook(std::make_unique<NamedHook>(), true);
    REQUIRE(g_loginStarts == 1);

    bot.Reconnect();
    CHECK(g_loginStarts == 1); // posted, not run inline
    bot.Tick(1);

    CHECK(g_loginStarts == 2);
    CHECK(bot.GetStage() == BotStage::LoginPortal);
    CHECK(bot.GetBrain().Size() == 1);       // the old login is gone, one new one runs
    CHECK(bot.GetBrain().HookCount() == 1);  // the persistent hook survived, the other went
    CHECK(bot.GetLastError().empty());
    CHECK(bot.GetFailedStage() == BotStage::Disconnected);
    CHECK(bot.GetDisconnectReason() == BotDisconnectReason::None);
}

TEST_CASE("Reconnect: from Failed it clears the failure and logs in again", "[reconnect]")
{
    StubLoginTask const stub;
    TestFakeBot bot;

    bot.SetStage(BotStage::WorldHandshake);
    bot.SetLastError("handshake timed out");
    bot.SetStage(BotStage::Failed);
    REQUIRE(bot.GetFailedStage() == BotStage::WorldHandshake);

    bot.Reconnect();
    bot.Tick(1);

    CHECK(g_loginStarts == 1);
    CHECK(bot.GetStage() == BotStage::LoginPortal);
    CHECK(bot.GetLastError().empty());
    CHECK(bot.GetFailedStage() == BotStage::Disconnected);
}

TEST_CASE("Reconnect: from Disconnected it just logs in", "[reconnect]")
{
    StubLoginTask const stub;
    TestFakeBot bot;
    REQUIRE(bot.GetStage() == BotStage::Disconnected);

    bot.Reconnect();
    bot.Tick(1);

    CHECK(g_loginStarts == 1);
    CHECK(bot.GetStage() == BotStage::LoginPortal);
    CHECK(bot.GetBrain().Size() == 1);
}

TEST_CASE("Reconnect: asking twice in a row still leaves exactly one login", "[reconnect]")
{
    StubLoginTask const stub;
    TestFakeBot bot;

    bot.Reconnect();
    bot.Reconnect();
    bot.Tick(1);

    CHECK(g_loginStarts == 2);
    CHECK(bot.GetBrain().Size() == 1);
}

TEST_CASE("Reconnect: is safe from the bot's own routine", "[reconnect]")
{
    StubLoginTask const stub;
    TestFakeBot bot;
    bot.SetStage(BotStage::InWorld);

    // Called from inside a task that the reconnect then destroys: it is posted, so it runs between
    // ticks instead of pulling the routine out from under itself.
    struct SelfReconnect
    {
        static BotRoutine Run(BotContext& ctx)
        {
            ctx.GetBot().Reconnect();
            co_await ctx.Delay(100000);
            co_return BotTaskResult::Done;
        }
    };
    bot.GetBrain().PushTask(std::make_unique<BotRoutineTask>("self_reconnect", SelfReconnect::Run(bot.GetContext())), bot.GetContext());

    bot.Tick(1);

    CHECK(g_loginStarts == 1);
    CHECK(bot.GetBrain().Size() == 1);
    CHECK(bot.GetStage() == BotStage::LoginPortal);
}

TEST_CASE("Disconnect(false): the abrupt one records Requested and sends no logout request", "[reconnect]")
{
    TestFakeBot bot;
    bot.SetStage(BotStage::InWorld);

    bot.Disconnect(false);

    CHECK(bot.GetStage() == BotStage::Disconnected);
    CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);
    CHECK(bot.GetSentPackets().empty()); // no CMSG_LOGOUT_REQUEST, nothing at all
}

TEST_CASE("Disconnect(true) with nothing to ask (no Instance connection) also goes straight to Disconnected", "[reconnect]")
{
    TestFakeBot bot;
    bot.SetStage(BotStage::InWorld);

    bot.Disconnect(true);

    CHECK(bot.GetStage() == BotStage::Disconnected);
    CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);
}
