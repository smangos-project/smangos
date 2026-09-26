#ifndef MANGOS_PACKETS_PETMESSAGES_H
#define MANGOS_PACKETS_PETMESSAGES_H

// The modern pet wire: the pet action bar and spell list, commands, renaming, name lookup, casts and
// the small feedback messages. Light like Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/SpellCast.h"
#include <array>
#include <string>
#include <vector>

constexpr uint32 CLASSIC_PET_ACTION_BAR_SIZE = 10;
constexpr uint32 CLASSIC_PET_MAX_LIST = 64;
constexpr uint32 CLASSIC_PET_MAX_NAME = 32;

// ==== Client packets ====

struct QueryPetName
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_PET_NAME;

    ClassicGuid Pet;

    void Write(ByteBuffer& data) const { Pet.Write(data); }
    bool Read(ByteBuffer& data) { return Pet.Read(data); }
};

struct RequestPetInfo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_PET_INFO;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The click position is parsed, unused: this engine has no click-to-move pet command.
struct PetAction
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_ACTION;

    ClassicGuid Pet;
    uint32 Action = 0;  // the packed action button: spell id in the low 24 bits, type in the high 8
    ClassicGuid Target;
    float Position[3] = { 0.0f, 0.0f, 0.0f };

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << Action;
        Target.Write(data);
        data << Position[0] << Position[1] << Position[2];
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Action;
        if (!Target.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(float))
            return false;
        data >> Position[0] >> Position[1] >> Position[2];
        return true;
    }
};

struct PetStopAttack
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_STOP_ATTACK;

    ClassicGuid Pet;

    void Write(ByteBuffer& data) const { Pet.Write(data); }
    bool Read(ByteBuffer& data) { return Pet.Read(data); }
};

struct PetAbandon
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_ABANDON;

    ClassicGuid Pet;

    void Write(ByteBuffer& data) const { Pet.Write(data); }
    bool Read(ByteBuffer& data) { return Pet.Read(data); }
};

struct PetSpellAutocast
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_SPELL_AUTOCAST;

    ClassicGuid Pet;
    uint32 SpellId = 0;
    bool Enabled = false;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << SpellId;
        data.WriteBit(Enabled);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> SpellId;
        Enabled = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// One action button changed per message. The two trailing words behind the optional bit are
// parsed, unused.
struct PetSetAction
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_SET_ACTION;

    ClassicGuid Pet;
    uint32 Index = 0;
    uint32 Action = 0;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << Index << Action;
        data.WriteBit(false);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32) + 1)
            return false;
        data >> Index >> Action;
        bool const hasExtra = data.ReadBit();
        data.ResetBitPos();
        if (hasExtra)
        {
            if (data.size() - data.rpos() < 2 * sizeof(uint32))
                return false;
            uint32 unused1, unused2;
            data >> unused1 >> unused2;
        }
        return true;
    }
};

struct PetCancelAura
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_CANCEL_AURA;

    ClassicGuid Pet;
    int32 SpellId = 0;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << SpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> SpellId;
        return true;
    }
};

// The declined-name block (Russian client cases) is parsed and dropped: this engine keeps none.
struct PetRename
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_RENAME;

    ClassicGuid Pet;
    int32 PetNumber = 0;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << PetNumber;
        data.WriteBits(uint32(Name.length()), 8);
        data.WriteBit(false);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 2)
            return false;
        data >> PetNumber;
        uint32 const nameLength = data.ReadBits(8);
        bool const hasDeclined = data.ReadBit();
        uint32 declinedLengths[5] = {};
        if (hasDeclined)
        {
            if (data.size() - data.rpos() < 4)
                return false;
            for (uint32& length : declinedLengths)
                length = data.ReadBits(7);
        }
        data.ResetBitPos();
        for (uint32 const length : declinedLengths)
        {
            if (data.size() - data.rpos() < length)
                return false;
            data.read_skip(length);
        }
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

struct PetCastSpell
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PET_CAST_SPELL;

    ClassicGuid Pet;
    SpellCastRequest Cast;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        Cast.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        return Pet.Read(data) && Cast.Read(data);
    }
};

// ==== Server packets ====

struct PetCooldownEntry
{
    int32 SpellId = 0;
    int32 Duration = 0;          // ms left on the spell's own cooldown
    int32 CategoryDuration = 0;  // ms left on its category cooldown
    uint16 Category = 0;
};

// The pet's (or a charmed unit's) action bar and spell list. Category charges (the spell history
// list) have no source here and are always empty; a body that carries one is rejected on read.
struct PetSpellsMessage
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_SPELLS_MESSAGE;

    ClassicGuid Pet;
    uint16 CreatureFamily = 0;
    uint32 TimeLimit = 0;  // ms of control left for a charm or possession, 0 for a permanent pet
    uint8 CommandState = 0;
    uint8 Flag = 0;        // 0x8: the action bar is disabled
    uint8 ReactState = 0;
    std::array<uint32, CLASSIC_PET_ACTION_BAR_SIZE> ActionButtons{};
    std::vector<uint32> Actions;  // learned spells as packed action buttons
    std::vector<PetCooldownEntry> Cooldowns;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << CreatureFamily << uint16(0);  // specialization
        data << TimeLimit;
        data << CommandState << Flag << ReactState;
        for (uint32 const button : ActionButtons)
            data << button;
        data << uint32(Actions.size()) << uint32(Cooldowns.size()) << uint32(0);
        for (uint32 const action : Actions)
            data << action;
        for (PetCooldownEntry const& cooldown : Cooldowns)
        {
            data << cooldown.SpellId << cooldown.Duration << cooldown.CategoryDuration;
            data << 1.0f << cooldown.Category;  // ModRate
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 + 2 + 4 + 3 + 4 * CLASSIC_PET_ACTION_BAR_SIZE + 12)
            return false;
        uint16 specialization;
        data >> CreatureFamily >> specialization >> TimeLimit;
        data >> CommandState >> Flag >> ReactState;
        for (uint32& button : ActionButtons)
            data >> button;
        uint32 actionCount, cooldownCount, historyCount;
        data >> actionCount >> cooldownCount >> historyCount;
        if (specialization || historyCount || actionCount > CLASSIC_PET_MAX_LIST || cooldownCount > CLASSIC_PET_MAX_LIST)
            return false;
        if (data.size() - data.rpos() < uint64(actionCount) * 4 + uint64(cooldownCount) * 18)
            return false;

        Actions.assign(actionCount, 0);
        for (uint32& action : Actions)
            data >> action;
        Cooldowns.assign(cooldownCount, PetCooldownEntry());
        for (PetCooldownEntry& cooldown : Cooldowns)
        {
            float modRate;
            data >> cooldown.SpellId >> cooldown.Duration >> cooldown.CategoryDuration >> modRate >> cooldown.Category;
        }
        return true;
    }
};

// The pet's bar goes away (dismissed, or a charm ends).
struct PetClearSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_CLEAR_SPELLS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct PetMode
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_MODE;

    ClassicGuid Pet;
    uint8 CommandState = 0;
    uint8 Flag = 0;  // 0x8: the action bar is disabled
    uint8 ReactState = 0;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data << CommandState << Flag << ReactState;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < 3)
            return false;
        data >> CommandState >> Flag >> ReactState;
        return true;
    }
};

struct PetActionFeedback
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_ACTION_FEEDBACK;

    int32 SpellId = 0;
    uint8 Response = 0;

    void Write(ByteBuffer& data) const { data << SpellId << Response; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> SpellId >> Response;
        return true;
    }
};

struct PetActionSound
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_ACTION_SOUND;

    ClassicGuid Unit;
    int32 Action = 0;

    void Write(ByteBuffer& data) const
    {
        Unit.Write(data);
        data << Action;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Unit.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Action;
        return true;
    }
};

struct PetTameFailure
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_TAME_FAILURE;

    uint8 Result = 0;

    void Write(ByteBuffer& data) const { data << Result; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        return true;
    }
};

struct PetCastFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_CAST_FAILED;

    ClassicGuid CastId;
    int32 SpellId = 0;
    int32 Reason = 0;
    int32 FailedArg1 = -1;
    int32 FailedArg2 = -1;

    void Write(ByteBuffer& data) const
    {
        CastId.Write(data);
        data << SpellId << Reason << FailedArg1 << FailedArg2;
    }

    bool Read(ByteBuffer& data)
    {
        if (!CastId.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(int32))
            return false;
        data >> SpellId >> Reason >> FailedArg1 >> FailedArg2;
        return true;
    }
};

// The declined-name block of both name messages is always empty: this engine keeps no declensions.
struct PetNameInvalid
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PET_NAME_INVALID;

    uint8 Result = 0;
    ClassicGuid Pet;
    int32 PetNumber = 0;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        data << Result;
        Pet.Write(data);
        data << PetNumber;
        data << uint8(Name.length());
        data.WriteBit(false);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + 2)
            return false;
        uint8 nameLength;
        data >> PetNumber >> nameLength;
        bool const hasDeclined = data.ReadBit();
        data.ResetBitPos();
        if (hasDeclined)
            return false;
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

struct QueryPetNameResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_PET_NAME_RESPONSE;

    ClassicGuid Pet;
    bool Allow = false;
    std::string Name;
    int64 Timestamp = 0;

    void Write(ByteBuffer& data) const
    {
        Pet.Write(data);
        data.WriteBit(Allow);
        if (Allow)
        {
            data.WriteBits(uint32(Name.length()), 8);
            data.WriteBit(false);  // has declined names
            for (uint32 i = 0; i < 5; ++i)
                data.WriteBits(0, 7);
        }
        data.FlushBits();
        if (Allow)
        {
            data << Timestamp;
            data.WriteString(Name);
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (!Pet.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Allow = data.ReadBit();
        if (!Allow)
        {
            data.ResetBitPos();
            return true;
        }
        // 45 bits in all: the first byte is consumed
        if (data.size() - data.rpos() < 5)
            return false;
        uint32 const nameLength = data.ReadBits(8);
        bool const hasDeclined = data.ReadBit();
        uint32 declinedLengths = 0;
        for (uint32 i = 0; i < 5; ++i)
            declinedLengths += data.ReadBits(7);
        data.ResetBitPos();
        if (hasDeclined || declinedLengths)
            return false;
        if (data.size() - data.rpos() < sizeof(int64) + nameLength)
            return false;
        data >> Timestamp;
        Name = data.ReadString(nameLength);
        return true;
    }
};

#endif
