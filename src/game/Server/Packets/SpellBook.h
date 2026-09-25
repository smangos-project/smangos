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
