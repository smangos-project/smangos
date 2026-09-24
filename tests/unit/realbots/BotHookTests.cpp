#include "TestFakeBot.h"
#include <catch_amalgamated.hpp>
#include <optional>
#include <string>
#include <vector>

// bot-phase-2.md, "Hooks off the stack": a hook lives beside BotBrain's task stack. It sees packets
// (before the stack, in insertion order) and stage changes, is never Update()d, and never buries
// the routine that is running.
namespace
{
struct HookPacket
{
    static constexpr uint32 Opcode = 0xF00E;
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

// Records what it saw into a shared log, tagged with its own label, and optionally swallows or
// drops HookPacket. Update is never expected to run.
class RecordingHook : public BotTask
{
    public:
        RecordingHook(std::string label, std::vector<std::string>& log) : m_label(std::move(label)), m_log(log) {}

        char const* GetName() const override { return "recording_hook"; }

        BotTaskResult Update(BotContext&, uint32) override
        {
            ++Updates;
            return BotTaskResult::Running;
        }

        void OnStart(BotContext&) override { m_log.push_back(m_label + ":start"); }

        void OnStageChanged(BotContext&, BotStage from, BotStage to) override
        {
            m_log.push_back(m_label + ":stage " + std::to_string(int(from)) + ">" + std::to_string(int(to)));
        }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            m_log.push_back(m_label + (dir == BotPacketDir::Incoming ? ":in" : ":out"));
            if (packet.Opcode != HookPacket::Opcode)
                return BotPacketAction::Continue;
            return Action;
        }

        BotPacketAction Action = BotPacketAction::Continue;
        uint32 Updates = 0;

    private:
        std::string m_label;
        std::vector<std::string>& m_log;
};

// Removes itself the first time it sees a packet.
class SelfRemovingHook : public BotTask
{
    public:
        char const* GetName() const override { return "self_removing_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext& ctx, BotPacket&, BotPacketDir) override
        {
            ctx.RemoveHook(this);
            return BotPacketAction::Continue;
        }
};

// On its first packet, starts a disconnect: SetStage(Disconnected) clears the brain from inside a
// dispatch that is running this very hook.
class DisconnectingHook : public BotTask
{
    public:
        char const* GetName() const override { return "disconnecting_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext& ctx, BotPacket&, BotPacketDir) override
        {
            ctx.SetStage(BotStage::Disconnected);
            return BotPacketAction::Continue;
        }
};

BotRoutine AwaitHookPacket(BotContext& ctx, std::optional<HookPacket>& received)
{
    received = co_await ctx.Packet<HookPacket>(1000);
    co_return BotTaskResult::Done;
}

BotRoutine DelayThenFlag(BotContext& ctx, uint32 ms, bool& finished)
{
    co_await ctx.Delay(ms);
    finished = true;
    co_return BotTaskResult::Done;
}

HookPacket MakePacket(uint32 value)
{
    HookPacket packet;
    packet.Value = value;
    return packet;
}
}

TEST_CASE("hooks: OnStart runs on add, and they see packets in insertion order before the stack", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;
    std::optional<HookPacket> received;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitHookPacket(ctx, received); });

    bot.GetContext().AddHook(std::make_unique<RecordingHook>("a", log));
    bot.GetContext().AddHook(std::make_unique<RecordingHook>("b", log));
    CHECK(log == std::vector<std::string>{"a:start", "b:start"});

    log.clear();
    bot.InjectIncoming(MakePacket(5));
    bot.Tick(10);

    CHECK(log == std::vector<std::string>{"a:in", "b:in"});
    REQUIRE(received.has_value());
    CHECK(received->Value == 5);
}

TEST_CASE("hooks: Handled from a hook keeps a packet from every later hook and from the stack", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;
    std::optional<HookPacket> received;

    bot.PushRoutine([&](BotContext& ctx) { return AwaitHookPacket(ctx, received); });

    auto first = std::make_unique<RecordingHook>("a", log);
    first->Action = BotPacketAction::Handled;
    bot.GetContext().AddHook(std::move(first));
    bot.GetContext().AddHook(std::make_unique<RecordingHook>("b", log));
    log.clear();

    bot.InjectIncoming(MakePacket(1));
    bot.Tick(10);

    CHECK(log == std::vector<std::string>{"a:in"});
    CHECK_FALSE(received.has_value());
}

TEST_CASE("hooks: Drop from a hook keeps an outgoing packet off the wire", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;

    auto hook = std::make_unique<RecordingHook>("a", log);
    hook->Action = BotPacketAction::Drop;
    bot.GetContext().AddHook(std::move(hook));

    bot.GetContext().Send(MakePacket(1));

    CHECK(bot.GetSentPackets().empty());
}

TEST_CASE("hooks: a hook is never Update()d and a never-finishing one does not bury the running routine", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;
    bool finished = false;

    bot.PushRoutine([&](BotContext& ctx) { return DelayThenFlag(ctx, 50, finished); });

    auto* hook = static_cast<RecordingHook*>(bot.GetContext().AddHook(std::make_unique<RecordingHook>("a", log)));

    bot.Tick(20);
    bot.Tick(20);
    CHECK_FALSE(finished);
    bot.Tick(20);
    CHECK(finished);
    CHECK(hook->Updates == 0);
}

TEST_CASE("hooks: they see stage changes", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;

    bot.GetContext().AddHook(std::make_unique<RecordingHook>("a", log));
    log.clear();

    bot.GetContext().SetStage(BotStage::LoginPortal);

    REQUIRE(log.size() == 1);
    CHECK(log[0] == "a:stage " + std::to_string(int(BotStage::Disconnected)) + ">" + std::to_string(int(BotStage::LoginPortal)));
}

TEST_CASE("hooks: Clear drops non-persistent hooks and keeps persistent ones", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;

    bot.GetContext().AddHook(std::make_unique<RecordingHook>("temp", log));
    bot.GetContext().AddHook(std::make_unique<RecordingHook>("keep", log), true);
    CHECK(bot.GetBrain().HookCount() == 2);

    bot.GetBrain().Clear();
    CHECK(bot.GetBrain().HookCount() == 1);

    log.clear();
    bot.InjectIncoming(MakePacket(1));
    bot.Tick(10);
    CHECK(log == std::vector<std::string>{"keep:in"});
}

TEST_CASE("hooks: a disconnect triggered from inside a hook's own OnPacket is safe", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;

    bot.GetContext().AddHook(std::make_unique<DisconnectingHook>());
    bot.GetContext().AddHook(std::make_unique<RecordingHook>("later", log));
    bot.GetContext().SetStage(BotStage::LoginPortal); // so the hook's SetStage(Disconnected) is a real change
    log.clear();

    bot.InjectIncoming(MakePacket(1));
    bot.Tick(10);

    // The disconnect cleared the brain mid-dispatch without destroying the hook that triggered it;
    // both non-persistent hooks are gone once dispatch ends. ASan is the real check that nothing
    // was touched after free.
    CHECK(bot.GetBrain().HookCount() == 0);
}

TEST_CASE("hooks: RemoveHook removes, is a no-op for a stale handle, and works from inside OnPacket", "[hooks]")
{
    TestFakeBot bot;
    std::vector<std::string> log;

    BotTask* recording = bot.GetContext().AddHook(std::make_unique<RecordingHook>("a", log));
    bot.GetContext().AddHook(std::make_unique<SelfRemovingHook>());
    CHECK(bot.GetBrain().HookCount() == 2);

    bot.GetContext().RemoveHook(recording);
    CHECK(bot.GetBrain().HookCount() == 1);
    bot.GetContext().RemoveHook(recording); // stale: already gone, only ever compared
    CHECK(bot.GetBrain().HookCount() == 1);

    bot.InjectIncoming(MakePacket(1));
    bot.Tick(10);
    CHECK(bot.GetBrain().HookCount() == 0);
}
