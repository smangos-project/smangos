// Round trips for the modern pet wire.
#include "Packets/PetMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid PetGuid(uint64 low) { return { low, 0x3000000000000000ULL }; }
}

TEST_CASE("Pet commands round-trip", "[packets]")
{
    QueryPetName query;
    query.Pet = PetGuid(5);
    RoundTrip(query, [](QueryPetName const& a, QueryPetName const& b) { CHECK(a.Pet == b.Pet); });
    RejectsEveryTruncation(query);

    RoundTrip(RequestPetInfo(), [](RequestPetInfo const&, RequestPetInfo const&) {});

    PetAction action;
    action.Pet = PetGuid(5);
    action.Action = 2 | (7u << 24);
    action.Target = PetGuid(9);
    action.Position[0] = 1.5f;
    action.Position[1] = -2.5f;
    action.Position[2] = 3.5f;
    RoundTrip(action, [](PetAction const& a, PetAction const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.Action == b.Action);
        CHECK(a.Target == b.Target);
        CHECK(a.Position[0] == b.Position[0]);
        CHECK(a.Position[1] == b.Position[1]);
        CHECK(a.Position[2] == b.Position[2]);
    });
    RejectsEveryTruncation(action);

    PetStopAttack stop;
    stop.Pet = PetGuid(5);
    RoundTrip(stop, [](PetStopAttack const& a, PetStopAttack const& b) { CHECK(a.Pet == b.Pet); });
    RejectsEveryTruncation(stop);

    PetAbandon abandon;
    abandon.Pet = PetGuid(5);
    RoundTrip(abandon, [](PetAbandon const& a, PetAbandon const& b) { CHECK(a.Pet == b.Pet); });
    RejectsEveryTruncation(abandon);

    PetSpellAutocast autocast;
    autocast.Pet = PetGuid(5);
    autocast.SpellId = 3110;
    autocast.Enabled = true;
    RoundTrip(autocast, [](PetSpellAutocast const& a, PetSpellAutocast const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.SpellId == b.SpellId);
        CHECK(b.Enabled);
    });
    RejectsEveryTruncation(autocast);

    PetSetAction setAction;
    setAction.Pet = PetGuid(5);
    setAction.Index = 3;
    setAction.Action = 3110 | (0xC1u << 24);
    RoundTrip(setAction, [](PetSetAction const& a, PetSetAction const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.Index == b.Index);
        CHECK(a.Action == b.Action);
    });
    RejectsEveryTruncation(setAction);

    PetCancelAura cancel;
    cancel.Pet = PetGuid(5);
    cancel.SpellId = 20600;
    RoundTrip(cancel, [](PetCancelAura const& a, PetCancelAura const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(cancel);
}

TEST_CASE("PetSetAction skips the optional trailing words", "[packets]")
{
    ByteBuffer data;
    PetGuid(5).Write(data);
    data << uint32(3) << uint32(7);
    data.WriteBit(true);
    data.FlushBits();
    data << uint32(11) << uint32(12);

    PetSetAction in;
    REQUIRE(in.Read(data));
    CHECK(in.Index == 3);
    CHECK(in.Action == 7);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("PetRename round-trips a name and drops declined names", "[packets]")
{
    PetRename rename;
    rename.Pet = PetGuid(5);
    rename.PetNumber = 42;
    rename.Name = "Fluffy";
    RoundTrip(rename, [](PetRename const& a, PetRename const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.PetNumber == b.PetNumber);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(rename);

    ByteBuffer data;
    PetGuid(5).Write(data);
    data << int32(42);
    data.WriteBits(6, 8);
    data.WriteBit(true);
    for (uint32 length : { 1u, 0u, 2u, 0u, 0u })
        data.WriteBits(length, 7);
    data.FlushBits();
    data.WriteString("a");
    data.WriteString("bc");
    data.WriteString("Fluffy");

    PetRename declined;
    REQUIRE(declined.Read(data));
    CHECK(declined.PetNumber == 42);
    CHECK(declined.Name == "Fluffy");
    CHECK(data.rpos() == data.size());
}

TEST_CASE("PetCastSpell round-trips the pet and the request", "[packets]")
{
    PetCastSpell cast;
    cast.Pet = PetGuid(5);
    cast.Cast.CastId = { 77, 0x1000000000000000ULL };
    cast.Cast.SpellId = 3110;
    cast.Cast.Target.Flags = 0x2;
    cast.Cast.Target.UnitGuid = PetGuid(9);
    RoundTrip(cast, [](PetCastSpell const& a, PetCastSpell const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.Cast.CastId == b.Cast.CastId);
        CHECK(a.Cast.SpellId == b.Cast.SpellId);
        CHECK(a.Cast.Target.Flags == b.Cast.Target.Flags);
        CHECK(a.Cast.Target.UnitGuid == b.Cast.Target.UnitGuid);
    });
    RejectsEveryTruncation(cast);
}

TEST_CASE("PetSpellsMessage round-trips the bar, spells and cooldowns", "[packets]")
{
    PetSpellsMessage spells;
    spells.Pet = PetGuid(5);
    spells.CreatureFamily = 23;
    spells.TimeLimit = 30000;
    spells.CommandState = 1;
    spells.Flag = 8;
    spells.ReactState = 2;
    for (uint32 i = 0; i < CLASSIC_PET_ACTION_BAR_SIZE; ++i)
        spells.ActionButtons[i] = i + 1;
    spells.Actions = { 3110u | (0xC1u << 24), 6307u | (0x01u << 24) };
    spells.Cooldowns.resize(2);
    spells.Cooldowns[0] = { 3110, 2000, 0, 0 };
    spells.Cooldowns[1] = { 6307, 5000, 8000, 3 };
    RoundTrip(spells, [](PetSpellsMessage const& a, PetSpellsMessage const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.CreatureFamily == b.CreatureFamily);
        CHECK(a.TimeLimit == b.TimeLimit);
        CHECK(a.CommandState == b.CommandState);
        CHECK(a.Flag == b.Flag);
        CHECK(a.ReactState == b.ReactState);
        CHECK(a.ActionButtons == b.ActionButtons);
        CHECK(a.Actions == b.Actions);
        REQUIRE(a.Cooldowns.size() == b.Cooldowns.size());
        for (size_t i = 0; i < a.Cooldowns.size(); ++i)
        {
            CHECK(a.Cooldowns[i].SpellId == b.Cooldowns[i].SpellId);
            CHECK(a.Cooldowns[i].Duration == b.Cooldowns[i].Duration);
            CHECK(a.Cooldowns[i].CategoryDuration == b.Cooldowns[i].CategoryDuration);
            CHECK(a.Cooldowns[i].Category == b.Cooldowns[i].Category);
        }
    });
    RejectsEveryTruncation(spells);

    RoundTrip(PetClearSpells(), [](PetClearSpells const&, PetClearSpells const&) {});
}

TEST_CASE("Pet feedback messages round-trip", "[packets]")
{
    PetMode mode;
    mode.Pet = PetGuid(5);
    mode.CommandState = 2;
    mode.Flag = 8;
    mode.ReactState = 1;
    RoundTrip(mode, [](PetMode const& a, PetMode const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(a.CommandState == b.CommandState);
        CHECK(a.Flag == b.Flag);
        CHECK(a.ReactState == b.ReactState);
    });
    RejectsEveryTruncation(mode);

    PetActionFeedback feedback;
    feedback.SpellId = 3110;
    feedback.Response = 2;
    RoundTrip(feedback, [](PetActionFeedback const& a, PetActionFeedback const& b)
    {
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Response == b.Response);
    });
    RejectsEveryTruncation(feedback);

    PetActionSound sound;
    sound.Unit = PetGuid(5);
    sound.Action = 3;
    RoundTrip(sound, [](PetActionSound const& a, PetActionSound const& b)
    {
        CHECK(a.Unit == b.Unit);
        CHECK(a.Action == b.Action);
    });
    RejectsEveryTruncation(sound);

    PetTameFailure tame;
    tame.Result = 4;
    RoundTrip(tame, [](PetTameFailure const& a, PetTameFailure const& b) { CHECK(a.Result == b.Result); });
    RejectsEveryTruncation(tame);

    PetCastFailed failed;
    failed.CastId = { 77, 0x1000000000000000ULL };
    failed.SpellId = 3110;
    failed.Reason = 20;
    RoundTrip(failed, [](PetCastFailed const& a, PetCastFailed const& b)
    {
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Reason == b.Reason);
        CHECK(a.FailedArg1 == b.FailedArg1);
        CHECK(a.FailedArg2 == b.FailedArg2);
    });
    RejectsEveryTruncation(failed);
}

TEST_CASE("Pet name messages round-trip", "[packets]")
{
    PetNameInvalid invalid;
    invalid.Result = 8;
    invalid.Pet = PetGuid(5);
    invalid.PetNumber = 42;
    invalid.Name = "Fluffy";
    RoundTrip(invalid, [](PetNameInvalid const& a, PetNameInvalid const& b)
    {
        CHECK(a.Result == b.Result);
        CHECK(a.Pet == b.Pet);
        CHECK(a.PetNumber == b.PetNumber);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(invalid);

    QueryPetNameResponse known;
    known.Pet = PetGuid(5);
    known.Allow = true;
    known.Name = "Fluffy";
    known.Timestamp = 1700000000;
    RoundTrip(known, [](QueryPetNameResponse const& a, QueryPetNameResponse const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK(b.Allow);
        CHECK(a.Name == b.Name);
        CHECK(a.Timestamp == b.Timestamp);
    });
    RejectsEveryTruncation(known);

    QueryPetNameResponse unknown;
    unknown.Pet = PetGuid(6);
    RoundTrip(unknown, [](QueryPetNameResponse const& a, QueryPetNameResponse const& b)
    {
        CHECK(a.Pet == b.Pet);
        CHECK_FALSE(b.Allow);
    });
    RejectsEveryTruncation(unknown);
}
