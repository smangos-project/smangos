// Round trips for the modern aura wire.
#include "Packets/Auras.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
AuraDataInfo SampleAura()
{
    AuraDataInfo aura;
    aura.CastId = { 77, 0xBC00000000000000ULL };
    aura.SpellId = 6673;
    aura.Visual.SpellXSpellVisualId = 4242;
    aura.Flags = CLASSIC_AURA_FLAG_POSITIVE | CLASSIC_AURA_FLAG_CANCELABLE | CLASSIC_AURA_FLAG_DURATION;
    aura.ActiveFlags = 0x5;
    aura.CastLevel = 60;
    aura.Applications = 3;
    aura.Duration = 120000;
    aura.Remaining = 90000;
    return aura;
}

void CheckAura(AuraDataInfo const& a, AuraDataInfo const& b)
{
    CHECK(a.CastId == b.CastId);
    CHECK(a.SpellId == b.SpellId);
    CHECK(a.Visual.SpellXSpellVisualId == b.Visual.SpellXSpellVisualId);
    CHECK(a.Flags == b.Flags);
    CHECK(a.ActiveFlags == b.ActiveFlags);
    CHECK(a.CastLevel == b.CastLevel);
    CHECK(a.Applications == b.Applications);
    CHECK(a.CastUnit == b.CastUnit);
    CHECK(a.Duration == b.Duration);
    CHECK(a.Remaining == b.Remaining);
}
}

TEST_CASE("AuraUpdate round-trips a full list with timed and untimed auras", "[packets]")
{
    AuraUpdate packet;
    packet.UpdateAll = true;
    packet.UnitGuid = { 5, 0x0800000000000000ULL };

    AuraInfo timed;
    timed.Slot = 2;
    timed.AuraData = SampleAura();
    timed.AuraData->CastUnit = ClassicGuid{ 9, 0x0800000000000000ULL };

    AuraInfo untimed;
    untimed.Slot = 40;
    untimed.AuraData = SampleAura();
    untimed.AuraData->Flags = CLASSIC_AURA_FLAG_NEGATIVE;
    untimed.AuraData->Duration.reset();
    untimed.AuraData->Remaining.reset();
    packet.Auras = { timed, untimed };

    RoundTrip(packet, [](AuraUpdate const& a, AuraUpdate const& b)
    {
        CHECK(b.UpdateAll);
        CHECK(a.UnitGuid == b.UnitGuid);
        REQUIRE(b.Auras.size() == 2);
        for (size_t i = 0; i < 2; ++i)
        {
            CHECK(a.Auras[i].Slot == b.Auras[i].Slot);
            REQUIRE(b.Auras[i].AuraData.has_value());
            CheckAura(*a.Auras[i].AuraData, *b.Auras[i].AuraData);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AuraUpdate round-trips a slot removal", "[packets]")
{
    AuraUpdate packet;
    packet.UnitGuid = { 5, 0x0800000000000000ULL };
    AuraInfo removed;
    removed.Slot = 7;
    packet.Auras = { removed };

    RoundTrip(packet, [](AuraUpdate const&, AuraUpdate const& b)
    {
        CHECK_FALSE(b.UpdateAll);
        REQUIRE(b.Auras.size() == 1);
        CHECK(b.Auras[0].Slot == 7);
        CHECK_FALSE(b.Auras[0].AuraData.has_value());
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CancelAura round-trips", "[packets]")
{
    CancelAura packet;
    packet.SpellId = 6673;
    packet.CasterGuid = { 5, 0x0800000000000000ULL };
    RoundTrip(packet, [](CancelAura const& a, CancelAura const& b)
    {
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.CasterGuid == b.CasterGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CancelGrowthAura round-trips", "[packets]")
{
    RoundTrip(CancelGrowthAura(), [](CancelGrowthAura const&, CancelGrowthAura const&) {});
}
