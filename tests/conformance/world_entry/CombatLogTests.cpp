// The combat log: what the caster and a bystander are told about a spell's damage, heals, kills and
// the experience they earn.
#include "Packets/CombatLog.h"
#include "Packets/CombatMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t FIREBALL = 133;        // rank 1, fire, hits after a short flight
constexpr int32_t CORRUPTION = 172;      // rank 1, shadow damage over time
constexpr int32_t LESSER_HEAL = 2050;    // rank 1
constexpr uint8_t SCHOOL_MASK_FIRE = 4;
constexpr int32_t AURA_PERIODIC_DAMAGE = 3;

void CastOnSelection(TestBot const& bot, int32_t spellId)
{
    SayCommand(bot, ".cast " + std::to_string(spellId) + " triggered");
}
}

TEST_CASE("combat log: a spell's damage is logged for the caster and a bystander", "[gameplay]")
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

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 6");
        std::optional<ClassicGuid> victim;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            victim = FindNewUnit(gm, before);
            return victim.has_value();
        }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return watcher.GetKnownObject(*victim).has_value(); }, 10000));

        SetSelection select;
        select.Target = *victim;
        gm.Send(select);
        co_await TestDelay(500);

        auto isFireballHit = [&](SpellNonMeleeDamageLog const& p)
        {
            return p.Caster == gmGuid && p.SpellId == FIREBALL && p.Victim == *victim;
        };

        // A cast can miss; the log to wait for is the one that lands
        bool landed = false;
        for (int attempt = 0; attempt < 4 && !landed; ++attempt)
        {
            CastOnSelection(gm, FIREBALL);
            landed = co_await gm.WaitReceived<SpellNonMeleeDamageLog>(8000, isFireballHit);
        }
        REQUIRE(landed);

        auto log = gm.FindReceived<SpellNonMeleeDamageLog>(isFireballHit);
        REQUIRE(log);
        CHECK(log->Damage > 0);
        CHECK(log->OriginalDamage >= log->Damage);
        CHECK(log->SchoolMask == SCHOOL_MASK_FIRE);
        CHECK_FALSE(log->Periodic);
        CHECK_FALSE(log->CastId.IsEmpty());

        REQUIRE(co_await watcher.WaitReceived<SpellNonMeleeDamageLog>(8000, isFireballHit));

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("combat log: a damage over time spell logs its ticks", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        ClassicGuid const gmGuid = gm.GetCharacterGuid();

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 6");
        std::optional<ClassicGuid> victim;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            victim = FindNewUnit(gm, before);
            return victim.has_value();
        }, 10000));

        SetSelection select;
        select.Target = *victim;
        gm.Send(select);
        co_await TestDelay(500);

        auto isTick = [&](SpellPeriodicAuraLog const& p)
        {
            return p.Caster == gmGuid && p.SpellId == CORRUPTION && p.Target == *victim;
        };

        CastOnSelection(gm, CORRUPTION);
        REQUIRE(co_await gm.WaitReceived<SpellPeriodicAuraLog>(15000, isTick));

        auto tick = gm.FindReceived<SpellPeriodicAuraLog>(isTick);
        REQUIRE(tick);
        REQUIRE(tick->Effects.size() == 1);
        CHECK(tick->Effects[0].Effect == AURA_PERIODIC_DAMAGE);
        CHECK(tick->Effects[0].Amount > 0);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("combat log: a heal is logged with the healer and the target", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        ClassicGuid const gmGuid = gm.GetCharacterGuid();

        SetSelection select;
        select.Target = gmGuid;
        gm.Send(select);
        co_await TestDelay(500);

        CastOnSelection(gm, LESSER_HEAL);

        auto isHeal = [&](SpellHealLog const& p) { return p.Caster == gmGuid && p.SpellId == LESSER_HEAL; };
        REQUIRE(co_await gm.WaitReceived<SpellHealLog>(10000, isHeal));

        auto heal = gm.FindReceived<SpellHealLog>(isHeal);
        REQUIRE(heal);
        CHECK(heal->Target == gmGuid);
        CHECK(heal->Health > 0);
    });
}

TEST_CASE("combat log: killing a creature sends the kill and its experience", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        ClassicGuid const gmGuid = gm.GetCharacterGuid();

        std::vector<ClassicGuid> const before = KnownUnits(gm);
        SayCommand(gm, ".npc spawn add 6");
        std::optional<ClassicGuid> victim;
        REQUIRE(co_await TestWaitUntil([&]()
        {
            victim = FindNewUnit(gm, before);
            return victim.has_value();
        }, 10000));

        SetSelection select;
        select.Target = *victim;
        gm.Send(select);
        co_await TestDelay(500);

        SayCommand(gm, ".damage 1000");

        auto isKill = [&](PartyKillLog const& p) { return p.Killer == gmGuid && p.Victim == *victim; };
        REQUIRE(co_await gm.WaitReceived<PartyKillLog>(10000, isKill));

        auto isXp = [&](LogXPGain const& p) { return p.Victim == *victim; };
        REQUIRE(co_await gm.WaitReceived<LogXPGain>(10000, isXp));
        auto xp = gm.FindReceived<LogXPGain>(isXp);
        REQUIRE(xp);
        CHECK(xp->Reason == CLASSIC_XP_REASON_KILL);
        CHECK(xp->Amount > 0);
        CHECK(xp->Original >= xp->Amount);
    });
}

TEST_CASE("combat log: entering an unexplored area gives exploration experience", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(gmGuid).has_value(); }, 10000));
        BotKnownObject const start = *gm.GetKnownObject(gmGuid);

        // Forgetting every explored area makes the one the bot stands in new again; a short hop
        // makes the server look at its position.
        SayCommand(gm, ".explorecheat 0");
        co_await TestDelay(1000);
        SayCommand(gm, ".go xyz " + std::to_string(start.X + 6.0f) + " " + std::to_string(start.Y) + " " + std::to_string(start.Z));

        auto any = [](ExplorationExperience const&) { return true; };
        REQUIRE(co_await gm.WaitReceived<ExplorationExperience>(15000, any));
        auto exploration = gm.FindReceived<ExplorationExperience>(any);
        REQUIRE(exploration);
        CHECK(exploration->AreaId > 0);
        CHECK(exploration->Experience >= 0);

        // Back where it started: the character is saved with its position
        SayCommand(gm, ".go xyz " + std::to_string(start.X) + " " + std::to_string(start.Y) + " " + std::to_string(start.Z));
        co_await TestDelay(1000);
    });
}
