// Target selection, melee and stand state: what a GM bot sees of its own swings, and what a bystander sees.
#include "Packets/CombatMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("combat: a GM bot attacks a spawned creature and a bystander sees the swings", "[gameplay]")
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

        SayCommand(gm, ".npc spawn add 6"); // Kobold Vermin: hostile to any player faction, level 1-2

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

        AttackSwing swing;
        swing.Victim = *victim;
        gm.Send(swing);

        auto started = co_await gm.Packet<AttackStart>(10000);
        REQUIRE(started);
        CHECK(started->Attacker == gmGuid);
        CHECK(started->Victim == *victim);

        auto state = co_await gm.Packet<MeleeAttackingStateUpdate>(10000, [&](MeleeAttackingStateUpdate const& p) { return p.Attacker == gmGuid; });
        REQUIRE(state);
        CHECK(state->Victim == *victim);

        auto seenByWatcher = co_await watcher.Packet<MeleeAttackingStateUpdate>(10000, [&](MeleeAttackingStateUpdate const& p) { return p.Attacker == gmGuid; });
        REQUIRE(seenByWatcher);
        CHECK(seenByWatcher->Victim == *victim);

        AttackStop stop;
        gm.Send(stop);
        auto stopped = co_await gm.Packet<SAttackStop>(10000, [&](SAttackStop const& p) { return p.Attacker == gmGuid; });
        REQUIRE(stopped);

        SayCommand(gm, ".npc despawn");
    });
}

TEST_CASE("combat: a bystander sees a creature's health drop, its tap and its death through values updates", "[gameplay]")
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
        // The guid carries the entry, so wandering wildlife isn't taken for the spawn.
        REQUIRE(co_await TestWaitUntil([&]()
        {
            for (ClassicGuid const& guid : KnownUnits(gm))
                if (((guid.High >> 6) & 0x7FFFFF) == 6 && std::find(before.begin(), before.end(), guid) == before.end())
                    victim = guid;
            return victim.has_value();
        }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return watcher.GetKnownObject(*victim).has_value(); }, 10000));

        SetSelection select;
        select.Target = *victim;
        gm.Send(select);

        AttackSwing swing;
        swing.Victim = *victim;
        gm.Send(swing);

        // The client ignores values blocks without the CGObject fragment marked active (0x01).
        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const known = watcher.GetKnownObject(*victim);
            return known && known->Health && known->MaxHealth && *known->Health < *known->MaxHealth;
        }, 15000));
        std::optional<BotKnownObject> hit = watcher.GetKnownObject(*victim);
        REQUIRE(hit);
        CHECK((hit->LastFragmentMask & 0x01) != 0);

        // Modern dynamic flags: tapped by someone else is 0x10, which the vanilla layout put at 0x4.
        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const known = watcher.GetKnownObject(*victim);
            return known && known->DynamicFlags && (*known->DynamicFlags & 0x10);
        }, 10000));

        SayCommand(gm, ".die");

        REQUIRE(co_await TestWaitUntil([&]()
        {
            std::optional<BotKnownObject> const known = watcher.GetKnownObject(*victim);
            return known && known->Health == 0;
        }, 10000));

        std::optional<BotKnownObject> const dead = watcher.GetKnownObject(*victim);
        REQUIRE(dead);
        REQUIRE(dead->DynamicFlags);
        CHECK((*dead->DynamicFlags & 0x1) == 0);  // not a modern flag
        CHECK((*dead->DynamicFlags & 0x4) == 0);  // lootable only for the looter
        CHECK((*dead->DynamicFlags & 0x10) != 0);

        std::optional<BotKnownObject> const deadForGm = gm.GetKnownObject(*victim);
        REQUIRE(deadForGm);
        CHECK(deadForGm->Health == 0);
        if (deadForGm->DynamicFlags)
            CHECK((*deadForGm->DynamicFlags & (0x1 | 0x10)) == 0);

        SayCommand(gm, ".npc spawn delete");
    });
}

TEST_CASE("combat: a bot sitting down and standing up is told each new stand state", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        StandStateChange sit;
        sit.StandState = 1; // UNIT_STAND_STATE_SIT
        bot.Send(sit);
        auto sitting = co_await bot.Packet<StandStateUpdate>(10000);
        REQUIRE(sitting);
        CHECK(sitting->StandState == 1);

        StandStateChange stand;
        stand.StandState = 0; // UNIT_STAND_STATE_STAND
        bot.Send(stand);
        auto standing = co_await bot.Packet<StandStateUpdate>(10000);
        REQUIRE(standing);
        CHECK(standing->StandState == 0);
    });
}

TEST_CASE("combat: a creature that is attacked turns hostile and both bots are told", "[gameplay]")
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
        AttackSwing swing;
        swing.Victim = *victim;
        gm.Send(swing);

        auto isHostile = [&](AIReactionUpdate const& p) { return p.Unit == *victim && p.Reaction == 2; }; // AI_REACTION_HOSTILE
        CHECK(co_await gm.WaitReceived<AIReactionUpdate>(10000, isHostile));
        CHECK(co_await watcher.WaitReceived<AIReactionUpdate>(10000, isHostile));

        AttackStop stop;
        gm.Send(stop);
        SayCommand(gm, ".npc despawn");
    });
}
