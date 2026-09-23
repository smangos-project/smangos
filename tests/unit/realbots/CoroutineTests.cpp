#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>
#include <optional>
#include <stdexcept>

// Every routine below is a named coroutine function, never a capturing lambda
// marked -> BotRoutine: PushRoutine's factory only runs long enough to obtain the
// BotRoutine value, but the routine itself can be resumed many Ticks later. A
// lambda that is itself the coroutine leaves the frame's implicit `this` pointing
// at a closure temporary that's long gone by then (see TestFakeBot::PushRoutine).
namespace
{
struct FakePacket
{
    static constexpr uint32 Opcode = 0xF00D;
    uint32 Value = 0;

    void Write(ByteBuffer& data) const { data << Value; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Value;
        return true;
    }
};

BotRoutine AwaitPacketAndRecord(BotContext& ctx, bool& timedOut)
{
    std::optional<FakePacket> pong = co_await ctx.Packet<FakePacket>(100);
    timedOut = !pong.has_value();
    co_return BotTaskResult::Done;
}

BotRoutine DelayThenDone(BotContext& ctx, uint32 ms)
{
    co_await ctx.Delay(ms);
    co_return BotTaskResult::Done;
}

BotRoutine RunChildAndRecord(BotContext& ctx, BotTaskResult& childResult, bool& parentResumed)
{
    childResult = co_await ctx.Run(DelayThenDone(ctx, 10));
    parentResumed = true;
    co_return BotTaskResult::Done;
}

BotRoutine ThrowingRoutine(BotContext&)
{
    throw std::runtime_error("boom");
    co_return BotTaskResult::Done; // unreachable; keeps this a coroutine with the right return type
}

BotRoutine RunThrowingChildAndRecord(BotContext& ctx, bool& sawFailed)
{
    BotTaskResult result = co_await ctx.Run(ThrowingRoutine(ctx));
    sawFailed = (result == BotTaskResult::Failed);
    co_return result;
}

BotRoutine HoldForever(BotContext& ctx, bool& reachedEnd)
{
    co_await ctx.Hold();
    reachedEnd = true; // never reached
    co_return BotTaskResult::Done;
}

BotRoutine DelayLong(BotContext& ctx, bool& resumed)
{
    co_await ctx.Delay(100000);
    resumed = true;
    co_return BotTaskResult::Done;
}

BotRoutine AwaitStageAndRecord(BotContext& ctx, std::optional<bool>& reached)
{
    reached = co_await ctx.Stage(BotStage::InWorld, 1000);
    co_return BotTaskResult::Done;
}

BotRoutine AwaitPacketValue(BotContext& ctx, std::optional<FakePacket>& received)
{
    received = co_await ctx.Packet<FakePacket>(1000);
    co_return BotTaskResult::Done;
}

class BlockingHookTask : public BotTask
{
    public:
        char const* GetName() const override { return "blocking_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            if (dir == BotPacketDir::Incoming && packet.Opcode == FakePacket::Opcode)
                return BotPacketAction::Handled;
            return BotPacketAction::Continue;
        }
};
}

TEST_CASE("BotRoutine only resumes from Tick and times out", "[coroutines]")
{
    TestFakeBot bot;
    bool timedOut = false;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitPacketAndRecord(ctx, timedOut); });

    bot.Tick(50);
    CHECK_FALSE(timedOut);
    bot.Tick(60);
    CHECK(timedOut);
}

TEST_CASE("BotRoutine: ctx.Run pushes a child and resumes once it finishes", "[coroutines]")
{
    TestFakeBot bot;
    BotTaskResult childResult = BotTaskResult::Running;
    bool parentResumed = false;

    bot.PushRoutine([&](BotContext& ctx) { return RunChildAndRecord(ctx, childResult, parentResumed); });

    CHECK(bot.GetBrain().Size() == 2);

    bot.Tick(5);
    CHECK_FALSE(parentResumed);

    bot.Tick(10);
    CHECK(parentResumed);
    CHECK(childResult == BotTaskResult::Done);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("BotRoutine: an escaping exception fails the task instead of propagating", "[coroutines]")
{
    TestFakeBot bot;
    bool sawFailed = false;

    bot.PushRoutine([&](BotContext& ctx) { return RunThrowingChildAndRecord(ctx, sawFailed); });

    bot.Tick(1);

    CHECK(sawFailed);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("BotRoutine: Hold suspends until the brain is cleared, never resuming", "[coroutines]")
{
    TestFakeBot bot;
    bool reachedEnd = false;

    bot.PushRoutine([&](BotContext& ctx) { return HoldForever(ctx, reachedEnd); });

    for (int i = 0; i < 10; ++i)
        bot.Tick(1000);

    CHECK_FALSE(reachedEnd);
    CHECK(bot.GetBrain().Size() == 1);

    bot.SetStage(BotStage::InWorld);
    bot.SetStage(BotStage::Disconnected);

    CHECK(bot.GetBrain().Empty());
    CHECK_FALSE(reachedEnd);
}

TEST_CASE("BotBrain: clearing the stack destroys a suspended routine without resuming it", "[coroutines]")
{
    TestFakeBot bot;
    bool resumed = false;

    bot.PushRoutine([&](BotContext& ctx) { return DelayLong(ctx, resumed); });

    CHECK(bot.GetBrain().Size() == 1);
    bot.GetBrain().Clear();
    CHECK(bot.GetBrain().Empty());
    CHECK_FALSE(resumed);
}

TEST_CASE("BotRoutine: ctx.Stage waits for a stage change or times out", "[coroutines]")
{
    TestFakeBot bot;
    std::optional<bool> reached;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitStageAndRecord(ctx, reached); });

    bot.Tick(10);
    CHECK_FALSE(reached.has_value());

    bot.SetStage(BotStage::InWorld);
    bot.Tick(10);

    REQUIRE(reached.has_value());
    CHECK(*reached);
}

TEST_CASE("BotBrain: a routine observes an injected packet via ctx.Packet", "[coroutines]")
{
    TestFakeBot bot;
    std::optional<FakePacket> received;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitPacketValue(ctx, received); });

    FakePacket packet;
    packet.Value = 42;
    bot.InjectIncoming(packet);
    bot.Tick(10);

    REQUIRE(received.has_value());
    CHECK(received->Value == 42);
}

TEST_CASE("BotBrain: Handled from a higher task stops a packet reaching a lower one", "[coroutines]")
{
    TestFakeBot bot;
    std::optional<FakePacket> received;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitPacketValue(ctx, received); });

    bot.GetBrain().PushTask(std::make_unique<BlockingHookTask>(), bot.GetContext());

    FakePacket packet;
    packet.Value = 7;
    bot.InjectIncoming(packet);
    bot.Tick(10);

    CHECK_FALSE(received.has_value());
}
