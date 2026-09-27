// Round trips for the login-burst packets: the player's own state, spell book and zone state.
#include "Packets/PlayerSetup.h"
#include "Packets/SpellBook.h"
#include "Packets/WorldStates.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("BindPointUpdate round-trips", "[packets]")
{
    BindPointUpdate out;
    out.X = -8949.95f;
    out.Y = -132.49f;
    out.Z = 83.53f;
    out.MapId = 0;
    out.AreaId = 12;

    RoundTrip(out, [](BindPointUpdate const& a, BindPointUpdate const& b)
    {
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.MapId == b.MapId);
        CHECK(a.AreaId == b.AreaId);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("LoginSetTimeSpeed round-trips", "[packets]")
{
    LoginSetTimeSpeed out;
    out.ServerTime = 0x19A2B3C4;
    out.GameTime = 0x19A2B3C5;
    out.NewSpeed = 1.0f / 60.0f;
    out.ServerTimeHolidayOffset = 3;
    out.GameTimeHolidayOffset = 4;

    RoundTrip(out, [](LoginSetTimeSpeed const& a, LoginSetTimeSpeed const& b)
    {
        CHECK(a.ServerTime == b.ServerTime);
        CHECK(a.GameTime == b.GameTime);
        CHECK(a.NewSpeed == b.NewSpeed);
        CHECK(a.ServerTimeHolidayOffset == b.ServerTimeHolidayOffset);
        CHECK(a.GameTimeHolidayOffset == b.GameTimeHolidayOffset);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("TutorialFlags round-trips its eight words", "[packets]")
{
    TutorialFlags out;
    for (uint32 i = 0; i < out.Flags.size(); ++i)
        out.Flags[i] = 0x01010101u * (i + 1);

    ByteBuffer data;
    out.Write(data);
    CHECK(data.size() == 32);

    RoundTrip(out, [](TutorialFlags const& a, TutorialFlags const& b) { CHECK(a.Flags == b.Flags); });
    RejectsEveryTruncation(out);
}

TEST_CASE("SetProficiency writes the mask before the item class", "[packets]")
{
    SetProficiency out;
    out.SubclassMask = 0x00012345;
    out.ItemClass = 2;

    ByteBuffer data;
    out.Write(data);
    REQUIRE(data.size() == 5);
    CHECK(data.read<uint32>(0) == 0x00012345);
    CHECK(data[4] == 2);

    RoundTrip(out, [](SetProficiency const& a, SetProficiency const& b)
    {
        CHECK(a.SubclassMask == b.SubclassMask);
        CHECK(a.ItemClass == b.ItemClass);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("UpdateActionButtons is 180 64-bit buttons and a reason", "[packets]")
{
    UpdateActionButtons out;
    out.Buttons[0] = 6603;
    out.Buttons[11] = 0x80000000u | 159;
    out.Buttons[179] = 42;
    out.Reason = UpdateActionButtons::REASON_UPDATE;

    ByteBuffer data;
    out.Write(data);
    CHECK(data.size() == 180 * 8 + 1);

    RoundTrip(out, [](UpdateActionButtons const& a, UpdateActionButtons const& b)
    {
        CHECK(a.Buttons == b.Buttons);
        CHECK(a.Reason == b.Reason);
    });
}

TEST_CASE("InitializeFactions round-trips factions and bonuses", "[packets]")
{
    InitializeFactions out;
    out.Factions.push_back({ 72, 0x11, 3100 });
    out.Factions.push_back({ 47, 0x01, -2500 });
    out.Bonuses.push_back({ 72, false });
    out.Bonuses.push_back({ 47, true });

    RoundTrip(out, [](InitializeFactions const& a, InitializeFactions const& b)
    {
        REQUIRE(a.Factions.size() == b.Factions.size());
        for (size_t i = 0; i < a.Factions.size(); ++i)
        {
            CHECK(a.Factions[i].FactionId == b.Factions[i].FactionId);
            CHECK(a.Factions[i].Flags == b.Factions[i].Flags);
            CHECK(a.Factions[i].Standing == b.Factions[i].Standing);
        }
        REQUIRE(a.Bonuses.size() == b.Bonuses.size());
        for (size_t i = 0; i < a.Bonuses.size(); ++i)
        {
            CHECK(a.Bonuses[i].FactionId == b.Bonuses[i].FactionId);
            CHECK(a.Bonuses[i].HasBonus == b.Bonuses[i].HasBonus);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("InitializeFactions::Read rejects counts the body can't hold", "[packets]")
{
    ByteBuffer data;
    data << uint32(0x10000000) << uint32(0);

    InitializeFactions in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("AccountDataTimes round-trips", "[packets]")
{
    AccountDataTimes out;
    out.PlayerGuid.Low = 0x42;
    out.PlayerGuid.High = 0x0800000000000000ull;
    out.ServerTime = 1758800000;
    out.Times[0] = 1758700000;
    out.Times[CLASSIC_ACCOUNT_DATA_TYPES - 1] = 1758600000;

    RoundTrip(out, [](AccountDataTimes const& a, AccountDataTimes const& b)
    {
        CHECK(a.PlayerGuid.Low == b.PlayerGuid.Low);
        CHECK(a.PlayerGuid.High == b.PlayerGuid.High);
        CHECK(a.ServerTime == b.ServerTime);
        CHECK(a.Times == b.Times);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendKnownSpells round-trips", "[packets]")
{
    SendKnownSpells out;
    out.InitialLogin = true;
    out.KnownSpells = { 6603, 78, 2457, 107 };
    out.FavoriteSpells = { 2457 };

    RoundTrip(out, [](SendKnownSpells const& a, SendKnownSpells const& b)
    {
        CHECK(a.InitialLogin == b.InitialLogin);
        CHECK(a.KnownSpells == b.KnownSpells);
        CHECK(a.FavoriteSpells == b.FavoriteSpells);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendUnlearnSpells round-trips, empty and not", "[packets]")
{
    SendUnlearnSpells empty;
    RoundTrip(empty, [](SendUnlearnSpells const&, SendUnlearnSpells const& b) { CHECK(b.Spells.empty()); });

    SendUnlearnSpells out;
    out.Spells = { 1, 2, 3 };
    RoundTrip(out, [](SendUnlearnSpells const& a, SendUnlearnSpells const& b) { CHECK(a.Spells == b.Spells); });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendSpellHistory round-trips entries with and without the optional fields", "[packets]")
{
    SendSpellHistory out;
    SpellHistoryEntry plain;
    plain.SpellId = 20271;
    plain.Category = 0;
    plain.RecoveryTime = 5400;
    out.Entries.push_back(plain);

    SpellHistoryEntry full;
    full.SpellId = 1;
    full.ItemId = 2;
    full.Category = 3;
    full.RecoveryTime = -1;
    full.CategoryRecoveryTime = 60000;
    full.ModRate = 0.5f;
    full.OnHold = true;
    full.Unused1 = 7;
    full.Unused2 = 8;
    out.Entries.push_back(full);

    RoundTrip(out, [](SendSpellHistory const& a, SendSpellHistory const& b)
    {
        REQUIRE(a.Entries.size() == b.Entries.size());
        for (size_t i = 0; i < a.Entries.size(); ++i)
        {
            SpellHistoryEntry const& x = a.Entries[i];
            SpellHistoryEntry const& y = b.Entries[i];
            CHECK(x.SpellId == y.SpellId);
            CHECK(x.ItemId == y.ItemId);
            CHECK(x.Category == y.Category);
            CHECK(x.RecoveryTime == y.RecoveryTime);
            CHECK(x.CategoryRecoveryTime == y.CategoryRecoveryTime);
            CHECK(x.ModRate == y.ModRate);
            CHECK(x.OnHold == y.OnHold);
            CHECK(x.Unused1 == y.Unused1);
            CHECK(x.Unused2 == y.Unused2);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SpellCooldownUpdate round-trips", "[packets]")
{
    SpellCooldownUpdate out;
    out.Caster.Low = 0x42;
    out.Caster.High = 0x0800000000000000ull;
    out.Flags = SPELL_COOLDOWN_FLAG_ON_HOLD;
    out.Entries.push_back({ 6603, 1500, 1.0f });
    out.Entries.push_back({ 133, 0, 0.5f });

    RoundTrip(out, [](SpellCooldownUpdate const& a, SpellCooldownUpdate const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.Flags == b.Flags);
        REQUIRE(a.Entries.size() == b.Entries.size());
        for (size_t i = 0; i < a.Entries.size(); ++i)
        {
            CHECK(a.Entries[i].SpellId == b.Entries[i].SpellId);
            CHECK(a.Entries[i].ForcedCooldown == b.Entries[i].ForcedCooldown);
            CHECK(a.Entries[i].ModRate == b.Entries[i].ModRate);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClearSpellCooldown round-trips", "[packets]")
{
    ClearSpellCooldown out;
    out.SpellId = 6603;
    out.IsPet = true;
    out.ClearOnHold = true;

    RoundTrip(out, [](ClearSpellCooldown const& a, ClearSpellCooldown const& b)
    {
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.IsPet == b.IsPet);
        CHECK(a.ClearOnHold == b.ClearOnHold);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClearAllSpellCooldowns round-trips", "[packets]")
{
    ClearAllSpellCooldowns out;
    out.SpellIds = { 6603, 133, 585 };
    out.IsPet = true;

    RoundTrip(out, [](ClearAllSpellCooldowns const& a, ClearAllSpellCooldowns const& b)
    {
        CHECK(a.SpellIds == b.SpellIds);
        CHECK(a.IsPet == b.IsPet);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SpellCooldownEvent round-trips", "[packets]")
{
    SpellCooldownEvent out;
    out.SpellId = 6603;
    out.IsPet = true;

    RoundTrip(out, [](SpellCooldownEvent const& a, SpellCooldownEvent const& b)
    {
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.IsPet == b.IsPet);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SetFlatSpellModifier round-trips one mod with several class-mask values", "[packets]")
{
    SetFlatSpellModifier out;
    SpellModifierData mod;
    mod.ModIndex = 3;
    mod.Values.push_back({ -1.0f, 0 });
    mod.Values.push_back({ 2.5f, 63 });
    out.Modifiers.push_back(mod);

    RoundTrip(out, [](SetFlatSpellModifier const& a, SetFlatSpellModifier const& b)
    {
        REQUIRE(a.Modifiers.size() == b.Modifiers.size());
        CHECK(a.Modifiers[0].ModIndex == b.Modifiers[0].ModIndex);
        REQUIRE(a.Modifiers[0].Values.size() == b.Modifiers[0].Values.size());
        for (size_t i = 0; i < a.Modifiers[0].Values.size(); ++i)
        {
            CHECK(a.Modifiers[0].Values[i].Value == b.Modifiers[0].Values[i].Value);
            CHECK(a.Modifiers[0].Values[i].ClassIndex == b.Modifiers[0].Values[i].ClassIndex);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SetPctSpellModifier round-trips, empty and not", "[packets]")
{
    SetPctSpellModifier empty;
    RoundTrip(empty, [](SetPctSpellModifier const&, SetPctSpellModifier const& b) { CHECK(b.Modifiers.empty()); });

    SetPctSpellModifier out;
    SpellModifierData mod;
    mod.ModIndex = 7;
    mod.Values.push_back({ 20.0f, 5 });
    out.Modifiers.push_back(mod);
    RoundTrip(out, [](SetPctSpellModifier const& a, SetPctSpellModifier const& b)
    {
        REQUIRE(a.Modifiers.size() == b.Modifiers.size());
        CHECK(a.Modifiers[0].ModIndex == b.Modifiers[0].ModIndex);
        REQUIRE(a.Modifiers[0].Values.size() == b.Modifiers[0].Values.size());
        CHECK(a.Modifiers[0].Values[0].Value == b.Modifiers[0].Values[0].Value);
        CHECK(a.Modifiers[0].Values[0].ClassIndex == b.Modifiers[0].Values[0].ClassIndex);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendLearnedSpells round-trips entries with and without the optional fields", "[packets]")
{
    SendLearnedSpells out;
    out.SpecializationID = 3;
    out.SuppressMessaging = true;

    LearnedSpellInfo plain;
    plain.SpellID = 6603;
    out.Spells.push_back(plain);

    LearnedSpellInfo superceding;
    superceding.SpellID = 7369;
    superceding.IsFavorite = true;
    superceding.Superceded = 6603;
    superceding.TraitDefinitionID = 42;
    out.Spells.push_back(superceding);

    RoundTrip(out, [](SendLearnedSpells const& a, SendLearnedSpells const& b)
    {
        CHECK(a.SpecializationID == b.SpecializationID);
        CHECK(a.SuppressMessaging == b.SuppressMessaging);
        REQUIRE(a.Spells.size() == b.Spells.size());
        for (size_t i = 0; i < a.Spells.size(); ++i)
        {
            CHECK(a.Spells[i].SpellID == b.Spells[i].SpellID);
            CHECK(a.Spells[i].IsFavorite == b.Spells[i].IsFavorite);
            CHECK(a.Spells[i].Superceded == b.Spells[i].Superceded);
            CHECK(a.Spells[i].TraitDefinitionID == b.Spells[i].TraitDefinitionID);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendSupercededSpells round-trips", "[packets]")
{
    SendSupercededSpells out;
    LearnedSpellInfo entry;
    entry.SpellID = 7369;
    entry.Superceded = 6603;
    out.Spells.push_back(entry);

    RoundTrip(out, [](SendSupercededSpells const& a, SendSupercededSpells const& b)
    {
        REQUIRE(a.Spells.size() == b.Spells.size());
        CHECK(a.Spells[0].SpellID == b.Spells[0].SpellID);
        CHECK(a.Spells[0].Superceded == b.Spells[0].Superceded);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("SendUnlearnedSpells round-trips", "[packets]")
{
    SendUnlearnedSpells out;
    out.SpellIds = { 6603, 78 };
    out.SuppressMessaging = true;

    RoundTrip(out, [](SendUnlearnedSpells const& a, SendUnlearnedSpells const& b)
    {
        CHECK(a.SpellIds == b.SpellIds);
        CHECK(a.SuppressMessaging == b.SuppressMessaging);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("InitWorldStates round-trips", "[packets]")
{
    InitWorldStates out;
    out.MapId = 0;
    out.AreaId = 12;
    out.SubareaId = 9;
    out.States.push_back({ 2322, 1 });
    out.States.push_back({ 2323, 5 });

    RoundTrip(out, [](InitWorldStates const& a, InitWorldStates const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.AreaId == b.AreaId);
        CHECK(a.SubareaId == b.SubareaId);
        REQUIRE(a.States.size() == b.States.size());
        for (size_t i = 0; i < a.States.size(); ++i)
        {
            CHECK(a.States[i].VariableId == b.States[i].VariableId);
            CHECK(a.States[i].Value == b.States[i].Value);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("WeatherUpdate round-trips both values of Abrupt", "[packets]")
{
    for (bool abrupt : { false, true })
    {
        WeatherUpdate out;
        out.WeatherId = 4;
        out.Intensity = 0.55f;
        out.Abrupt = abrupt;

        ByteBuffer data;
        out.Write(data);
        CHECK(data.size() == 9);

        RoundTrip(out, [](WeatherUpdate const& a, WeatherUpdate const& b)
        {
            CHECK(a.WeatherId == b.WeatherId);
            CHECK(a.Intensity == b.Intensity);
            CHECK(a.Abrupt == b.Abrupt);
        });
        RejectsEveryTruncation(out);
    }
}

TEST_CASE("CancelCombat is an empty body", "[packets]")
{
    ByteBuffer data;
    CancelCombat{}.Write(data);
    CHECK(data.empty());
    CancelCombat in;
    CHECK(in.Read(data));
}
