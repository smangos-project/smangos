// Round trips for the modern casting wire.
#include "Packets/ClassicCastResult.h"
#include "Packets/SpellCast.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
SpellTargetData UnitTarget()
{
    SpellTargetData target;
    target.Flags = 0x2; // unit
    target.UnitGuid = { 0x1234, 0x2000000000000000ULL };
    return target;
}

void CheckTarget(SpellTargetData const& a, SpellTargetData const& b)
{
    CHECK(a.Flags == b.Flags);
    CHECK(a.UnitGuid == b.UnitGuid);
    CHECK(a.ItemGuid == b.ItemGuid);
    CHECK(a.Name == b.Name);
    CHECK(a.SrcLocation.has_value() == b.SrcLocation.has_value());
    CHECK(a.DstLocation.has_value() == b.DstLocation.has_value());
    CHECK(a.Orientation == b.Orientation);
    CHECK(a.MapId == b.MapId);
    if (a.DstLocation && b.DstLocation)
    {
        CHECK(a.DstLocation->X == b.DstLocation->X);
        CHECK(a.DstLocation->Y == b.DstLocation->Y);
        CHECK(a.DstLocation->Z == b.DstLocation->Z);
    }
}

SpellCastData SampleCastData()
{
    SpellCastData data;
    data.CasterGuid = { 5, 0x0800000000000000ULL };
    data.CasterUnit = data.CasterGuid;
    data.CastId = { 77, 0xBC00000000000000ULL };
    data.SpellId = 6673;
    data.Visual.SpellXSpellVisualId = 12345;
    data.CastFlags = 0x102;
    data.CastTime = 1500;
    data.Target = UnitTarget();
    data.HitTargets = { { 5, 0x0800000000000000ULL } };
    return data;
}

void CheckCastData(SpellCastData const& a, SpellCastData const& b)
{
    CHECK(a.CasterGuid == b.CasterGuid);
    CHECK(a.CasterUnit == b.CasterUnit);
    CHECK(a.CastId == b.CastId);
    CHECK(a.OriginalCastId == b.OriginalCastId);
    CHECK(a.SpellId == b.SpellId);
    CHECK(a.Visual.SpellXSpellVisualId == b.Visual.SpellXSpellVisualId);
    CHECK(a.CastFlags == b.CastFlags);
    CHECK(a.CastTime == b.CastTime);
    CHECK(a.HitTargets == b.HitTargets);
    CHECK(a.MissTargets == b.MissTargets);
    REQUIRE(a.MissStatus.size() == b.MissStatus.size());
    for (size_t i = 0; i < a.MissStatus.size(); ++i)
    {
        CHECK(a.MissStatus[i].Reason == b.MissStatus[i].Reason);
        CHECK(a.MissStatus[i].ReflectStatus == b.MissStatus[i].ReflectStatus);
    }
    REQUIRE(a.RemainingPower.size() == b.RemainingPower.size());
    for (size_t i = 0; i < a.RemainingPower.size(); ++i)
    {
        CHECK(a.RemainingPower[i].Type == b.RemainingPower[i].Type);
        CHECK(a.RemainingPower[i].Cost == b.RemainingPower[i].Cost);
    }
    CHECK(a.AmmoDisplayId == b.AmmoDisplayId);
    CheckTarget(a.Target, b.Target);
}
}

TEST_CASE("CastSpell round-trips a plain unit-targeted request", "[packets]")
{
    CastSpell packet;
    packet.Cast.CastId = { 9, 0xBC00000000000000ULL };
    packet.Cast.SpellId = 133;
    packet.Cast.Visual.SpellXSpellVisualId = 42;
    packet.Cast.Target = UnitTarget();
    RoundTrip(packet, [](CastSpell const& a, CastSpell const& b)
    {
        CHECK(a.Cast.CastId == b.Cast.CastId);
        CHECK(a.Cast.SpellId == b.Cast.SpellId);
        CHECK(a.Cast.Visual.SpellXSpellVisualId == b.Cast.Visual.SpellXSpellVisualId);
        CheckTarget(a.Cast.Target, b.Cast.Target);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CastSpell round-trips every optional section", "[packets]")
{
    CastSpell packet;
    packet.Cast.CastId = { 1, 0xBC00000000000000ULL };
    packet.Cast.SpellId = 5;
    packet.Cast.Misc[0] = 3;
    packet.Cast.Misc[1] = -4;
    packet.Cast.MissileTrajectory = { 0.5f, 20.0f };
    packet.Cast.CraftingFlags = 1;
    packet.Cast.SendCastFlags = 0x11;
    packet.Cast.OptionalCurrencies = { { 1, 2 }, { 3, 4 } };
    SpellCraftingReagent reagent;
    reagent.ItemId = 100;
    reagent.DataSlotIndex = 2;
    reagent.Quantity = 3;
    reagent.Unknown1000 = 7;
    packet.Cast.OptionalReagents = { reagent };
    packet.Cast.RemovedModifications = { SpellCraftingReagent{ 9, 8, 7, std::nullopt } };
    packet.Cast.CraftingOrderId = 0x1122334455667788ULL;
    packet.Cast.Weight = { { 2, 55, 6 } };
    packet.Cast.Target.Flags = 0x40 | 0x2000; // dest location + string
    packet.Cast.Target.DstLocation = TargetLocation{ {}, 1.0f, 2.0f, 3.0f };
    packet.Cast.Target.Orientation = 1.5f;
    packet.Cast.Target.MapId = 530;
    packet.Cast.Target.Name = "target";

    ClassicMovementInfo move;
    move.Guid = { 5, 0x0800000000000000ULL };
    move.X = 10.0f;
    packet.Cast.MoveUpdate = move;

    RoundTrip(packet, [](CastSpell const& a, CastSpell const& b)
    {
        CHECK(b.Cast.Misc[0] == 3);
        CHECK(b.Cast.Misc[1] == -4);
        CHECK(b.Cast.MissileTrajectory.Speed == 20.0f);
        CHECK(b.Cast.CraftingFlags == 1);
        CHECK(b.Cast.SendCastFlags == 0x11);
        REQUIRE(b.Cast.OptionalCurrencies.size() == 2);
        CHECK(b.Cast.OptionalCurrencies[1].Count == 4);
        REQUIRE(b.Cast.OptionalReagents.size() == 1);
        CHECK(b.Cast.OptionalReagents[0].Unknown1000 == std::optional<uint8>(7));
        REQUIRE(b.Cast.RemovedModifications.size() == 1);
        CHECK_FALSE(b.Cast.RemovedModifications[0].Unknown1000.has_value());
        CHECK(b.Cast.CraftingOrderId == a.Cast.CraftingOrderId);
        REQUIRE(b.Cast.Weight.size() == 1);
        CHECK(b.Cast.Weight[0].Type == 2);
        CHECK(b.Cast.Weight[0].Id == 55);
        CHECK(b.Cast.Weight[0].Quantity == 6);
        REQUIRE(b.Cast.MoveUpdate.has_value());
        CHECK(b.Cast.MoveUpdate->X == 10.0f);
        CheckTarget(a.Cast.Target, b.Cast.Target);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CastSpell rejects more reagents than the request allows", "[packets]")
{
    CastSpell packet;
    packet.Cast.OptionalReagents.assign(SpellCastRequest::MAX_OPTIONAL_REAGENTS + 1, SpellCraftingReagent{});
    ByteBuffer data;
    packet.Write(data);
    CastSpell in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("CancelCast round-trips", "[packets]")
{
    CancelCast packet;
    packet.CastId = { 3, 0xBC00000000000000ULL };
    packet.SpellId = 133;
    RoundTrip(packet, [](CancelCast const& a, CancelCast const& b)
    {
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CancelChannelling round-trips", "[packets]")
{
    CancelChannelling packet;
    packet.ChannelSpell = 689;
    packet.Reason = 40;
    RoundTrip(packet, [](CancelChannelling const& a, CancelChannelling const& b)
    {
        CHECK(a.ChannelSpell == b.ChannelSpell);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CancelAutoRepeatSpell is an empty body", "[packets]")
{
    ByteBuffer data;
    CancelAutoRepeatSpell{}.Write(data);
    CHECK(data.empty());
    CancelAutoRepeatSpell in;
    CHECK(in.Read(data));
}

TEST_CASE("CancelQueuedSpell is an empty body", "[packets]")
{
    ByteBuffer data;
    CancelQueuedSpell{}.Write(data);
    CHECK(data.empty());
    CancelQueuedSpell in;
    CHECK(in.Read(data));
}

TEST_CASE("CancelMountAura is an empty body", "[packets]")
{
    ByteBuffer data;
    CancelMountAura{}.Write(data);
    CHECK(data.empty());
    CancelMountAura in;
    CHECK(in.Read(data));
}

TEST_CASE("TotemDestroyed round-trips", "[packets]")
{
    TotemDestroyed packet;
    packet.Slot = 2;
    packet.TotemGuid = { 0x5678, 0x2000000000000000ULL };
    RoundTrip(packet, [](TotemDestroyed const& a, TotemDestroyed const& b)
    {
        CHECK(a.Slot == b.Slot);
        CHECK(a.TotemGuid == b.TotemGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellClick round-trips", "[packets]")
{
    SpellClick packet;
    packet.TargetGuid = { 0x1234, 0x2000000000000000ULL };
    packet.TryAutoDismount = true;
    RoundTrip(packet, [](SpellClick const& a, SpellClick const& b)
    {
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.TryAutoDismount == b.TryAutoDismount);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellPrepare round-trips", "[packets]")
{
    SpellPrepare packet;
    packet.ClientCastId = { 1, 0xBC00000000000000ULL };
    packet.ServerCastId = { 2, 0xBC00000000000000ULL };
    RoundTrip(packet, [](SpellPrepare const& a, SpellPrepare const& b)
    {
        CHECK(a.ClientCastId == b.ClientCastId);
        CHECK(a.ServerCastId == b.ServerCastId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellStart round-trips", "[packets]")
{
    SpellStart packet;
    packet.Cast = SampleCastData();
    packet.Cast.RemainingPower = { { 1, 40 } };
    RoundTrip(packet, [](SpellStart const& a, SpellStart const& b)
    {
        CheckCastData(a.Cast, b.Cast);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellGo round-trips with hits, misses, a reflect and ammo", "[packets]")
{
    SpellGo packet;
    packet.Cast = SampleCastData();
    packet.Cast.MissTargets = { { 6, 0x0800000000000000ULL }, { 7, 0x0800000000000000ULL } };
    packet.Cast.MissStatus = { { 1, 0 }, { CLASSIC_SPELL_MISS_REFLECT, 3 } };
    packet.Cast.AmmoDisplayId = 5996;
    RoundTrip(packet, [](SpellGo const& a, SpellGo const& b)
    {
        CheckCastData(a.Cast, b.Cast);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellGo with advanced combat log data is rejected", "[packets]")
{
    ByteBuffer data;
    SpellGo packet;
    packet.Cast = SampleCastData();
    packet.Cast.Write(data);
    data.WriteBit(true);
    data.FlushBits();
    SpellGo in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("CastFailed round-trips and defaults its arguments to none", "[packets]")
{
    CastFailed packet;
    packet.CastId = { 4, 0xBC00000000000000ULL };
    packet.SpellId = 133;
    packet.Visual.SpellXSpellVisualId = 9;
    packet.Reason = 85;
    CHECK(packet.FailedArg1 == -1);
    CHECK(packet.FailedArg2 == -1);
    RoundTrip(packet, [](CastFailed const& a, CastFailed const& b)
    {
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Visual.SpellXSpellVisualId == b.Visual.SpellXSpellVisualId);
        CHECK(a.Reason == b.Reason);
        CHECK(a.FailedArg1 == b.FailedArg1);
        CHECK(a.FailedArg2 == b.FailedArg2);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellFailure and SpellFailedOther round-trip", "[packets]")
{
    SpellFailure failure;
    failure.CasterUnit = { 5, 0x0800000000000000ULL };
    failure.CastId = { 6, 0xBC00000000000000ULL };
    failure.SpellId = 133;
    failure.Reason = 34;
    RoundTrip(failure, [](SpellFailure const& a, SpellFailure const& b)
    {
        CHECK(a.CasterUnit == b.CasterUnit);
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(failure);

    SpellFailedOther other;
    other.CasterUnit = failure.CasterUnit;
    other.CastId = failure.CastId;
    other.SpellId = 133;
    other.Reason = 34;
    RoundTrip(other, [](SpellFailedOther const& a, SpellFailedOther const& b)
    {
        CHECK(a.CasterUnit == b.CasterUnit);
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(other);
}

TEST_CASE("SpellDelayed round-trips", "[packets]")
{
    SpellDelayed packet;
    packet.Caster = { 5, 0x0800000000000000ULL };
    packet.ActualDelay = 500;
    RoundTrip(packet, [](SpellDelayed const& a, SpellDelayed const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.ActualDelay == b.ActualDelay);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellChannelStart round-trips with and without its optional parts", "[packets]")
{
    SpellChannelStart plain;
    plain.CasterGuid = { 5, 0x0800000000000000ULL };
    plain.SpellId = 689;
    plain.ChannelDuration = 5000;
    RoundTrip(plain, [](SpellChannelStart const& a, SpellChannelStart const& b)
    {
        CHECK(a.CasterGuid == b.CasterGuid);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.ChannelDuration == b.ChannelDuration);
        CHECK_FALSE(b.Interrupts.has_value());
        CHECK_FALSE(b.HealPrediction.has_value());
    });
    RejectsEveryTruncation(plain);

    SpellChannelStart full = plain;
    full.Interrupts = SpellChannelStart::InterruptImmunities{ 4, 8 };
    full.HealPrediction = SpellChannelStart::TargetedHealPrediction{ { 9, 0x0800000000000000ULL }, { 100, 1, {} } };
    RoundTrip(full, [](SpellChannelStart const&, SpellChannelStart const& b)
    {
        REQUIRE(b.Interrupts.has_value());
        CHECK(b.Interrupts->Immunities == 8);
        REQUIRE(b.HealPrediction.has_value());
        CHECK(b.HealPrediction->TargetGuid == ClassicGuid{ 9, 0x0800000000000000ULL });
        CHECK(b.HealPrediction->Predict.Points == 100);
    });
    RejectsEveryTruncation(full);
}

TEST_CASE("SpellChannelUpdate round-trips", "[packets]")
{
    SpellChannelUpdate packet;
    packet.CasterGuid = { 5, 0x0800000000000000ULL };
    packet.TimeRemaining = 2500;
    RoundTrip(packet, [](SpellChannelUpdate const& a, SpellChannelUpdate const& b)
    {
        CHECK(a.CasterGuid == b.CasterGuid);
        CHECK(a.TimeRemaining == b.TimeRemaining);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("A server cast result maps onto the modern reason by name", "[packets]")
{
    CHECK(ToClassicCastResult(0) == 1);       // SPELL_FAILED_AFFECTING_COMBAT
    CHECK(ToClassicCastResult(0xFF) == 0);    // SPELL_CAST_OK
    CHECK(ToClassicCastResult(9999) == CLASSIC_CAST_FAILED_UNKNOWN);
}
