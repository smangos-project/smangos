#ifndef MANGOS_PACKETS_INSPECTMESSAGES_H
#define MANGOS_PACKETS_INSPECTMESSAGES_H

// The modern inspect window: equipment/honor tab, the PvP tab (always empty -- vanilla has no
// arenas) and the achievements-tab request (always a no-op -- vanilla predates achievements).
// Light like Packets/DuelMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <array>
#include <string>
#include <vector>

// --- Client packets ---

struct Inspect
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_INSPECT;

    ClassicGuid TargetGuid;

    void Write(ByteBuffer& data) const { TargetGuid.Write(data); }
    bool Read(ByteBuffer& data) { return TargetGuid.Read(data); }
};

// Same guid128-only body as Inspect, but its own opcode: the client opens the PvP tab of the
// inspect window with this instead of re-sending Inspect.
struct InspectPvp
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_INSPECT_PVP;

    ClassicGuid TargetGuid;

    void Write(ByteBuffer& data) const { TargetGuid.Write(data); }
    bool Read(ByteBuffer& data) { return TargetGuid.Read(data); }
};

// Same guid128-only body again, fired when the achievements tab of the inspect window opens.
// This engine has no achievement system (vanilla 1.12 predates them); the handler consumes the
// request and sends nothing back, matching the client's own tolerance for a window with no
// achievements tab content.
struct QueryInspectAchievements
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_INSPECT_ACHIEVEMENTS;

    ClassicGuid TargetGuid;

    void Write(ByteBuffer& data) const { TargetGuid.Write(data); }
    bool Read(ByteBuffer& data) { return TargetGuid.Read(data); }
};

// --- Server packets ---

// One permanent or temporary enchant on an inspected item (MAX_INSPECTED_ENCHANTMENT_SLOT slots
// in this engine); the modern socket/use-enchant slots occupy the same field indices and are
// never shown here.
struct InspectEnchantEntry
{
    uint32 Id = 0;
    uint8 Index = 0;

    void Write(ByteBuffer& data) const { data << Id << Index; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint8))
            return false;
        data >> Id >> Index;
        return true;
    }
};

// One equipped item shown in the inspect window. Azerite fields have no vanilla equivalent and
// are always empty; `Usable` is a real field on the wire but this engine never varies it.
struct InspectItemEntry
{
    ClassicGuid CreatorGuid;
    uint8 Index = 0;
    bool Usable = true;
    ClassicItemInstance Item;
    std::vector<InspectEnchantEntry> Enchants;

    void Write(ByteBuffer& data) const
    {
        CreatorGuid.Write(data);
        data << Index;
        data << int32(0); // AzeritePowers.size() -- none in vanilla
        data << int32(0); // AzeriteEssences count -- none in vanilla
        Item.Write(data);
        data.WriteBit(Usable);
        data.WriteBits(uint32(Enchants.size()), 4);
        data.WriteBits(0, 2); // Gems -- no socketed gems in vanilla
        data.FlushBits();
        for (InspectEnchantEntry const& enchant : Enchants)
            enchant.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!CreatorGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint8) + 2 * sizeof(int32))
            return false;
        data >> Index;
        int32 azeritePowerCount, azeriteEssenceCount;
        data >> azeritePowerCount >> azeriteEssenceCount;
        if (azeritePowerCount != 0 || azeriteEssenceCount != 0)
            return false;
        if (!Item.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Usable = data.ReadBit();
        uint32 const enchantCount = data.ReadBits(4);
        if (data.ReadBits(2) != 0)
            return false;
        data.ResetBitPos();
        Enchants.clear();
        for (uint32 i = 0; i < enchantCount; ++i)
        {
            InspectEnchantEntry enchant;
            if (!enchant.Read(data))
                return false;
            Enchants.push_back(enchant);
        }
        return true;
    }
};

// The inspected character's model, name and gear. No specs, glyphs, talents or character
// customizations in vanilla -- those fields are always the empty/zero default.
struct InspectDisplayInfo
{
    ClassicGuid PlayerGuid;
    std::string Name;
    uint8 Gender = 0;
    uint8 Race = 0;
    uint8 ClassId = 0;
    std::vector<InspectItemEntry> Items;

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data << int32(0); // SpecializationID -- no specs in vanilla
        data << uint32(Items.size());
        data.WriteBits(uint32(Name.length()), 6);
        data << Gender << Race << ClassId;
        data << uint32(0); // Customizations count -- no character customizations in vanilla
        data.WriteString(Name);
        for (InspectItemEntry const& item : Items)
            item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        int32 specializationId;
        uint32 itemCount;
        data >> specializationId >> itemCount;
        if (specializationId != 0)
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(6);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 3 * sizeof(uint8) + sizeof(uint32))
            return false;
        data >> Gender >> Race >> ClassId;
        uint32 customizationsCount;
        data >> customizationsCount;
        if (customizationsCount != 0)
            return false;
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        Items.clear();
        for (uint32 i = 0; i < itemCount; ++i)
        {
            InspectItemEntry item;
            if (!item.Read(data))
                return false;
            Items.push_back(item);
        }
        return true;
    }
};

// No achievements in vanilla -- AchievementPoints is always 0.
struct InspectGuildEntry
{
    ClassicGuid GuildGuid;
    int32 NumMembers = 0;

    void Write(ByteBuffer& data) const
    {
        GuildGuid.Write(data);
        data << NumMembers;
        data << int32(0); // AchievementPoints
    }

    bool Read(ByteBuffer& data)
    {
        if (!GuildGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        int32 achievementPoints;
        data >> NumMembers >> achievementPoints;
        return achievementPoints == 0;
    }
};

// One rated-bracket row on the PvP tab. Vanilla has no arenas or rated brackets, so every field
// this engine ever sends is 0/false; kept as real fields for wire completeness (the client reads
// a fixed number of these regardless of content).
struct InspectPvpBracketEntry
{
    uint8 Bracket = 0;
    int32 Rating = 0;
    int32 Rank = 0;
    int32 WeeklyPlayed = 0;
    int32 WeeklyWon = 0;
    int32 SeasonPlayed = 0;
    int32 SeasonWon = 0;
    int32 WeeklyBestRating = 0;
    int32 SeasonBestRating = 0;
    int32 PvpTierId = 0;
    int32 WeeklyBestWinPvpTierId = 0;
    int32 Unused1 = 0;
    int32 Unused2 = 0;
    bool Disqualified = false;

    void Write(ByteBuffer& data) const
    {
        data << Bracket << Rating << Rank << WeeklyPlayed << WeeklyWon << SeasonPlayed << SeasonWon;
        data << WeeklyBestRating << SeasonBestRating << PvpTierId << WeeklyBestWinPvpTierId << Unused1 << Unused2;
        data.WriteBit(Disqualified);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint8) + 12 * sizeof(int32))
            return false;
        data >> Bracket >> Rating >> Rank >> WeeklyPlayed >> WeeklyWon >> SeasonPlayed >> SeasonWon;
        data >> WeeklyBestRating >> SeasonBestRating >> PvpTierId >> WeeklyBestWinPvpTierId >> Unused1 >> Unused2;
        if (data.size() - data.rpos() < 1)
            return false;
        Disqualified = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Full modern SMSG_INSPECT_RESULT. The 1.12 client reconstructed the inspect window from the
// target's visible-item update fields and a separate MSG_INSPECT_HONOR_STATS round trip; the
// 1.14 client expects this one packet with everything (equipment and honor both) up front, and
// has no modern request left that the honor stats used to answer. Glyphs, talents and the
// azerite level never applied in vanilla and are always empty/absent.
struct InspectResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_INSPECT_RESULT;

    static constexpr uint32 BracketCount = 6;

    InspectDisplayInfo DisplayInfo;
    bool HasGuildData = false;
    InspectGuildEntry GuildData;
    std::array<InspectPvpBracketEntry, BracketCount> Brackets;
    int32 ItemLevel = 0;
    uint32 LifetimeHK = 0;
    uint32 HonorLevel = 0;
    uint16 TodayHK = 0;
    uint16 YesterdayHK = 0;
    uint8 LifetimeMaxRank = 0;

    void Write(ByteBuffer& data) const
    {
        DisplayInfo.Write(data);
        data << uint32(0); // Glyphs count -- no glyphs in vanilla
        data << uint32(0); // Talents count -- no talent inspect in vanilla
        data << ItemLevel;
        data << LifetimeMaxRank;
        data << TodayHK;
        data << YesterdayHK;
        data << LifetimeHK;
        data << HonorLevel;
        data.WriteBit(HasGuildData);
        data.WriteBit(false); // HasAzeriteLevel -- no azerite in vanilla
        data.FlushBits();
        for (InspectPvpBracketEntry const& bracket : Brackets)
            bracket.Write(data);
        if (HasGuildData)
            GuildData.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!DisplayInfo.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        uint32 glyphCount, talentCount;
        data >> glyphCount >> talentCount;
        if (glyphCount != 0 || talentCount != 0)
            return false;
        if (data.size() - data.rpos() < sizeof(int32) + sizeof(uint8) + 2 * sizeof(uint16) + 2 * sizeof(uint32))
            return false;
        data >> ItemLevel >> LifetimeMaxRank >> TodayHK >> YesterdayHK >> LifetimeHK >> HonorLevel;
        if (data.size() - data.rpos() < 1)
            return false;
        HasGuildData = data.ReadBit();
        bool const hasAzeriteLevel = data.ReadBit();
        data.ResetBitPos();
        if (hasAzeriteLevel)
            return false;
        for (InspectPvpBracketEntry& bracket : Brackets)
            if (!bracket.Read(data))
                return false;
        if (HasGuildData && !GuildData.Read(data))
            return false;
        return true;
    }
};

// One arena team slot on the PvP tab (2v2/3v3/5v5). Always empty: vanilla has no arena teams.
struct InspectArenaTeamEntry
{
    ClassicGuid TeamGuid;
    int32 TeamRating = 0;
    int32 TeamGamesPlayed = 0;
    int32 TeamGamesWon = 0;
    int32 PersonalGamesPlayed = 0;
    int32 PersonalRating = 0;

    void Write(ByteBuffer& data) const
    {
        TeamGuid.Write(data);
        data << TeamRating << TeamGamesPlayed << TeamGamesWon << PersonalGamesPlayed << PersonalRating;
    }

    bool Read(ByteBuffer& data)
    {
        if (!TeamGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5 * sizeof(int32))
            return false;
        data >> TeamRating >> TeamGamesPlayed >> TeamGamesWon >> PersonalGamesPlayed >> PersonalRating;
        return true;
    }
};

// Answers CMSG_INSPECT_PVP. Vanilla has no arenas or rated brackets, so this always carries an
// empty bracket list and three empty arena-team slots (2v2/3v3/5v5) -- the client still needs the
// three slots present to render the tab.
struct InspectPvpResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_INSPECT_PVP;

    ClassicGuid PlayerGuid;
    std::vector<InspectPvpBracketEntry> Brackets;
    std::vector<InspectArenaTeamEntry> ArenaTeams;

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data.WriteBits(uint32(Brackets.size()), 3);
        data.WriteBits(uint32(ArenaTeams.size()), 2);
        data.FlushBits();
        for (InspectPvpBracketEntry const& bracket : Brackets)
            bracket.Write(data);
        for (InspectArenaTeamEntry const& team : ArenaTeams)
            team.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const bracketCount = data.ReadBits(3);
        uint32 const teamCount = data.ReadBits(2);
        data.ResetBitPos();
        Brackets.clear();
        for (uint32 i = 0; i < bracketCount; ++i)
        {
            InspectPvpBracketEntry bracket;
            if (!bracket.Read(data))
                return false;
            Brackets.push_back(bracket);
        }
        ArenaTeams.clear();
        for (uint32 i = 0; i < teamCount; ++i)
        {
            InspectArenaTeamEntry team;
            if (!team.Read(data))
                return false;
            ArenaTeams.push_back(team);
        }
        return true;
    }
};

#endif
