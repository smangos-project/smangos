#ifndef MANGOS_PACKETS_SPELLBOOK_H
#define MANGOS_PACKETS_SPELLBOOK_H

// The spell book and its cooldowns, as sent at login. Light like Packets/Session.h, so RealBots
// can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include <optional>
#include <vector>

// Reads `count` spell ids, rejecting a count the body can't hold.
inline bool ReadSpellIdList(ByteBuffer& data, uint32 count, std::vector<uint32>& out)
{
    if ((data.size() - data.rpos()) / sizeof(uint32) < count)
        return false;
    out.resize(count);
    for (uint32& spellId : out)
        data >> spellId;
    return true;
}

struct SendKnownSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SEND_KNOWN_SPELLS;

    bool InitialLogin = false;
    std::vector<uint32> KnownSpells;
    std::vector<uint32> FavoriteSpells;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(InitialLogin);
        data << uint32(KnownSpells.size());
        data << uint32(FavoriteSpells.size());
        for (uint32 spellId : KnownSpells)
            data << spellId;
        for (uint32 spellId : FavoriteSpells)
            data << spellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1 + sizeof(uint32) * 2)
            return false;
        InitialLogin = data.ReadBit();
        uint32 knownCount, favoriteCount;
        data >> knownCount >> favoriteCount;
        return ReadSpellIdList(data, knownCount, KnownSpells) && ReadSpellIdList(data, favoriteCount, FavoriteSpells);
    }
};

// Shared by SendLearnedSpells and SendSupercededSpells: the spell just learned, and (for the
// superceded case) the older rank it replaces in the spellbook and action bars.
struct LearnedSpellInfo
{
    int32 SpellID = 0;
    bool IsFavorite = false;
    std::optional<int32> Superceded;
    std::optional<int32> TraitDefinitionID;

    void Write(ByteBuffer& data) const
    {
        data << SpellID;
        data.WriteBit(IsFavorite);
        data.WriteBit(false); // field_8: no known TrinityCore use, never set
        data.WriteBit(Superceded.has_value());
        data.WriteBit(TraitDefinitionID.has_value());
        data.FlushBits();
        if (Superceded)
            data << *Superceded;
        if (TraitDefinitionID)
            data << *TraitDefinitionID;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> SpellID;
        IsFavorite = data.ReadBit();
        bool const hasField8 = data.ReadBit();
        bool const hasSuperceded = data.ReadBit();
        bool const hasTraitDefinitionID = data.ReadBit();
        data.ResetBitPos();

        size_t const optionalSize = (hasField8 ? sizeof(int32) : 0) + (hasSuperceded ? sizeof(int32) : 0) + (hasTraitDefinitionID ? sizeof(int32) : 0);
        if (data.size() - data.rpos() < optionalSize)
            return false;
        if (hasField8)
            data.read<int32>(); // discarded: no known TrinityCore use
        if (hasSuperceded)
            Superceded = data.read<int32>();
        if (hasTraitDefinitionID)
            TraitDefinitionID = data.read<int32>();
        return true;
    }
};

struct SendLearnedSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LEARNED_SPELLS;

    std::vector<LearnedSpellInfo> Spells;
    uint32 SpecializationID = 0;
    bool SuppressMessaging = false;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Spells.size());
        data << SpecializationID;
        data.WriteBit(SuppressMessaging);
        data.FlushBits();
        for (LearnedSpellInfo const& spell : Spells)
            spell.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 2 + 1)
            return false;
        uint32 count;
        data >> count >> SpecializationID;
        SuppressMessaging = data.ReadBit();
        data.ResetBitPos();

        Spells.resize(count);
        for (LearnedSpellInfo& spell : Spells)
            if (!spell.Read(data))
                return false;
        return true;
    }
};

struct SendSupercededSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SUPERCEDED_SPELLS;

    std::vector<LearnedSpellInfo> Spells;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Spells.size());
        for (LearnedSpellInfo const& spell : Spells)
            spell.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;

        Spells.resize(count);
        for (LearnedSpellInfo& spell : Spells)
            if (!spell.Read(data))
                return false;
        return true;
    }
};

// A spell disappearing from the spellbook (unlearned, or a lower rank dropped without a
// replacement). Not to be confused with SendUnlearnSpells (SMSG_SEND_UNLEARN_SPELLS), the
// talent-respec list sent once at login.
struct SendUnlearnedSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_UNLEARNED_SPELLS;

    std::vector<uint32> SpellIds;
    bool SuppressMessaging = false;

    void Write(ByteBuffer& data) const
    {
        data << uint32(SpellIds.size());
        for (uint32 spellId : SpellIds)
            data << spellId;
        data.WriteBit(SuppressMessaging);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        if (!ReadSpellIdList(data, count, SpellIds))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        SuppressMessaging = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct SendUnlearnSpells
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SEND_UNLEARN_SPELLS;

    std::vector<uint32> Spells;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Spells.size());
        for (uint32 spellId : Spells)
            data << spellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        return ReadSpellIdList(data, count, Spells);
    }
};

// Remaining cooldowns in milliseconds; a negative recovery time is an on-hold cooldown that
// starts when its trigger ends.
struct SpellHistoryEntry
{
    uint32 SpellId = 0;
    uint32 ItemId = 0;
    uint32 Category = 0;
    int32 RecoveryTime = 0;
    int32 CategoryRecoveryTime = 0;
    float ModRate = 1.0f;
    bool OnHold = false;
    std::optional<uint32> Unused1;
    std::optional<uint32> Unused2;
};

struct SendSpellHistory
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SEND_SPELL_HISTORY;

    std::vector<SpellHistoryEntry> Entries;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Entries.size());
        for (SpellHistoryEntry const& entry : Entries)
        {
            data << entry.SpellId << entry.ItemId << entry.Category;
            data << entry.RecoveryTime << entry.CategoryRecoveryTime << entry.ModRate;
            data.WriteBit(entry.Unused1.has_value());
            data.WriteBit(entry.Unused2.has_value());
            data.WriteBit(entry.OnHold);
            if (entry.Unused1)
                data << *entry.Unused1;
            if (entry.Unused2)
                data << *entry.Unused2;
            data.FlushBits();
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;

        size_t const minEntrySize = sizeof(uint32) * 5 + sizeof(float) + 1;
        if ((data.size() - data.rpos()) / minEntrySize < count)
            return false;

        Entries.resize(count);
        for (SpellHistoryEntry& entry : Entries)
        {
            if (data.size() - data.rpos() < minEntrySize)
                return false;
            data >> entry.SpellId >> entry.ItemId >> entry.Category;
            data >> entry.RecoveryTime >> entry.CategoryRecoveryTime >> entry.ModRate;
            bool const hasUnused1 = data.ReadBit();
            bool const hasUnused2 = data.ReadBit();
            entry.OnHold = data.ReadBit();
            size_t const optionalSize = (hasUnused1 ? sizeof(uint32) : 0) + (hasUnused2 ? sizeof(uint32) : 0);
            if (data.size() - data.rpos() < optionalSize)
                return false;
            if (hasUnused1)
                entry.Unused1 = data.read<uint32>();
            if (hasUnused2)
                entry.Unused2 = data.read<uint32>();
            data.ResetBitPos();
        }
        return true;
    }
};

#endif
