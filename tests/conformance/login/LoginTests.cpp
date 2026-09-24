// "login/" scenarios (port-plan.md step 2: the Battle.net login server replacing realmd), against
// ConformanceRun::Target(). One scenario per piece of that step, so vmangos.txt shrinks as each piece
// lands rather than all at once. A bot's stage is the step in progress and StopAt(X) holds as X
// begins (README, "Stages and stop_at"); bot.Held(X) is "held there, with everything the steps before
// it produced in place".
#include "ClassicPackets.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <random>

namespace
{
// A name no account has, different on every run.
std::string UnknownAccountName()
{
    std::random_device device;
    return "nobody" + std::to_string(device()) + "@bots";
}
}

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

TEST_CASE("login: the portal issues a login ticket", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Held as LoginRpc begins: the portal login is done, the RPC port not yet touched.
        TestBot bot(TestBotOptions().StopAt(BotStage::LoginRpc));
        REQUIRE(co_await bot.Held(BotStage::LoginRpc, 15000));
        CHECK_FALSE(bot.Info().Session.LoginTicket.empty());
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

TEST_CASE("login: an unknown account fails at the login portal", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().AccountName(UnknownAccountName()));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 10000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
        INFO("server said: " << bot.GetLastError());
    });
}

TEST_CASE("login: the RPC logon completes", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Held as RealmSelect begins: ConnectionService.Connect, Logon and VerifyWebCredentials
        // answered, and the AuthenticationListener.OnLogonComplete push received.
        TestBot bot(TestBotOptions().StopAt(BotStage::RealmSelect));
        REQUIRE(co_await bot.Held(BotStage::RealmSelect, 15000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("login: a forged login ticket is rejected at the RPC step", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().LoginTicket("forged-ticket-0123456789abcdef0123456789abcdef"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 15000));
        CHECK(bot.GetFailedStage() == BotStage::LoginRpc);
        INFO("server said: " << bot.GetLastError());
    });
}

// Not here, though bot-phase-2.md lists it: "a reused login ticket is rejected". Neither reference
// server rejects one: TrinityCore's bnetserver and vibemangos's LoginTicketStore both only *expire*
// tickets (a TTL, refreshable via refreshLoginTicket), never consume them, so a used ticket that has
// not expired logs in again. Asserting otherwise would encode a rule no server has; an expired
// ticket is the thing a server must reject, and that is not testable inside a run.

TEST_CASE("login: the realm list contains the configured realm and parses", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // The bot picks its realm out of the parsed realm list by the target's configured name or
        // id; held as WorldConnecting begins means the list was received, parsed and contained it.
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldConnecting));
        REQUIRE(co_await bot.Held(BotStage::WorldConnecting, 20000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("login: realm join returns an address, a ticket and a server secret", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldConnecting));
        REQUIRE(co_await bot.Held(BotStage::WorldConnecting, 20000));

        BotSessionInfo const session = bot.Info().Session;
        CHECK_FALSE(session.WorldAddress.empty());
        CHECK(session.WorldPort != 0);
        CHECK_FALSE(session.RealmJoinTicket.empty());
        // client secret (32) || server secret (32): the world session's join key.
        CHECK(session.JoinSessionKey.size() == 64);
    });
}

TEST_CASE("login: joining a realm that is not listed fails at realm select", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().RealmId(0x7FFFFF01));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 20000));
        CHECK(bot.GetFailedStage() == BotStage::RealmSelect);
        INFO("bot said: " << bot.GetLastError());
    });
}
