#ifndef MANGOS_PACKETS_SPELLCAST_H
#define MANGOS_PACKETS_SPELLCAST_H

// The modern casting wire: cast requests, cast start/go, failures, channels and delays. Light like
// Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/MovementMessages.h"
#include <optional>
#include <string>
#include <vector>

// The one visual id every cast message carries.
struct SpellCastVisual
{
    int32 SpellXSpellVisualId = 0;

    void Write(ByteBuffer& data) const { data << SpellXSpellVisualId; }
    void Read(ByteBuffer& data) { data >> SpellXSpellVisualId; }
};

struct MissileTrajectoryRequest
{
    float Pitch = 0.0f;
    float Speed = 0.0f;
};

struct TargetLocation
{
    ClassicGuid Transport;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Transport.Write(data);
        data << X << Y << Z;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Transport.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(float))
            return false;
        data >> X >> Y >> Z;
        return true;
    }
};

// The wire's SpellCastTargetFlags values this header's optional fields are gated on come from the
// flags word itself; the optionals below are gated by their own bits.
struct SpellTargetData
{
    uint32 Flags = 0;
    std::optional<TargetLocation> SrcLocation;
    std::optional<TargetLocation> DstLocation;
    std::optional<float> Orientation;
    std::optional<int32> MapId;
    ClassicGuid UnitGuid;
    ClassicGuid ItemGuid;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Flags, 28);
        data.WriteBit(SrcLocation.has_value());
        data.WriteBit(DstLocation.has_value());
        data.WriteBit(Orientation.has_value());
        data.WriteBit(MapId.has_value());
        data.WriteBits(uint32(Name.size()), 7);
        data.FlushBits();

        UnitGuid.Write(data);
        ItemGuid.Write(data);
        if (SrcLocation)
            SrcLocation->Write(data);
        if (DstLocation)
            DstLocation->Write(data);
        if (Orientation)
            data << *Orientation;
        if (MapId)
            data << *MapId;
        data.append(Name.data(), Name.size());
    }

    // Reads throw on truncation; callers that need a bool go through the packet's Read.
    void ReadUnchecked(ByteBuffer& data)
    {
        data.ResetBitPos();
        Flags = data.ReadBits(28);
        bool const hasSrc = data.ReadBit();
        bool const hasDst = data.ReadBit();
        bool const hasOrientation = data.ReadBit();
        bool const hasMapId = data.ReadBit();
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();

        if (!UnitGuid.Read(data) || !ItemGuid.Read(data))
            throw ByteBufferException(false, data.rpos(), 0, data.size());

        SrcLocation.reset();
        if (hasSrc)
        {
            TargetLocation location;
            if (!location.Read(data))
                throw ByteBufferException(false, data.rpos(), 0, data.size());
            SrcLocation = location;
        }

        DstLocation.reset();
        if (hasDst)
        {
            TargetLocation location;
            if (!location.Read(data))
                throw ByteBufferException(false, data.rpos(), 0, data.size());
            DstLocation = location;
        }

        Orientation.reset();
        if (hasOrientation)
            Orientation = data.read<float>();

        MapId.reset();
        if (hasMapId)
            MapId = data.read<int32>();

        Name = data.ReadString(nameLength);
    }
};

struct SpellExtraCurrencyCost
{
    int32 CurrencyId = 0;
    int32 Count = 0;
};

struct SpellCraftingReagent
{
    int32 ItemId = 0;
    int32 DataSlotIndex = 0;
    int32 Quantity = 0;
    std::optional<uint8> Unknown1000;

    void Write(ByteBuffer& data) const
    {
        data << ItemId << DataSlotIndex << Quantity;
        data.WriteBit(Unknown1000.has_value());
        data.FlushBits();
        if (Unknown1000)
            data << *Unknown1000;
    }

    void ReadUnchecked(ByteBuffer& data)
    {
        data >> ItemId >> DataSlotIndex >> Quantity;
        Unknown1000.reset();
        if (data.ReadBit())
            Unknown1000 = data.read<uint8>();
    }
};

struct SpellWeight
{
    uint8 Type = 0;
    int32 Id = 0;
    uint32 Quantity = 0;
};

// What a client asks for when it casts; shared by every message that carries one.
struct SpellCastRequest
{
    static constexpr uint32 MAX_OPTIONAL_CURRENCIES = 5;
    static constexpr uint32 MAX_OPTIONAL_REAGENTS = 6;
    static constexpr uint32 MAX_WEIGHTS = 3;

    ClassicGuid CastId;
    int32 Misc[2] = { 0, 0 };
    int32 SpellId = 0;
    SpellCastVisual Visual;
    MissileTrajectoryRequest MissileTrajectory;
    ClassicGuid CraftingNpc;
    std::vector<SpellExtraCurrencyCost> OptionalCurrencies;
    std::vector<SpellCraftingReagent> OptionalReagents;
    std::vector<SpellCraftingReagent> RemovedModifications;
    uint8 CraftingFlags = 0;
    uint8 SendCastFlags = 0;
    SpellTargetData Target;
    std::optional<uint64> CraftingOrderId;
    std::optional<ClassicMovementInfo> MoveUpdate;
    std::vector<SpellWeight> Weight;

    void Write(ByteBuffer& data) const
    {
        CastId.Write(data);
        data << Misc[0] << Misc[1];
        data << SpellId;
        Visual.Write(data);
        data << MissileTrajectory.Pitch << MissileTrajectory.Speed;
        CraftingNpc.Write(data);
        data << uint32(OptionalCurrencies.size());
        data << uint32(OptionalReagents.size());
        data << uint32(RemovedModifications.size());
        data << CraftingFlags;
        for (SpellExtraCurrencyCost const& currency : OptionalCurrencies)
            data << currency.CurrencyId << currency.Count;

        data.WriteBits(SendCastFlags, 5);
        data.WriteBit(MoveUpdate.has_value());
        data.WriteBits(uint32(Weight.size()), 2);
        data.WriteBit(CraftingOrderId.has_value());
        data.FlushBits();
        Target.Write(data);
        if (CraftingOrderId)
            data << *CraftingOrderId;
        for (SpellCraftingReagent const& reagent : OptionalReagents)
            reagent.Write(data);
        for (SpellCraftingReagent const& reagent : RemovedModifications)
            reagent.Write(data);
        if (MoveUpdate)
            MoveUpdate->Write(data);
        for (SpellWeight const& weight : Weight)
        {
            data.WriteBits(weight.Type, 2);
            data.FlushBits();
            data << weight.Id << weight.Quantity;
        }
    }

    bool Read(ByteBuffer& data)
    {
        try
        {
            return ReadUnchecked(data);
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }

    private:
        bool ReadUnchecked(ByteBuffer& data)
        {
            if (!CastId.Read(data))
                return false;
            data >> Misc[0] >> Misc[1];
            data >> SpellId;
            Visual.Read(data);
            data >> MissileTrajectory.Pitch >> MissileTrajectory.Speed;
            if (!CraftingNpc.Read(data))
                return false;

            uint32 const currencyCount = data.read<uint32>();
            uint32 const reagentCount = data.read<uint32>();
            uint32 const removedCount = data.read<uint32>();
            if (currencyCount > MAX_OPTIONAL_CURRENCIES || reagentCount > MAX_OPTIONAL_REAGENTS || removedCount > MAX_OPTIONAL_REAGENTS)
                return false;
            data >> CraftingFlags;

            OptionalCurrencies.assign(currencyCount, SpellExtraCurrencyCost{});
            for (SpellExtraCurrencyCost& currency : OptionalCurrencies)
                data >> currency.CurrencyId >> currency.Count;

            SendCastFlags = uint8(data.ReadBits(5));
            bool const hasMoveUpdate = data.ReadBit();
            uint32 const weightCount = data.ReadBits(2);
            bool const hasCraftingOrder = data.ReadBit();
            Target.ReadUnchecked(data);

            CraftingOrderId.reset();
            if (hasCraftingOrder)
                CraftingOrderId = data.read<uint64>();

            OptionalReagents.assign(reagentCount, SpellCraftingReagent{});
            for (SpellCraftingReagent& reagent : OptionalReagents)
                reagent.ReadUnchecked(data);
            RemovedModifications.assign(removedCount, SpellCraftingReagent{});
            for (SpellCraftingReagent& reagent : RemovedModifications)
                reagent.ReadUnchecked(data);

            MoveUpdate.reset();
            if (hasMoveUpdate)
            {
                ClassicMovementInfo info;
                if (!info.Read(data))
                    return false;
                MoveUpdate = info;
            }

            Weight.assign(weightCount, SpellWeight{});
            for (SpellWeight& weight : Weight)
            {
                data.ResetBitPos();
                weight.Type = uint8(data.ReadBits(2));
                data.ResetBitPos();
                data >> weight.Id >> weight.Quantity;
            }
            return true;
        }
};

struct CastSpell
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CAST_SPELL;

    SpellCastRequest Cast;

    void Write(ByteBuffer& data) const { Cast.Write(data); }
    bool Read(ByteBuffer& data) { return Cast.Read(data); }
};

struct CancelCast
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_CAST;

    ClassicGuid CastId;
    int32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        CastId.Write(data);
        data << SpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> SpellId;
        return true;
    }
};

struct CancelChannelling
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_CHANNELLING;

    int32 ChannelSpell = 0;
    int32 Reason = 0;

    void Write(ByteBuffer& data) const { data << ChannelSpell << Reason; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        data >> ChannelSpell >> Reason;
        return true;
    }
};

struct CancelAutoRepeatSpell
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CANCEL_AUTO_REPEAT_SPELL;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Sent to a client when its cast request is accepted, pairing the id it sent with the id the cast
// runs under; the client's spell queue only advances on it.
struct SpellPrepare
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_PREPARE;

    ClassicGuid ClientCastId;
    ClassicGuid ServerCastId;

    void Write(ByteBuffer& data) const
    {
        ClientCastId.Write(data);
        ServerCastId.Write(data);
    }

    bool Read(ByteBuffer& data) { return ClientCastId.Read(data) && ServerCastId.Read(data); }
};

// The wire's miss reason for a reflected spell; only that reason carries a second field. It has kept
// this value since 1.12.
constexpr uint8 CLASSIC_SPELL_MISS_REFLECT = 11;

struct SpellMissStatus
{
    uint8 Reason = 0;
    uint8 ReflectStatus = 0;
};

struct SpellPowerData
{
    int8 Type = 0;
    int32 Cost = 0;
};

struct MissileTrajectoryResult
{
    uint32 TravelTime = 0;
    float Pitch = 0.0f;
};

struct CreatureImmunities
{
    int32 School = 0;
    int32 Value = 0;
};

struct SpellHealPrediction
{
    int32 Points = 0;
    uint8 Type = 0;
    ClassicGuid BeaconGuid;
};

// The body SpellStart and SpellGo share. A rune list is never sent (no rune classes), so a body
// carrying one is rejected on read.
struct SpellCastData
{
    ClassicGuid CasterGuid;
    ClassicGuid CasterUnit;
    ClassicGuid CastId;
    ClassicGuid OriginalCastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    uint32 CastFlags = 0;
    uint32 CastFlagsEx = 0;
    uint32 CastTime = 0;
    MissileTrajectoryResult MissileTrajectory;
    uint8 DestLocSpellCastIndex = 0;
    CreatureImmunities Immunities;
    SpellHealPrediction Predict;
    std::vector<ClassicGuid> HitTargets;
    std::vector<ClassicGuid> MissTargets;
    std::vector<SpellMissStatus> MissStatus;
    std::vector<SpellPowerData> RemainingPower;
    std::vector<TargetLocation> TargetPoints;
    std::optional<int32> AmmoDisplayId;
    std::optional<int32> AmmoInventoryType;
    SpellTargetData Target;

    void Write(ByteBuffer& data) const
    {
        CasterGuid.Write(data);
        CasterUnit.Write(data);
        CastId.Write(data);
        OriginalCastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << CastFlags << CastFlagsEx << CastTime;
        data << MissileTrajectory.TravelTime << MissileTrajectory.Pitch;
        data << DestLocSpellCastIndex;
        data << Immunities.School << Immunities.Value;
        data << Predict.Points << Predict.Type;
        Predict.BeaconGuid.Write(data);

        data.WriteBits(uint32(HitTargets.size()), 16);
        data.WriteBits(uint32(MissTargets.size()), 16);
        data.WriteBits(uint32(MissStatus.size()), 16);
        data.WriteBits(uint32(RemainingPower.size()), 9);
        data.WriteBit(false); // rune list
        data.WriteBits(uint32(TargetPoints.size()), 16);
        data.WriteBit(AmmoDisplayId.has_value());
        data.WriteBit(AmmoInventoryType.has_value());
        data.FlushBits();

        Target.Write(data);
        for (ClassicGuid const& guid : HitTargets)
            guid.Write(data);
        for (ClassicGuid const& guid : MissTargets)
            guid.Write(data);
        for (SpellMissStatus const& status : MissStatus)
        {
            data << status.Reason;
            if (status.Reason == CLASSIC_SPELL_MISS_REFLECT)
                data << status.ReflectStatus;
        }
        for (SpellPowerData const& power : RemainingPower)
            data << power.Type << power.Cost;
        for (TargetLocation const& location : TargetPoints)
            location.Write(data);
        if (AmmoDisplayId)
            data << *AmmoDisplayId;
        if (AmmoInventoryType)
            data << *AmmoInventoryType;
    }

    bool Read(ByteBuffer& data)
    {
        try
        {
            return ReadUnchecked(data);
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }

    private:
        bool ReadUnchecked(ByteBuffer& data)
        {
            if (!CasterGuid.Read(data) || !CasterUnit.Read(data) || !CastId.Read(data) || !OriginalCastId.Read(data))
                return false;
            data >> SpellId;
            Visual.Read(data);
            data >> CastFlags >> CastFlagsEx >> CastTime;
            data >> MissileTrajectory.TravelTime >> MissileTrajectory.Pitch;
            data >> DestLocSpellCastIndex;
            data >> Immunities.School >> Immunities.Value;
            data >> Predict.Points >> Predict.Type;
            if (!Predict.BeaconGuid.Read(data))
                return false;

            data.ResetBitPos();
            uint32 const hitCount = data.ReadBits(16);
            uint32 const missCount = data.ReadBits(16);
            uint32 const statusCount = data.ReadBits(16);
            uint32 const powerCount = data.ReadBits(9);
            bool const hasRunes = data.ReadBit();
            uint32 const pointCount = data.ReadBits(16);
            bool const hasAmmoDisplay = data.ReadBit();
            bool const hasAmmoInventory = data.ReadBit();
            data.ResetBitPos();
            if (hasRunes)
                return false;

            Target.ReadUnchecked(data);

            HitTargets.assign(hitCount, ClassicGuid{});
            for (ClassicGuid& guid : HitTargets)
                if (!guid.Read(data))
                    return false;
            MissTargets.assign(missCount, ClassicGuid{});
            for (ClassicGuid& guid : MissTargets)
                if (!guid.Read(data))
                    return false;

            MissStatus.assign(statusCount, SpellMissStatus{});
            for (SpellMissStatus& status : MissStatus)
            {
                data >> status.Reason;
                if (status.Reason == CLASSIC_SPELL_MISS_REFLECT)
                    data >> status.ReflectStatus;
            }

            RemainingPower.assign(powerCount, SpellPowerData{});
            for (SpellPowerData& power : RemainingPower)
                data >> power.Type >> power.Cost;

            TargetPoints.assign(pointCount, TargetLocation{});
            for (TargetLocation& location : TargetPoints)
                if (!location.Read(data))
                    return false;

            AmmoDisplayId.reset();
            if (hasAmmoDisplay)
                AmmoDisplayId = data.read<int32>();
            AmmoInventoryType.reset();
            if (hasAmmoInventory)
                AmmoInventoryType = data.read<int32>();
            return true;
        }
};

struct SpellStart
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_START;

    SpellCastData Cast;

    void Write(ByteBuffer& data) const { Cast.Write(data); }
    bool Read(ByteBuffer& data) { return Cast.Read(data); }
};

// Followed on the wire by a bit for advanced combat log data, which this port never sends.
struct SpellGo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_GO;

    SpellCastData Cast;

    void Write(ByteBuffer& data) const
    {
        Cast.Write(data);
        data.WriteBit(false);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Cast.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasLogData = data.ReadBit();
        data.ResetBitPos();
        return !hasLogData;
    }
};

// Only the caster receives it.
struct CastFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CAST_FAILED;

    ClassicGuid CastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    int32 Reason = 0;
    int32 FailedArg1 = -1;
    int32 FailedArg2 = -1;

    void Write(ByteBuffer& data) const
    {
        CastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << Reason << FailedArg1 << FailedArg2;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < 5 * sizeof(int32))
            return false;
        data >> SpellId;
        Visual.Read(data);
        data >> Reason >> FailedArg1 >> FailedArg2;
        return true;
    }
};

// SpellFailure is the interrupted cast's own bar dropping; SpellFailedOther is what everyone else
// sees. Both echo the id the cast ran under.
struct SpellFailure
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_FAILURE;

    ClassicGuid CasterUnit;
    ClassicGuid CastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    uint16 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        CasterUnit.Write(data);
        CastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << Reason;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CasterUnit.Read(data) || !CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint16))
            return false;
        data >> SpellId;
        Visual.Read(data);
        data >> Reason;
        return true;
    }
};

struct SpellFailedOther
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_FAILED_OTHER;

    ClassicGuid CasterUnit;
    ClassicGuid CastId;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    uint8 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        CasterUnit.Write(data);
        CastId.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << Reason;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CasterUnit.Read(data) || !CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + sizeof(uint8))
            return false;
        data >> SpellId;
        Visual.Read(data);
        data >> Reason;
        return true;
    }
};

struct SpellDelayed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_DELAYED;

    ClassicGuid Caster;
    uint32 ActualDelay = 0;

    void Write(ByteBuffer& data) const
    {
        Caster.Write(data);
        data << ActualDelay;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Caster.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> ActualDelay;
        return true;
    }
};

struct SpellChannelStart
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_CHANNEL_START;

    struct InterruptImmunities
    {
        int32 SchoolImmunities = 0;
        int32 Immunities = 0;
    };

    struct TargetedHealPrediction
    {
        ClassicGuid TargetGuid;
        SpellHealPrediction Predict;
    };

    ClassicGuid CasterGuid;
    int32 SpellId = 0;
    SpellCastVisual Visual;
    uint32 ChannelDuration = 0;
    std::optional<InterruptImmunities> Interrupts;
    std::optional<TargetedHealPrediction> HealPrediction;

    void Write(ByteBuffer& data) const
    {
        CasterGuid.Write(data);
        data << SpellId;
        Visual.Write(data);
        data << ChannelDuration;
        data.WriteBit(Interrupts.has_value());
        data.WriteBit(HealPrediction.has_value());
        data.FlushBits();
        if (Interrupts)
            data << Interrupts->SchoolImmunities << Interrupts->Immunities;
        if (HealPrediction)
        {
            HealPrediction->TargetGuid.Write(data);
            data << HealPrediction->Predict.Points << HealPrediction->Predict.Type;
            HealPrediction->Predict.BeaconGuid.Write(data);
        }
    }

    bool Read(ByteBuffer& data)
    {
        try
        {
            if (!CasterGuid.Read(data))
                return false;
            data >> SpellId;
            Visual.Read(data);
            data >> ChannelDuration;
            bool const hasInterrupts = data.ReadBit();
            bool const hasHealPrediction = data.ReadBit();
            data.ResetBitPos();

            Interrupts.reset();
            if (hasInterrupts)
            {
                InterruptImmunities immunities;
                data >> immunities.SchoolImmunities >> immunities.Immunities;
                Interrupts = immunities;
            }

            HealPrediction.reset();
            if (hasHealPrediction)
            {
                TargetedHealPrediction prediction;
                if (!prediction.TargetGuid.Read(data))
                    return false;
                data >> prediction.Predict.Points >> prediction.Predict.Type;
                if (!prediction.Predict.BeaconGuid.Read(data))
                    return false;
                HealPrediction = prediction;
            }
            return true;
        }
        catch (ByteBufferException const&)
        {
            return false;
        }
    }
};

struct SpellChannelUpdate
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPELL_CHANNEL_UPDATE;

    ClassicGuid CasterGuid;
    int32 TimeRemaining = 0;

    void Write(ByteBuffer& data) const
    {
        CasterGuid.Write(data);
        data << TimeRemaining;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CasterGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> TimeRemaining;
        return true;
    }
};

#endif // MANGOS_PACKETS_SPELLCAST_H
