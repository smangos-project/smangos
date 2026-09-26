// Casting: what a bot gets back for a cast request, and what a bystander sees of it.
#include "Packets/SpellCast.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t PERCEPTION = 20600;   // instant, free, self-applied
constexpr int32_t CONJURE_WATER = 5504; // 3 second cast, self
constexpr int32_t FIREBALL = 133;       // not in a warrior's spellbook
constexpr int32_t REASON_INTERRUPTED = 63;
constexpr int32_t REASON_NOT_KNOWN = 87;

// A cast guid as a client makes one: the cast type in the top bits, the spell in the entry field.
ClassicGuid ClientCastId(int32_t spellId, uint64_t counter)
{
    return { counter, (uint64_t(47) << 58) | (uint64_t(uint32_t(spellId)) << 6) };
}

CastSpell SelfCast(int32_t spellId, ClassicGuid castId)
{
    CastSpell request;
    request.Cast.CastId = castId;
    request.Cast.SpellId = spellId;
    return request;
}

// Learns the spell and removes the costs and cooldowns that would get in the way of casting it.
void PrepareCaster(TestBot const& gm, int32_t spellId)
{
    SayCommand(gm, ".learn " + std::to_string(spellId));
    SayCommand(gm, ".cheat powercost on");
    SayCommand(gm, ".cheat cooldown on");
}

void ReleaseCaster(TestBot const& gm)
{
    SayCommand(gm, ".cheat powercost off");
    SayCommand(gm, ".cheat cooldown off");
}
}

TEST_CASE("cast: an instant spell is paired with its request and a bystander sees it go", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot watcher;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return watcher.GetKnownObject(gmGuid).has_value(); }, 10000));

        PrepareCaster(gm, PERCEPTION);
        co_await TestDelay(1000);

        ClassicGuid const requestId = ClientCastId(PERCEPTION, 1);
        gm.Send(SelfCast(PERCEPTION, requestId));

        REQUIRE(co_await gm.WaitReceived<SpellPrepare>(10000));
        auto prepare = gm.FindReceived<SpellPrepare>();
        REQUIRE(prepare);
        CHECK(prepare->ClientCastId == requestId);
        CHECK_FALSE(prepare->ServerCastId.IsEmpty());

        auto isGo = [](SpellGo const& p) { return p.Cast.SpellId == PERCEPTION; };
        REQUIRE(co_await gm.WaitReceived<SpellGo>(10000, isGo));
        auto go = gm.FindReceived<SpellGo>(isGo);
        REQUIRE(go);
        CHECK(go->Cast.CastId == prepare->ServerCastId);
        CHECK(go->Cast.CasterUnit == gmGuid);
        CHECK(go->Cast.OriginalCastId.IsEmpty());

        REQUIRE(co_await watcher.WaitReceived<SpellGo>(10000, isGo));
        auto seen = watcher.FindReceived<SpellGo>(isGo);
        REQUIRE(seen);
        CHECK(seen->Cast.CastId == prepare->ServerCastId);
        CHECK(seen->Cast.CasterUnit == gmGuid);

        ReleaseCaster(gm);
    });
}

TEST_CASE("cast: a spell the character doesn't know is refused under the request's own id", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        ClassicGuid const requestId = ClientCastId(FIREBALL, 2);
        bot.Send(SelfCast(FIREBALL, requestId));

        auto isFailed = [](CastFailed const& p) { return p.SpellId == FIREBALL; };
        REQUIRE(co_await bot.WaitReceived<CastFailed>(10000, isFailed));
        auto failed = bot.FindReceived<CastFailed>(isFailed);
        REQUIRE(failed);
        CHECK(failed->CastId == requestId);
        CHECK(failed->Reason == REASON_NOT_KNOWN);
    });
}

TEST_CASE("cast: a cast with a cast time starts before it goes, and cancelling it fails the cast for everyone", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot watcher;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await watcher.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return watcher.GetKnownObject(gmGuid).has_value(); }, 10000));

        PrepareCaster(gm, CONJURE_WATER);
        co_await TestDelay(1000);

        ClassicGuid const requestId = ClientCastId(CONJURE_WATER, 3);
        gm.Send(SelfCast(CONJURE_WATER, requestId));

        REQUIRE(co_await gm.WaitReceived<SpellPrepare>(10000));
        auto prepare = gm.FindReceived<SpellPrepare>();
        REQUIRE(prepare);

        auto isStart = [](SpellStart const& p) { return p.Cast.SpellId == CONJURE_WATER; };
        REQUIRE(co_await gm.WaitReceived<SpellStart>(10000, isStart));
        auto start = gm.FindReceived<SpellStart>(isStart);
        REQUIRE(start);
        CHECK(start->Cast.CastId == prepare->ServerCastId);
        CHECK(start->Cast.CastTime > 0);

        REQUIRE(co_await watcher.WaitReceived<SpellStart>(10000, isStart));

        CancelCast cancel;
        cancel.CastId = requestId;
        cancel.SpellId = CONJURE_WATER;
        gm.Send(cancel);

        auto isFailure = [](SpellFailure const& p) { return p.SpellId == CONJURE_WATER; };
        REQUIRE(co_await gm.WaitReceived<SpellFailure>(10000, isFailure));
        auto failure = gm.FindReceived<SpellFailure>(isFailure);
        REQUIRE(failure);
        CHECK(failure->CastId == prepare->ServerCastId);
        CHECK(failure->CasterUnit == gmGuid);
        CHECK(failure->Reason == REASON_INTERRUPTED);

        auto isOther = [](SpellFailedOther const& p) { return p.SpellId == CONJURE_WATER; };
        REQUIRE(co_await watcher.WaitReceived<SpellFailedOther>(10000, isOther));
        auto other = watcher.FindReceived<SpellFailedOther>(isOther);
        REQUIRE(other);
        CHECK(other->CastId == prepare->ServerCastId);
        CHECK(other->Reason == REASON_INTERRUPTED);

        ReleaseCaster(gm);
    });
}
