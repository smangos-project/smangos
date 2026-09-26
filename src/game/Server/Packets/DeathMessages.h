#ifndef MANGOS_PACKETS_DEATHMESSAGES_H
#define MANGOS_PACKETS_DEATHMESSAGES_H

// The modern death and resurrection wire: releasing the spirit, resurrection offers, corpse reclaim.
// Light like Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>

// "Release Spirit". The client's only field says whether it wants the instance entrance check.
struct RepopRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REPOP_REQUEST;

    bool CheckInstance = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(CheckInstance);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        CheckInstance = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The client's answer to a resurrection offer: 0 accepts, anything else declines or times out.
struct ResurrectResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_RESURRECT_RESPONSE;

    ClassicGuid Resurrecter;
    uint32 Response = 0;

    void Write(ByteBuffer& data) const
    {
        Resurrecter.Write(data);
        data << Response;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Resurrecter.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Response;
        return true;
    }
};

// A ghost asking for its body back.
struct ReclaimCorpse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_RECLAIM_CORPSE;

    ClassicGuid Corpse;

    void Write(ByteBuffer& data) const { Corpse.Write(data); }
    bool Read(ByteBuffer& data) { return Corpse.Read(data); }
};

// A resurrection offer. The name is only sent for a creature offering; a player's is resolved from the guid.
struct ResurrectRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_RESURRECT_REQUEST;

    ClassicGuid Offerer;
    uint32 VirtualRealmAddress = 0;
    uint32 PetNumber = 0;
    int32 SpellId = 0;
    bool UseTimer = false;
    bool Sickness = false;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        Offerer.Write(data);
        data << VirtualRealmAddress;
        data << PetNumber;
        data << SpellId;
        data.WriteBits(uint32(Name.length()), 11);
        data.WriteBit(UseTimer);
        data.WriteBit(Sickness);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Offerer.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32) + 2) // 13 bits, flushed to two bytes
            return false;
        data >> VirtualRealmAddress >> PetNumber >> SpellId;
        uint32 const length = data.ReadBits(11);
        UseTimer = data.ReadBit();
        Sickness = data.ReadBit();
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        Name = data.ReadString(length);
        return true;
    }
};

// Where a released spirit goes. A map of -1 clears the marker on resurrection.
struct DeathReleaseLoc
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DEATH_RELEASE_LOC;

    int32 MapId = -1;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;

    void Write(ByteBuffer& data) const { data << MapId << X << Y << Z; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 3 * sizeof(float))
            return false;
        data >> MapId >> X >> Y >> Z;
        return true;
    }
};

// How long, in milliseconds, before a ghost may reclaim its corpse.
struct CorpseReclaimDelay
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CORPSE_RECLAIM_DELAY;

    uint32 Remaining = 0;

    void Write(ByteBuffer& data) const { data << Remaining; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Remaining;
        return true;
    }
};

// The equipment durability lost to a death, as a percentage.
struct DurabilityDamageDeath
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DURABILITY_DAMAGE_DEATH;

    int32 Percent = 0;

    void Write(ByteBuffer& data) const { data << Percent; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Percent;
        return true;
    }
};

#endif // MANGOS_PACKETS_DEATHMESSAGES_H
