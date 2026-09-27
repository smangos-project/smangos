// Round trips for the modern combat log wire.
#include "Packets/CombatLog.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid const Player = { 5, 0x0800000000000000ULL };
ClassicGuid const Creature = { 9, 0x0B00000000000000ULL };
}

TEST_CASE("SpellNonMeleeDamageLog round-trips", "[packets]")
{
    SpellNonMeleeDamageLog packet;
    packet.Victim = Creature;
    packet.Caster = Player;
    packet.CastId = { 77, 0xBC00000000000000ULL };
    packet.SpellId = 133;
    packet.Visual.SpellXSpellVisualId = 4242;
    packet.Damage = 90;
    packet.OriginalDamage = 120;
    packet.Overkill = 12;
    packet.SchoolMask = 4;
    packet.Absorbed = 20;
    packet.Resisted = 10;
    packet.ShieldBlock = 3;
    packet.Periodic = true;
    packet.Flags = 0x0A;

    RoundTrip(packet, [](SpellNonMeleeDamageLog const& a, SpellNonMeleeDamageLog const& b)
    {
        CHECK(a.Victim == b.Victim);
        CHECK(a.Caster == b.Caster);
        CHECK(a.CastId == b.CastId);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Visual.SpellXSpellVisualId == b.Visual.SpellXSpellVisualId);
        CHECK(a.Damage == b.Damage);
        CHECK(a.OriginalDamage == b.OriginalDamage);
        CHECK(a.Overkill == b.Overkill);
        CHECK(a.SchoolMask == b.SchoolMask);
        CHECK(a.Absorbed == b.Absorbed);
        CHECK(a.Resisted == b.Resisted);
        CHECK(a.ShieldBlock == b.ShieldBlock);
        CHECK(a.Periodic == b.Periodic);
        CHECK(a.Flags == b.Flags);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellNonMeleeDamageLog rejects a body with a content tuning block", "[packets]")
{
    ByteBuffer data;
    SpellNonMeleeDamageLog packet;
    packet.Victim = Creature;
    packet.Caster = Player;
    packet.CastId = { 1, 0xBC00000000000000ULL };
    packet.Write(data);

    // Bit 10 of the trailing 11 bits is the content tuning flag
    REQUIRE(data.size() >= 2);
    data.put<uint8>(data.size() - 1, uint8(data.contents()[data.size() - 1] | 0x20));

    SpellNonMeleeDamageLog in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("EnvironmentalDamageLog round-trips", "[packets]")
{
    EnvironmentalDamageLog packet;
    packet.Victim = Player;
    packet.Type = 2;
    packet.Amount = 31;
    packet.Resisted = 4;
    packet.Absorbed = 5;
    RoundTrip(packet, [](EnvironmentalDamageLog const& a, EnvironmentalDamageLog const& b)
    {
        CHECK(a.Victim == b.Victim);
        CHECK(a.Type == b.Type);
        CHECK(a.Amount == b.Amount);
        CHECK(a.Resisted == b.Resisted);
        CHECK(a.Absorbed == b.Absorbed);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellExecuteLog round-trips every target list", "[packets]")
{
    SpellExecuteLog packet;
    packet.Caster = Player;
    packet.SpellId = 8921;

    SpellLogEffect drain;
    drain.Effect = 62;
    drain.PowerDrainTargets.push_back({ Creature, 45, 0, 1.5f });
    drain.ExtraAttacksTargets.push_back({ Creature, 2 });
    drain.DurabilityDamageTargets.push_back({ Player, 2589, -1 });
    drain.GenericVictimTargets = { Creature, Player };
    drain.TradeSkillTargets = { 2589 };
    drain.FeedPetTargets = { 117, 118 };
    SpellLogEffect plain;
    plain.Effect = 3;
    packet.Effects = { drain, plain };

    RoundTrip(packet, [](SpellExecuteLog const& a, SpellExecuteLog const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.SpellId == b.SpellId);
        REQUIRE(b.Effects.size() == 2);
        SpellLogEffect const& x = a.Effects[0];
        SpellLogEffect const& y = b.Effects[0];
        CHECK(x.Effect == y.Effect);
        REQUIRE(y.PowerDrainTargets.size() == 1);
        CHECK(x.PowerDrainTargets[0].Victim == y.PowerDrainTargets[0].Victim);
        CHECK(x.PowerDrainTargets[0].Points == y.PowerDrainTargets[0].Points);
        CHECK(x.PowerDrainTargets[0].PowerType == y.PowerDrainTargets[0].PowerType);
        CHECK(x.PowerDrainTargets[0].Amplitude == y.PowerDrainTargets[0].Amplitude);
        REQUIRE(y.ExtraAttacksTargets.size() == 1);
        CHECK(y.ExtraAttacksTargets[0].NumAttacks == 2);
        REQUIRE(y.DurabilityDamageTargets.size() == 1);
        CHECK(y.DurabilityDamageTargets[0].ItemId == 2589);
        CHECK(y.DurabilityDamageTargets[0].Amount == -1);
        CHECK(y.GenericVictimTargets == x.GenericVictimTargets);
        CHECK(y.TradeSkillTargets == x.TradeSkillTargets);
        CHECK(y.FeedPetTargets == x.FeedPetTargets);
        CHECK(b.Effects[1].Effect == 3);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellExecuteLog rejects a target count larger than its body", "[packets]")
{
    SpellExecuteLog packet;
    packet.Caster = Player;
    SpellLogEffect effect;
    effect.Effect = 3;
    packet.Effects = { effect };

    ByteBuffer data;
    packet.Write(data);
    // The effect's first target count sits after the caster guid, spell id, effect count and effect id
    ByteBuffer guid;
    Player.Write(guid);
    data.put<uint32>(guid.size() + 4 + 4 + 4, 0x7FFFFFFF);

    SpellExecuteLog in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("SpellHealLog round-trips", "[packets]")
{
    SpellHealLog packet;
    packet.Target = Player;
    packet.Caster = Creature;
    packet.SpellId = 2050;
    packet.Health = 140;
    packet.OriginalHeal = 150;
    packet.OverHeal = 10;
    packet.Absorbed = 0;
    packet.Crit = true;
    RoundTrip(packet, [](SpellHealLog const& a, SpellHealLog const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.Caster == b.Caster);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Health == b.Health);
        CHECK(a.OriginalHeal == b.OriginalHeal);
        CHECK(a.OverHeal == b.OverHeal);
        CHECK(a.Absorbed == b.Absorbed);
        CHECK(a.Crit == b.Crit);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellEnergizeLog round-trips", "[packets]")
{
    SpellEnergizeLog packet;
    packet.Target = Player;
    packet.Caster = Player;
    packet.SpellId = 29166;
    packet.Type = 0;
    packet.Amount = 300;
    packet.OverEnergize = 7;
    RoundTrip(packet, [](SpellEnergizeLog const& a, SpellEnergizeLog const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.Caster == b.Caster);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Type == b.Type);
        CHECK(a.Amount == b.Amount);
        CHECK(a.OverEnergize == b.OverEnergize);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellPeriodicAuraLog round-trips several effects", "[packets]")
{
    SpellPeriodicAuraLog packet;
    packet.Target = Creature;
    packet.Caster = Player;
    packet.SpellId = 172;

    SpellPeriodicAuraLogEffect damage;
    damage.Effect = 3;
    damage.Amount = 11;
    damage.OriginalDamage = 12;
    damage.SchoolMaskOrPower = 32;
    damage.AbsorbedOrAmplitude = 1;
    damage.Resisted = 2;
    damage.Crit = true;
    SpellPeriodicAuraLogEffect heal;
    heal.Effect = 8;
    heal.Amount = 20;
    packet.Effects = { damage, heal };

    RoundTrip(packet, [](SpellPeriodicAuraLog const& a, SpellPeriodicAuraLog const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.Caster == b.Caster);
        CHECK(a.SpellId == b.SpellId);
        REQUIRE(b.Effects.size() == 2);
        for (size_t i = 0; i < 2; ++i)
        {
            CHECK(a.Effects[i].Effect == b.Effects[i].Effect);
            CHECK(a.Effects[i].Amount == b.Effects[i].Amount);
            CHECK(a.Effects[i].OriginalDamage == b.Effects[i].OriginalDamage);
            CHECK(a.Effects[i].OverHealOrKill == b.Effects[i].OverHealOrKill);
            CHECK(a.Effects[i].SchoolMaskOrPower == b.Effects[i].SchoolMaskOrPower);
            CHECK(a.Effects[i].AbsorbedOrAmplitude == b.Effects[i].AbsorbedOrAmplitude);
            CHECK(a.Effects[i].Resisted == b.Effects[i].Resisted);
            CHECK(a.Effects[i].Crit == b.Effects[i].Crit);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellInterruptLog round-trips", "[packets]")
{
    SpellInterruptLog packet;
    packet.Caster = Player;
    packet.Victim = Creature;
    packet.InterruptedSpellId = 2139;
    packet.SpellId = 133;
    RoundTrip(packet, [](SpellInterruptLog const& a, SpellInterruptLog const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.Victim == b.Victim);
        CHECK(a.InterruptedSpellId == b.InterruptedSpellId);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SpellMissLog round-trips several targets", "[packets]")
{
    SpellMissLog packet;
    packet.SpellId = 133;
    packet.Caster = Player;
    packet.Entries = { { Creature, 1 }, { Player, 5 } };
    RoundTrip(packet, [](SpellMissLog const& a, SpellMissLog const& b)
    {
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.Caster == b.Caster);
        REQUIRE(b.Entries.size() == 2);
        for (size_t i = 0; i < 2; ++i)
        {
            CHECK(a.Entries[i].Victim == b.Entries[i].Victim);
            CHECK(a.Entries[i].MissReason == b.Entries[i].MissReason);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ProcResist and SpellOrDamageImmune round-trip", "[packets]")
{
    ProcResist resist;
    resist.Caster = Player;
    resist.Target = Creature;
    resist.SpellId = 133;
    RoundTrip(resist, [](ProcResist const& a, ProcResist const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.Target == b.Target);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(resist);

    SpellOrDamageImmune immune;
    immune.Caster = Creature;
    immune.Victim = Player;
    immune.SpellId = 116;
    immune.IsPeriodic = true;
    RoundTrip(immune, [](SpellOrDamageImmune const& a, SpellOrDamageImmune const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.Victim == b.Victim);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.IsPeriodic == b.IsPeriodic);
    });
    RejectsEveryTruncation(immune);
}

TEST_CASE("SpellDispellLog and DispelFailed round-trip", "[packets]")
{
    SpellDispellLog dispel;
    dispel.IsSteal = true;
    dispel.Target = Creature;
    dispel.Caster = Player;
    dispel.DispelledBySpellId = 527;
    dispel.Dispelled = { { 1459, false }, { 172, true } };
    RoundTrip(dispel, [](SpellDispellLog const& a, SpellDispellLog const& b)
    {
        CHECK(b.IsSteal);
        CHECK_FALSE(b.IsBreak);
        CHECK(a.Target == b.Target);
        CHECK(a.Caster == b.Caster);
        CHECK(a.DispelledBySpellId == b.DispelledBySpellId);
        REQUIRE(b.Dispelled.size() == 2);
        CHECK(b.Dispelled[0].SpellId == 1459);
        CHECK_FALSE(b.Dispelled[0].Harmful);
        CHECK(b.Dispelled[1].SpellId == 172);
        CHECK(b.Dispelled[1].Harmful);
    });
    RejectsEveryTruncation(dispel);

    DispelFailed failed;
    failed.Caster = Player;
    failed.Victim = Creature;
    failed.SpellId = 527;
    failed.FailedSpells = { 1459, 172 };
    RoundTrip(failed, [](DispelFailed const& a, DispelFailed const& b)
    {
        CHECK(a.Caster == b.Caster);
        CHECK(a.Victim == b.Victim);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.FailedSpells == b.FailedSpells);
    });
    RejectsEveryTruncation(failed);
}

TEST_CASE("SpellInstakillLog and SpellDamageShield round-trip", "[packets]")
{
    SpellInstakillLog instakill;
    instakill.Target = Creature;
    instakill.Caster = Player;
    instakill.SpellId = 5;
    RoundTrip(instakill, [](SpellInstakillLog const& a, SpellInstakillLog const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.Caster == b.Caster);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(instakill);

    SpellDamageShield shield;
    shield.Attacker = Creature;
    shield.Defender = Player;
    shield.SpellId = 7294;
    shield.TotalDamage = 8;
    shield.OriginalDamage = 9;
    shield.Overkill = 1;
    shield.SchoolMask = 2;
    shield.Absorbed = 1;
    RoundTrip(shield, [](SpellDamageShield const& a, SpellDamageShield const& b)
    {
        CHECK(a.Attacker == b.Attacker);
        CHECK(a.Defender == b.Defender);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.TotalDamage == b.TotalDamage);
        CHECK(a.OriginalDamage == b.OriginalDamage);
        CHECK(a.Overkill == b.Overkill);
        CHECK(a.SchoolMask == b.SchoolMask);
        CHECK(a.Absorbed == b.Absorbed);
    });
    RejectsEveryTruncation(shield);
}

TEST_CASE("PartyKillLog, LogXPGain and ExplorationExperience round-trip", "[packets]")
{
    PartyKillLog kill;
    kill.Killer = Player;
    kill.Victim = Creature;
    RoundTrip(kill, [](PartyKillLog const& a, PartyKillLog const& b)
    {
        CHECK(a.Killer == b.Killer);
        CHECK(a.Victim == b.Victim);
    });
    RejectsEveryTruncation(kill);

    LogXPGain xp;
    xp.Victim = Creature;
    xp.Original = 120;
    xp.Reason = CLASSIC_XP_REASON_KILL;
    xp.Amount = 100;
    xp.GroupBonus = 1.5f;
    RoundTrip(xp, [](LogXPGain const& a, LogXPGain const& b)
    {
        CHECK(a.Victim == b.Victim);
        CHECK(a.Original == b.Original);
        CHECK(a.Reason == b.Reason);
        CHECK(a.Amount == b.Amount);
        CHECK(a.GroupBonus == b.GroupBonus);
    });
    RejectsEveryTruncation(xp);

    ExplorationExperience exploration;
    exploration.AreaId = 12;
    exploration.Experience = 90;
    RoundTrip(exploration, [](ExplorationExperience const& a, ExplorationExperience const& b)
    {
        CHECK(a.AreaId == b.AreaId);
        CHECK(a.Experience == b.Experience);
    });
    RejectsEveryTruncation(exploration);
}

TEST_CASE("LevelUpInfo round-trips", "[packets]")
{
    LevelUpInfo packet;
    packet.Level = 11;
    packet.HealthGain = 42;
    packet.PowerGains[0] = 15; // mana
    packet.StatGains[0] = 2; // strength
    packet.StatGains[2] = 3; // stamina
    RoundTrip(packet, [](LevelUpInfo const& a, LevelUpInfo const& b)
    {
        CHECK(a.Level == b.Level);
        CHECK(a.HealthGain == b.HealthGain);
        for (size_t i = 0; i < LevelUpInfo::PowerCount; ++i)
            CHECK(a.PowerGains[i] == b.PowerGains[i]);
        for (size_t i = 0; i < 5; ++i)
            CHECK(a.StatGains[i] == b.StatGains[i]);
        CHECK(a.NumNewTalents == b.NumNewTalents);
        CHECK(a.NumNewPvpTalentSlots == b.NumNewPvpTalentSlots);
    });
    RejectsEveryTruncation(packet);
}
