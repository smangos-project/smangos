// Target selection and melee: what a GM bot sees of its own swings, and what a bystander sees.
#include "Packets/ChatMessages.h"
#include "Packets/CombatMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
void SayCommand(TestBot const& bot, std::string const& text)
{
    ChatMessageSay say;
    say.Language = 7; // LANG_COMMON
    say.Message = text;
    bot.Send(say);
}

// The creature the GM spawns beside itself isn't the only unit nearby (Northshire Abbey has its
// own wildlife and NPCs), so the new one is found by diffing the known-object set from before the
// spawn to after, rather than assuming it's the only BOT_TYPEID_UNIT around.
std::optional<ClassicGuid> FindNewUnit(TestBot const& bot, std::vector<ClassicGuid> const& before)
{
    for (BotKnownObject const& object : bot.GetKnownObjects())
    {
        if (object.ObjectType != BOT_TYPEID_UNIT)
            continue;
        if (std::find(before.begin(), before.end(), object.Guid) == before.end())
            return object.Guid;
    }
    return std::nullopt;
}
}

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

        std::vector<ClassicGuid> before;
        for (BotKnownObject const& object : gm.GetKnownObjects())
            if (object.ObjectType == BOT_TYPEID_UNIT)
                before.push_back(object.Guid);

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
