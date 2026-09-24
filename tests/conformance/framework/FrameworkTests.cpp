// Phase 1 "framework/" scenarios (bot-phase-1.md's Tests section), against
// ConformanceRun::Target() (plus a deliberately unreachable "deadhost" target for the last one).
#include "BotContext.h"
#include "BotTask.h"
#include "ClassicOpcodes.h"
#include "ClassicPackets.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <memory>
#include <vector>

namespace
{
// Drops every outgoing CMSG_PING -- the "framework: Drop keeps a CMSG_PING off the wire" scenario
// checks that the server then never has anything to answer, i.e. SMSG_PONG never arrives
// (bot-examples.md #5's IgnoreTimeSyncTask pattern, applied outbound instead of inbound).
class DropPingTask : public BotTask
{
    public:
        char const* GetName() const override { return "drop_ping"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            if (dir == BotPacketDir::Outgoing && packet.Opcode == CLASSIC_CMSG_PING)
                return BotPacketAction::Drop;
            return BotPacketAction::Continue;
        }
};
}

TEST_CASE("framework: stop_at holds a stage", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldHandshake));
        REQUIRE(co_await bot.Stage(BotStage::WorldHandshake, 30000));

        // It holds -- waiting again for the same stage (already true) resolves immediately, and
        // it must never have drifted past it on its own.
        REQUIRE(co_await bot.Stage(BotStage::WorldHandshake, 1000));
        CHECK(bot.GetStage() == BotStage::WorldHandshake);
    });
}

TEST_CASE("framework: Drop keeps a CMSG_PING off the wire", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // A hook, not a pushed task: it never sits on the stack, so it can't bury `login`.
        bot.AddHook(std::make_unique<DropPingTask>());

        // RealBots.PingIntervalMs defaults to 5000ms; give it margin past that.
        CHECK(co_await bot.NoPacket(CLASSIC_SMSG_PONG, 8000));
    });
}

TEST_CASE("framework: ten bots log in at once", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        std::vector<std::unique_ptr<TestBot>> bots;
        for (uint32 i = 0; i < 10; ++i)
            bots.push_back(std::make_unique<TestBot>());

        std::vector<bool> inWorld = co_await WhenAll(bots, [](std::unique_ptr<TestBot>& bot) { return bot->Stage(BotStage::InWorld, 60000); });
        CHECK(std::all_of(inWorld.begin(), inWorld.end(), [](bool ok) { return ok; }));

        for (auto& bot : bots)
            bot->Disconnect(true);
        co_await WhenAll(bots, [](std::unique_ptr<TestBot>& bot) { return bot->Disconnected(10000); });
    });
}

TEST_CASE("framework: connecting to a dead address fails cleanly", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // RealBotTarget::ConnectTimeoutMs is read and stored but not yet consumed anywhere
        // (a known gap, bot-phase-2.md "Timeouts, used") -- BotConnector::ConnectAsync's callers all still use a
        // hardcoded 10s, so this waits comfortably past that rather than the target's configured
        // value. "deadhost" (targets/tests-conformance.conf.dist) points at 203.0.113.1, RFC
        // 5737's reserved documentation range -- guaranteed unreachable, never just slow.
        TestBot bot(TestBotOptions().TargetOverride("deadhost"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 15000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
    });
}
