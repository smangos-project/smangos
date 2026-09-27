#ifndef MANGOS_PACKETS_AURAS_H
#define MANGOS_PACKETS_AURAS_H

// The modern aura wire: a unit's visible aura slots, and the request to cancel one. Light like
// Packets/SpellCast.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/SpellCast.h"
#include <optional>
#include <vector>

enum ClassicAuraFlags : uint16
{
    CLASSIC_AURA_FLAG_NONE       = 0x0000,
    CLASSIC_AURA_FLAG_NO_CASTER  = 0x0001,
    CLASSIC_AURA_FLAG_CANCELABLE = 0x0002,
    CLASSIC_AURA_FLAG_DURATION   = 0x0004,
    CLASSIC_AURA_FLAG_SCALABLE   = 0x0008,
    CLASSIC_AURA_FLAG_NEGATIVE   = 0x0010,
    CLASSIC_AURA_FLAG_POSITIVE   = 0x0100,
    CLASSIC_AURA_FLAG_PASSIVE    = 0x0200,
};

// The scaling fields (points, estimated points, time mod, content tuning) are never sent: no aura
// here scales, so the bits announcing them are always clear and a body that sets one is rejected.
struct AuraDataInfo
{
    ClassicGuid CastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    uint16 Flags = CLASSIC_AURA_FLAG_NONE;
    uint32 ActiveFlags = 0;
    uint16 CastLevel = 1;
    uint8 Applications = 1;
    int32 ContentTuningId = 0;
    std::optional<ClassicGuid> CastUnit;
    std::optional<uint32> Duration;
    std::optional<uint32> Remaining;

    void Write(ByteBuffer& data) const
    {
        CastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << Flags << ActiveFlags << CastLevel << Applications << ContentTuningId;
        data.WriteBit(CastUnit.has_value());
        data.WriteBit(Duration.has_value());
        data.WriteBit(Remaining.has_value());
        data.WriteBit(false); // time mod
        data.WriteBits(0, 6); // points
        data.WriteBits(0, 6); // estimated points
        data.WriteBit(false); // content tuning
        data.FlushBits();
        if (CastUnit)
            CastUnit->Write(data);
        if (Duration)
            data << *Duration;
        if (Remaining)
            data << *Remaining;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint16) + sizeof(uint32) + sizeof(uint16) + sizeof(uint8) + sizeof(int32))
            return false;
        data >> SpellId;
        Visual.Read(data);
        data >> Flags >> ActiveFlags >> CastLevel >> Applications >> ContentTuningId;

        if (data.size() - data.rpos() < 3)
            return false;
        bool const hasCastUnit = data.ReadBit();
        bool const hasDuration = data.ReadBit();
        bool const hasRemaining = data.ReadBit();
        bool const hasTimeMod = data.ReadBit();
        uint32 const pointCount = data.ReadBits(6);
        uint32 const estimatedPointCount = data.ReadBits(6);
        bool const hasContentTuning = data.ReadBit();
        data.ResetBitPos();
        if (hasTimeMod || pointCount || estimatedPointCount || hasContentTuning)
            return false;

        CastUnit.reset();
        if (hasCastUnit)
        {
            ClassicGuid caster;
            if (!caster.Read(data))
                return false;
            CastUnit = caster;
        }

        Duration.reset();
        if (hasDuration)
        {
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            Duration = data.read<uint32>();
        }

        Remaining.reset();
        if (hasRemaining)
        {
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            Remaining = data.read<uint32>();
        }
        return true;
    }
};

// An aura data block that is absent means the slot's aura went away.
struct AuraInfo
{
    uint8 Slot = 0;
    std::optional<AuraDataInfo> AuraData;

    void Write(ByteBuffer& data) const
    {
        data << Slot;
        data.WriteBit(AuraData.has_value());
        data.FlushBits();
        if (AuraData)
            AuraData->Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8) + 1)
            return false;
        data >> Slot;
        bool const hasData = data.ReadBit();
        data.ResetBitPos();

        AuraData.reset();
        if (hasData)
        {
            AuraDataInfo info;
            if (!info.Read(data))
                return false;
            AuraData = info;
        }
        return true;
    }
};

// UpdateAll replaces everything the receiver knows about the unit's auras; otherwise each entry
// adds, changes or (with no data) removes one slot.
struct AuraUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AURA_UPDATE;

    bool UpdateAll = false;
    std::vector<AuraInfo> Auras;
    ClassicGuid UnitGuid;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(UpdateAll);
        data.WriteBits(uint32(Auras.size()), 9);
        for (AuraInfo const& aura : Auras)
            aura.Write(data);
        UnitGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        try
        {
            UpdateAll = data.ReadBit();
            uint32 const count = data.ReadBits(9);
            data.ResetBitPos();

            Auras.assign(count, AuraInfo{});
            for (AuraInfo& aura : Auras)
                if (!aura.Read(data))
                    return false;
            return UnitGuid.Read(data);
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }
};

struct CancelAura
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_AURA;

    int32 SpellId = 0;
    ClassicGuid CasterGuid;

    void Write(ByteBuffer& data) const
    {
        data << SpellId;
        CasterGuid.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> SpellId;
        return CasterGuid.Read(data);
    }
};

// The "growth" (levitate/hover-family) aura's client-side cancel button. vmangos never wired
// this up to anything (the aura is removed server-side); this port keeps that no-op.
struct CancelGrowthAura
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_GROWTH_AURA;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

#endif // MANGOS_PACKETS_AURAS_H
