#ifndef MANGOS_PACKETS_COMBATMESSAGES_H
#define MANGOS_PACKETS_COMBATMESSAGES_H

// The modern combat wire: target selection, melee and stand state. Light like Packets/TaxiMessages.h, so
// RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <vector>

struct SetSelection
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_SELECTION;

    ClassicGuid Target;

    void Write(ByteBuffer& data) const { Target.Write(data); }
    bool Read(ByteBuffer& data) { return Target.Read(data); }
};

struct AttackSwing
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ATTACK_SWING;

    ClassicGuid Victim;

    void Write(ByteBuffer& data) const { Victim.Write(data); }
    bool Read(ByteBuffer& data) { return Victim.Read(data); }
};

struct AttackStop
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ATTACK_STOP;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The five vanilla SMSG_ATTACKSWING_* opcodes (one per reason) collapse into one 3-bit reason code.
enum ClassicAttackSwingErr : uint8
{
    CLASSIC_ATTACKSWINGERR_NOT_IN_RANGE = 0,
    CLASSIC_ATTACKSWINGERR_BAD_FACING   = 1,
    CLASSIC_ATTACKSWINGERR_CANT_ATTACK  = 2,
    CLASSIC_ATTACKSWINGERR_DEAD_TARGET  = 3,
};

struct AttackSwingError
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ATTACK_SWING_ERROR;

    uint8 Reason = CLASSIC_ATTACKSWINGERR_NOT_IN_RANGE;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Reason, 3);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Reason = uint8(data.ReadBits(3));
        data.ResetBitPos();
        return true;
    }
};

struct AttackStart
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ATTACK_START;

    ClassicGuid Attacker;
    ClassicGuid Victim;

    void Write(ByteBuffer& data) const
    {
        Attacker.Write(data);
        Victim.Write(data);
    }

    bool Read(ByteBuffer& data) { return Attacker.Read(data) && Victim.Read(data); }
};

// The server's own attack-stop echo (the client -> server direction has no body at all: see
// AttackStop above).
struct SAttackStop
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ATTACK_STOP;

    ClassicGuid Attacker;
    ClassicGuid Victim;
    bool NowDead = false;

    void Write(ByteBuffer& data) const
    {
        Attacker.Write(data);
        Victim.Write(data);
        data.WriteBit(NowDead);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Attacker.Read(data) || !Victim.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        NowDead = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The client's sit/stand/kneel/sleep request. Only the low byte of the value is read.
struct StandStateChange
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_STAND_STATE_CHANGE;

    uint8 StandState = 0;

    void Write(ByteBuffer& data) const { data << StandState; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8))
            return false;
        data >> StandState;
        return true;
    }
};

struct StandStateUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_STAND_STATE_UPDATE;

    uint32 AnimKitId = 0;
    uint8 StandState = 0;

    void Write(ByteBuffer& data) const
    {
        data << AnimKitId;
        data << StandState;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint8))
            return false;
        data >> AnimKitId;
        data >> StandState;
        return true;
    }
};

struct MeleeStateSubDamage
{
    uint32 SchoolMask = 0;
    uint32 Damage = 0;
    uint32 Absorb = 0;
    int32 Resist = 0;
};

// The wire's HitInfo bits this struct's optional fields are gated on, in the modern client's bit
// positions (TrinityCore UnitDefines.h), not this engine's own 1.12 HitInfo enum -- HitInfoFlags is
// always filled by Unit::ToModernHitInfo before it reaches this struct. Duplicated here (rather
// than pulling in the game's own HitInfo enum) so this header stays includable by RealBots.
enum : uint32
{
    CLASSIC_HITINFO_ABSORB = 0x00000020,
    CLASSIC_HITINFO_RESIST = 0x00000080,
    CLASSIC_HITINFO_BLOCK  = 0x00002000,
};

// The melee swing result. Through the mandatory content-tuning tail, the body is a length-prefixed
// sub-blob on the wire, ahead of an advanced-combat-log bit/block this port never sets (only the
// basic combat log form is ever sent).
struct MeleeAttackingStateUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ATTACKER_STATE_UPDATE;

    uint32 HitInfoFlags = 0;
    ClassicGuid Attacker;
    ClassicGuid Victim;
    int32 TotalDamage = 0;
    int32 OriginalDamage = 0;
    int32 OverDamage = -1;
    std::vector<MeleeStateSubDamage> SubDamage;
    uint8 VictimState = 0;
    int32 AttackerState = 0;
    uint32 MeleeSpellId = 0;
    int32 BlockedAmount = 0;
    // No level-scaling system exists here; these are the "scaling disabled" defaults TrinityCore
    // itself falls back to, except Type/TargetLevel, which are cheap to fill for real.
    uint8 ContentTuningType = 0;
    uint8 ContentTuningTargetLevel = 0;

    void Write(ByteBuffer& data) const
    {
        ByteBuffer inner;
        inner << HitInfoFlags;
        Attacker.Write(inner);
        Victim.Write(inner);
        inner << TotalDamage;
        inner << OriginalDamage;
        inner << OverDamage;
        inner << uint8(SubDamage.size());
        for (MeleeStateSubDamage const& sub : SubDamage)
        {
            inner << sub.SchoolMask;
            inner << float(sub.Damage);
            inner << int32(sub.Damage);
            if (HitInfoFlags & CLASSIC_HITINFO_ABSORB)
                inner << int32(sub.Absorb);
            if (HitInfoFlags & CLASSIC_HITINFO_RESIST)
                inner << sub.Resist;
        }
        inner << VictimState;
        inner << AttackerState;
        inner << MeleeSpellId;
        if (HitInfoFlags & CLASSIC_HITINFO_BLOCK)
        {
            inner << BlockedAmount;
            inner << float(0.0f); // "Unk12" per client decompiles, no data source
        }
        inner << ContentTuningType;
        inner << ContentTuningTargetLevel;
        inner << uint8(0);    // ContentTuningExpansion
        inner << int16(0);    // ContentTuningPlayerLevelDelta
        inner << float(0.0f); // ContentTuningPlayerItemLevel
        inner << float(0.0f); // ContentTuningTargetItemLevel

        data.WriteBit(false); // has advanced combat log data
        data.FlushBits();
        data << uint32(inner.size());
        data.append(inner);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        if (hasLogData)
            return false; // this port never sends it; nothing to parse it against yet

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 innerSize;
        data >> innerSize;
        if (data.size() - data.rpos() < innerSize)
            return false;
        size_t const innerEnd = data.rpos() + innerSize;

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> HitInfoFlags;
        if (!Attacker.Read(data) || !Victim.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(int32) + 1)
            return false;
        data >> TotalDamage >> OriginalDamage >> OverDamage;
        uint8 subCount;
        data >> subCount;

        SubDamage.assign(subCount, MeleeStateSubDamage{});
        for (MeleeStateSubDamage& sub : SubDamage)
        {
            if (data.size() - data.rpos() < sizeof(uint32) + sizeof(float) + sizeof(int32))
                return false;
            float unusedFloatDamage;
            int32 damage;
            data >> sub.SchoolMask >> unusedFloatDamage >> damage;
            sub.Damage = uint32(damage);
            if (HitInfoFlags & CLASSIC_HITINFO_ABSORB)
            {
                if (data.size() - data.rpos() < sizeof(int32))
                    return false;
                int32 absorb;
                data >> absorb;
                sub.Absorb = uint32(absorb);
            }
            if (HitInfoFlags & CLASSIC_HITINFO_RESIST)
            {
                if (data.size() - data.rpos() < sizeof(int32))
                    return false;
                data >> sub.Resist;
            }
        }

        if (data.size() - data.rpos() < 1 + sizeof(int32) + sizeof(uint32))
            return false;
        data >> VictimState;
        data >> AttackerState;
        data >> MeleeSpellId;

        if (HitInfoFlags & CLASSIC_HITINFO_BLOCK)
        {
            if (data.size() - data.rpos() < sizeof(int32) + sizeof(float))
                return false;
            float unusedBlockFloat;
            data >> BlockedAmount >> unusedBlockFloat;
        }

        if (data.size() - data.rpos() < 2)
            return false;
        data >> ContentTuningType;
        data >> ContentTuningTargetLevel;

        if (data.size() < innerEnd)
            return false;
        data.rpos(innerEnd); // the rest of the content-tuning tail is a fixed hardcoded default, skipped
        return true;
    }
};

// A creature's reaction to a target (alert, hostile, ...); `Reaction` is the AiReaction value.
struct AIReactionUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AI_REACTION;

    ClassicGuid Unit;
    uint32 Reaction = 0;

    void Write(ByteBuffer& data) const
    {
        Unit.Write(data);
        data << Reaction;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Unit.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Reaction;
        return true;
    }
};

// Tells the client an auto-repeat spell (Auto Shot, Shoot) ended; names the target it was aimed at.
struct CancelAutoRepeat
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CANCEL_AUTO_REPEAT;

    ClassicGuid Target;

    void Write(ByteBuffer& data) const { Target.Write(data); }
    bool Read(ByteBuffer& data) { return Target.Read(data); }
};

// The sheathe/unsheathe weapon button. Replaces the vanilla CMSG_SETSHEATHED body (a plain
// uint32), which this client never sends.
struct SetSheathed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SET_SHEATHED;

    int32 CurrentSheathState = 0;
    bool Animate = true;

    void Write(ByteBuffer& data) const
    {
        data << CurrentSheathState;
        data.WriteBit(Animate);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> CurrentSheathState;
        Animate = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

#endif // MANGOS_PACKETS_COMBATMESSAGES_H
