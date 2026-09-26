// Auras: what the owner and a bystander are told about a unit's aura slots.
#include "Packets/Auras.h"
#include "Packets/SpellCast.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
constexpr int32_t PERCEPTION = 20600;       // instant, free, self-applied buff
constexpr int32_t ARCANE_INTELLECT = 1459;  // half-hour buff, applied with .aura

std::optional<BotAura> FindAura(TestBot const& viewer, ClassicGuid const& unit, int32_t spellId)
{
    std::optional<BotKnownObject> const known = viewer.GetKnownObject(unit);
    if (!known)
        return std::nullopt;
    for (BotAura const& aura : known->Auras)
        if (aura.SpellId == spellId)
            return aura;
    return std::nullopt;
}

bool HasAura(TestBot const& viewer, ClassicGuid const& unit, int32_t spellId)
{
    return FindAura(viewer, unit, spellId).has_value();
}
}

TEST_CASE("aura: casting a self-buff sends its aura to the caster and a bystander", "[gameplay]")
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
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(gmGuid).has_value(); }, 10000));

        SayCommand(gm, ".learn " + std::to_string(PERCEPTION));
        SayCommand(gm, ".cheat powercost on");
        SayCommand(gm, ".cheat cooldown on");
        co_await TestDelay(1000);

        CastSpell request;
        request.Cast.CastId = { 1, (uint64_t(47) << 58) | (uint64_t(PERCEPTION) << 6) };
        request.Cast.SpellId = PERCEPTION;
        gm.Send(request);

        auto isPerception = [gmGuid](AuraUpdate const& p)
        {
            if (p.UnitGuid != gmGuid)
                return false;
            for (AuraInfo const& info : p.Auras)
                if (info.AuraData && info.AuraData->SpellId == PERCEPTION)
                    return true;
            return false;
        };
        REQUIRE(co_await gm.WaitReceived<AuraUpdate>(10000, isPerception));
        auto update = gm.FindReceived<AuraUpdate>(isPerception);
        REQUIRE(update);
        AuraDataInfo const* data = nullptr;
        for (AuraInfo const& info : update->Auras)
            if (info.AuraData && info.AuraData->SpellId == PERCEPTION)
                data = &*info.AuraData;
        REQUIRE(data);
        CHECK_FALSE(data->CastId.IsEmpty());
        CHECK((data->Flags & CLASSIC_AURA_FLAG_POSITIVE) != 0);
        CHECK((data->Flags & CLASSIC_AURA_FLAG_DURATION) != 0);
        CHECK(data->Duration.value_or(0) > 0);
        CHECK_FALSE(data->CastUnit.has_value());

        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(gm, gmGuid, PERCEPTION); }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(watcher, gmGuid, PERCEPTION); }, 10000));

        SayCommand(gm, ".cheat powercost off");
        SayCommand(gm, ".cheat cooldown off");
    });
}

TEST_CASE("aura: cancelling a buff removes it for the owner and a bystander", "[gameplay]")
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
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(gmGuid).has_value(); }, 10000));

        SayCommand(gm, ".aura " + std::to_string(ARCANE_INTELLECT));
        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(gm, gmGuid, ARCANE_INTELLECT); }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(watcher, gmGuid, ARCANE_INTELLECT); }, 10000));

        CancelAura cancel;
        cancel.SpellId = ARCANE_INTELLECT;
        cancel.CasterGuid = gmGuid;
        gm.Send(cancel);

        REQUIRE(co_await TestWaitUntil([&]() { return !HasAura(gm, gmGuid, ARCANE_INTELLECT); }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return !HasAura(watcher, gmGuid, ARCANE_INTELLECT); }, 10000));
    });
}
TEST_CASE("aura: a bot that arrives after a buff was applied gets it in the full list", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        ClassicGuid const gmGuid = gm.GetCharacterGuid();

        SayCommand(gm, ".aura " + std::to_string(ARCANE_INTELLECT));
        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(gm, gmGuid, ARCANE_INTELLECT); }, 10000));

        TestBot latecomer;
        REQUIRE(co_await latecomer.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await TestWaitUntil([&]() { return HasAura(latecomer, gmGuid, ARCANE_INTELLECT); }, 10000));

        std::optional<BotAura> const aura = FindAura(latecomer, gmGuid, ARCANE_INTELLECT);
        REQUIRE(aura);
        CHECK((aura->Flags & CLASSIC_AURA_FLAG_POSITIVE) != 0);

        SayCommand(gm, ".unaura " + std::to_string(ARCANE_INTELLECT));
        REQUIRE(co_await TestWaitUntil([&]() { return !HasAura(latecomer, gmGuid, ARCANE_INTELLECT); }, 10000));
    });
}
