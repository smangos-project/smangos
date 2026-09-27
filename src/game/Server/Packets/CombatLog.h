#ifndef MANGOS_PACKETS_COMBATLOG_H
#define MANGOS_PACKETS_COMBATLOG_H

// The modern combat log wire: spell damage, heals, misses, dispels, kills and experience. Light like
// Packets/CombatMessages.h, so RealBots can include it. Only the basic log form is written; the
// advanced-combat-log block and content-tuning block are never sent, and a body that sets either
// bit is rejected on read.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/SpellCast.h"
#include <vector>

inline bool CombatLogHasBytes(ByteBuffer const& data, size_t bytes)
{
    return data.size() - data.rpos() >= bytes;
}

struct SpellNonMeleeDamageLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_NON_MELEE_DAMAGE_LOG;

    ClassicGuid Victim;
    ClassicGuid Caster;
    ClassicGuid CastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    int32 Damage = 0;
    int32 OriginalDamage = 0;
    int32 Overkill = -1;
    uint8 SchoolMask = 0;
    int32 Absorbed = 0;
    int32 Resisted = 0;
    int32 ShieldBlock = 0;
    bool Periodic = false;
    uint8 Flags = 0; // SpellHitType bits, 7 on the wire

    void Write(ByteBuffer& data) const
    {
        Victim.Write(data);
        Caster.Write(data);
        CastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << Damage << OriginalDamage << Overkill << SchoolMask << Absorbed << Resisted << ShieldBlock;
        data.WriteBit(Periodic);
        data.WriteBits(Flags, 7);
        data.WriteBit(false); // debug info
        data.WriteBit(false); // advanced log
        data.WriteBit(false); // content tuning
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Victim.Read(data) || !Caster.Read(data) || !CastId.Read(data))
            return false;
        if (!CombatLogHasBytes(data, 2 * sizeof(int32) + 3 * sizeof(int32) + sizeof(uint8) + 3 * sizeof(int32) + 2))
            return false;
        data >> SpellId;
        Visual.Read(data);
        data >> Damage >> OriginalDamage >> Overkill >> SchoolMask >> Absorbed >> Resisted >> ShieldBlock;
        Periodic = data.ReadBit();
        Flags = uint8(data.ReadBits(7));
        bool const hasDebug = data.ReadBit();
        bool const hasLogData = data.ReadBit();
        bool const hasContentTuning = data.ReadBit();
        data.ResetBitPos();
        return !hasDebug && !hasLogData && !hasContentTuning;
    }
};

struct EnvironmentalDamageLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENVIRONMENTAL_DAMAGE_LOG;

    ClassicGuid Victim;
    uint8 Type = 0;
    int32 Amount = 0;
    int32 Resisted = 0;
    int32 Absorbed = 0;

    void Write(ByteBuffer& data) const
    {
        Victim.Write(data);
        data << Type << Amount << Resisted << Absorbed;
        data.WriteBit(false); // advanced log
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Victim.Read(data) || !CombatLogHasBytes(data, sizeof(uint8) + 3 * sizeof(int32) + 1))
            return false;
        data >> Type >> Amount >> Resisted >> Absorbed;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        return !hasLogData;
    }
};

// The per-effect target lists of a spell's execute log. Heals and energizes have their own log
// messages, so an effect carries only what has no other home.
struct SpellLogPowerDrain
{
    ClassicGuid Victim;
    uint32 Points = 0;
    int8 PowerType = 0;
    float Amplitude = 0.0f;
};

struct SpellLogExtraAttacks
{
    ClassicGuid Victim;
    uint32 NumAttacks = 0;
};

struct SpellLogDurabilityDamage
{
    ClassicGuid Victim;
    int32 ItemId = 0;
    int32 Amount = 0;
};

struct SpellLogEffect
{
    int32 Effect = 0;
    std::vector<SpellLogPowerDrain> PowerDrainTargets;
    std::vector<SpellLogExtraAttacks> ExtraAttacksTargets;
    std::vector<SpellLogDurabilityDamage> DurabilityDamageTargets;
    std::vector<ClassicGuid> GenericVictimTargets;
    std::vector<int32> TradeSkillTargets;
    std::vector<int32> FeedPetTargets;
};

struct SpellExecuteLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_EXECUTE_LOG;

    ClassicGuid Caster;
    int32 SpellId = 0;
    std::vector<SpellLogEffect> Effects;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        data << SpellId << uint32(Effects.size());
        for (SpellLogEffect const& effect : Effects)
        {
            data << effect.Effect;
            data << uint32(effect.PowerDrainTargets.size());
            data << uint32(effect.ExtraAttacksTargets.size());
            data << uint32(effect.DurabilityDamageTargets.size());
            data << uint32(effect.GenericVictimTargets.size());
            data << uint32(effect.TradeSkillTargets.size());
            data << uint32(effect.FeedPetTargets.size());
            for (SpellLogPowerDrain const& drain : effect.PowerDrainTargets)
            {
                drain.Victim.Write(data);
                data << drain.Points << drain.PowerType << drain.Amplitude;
            }
            for (SpellLogExtraAttacks const& attacks : effect.ExtraAttacksTargets)
            {
                attacks.Victim.Write(data);
                data << attacks.NumAttacks;
            }
            for (SpellLogDurabilityDamage const& durability : effect.DurabilityDamageTargets)
            {
                durability.Victim.Write(data);
                data << durability.ItemId << durability.Amount;
            }
            for (ClassicGuid const& victim : effect.GenericVictimTargets)
                victim.Write(data);
            for (int32 itemId : effect.TradeSkillTargets)
                data << itemId;
            for (int32 itemId : effect.FeedPetTargets)
                data << itemId;
        }
        data.WriteBit(false); // advanced log
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(uint32)))
            return false;
        uint32 effectCount;
        data >> SpellId >> effectCount;

        Effects.clear();
        for (uint32 i = 0; i < effectCount; ++i)
        {
            if (!CombatLogHasBytes(data, 7 * sizeof(uint32)))
                return false;
            SpellLogEffect effect;
            uint32 counts[6];
            data >> effect.Effect;
            for (uint32& count : counts)
                data >> count;

            // Every entry is at least a byte, so a count past the remaining bytes is a lie
            for (uint32 count : counts)
                if (count > data.size() - data.rpos())
                    return false;

            effect.PowerDrainTargets.resize(counts[0]);
            for (SpellLogPowerDrain& drain : effect.PowerDrainTargets)
            {
                if (!drain.Victim.Read(data) || !CombatLogHasBytes(data, sizeof(uint32) + sizeof(int8) + sizeof(float)))
                    return false;
                data >> drain.Points >> drain.PowerType >> drain.Amplitude;
            }
            effect.ExtraAttacksTargets.resize(counts[1]);
            for (SpellLogExtraAttacks& attacks : effect.ExtraAttacksTargets)
            {
                if (!attacks.Victim.Read(data) || !CombatLogHasBytes(data, sizeof(uint32)))
                    return false;
                data >> attacks.NumAttacks;
            }
            effect.DurabilityDamageTargets.resize(counts[2]);
            for (SpellLogDurabilityDamage& durability : effect.DurabilityDamageTargets)
            {
                if (!durability.Victim.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(int32)))
                    return false;
                data >> durability.ItemId >> durability.Amount;
            }
            effect.GenericVictimTargets.resize(counts[3]);
            for (ClassicGuid& victim : effect.GenericVictimTargets)
                if (!victim.Read(data))
                    return false;
            effect.TradeSkillTargets.resize(counts[4]);
            for (int32& itemId : effect.TradeSkillTargets)
            {
                if (!CombatLogHasBytes(data, sizeof(int32)))
                    return false;
                data >> itemId;
            }
            effect.FeedPetTargets.resize(counts[5]);
            for (int32& itemId : effect.FeedPetTargets)
            {
                if (!CombatLogHasBytes(data, sizeof(int32)))
                    return false;
                data >> itemId;
            }
            Effects.push_back(std::move(effect));
        }

        if (!CombatLogHasBytes(data, 1))
            return false;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        return !hasLogData;
    }
};

struct SpellHealLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_HEAL_LOG;

    ClassicGuid Target;
    ClassicGuid Caster;
    int32 SpellId = 0;
    int32 Health = 0;
    int32 OriginalHeal = 0;
    int32 OverHeal = 0;
    int32 Absorbed = 0;
    bool Crit = false;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        Caster.Write(data);
        data << SpellId << Health << OriginalHeal << OverHeal << Absorbed;
        data << uint32(0); // supporters
        data.WriteBit(Crit);
        data.WriteBit(false); // crit roll made
        data.WriteBit(false); // crit roll needed
        data.WriteBit(false); // advanced log
        data.WriteBit(false); // content tuning
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data) || !Caster.Read(data) || !CombatLogHasBytes(data, 6 * sizeof(int32) + 1))
            return false;
        uint32 supporters;
        data >> SpellId >> Health >> OriginalHeal >> OverHeal >> Absorbed >> supporters;
        if (supporters)
            return false;
        Crit = data.ReadBit();
        bool const hasRollMade = data.ReadBit();
        bool const hasRollNeeded = data.ReadBit();
        bool const hasLogData = data.ReadBit();
        bool const hasContentTuning = data.ReadBit();
        data.ResetBitPos();
        return !hasRollMade && !hasRollNeeded && !hasLogData && !hasContentTuning;
    }
};

struct SpellEnergizeLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_ENERGIZE_LOG;

    ClassicGuid Target;
    ClassicGuid Caster;
    int32 SpellId = 0;
    int8 Type = 0;
    int32 Amount = 0;
    int32 OverEnergize = 0;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        Caster.Write(data);
        data << SpellId << Type << Amount << OverEnergize;
        data.WriteBit(false); // advanced log
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data) || !Caster.Read(data) || !CombatLogHasBytes(data, sizeof(int32) + sizeof(int8) + 2 * sizeof(int32) + 1))
            return false;
        data >> SpellId >> Type >> Amount >> OverEnergize;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        return !hasLogData;
    }
};

struct SpellPeriodicAuraLogEffect
{
    int32 Effect = 0; // aura type
    int32 Amount = 0;
    int32 OriginalDamage = 0;
    int32 OverHealOrKill = 0;
    int32 SchoolMaskOrPower = 0;
    int32 AbsorbedOrAmplitude = 0;
    int32 Resisted = 0;
    bool Crit = false;
};

struct SpellPeriodicAuraLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_PERIODIC_AURA_LOG;

    ClassicGuid Target;
    ClassicGuid Caster;
    int32 SpellId = 0;
    std::vector<SpellPeriodicAuraLogEffect> Effects;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        Caster.Write(data);
        data << SpellId << uint32(Effects.size());
        data.WriteBit(false); // advanced log
        data.FlushBits();
        for (SpellPeriodicAuraLogEffect const& effect : Effects)
        {
            data << effect.Effect << effect.Amount << effect.OriginalDamage << effect.OverHealOrKill
                 << effect.SchoolMaskOrPower << effect.AbsorbedOrAmplitude << effect.Resisted;
            data << uint32(0); // supporters
            data.WriteBit(effect.Crit);
            data.WriteBit(false); // debug info
            data.WriteBit(false); // content tuning
            data.FlushBits();
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data) || !Caster.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(uint32) + 1))
            return false;
        uint32 effectCount;
        data >> SpellId >> effectCount;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        if (hasLogData)
            return false;

        Effects.clear();
        for (uint32 i = 0; i < effectCount; ++i)
        {
            if (!CombatLogHasBytes(data, 8 * sizeof(int32) + 1))
                return false;
            SpellPeriodicAuraLogEffect effect;
            uint32 supporters;
            data >> effect.Effect >> effect.Amount >> effect.OriginalDamage >> effect.OverHealOrKill
                 >> effect.SchoolMaskOrPower >> effect.AbsorbedOrAmplitude >> effect.Resisted >> supporters;
            if (supporters)
                return false;
            effect.Crit = data.ReadBit();
            bool const hasDebug = data.ReadBit();
            bool const hasContentTuning = data.ReadBit();
            data.ResetBitPos();
            if (hasDebug || hasContentTuning)
                return false;
            Effects.push_back(effect);
        }
        return true;
    }
};

struct SpellInterruptLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_INTERRUPT_LOG;

    ClassicGuid Caster;
    ClassicGuid Victim;
    int32 InterruptedSpellId = 0;
    int32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        Victim.Write(data);
        data << InterruptedSpellId << SpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data) || !Victim.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(int32)))
            return false;
        data >> InterruptedSpellId >> SpellId;
        return true;
    }
};

struct SpellLogMissEntry
{
    ClassicGuid Victim;
    uint8 MissReason = 0;
};

struct SpellMissLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_MISS_LOG;

    int32 SpellId = 0;
    ClassicGuid Caster;
    std::vector<SpellLogMissEntry> Entries;

    void Write(ByteBuffer& data) const
    {
        data << SpellId;
        Caster.Write(data);
        data << uint32(Entries.size());
        for (SpellLogMissEntry const& entry : Entries)
        {
            entry.Victim.Write(data);
            data << entry.MissReason;
            data.WriteBit(false); // debug info
            data.FlushBits();
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!CombatLogHasBytes(data, sizeof(int32)))
            return false;
        data >> SpellId;
        if (!Caster.Read(data) || !CombatLogHasBytes(data, sizeof(uint32)))
            return false;
        uint32 count;
        data >> count;

        Entries.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            SpellLogMissEntry entry;
            if (!entry.Victim.Read(data) || !CombatLogHasBytes(data, sizeof(uint8) + 1))
                return false;
            data >> entry.MissReason;
            bool const hasDebug = data.ReadBit();
            data.ResetBitPos();
            if (hasDebug)
                return false;
            Entries.push_back(entry);
        }
        return true;
    }
};

struct ProcResist
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PROC_RESIST;

    ClassicGuid Caster;
    ClassicGuid Target;
    int32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        Target.Write(data);
        data << SpellId;
        data.WriteBit(false); // roll
        data.WriteBit(false); // needed
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data) || !Target.Read(data) || !CombatLogHasBytes(data, sizeof(int32) + 1))
            return false;
        data >> SpellId;
        bool const hasRolled = data.ReadBit();
        bool const hasNeeded = data.ReadBit();
        data.ResetBitPos();
        return !hasRolled && !hasNeeded;
    }
};

struct SpellOrDamageImmune
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_OR_DAMAGE_IMMUNE;

    ClassicGuid Caster;
    ClassicGuid Victim;
    uint32 SpellId = 0;
    bool IsPeriodic = false;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        Victim.Write(data);
        data << SpellId;
        data.WriteBit(IsPeriodic);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data) || !Victim.Read(data) || !CombatLogHasBytes(data, sizeof(uint32) + 1))
            return false;
        data >> SpellId;
        IsPeriodic = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct SpellDispellData
{
    int32 SpellId = 0;
    bool Harmful = false;
};

struct SpellDispellLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_DISPELL_LOG;

    bool IsSteal = false;
    bool IsBreak = false;
    ClassicGuid Target;
    ClassicGuid Caster;
    int32 DispelledBySpellId = 0;
    std::vector<SpellDispellData> Dispelled;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(IsSteal);
        data.WriteBit(IsBreak);
        data.FlushBits();
        Target.Write(data);
        Caster.Write(data);
        data << DispelledBySpellId << uint32(Dispelled.size());
        for (SpellDispellData const& entry : Dispelled)
        {
            data << entry.SpellId;
            data.WriteBit(entry.Harmful);
            data.WriteBit(false); // rolled
            data.WriteBit(false); // needed
            data.FlushBits();
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!CombatLogHasBytes(data, 1))
            return false;
        IsSteal = data.ReadBit();
        IsBreak = data.ReadBit();
        data.ResetBitPos();
        if (!Target.Read(data) || !Caster.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(uint32)))
            return false;
        uint32 count;
        data >> DispelledBySpellId >> count;

        Dispelled.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            if (!CombatLogHasBytes(data, sizeof(int32) + 1))
                return false;
            SpellDispellData entry;
            data >> entry.SpellId;
            entry.Harmful = data.ReadBit();
            bool const hasRolled = data.ReadBit();
            bool const hasNeeded = data.ReadBit();
            data.ResetBitPos();
            if (hasRolled || hasNeeded)
                return false;
            Dispelled.push_back(entry);
        }
        return true;
    }
};

struct DispelFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DISPEL_FAILED;

    ClassicGuid Caster;
    ClassicGuid Victim;
    uint32 SpellId = 0;
    std::vector<uint32> FailedSpells;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        Victim.Write(data);
        data << SpellId << uint32(FailedSpells.size());
        for (uint32 spellId : FailedSpells)
            data << spellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data) || !Victim.Read(data) || !CombatLogHasBytes(data, 2 * sizeof(uint32)))
            return false;
        uint32 count;
        data >> SpellId >> count;
        if (!CombatLogHasBytes(data, size_t(count) * sizeof(uint32)))
            return false;
        FailedSpells.resize(count);
        for (uint32& spellId : FailedSpells)
            data >> spellId;
        return true;
    }
};

struct SpellInstakillLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_INSTAKILL_LOG;

    ClassicGuid Target;
    ClassicGuid Caster;
    int32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        Caster.Write(data);
        data << SpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data) || !Caster.Read(data) || !CombatLogHasBytes(data, sizeof(int32)))
            return false;
        data >> SpellId;
        return true;
    }
};

struct SpellDamageShield
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_DAMAGE_SHIELD;

    ClassicGuid Attacker;
    ClassicGuid Defender;
    int32 SpellId = 0;
    int32 TotalDamage = 0;
    int32 OriginalDamage = 0;
    int32 Overkill = -1;
    int32 SchoolMask = 0;
    int32 Absorbed = 0;

    void Write(ByteBuffer& data) const
    {
        Attacker.Write(data);
        Defender.Write(data);
        data << SpellId << TotalDamage << OriginalDamage << Overkill << SchoolMask << Absorbed;
        data.WriteBit(false); // advanced log
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Attacker.Read(data) || !Defender.Read(data) || !CombatLogHasBytes(data, 6 * sizeof(int32) + 1))
            return false;
        data >> SpellId >> TotalDamage >> OriginalDamage >> Overkill >> SchoolMask >> Absorbed;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        return !hasLogData;
    }
};

struct PartyKillLog
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PARTY_KILL_LOG;

    ClassicGuid Killer;
    ClassicGuid Victim;

    void Write(ByteBuffer& data) const
    {
        Killer.Write(data);
        Victim.Write(data);
    }

    bool Read(ByteBuffer& data) { return Killer.Read(data) && Victim.Read(data); }
};

enum ClassicXpReason : uint8
{
    CLASSIC_XP_REASON_KILL    = 0,
    CLASSIC_XP_REASON_NO_KILL = 1,
};

struct LogXPGain
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOG_XP_GAIN;

    ClassicGuid Victim;
    int32 Original = 0;
    uint8 Reason = CLASSIC_XP_REASON_KILL;
    int32 Amount = 0;
    float GroupBonus = 1.0f;

    void Write(ByteBuffer& data) const
    {
        Victim.Write(data);
        data << Original << Reason << Amount << GroupBonus;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Victim.Read(data) || !CombatLogHasBytes(data, sizeof(int32) + sizeof(uint8) + sizeof(int32) + sizeof(float)))
            return false;
        data >> Original >> Reason >> Amount >> GroupBonus;
        return true;
    }
};

struct ExplorationExperience
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_EXPLORATION_EXPERIENCE;

    int32 AreaId = 0;
    int32 Experience = 0;

    void Write(ByteBuffer& data) const { data << AreaId << Experience; }

    bool Read(ByteBuffer& data)
    {
        if (!CombatLogHasBytes(data, 2 * sizeof(int32)))
            return false;
        data >> AreaId >> Experience;
        return true;
    }
};

// The level-up toast: health/power/stat gains and, on retail, new talent points. Field order and
// the 10-power-slot array are per TrinityCore's LevelUpInfo::Write() (MiscPackets.cpp,
// MAX_POWERS_PER_CLASS); this engine only ever fills slot 0 (mana), matching vanilla's own single
// mana-gain field. NumNewTalents/NumNewPvpTalentSlots are always 0: this engine reports new talent
// points through the spellbook messages (SpellBook.h), not this toast, and there are no PvP
// talents in Classic.
struct LevelUpInfo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LEVEL_UP_INFO;

    static constexpr size_t PowerCount = 10;

    int32 Level = 0;
    int32 HealthGain = 0;
    int32 PowerGains[PowerCount] = {};
    int32 StatGains[5] = {};
    int32 NumNewTalents = 0;
    int32 NumNewPvpTalentSlots = 0;

    void Write(ByteBuffer& data) const
    {
        data << Level << HealthGain;
        for (int32 power : PowerGains)
            data << power;
        for (int32 stat : StatGains)
            data << stat;
        data << NumNewTalents << NumNewPvpTalentSlots;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CombatLogHasBytes(data, (2 + PowerCount + 5 + 2) * sizeof(int32)))
            return false;
        data >> Level >> HealthGain;
        for (int32& power : PowerGains)
            data >> power;
        for (int32& stat : StatGains)
            data >> stat;
        data >> NumNewTalents >> NumNewPvpTalentSlots;
        return true;
    }
};

#endif // MANGOS_PACKETS_COMBATLOG_H
