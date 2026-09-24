// Ordering guarantees the login rewrite leans on (bot-phase-2.md, "Handshake through the normal
// dispatch"): a routine has resumed and registered its next await before the packet after the one
// that woke it is dispatched, even when both arrive in one tick and even across a ctx.Run boundary;
// and a routine that takes its own bot down (Disconnect, Failed) is never destroyed while it runs.
#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>
#include <optional>
#include <vector>

namespace
{
template<uint32 OpcodeValue>
struct OrderPacket
{
    static constexpr uint32 Opcode = OpcodeValue;
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

using PacketA = OrderPacket<0xF101>;
using PacketB = OrderPacket<0xF102>;
using PacketC = OrderPacket<0xF103>;

template<typename T>
T Make(uint32 value)
{
    T packet;
    packet.Value = value;
    return packet;
}

BotRoutine AwaitAThenB(BotContext& ctx, std::optional<PacketA>& a, std::optional<PacketB>& b)
{
    a = co_await ctx.Packet<PacketA>(1000);
    b = co_await ctx.Packet<PacketB>(1000);
    co_return BotTaskResult::Done;
}

BotRoutine AwaitAOnly(BotContext& ctx, std::optional<PacketA>& a)
{
    a = co_await ctx.Packet<PacketA>(1000);
    co_return BotTaskResult::Done;
}

// The parent runs a child that waits for A, then waits for B itself.
BotRoutine RunChildForAThenAwaitB(BotContext& ctx, std::optional<PacketA>& a, std::optional<PacketB>& b)
{
    co_await ctx.Run(AwaitAOnly(ctx, a));
    b = co_await ctx.Packet<PacketB>(1000);
    co_return BotTaskResult::Done;
}

// Two levels: a child that itself runs a child, then waits for C.
BotRoutine RunGrandchildThenAwaitC(BotContext& ctx, std::optional<PacketA>& a, std::optional<PacketC>& c)
{
    co_await ctx.Run(AwaitAOnly(ctx, a));
    c = co_await ctx.Packet<PacketC>(1000);
    co_return BotTaskResult::Done;
}

BotRoutine RunNestedThenAwaitB(BotContext& ctx, std::optional<PacketA>& a, std::optional<PacketC>& c, std::optional<PacketB>& b)
{
    co_await ctx.Run(RunGrandchildThenAwaitC(ctx, a, c));
    b = co_await ctx.Packet<PacketB>(1000);
    co_return BotTaskResult::Done;
}

// On the packet, takes its own bot down mid-resume (inside the dispatch that woke it), then keeps
// running to its next suspension.
BotRoutine DisconnectOnPacket(BotContext& ctx, bool& ranPastDisconnect)
{
    co_await ctx.Packet<PacketA>(1000);
    ctx.Disconnect(false);
    ranPastDisconnect = true;
    co_await ctx.Delay(100000);
    co_return BotTaskResult::Done;
}

BotRoutine FailOnPacket(BotContext& ctx)
{
    co_await ctx.Packet<PacketA>(1000);
    ctx.SetStage(BotStage::Failed);
    co_return BotTaskResult::Failed;
}

// Runs a child that takes the bot down, and would resume the parent afterwards.
BotRoutine DisconnectInUpdate(BotContext& ctx, bool& parentResumed)
{
    co_await ctx.Delay(10);
    ctx.Disconnect(false);
    co_return BotTaskResult::Done;
}

BotRoutine ParentOfDisconnectingChild(BotContext& ctx, bool& parentResumed)
{
    co_await ctx.Run(DisconnectInUpdate(ctx, parentResumed));
    parentResumed = true;
    co_return BotTaskResult::Done;
}
}

TEST_CASE("dispatch order: two packets in one tick reach a routine awaiting them one after another", "[dispatchorder]")
{
    TestFakeBot bot;
    std::optional<PacketA> a;
    std::optional<PacketB> b;
    bot.PushRoutine([&](BotContext& ctx) { return AwaitAThenB(ctx, a, b); });

    bot.InjectIncoming(Make<PacketA>(1));
    bot.InjectIncoming(Make<PacketB>(2));
    bot.Tick(10);

    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    CHECK(a->Value == 1);
    CHECK(b->Value == 2);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("dispatch order: the next packet still reaches the parent when the child that woke on the previous one finishes", "[dispatchorder]")
{
    TestFakeBot bot;
    std::optional<PacketA> a;
    std::optional<PacketB> b;
    bot.PushRoutine([&](BotContext& ctx) { return RunChildForAThenAwaitB(ctx, a, b); });

    // Both in one tick: A wakes the child, which finishes; B must find the parent already waiting.
    bot.InjectIncoming(Make<PacketA>(1));
    bot.InjectIncoming(Make<PacketB>(2));
    bot.Tick(10);

    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    CHECK(b->Value == 2);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("dispatch order: it holds across two levels of ctx.Run", "[dispatchorder]")
{
    TestFakeBot bot;
    std::optional<PacketA> a;
    std::optional<PacketC> c;
    std::optional<PacketB> b;
    bot.PushRoutine([&](BotContext& ctx) { return RunNestedThenAwaitB(ctx, a, c, b); });

    bot.InjectIncoming(Make<PacketA>(1));
    bot.InjectIncoming(Make<PacketC>(3));
    bot.InjectIncoming(Make<PacketB>(2));
    bot.Tick(10);

    REQUIRE(a.has_value());
    REQUIRE(c.has_value());
    REQUIRE(b.has_value());
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("dispatch order: a packet arriving before any routine awaits it is dropped, not queued for later", "[dispatchorder]")
{
    // The flip side of the guarantee: it orders packets against awaits, it is not a buffer. A packet
    // nobody is waiting for when it is dispatched is gone.
    TestFakeBot bot;
    std::optional<PacketA> a;
    std::optional<PacketB> b;
    bot.PushRoutine([&](BotContext& ctx) { return AwaitAThenB(ctx, a, b); });

    bot.InjectIncoming(Make<PacketB>(2)); // not what is being awaited yet
    bot.InjectIncoming(Make<PacketA>(1));
    bot.Tick(10);

    CHECK(a.has_value());
    CHECK_FALSE(b.has_value());
}

TEST_CASE("dispatch order: a routine that disconnects its own bot from inside the dispatch that woke it is not destroyed under itself", "[dispatchorder]")
{
    TestFakeBot bot;
    bool ranPastDisconnect = false;
    bot.SetStage(BotStage::InWorld); // so that Disconnected is a real change, which is what clears the brain
    bot.PushRoutine([&](BotContext& ctx) { return DisconnectOnPacket(ctx, ranPastDisconnect); });

    bot.InjectIncoming(Make<PacketA>(1));
    bot.Tick(10);

    // The routine ran on to its next suspension (its frame was alive), and only then was the brain
    // cleared: ASan is what would have caught the use after free.
    CHECK(ranPastDisconnect);
    CHECK(bot.GetStage() == BotStage::Disconnected);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("dispatch order: a routine that fails its own bot and returns is cleaned up safely", "[dispatchorder]")
{
    TestFakeBot bot;
    bot.PushRoutine([&](BotContext& ctx) { return FailOnPacket(ctx); });

    bot.InjectIncoming(Make<PacketA>(1));
    bot.Tick(10);

    CHECK(bot.GetStage() == BotStage::Failed);
    CHECK(bot.GetBrain().Empty());
}

TEST_CASE("dispatch order: a child that disconnects the bot from its own Update never resumes its parent", "[dispatchorder]")
{
    TestFakeBot bot;
    bool parentResumed = false;
    bot.SetStage(BotStage::InWorld);
    bot.PushRoutine([&](BotContext& ctx) { return ParentOfDisconnectingChild(ctx, parentResumed); });

    bot.Tick(20);

    CHECK(bot.GetStage() == BotStage::Disconnected);
    CHECK(bot.GetBrain().Empty());
    CHECK_FALSE(parentResumed);
}
