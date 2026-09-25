// Phase 1 "world_entry/" scenarios (bot-phase-1.md's Tests section), against
// ConformanceRun::Target(). Character creation itself isn't covered here yet -- every target's
// configured accounts already carry a character from earlier live runs, so CreateIfNone never
// actually triggers with the account pools available; BotCharSelectTests.cpp (unit, fake server)
// is what actually exercises that path today.
#include "BotTask.h"
#include "Packets/Auth.h"
#include "Packets/Session.h"
#include "Packets/Classic/Character.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <cmath>

namespace
{
// Refuses the first Instance connect the bot attempts (BotTask::OnConnect): to the login that is a
// connection failure, which it must report on the Realm connection and try again.
class RefuseFirstInstanceConnectHook : public BotTask
{
    public:
        char const* GetName() const override { return "refuse_first_instance_connect"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnConnect(BotContext&, BotConnectionKind kind, std::string const&, uint16_t) override
        {
            if (kind == BotConnectionKind::Instance && !m_refused)
            {
                m_refused = true;
                return BotPacketAction::Drop;
            }
            return BotPacketAction::Continue;
        }

    private:
        bool m_refused = false;
};

// Every SMSG_CONNECT_TO the bot received, in order.
std::vector<ConnectTo> ReceivedConnectTos(TestBot const& bot)
{
    std::vector<ConnectTo> out;
    for (BotPacket packet : bot.Received())
    {
        if (packet.Opcode != CLASSIC_SMSG_CONNECT_TO)
            continue;
        packet.Data.rpos(0);
        ConnectTo connectTo;
        if (connectTo.Read(packet.Data))
            out.push_back(connectTo);
    }
    return out;
}
}

TEST_CASE("world_entry: an existing character resolves at CharSelect", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // A bot's stage is the step in progress, and StopAt holds as that step begins: the
        // character is resolved (enumerated, or created) by the time it enters EnteringWorld, and
        // CMSG_PLAYER_LOGIN has not been sent.
        TestBot bot(TestBotOptions().StopAt(BotStage::EnteringWorld));
        REQUIRE(co_await bot.Held(BotStage::EnteringWorld, 30000));
        CHECK(bot.GetLastError().empty());
        CHECK_FALSE(bot.Info().CharacterGuid.IsEmpty());
    });
}

TEST_CASE("world_entry: ping gets a pong with the same serial", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        Ping ping;
        ping.Serial = 0x50000001;
        bot.Send(ping);

        auto pong = co_await bot.Packet<Pong>(5000, [](Pong const& p) { return p.Serial == 0x50000001; });
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

TEST_CASE("world_entry: SMSG_CONNECT_TO leads to an Instance connection that resumes", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // The redirect arrived on the Realm connection and the world session resumed on the new one.
        std::optional<BotPacket> connectTo = bot.LastReceived(CLASSIC_SMSG_CONNECT_TO);
        REQUIRE(connectTo);
        CHECK(connectTo->Connection == BotConnectionKind::Realm);

        std::optional<BotPacket> resume = bot.LastReceived(CLASSIC_SMSG_RESUME_COMMS);
        REQUIRE(resume);
        CHECK(resume->Connection == BotConnectionKind::Instance);
        CHECK(bot.Info().Stage == BotStage::InWorld);
    });
}

TEST_CASE("world_entry: a failed Instance connect is reported and the next SMSG_CONNECT_TO tried", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // The hook refuses the bot's first Instance connect, which the login reports with
        // CMSG_CONNECT_TO_FAILED; the server must answer with another SMSG_CONNECT_TO (the next
        // attempt's serial) and the login still reaches the world on it.
        TestBot bot(TestBotOptions().Hook([]() { return std::make_unique<RefuseFirstInstanceConnectHook>(); }));
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        std::vector<ConnectTo> const redirects = ReceivedConnectTos(bot);
        REQUIRE(redirects.size() == 2);
        CHECK(redirects[0].Serial == CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1);
        CHECK(redirects[1].Serial == CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2);
        CHECK(bot.LastReceived(CLASSIC_SMSG_LOGIN_VERIFY_WORLD).has_value());
    });
}

TEST_CASE("world_entry: SMSG_LOGIN_VERIFY_WORLD carries the character's map and a finite position", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        std::optional<ClassicLoginVerifyWorld> verify;
        {
            TestBot bot;
            REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
            verify = bot.LastReceived<ClassicLoginVerifyWorld>();
            REQUIRE(verify);
        }

        CHECK(verify->MapId < 10000);
        CHECK(std::isfinite(verify->X));
        CHECK(std::isfinite(verify->Y));
        CHECK(std::isfinite(verify->Z));
        CHECK(std::isfinite(verify->O));

        // It is the map the character screen lists the character on (one bot at a time: a pool
        // may have a single account).
        TestBot screen(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await screen.Held(BotStage::CharSelect, 30000));
        screen.Send(ClassicEnumCharacters{});
        auto characters = co_await screen.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(characters);
        REQUIRE_FALSE(characters->Characters.empty());
        CHECK(uint32(characters->Characters[0].Map) == verify->MapId);
    });
}

TEST_CASE("world_entry: a clean logout ends Disconnected, with SMSG_LOGOUT_COMPLETE on the Realm connection", "[protocol][slow]")
{
    RunTest([]() -> TestRoutine
    {
        // The server counts a logout down (about 20 s in the open world) before it answers with
        // SMSG_LOGOUT_COMPLETE; the bot has to be willing to wait that long.
        TestBot bot(TestBotOptions().LogoutTimeoutMs(45000));
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        bot.Disconnect(true);
        REQUIRE(co_await bot.Stage(BotStage::Disconnected, 50000));
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);

        std::optional<BotPacket> complete = bot.LastReceived(CLASSIC_SMSG_LOGOUT_COMPLETE);
        REQUIRE(complete);
        CHECK(complete->Connection == BotConnectionKind::Realm);
    });
}
