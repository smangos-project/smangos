// Phase 1 "framework/" scenarios (bot-phase-1.md's Tests section), against
// ConformanceRun::Target() (plus a deliberately unreachable "deadhost" target for the last one).
#include "BotContext.h"
#include "BotTask.h"
#include "ClassicOpcodes.h"

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
        REQUIRE(co_await bot.Held(BotStage::WorldHandshake, 30000));

        // It holds -- waiting again for the same stage (already true) resolves immediately, and
        // it must never have drifted past it on its own, however long the server keeps talking.
        co_await TestDelay(1000);
        REQUIRE(co_await bot.Held(BotStage::WorldHandshake, 1000));
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

TEST_CASE("framework: connecting to a dead address fails cleanly", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // "deadhost" (targets/tests-conformance.conf.dist) points at 203.0.113.1, RFC 5737's
        // reserved documentation range -- guaranteed unreachable, never just slow -- with a 2s
        // ConnectTimeoutMs (and a 5s StageTimeoutMs behind it): the target's own timeouts are what
        // bound this, so failing well inside the 10s default proves they are used.
        TestBot bot(TestBotOptions().TargetOverride("deadhost"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 6000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
    });
}
