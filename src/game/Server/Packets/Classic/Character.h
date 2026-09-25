#ifndef MANGOS_CLASSIC_CHARACTER_H
#define MANGOS_CLASSIC_CHARACTER_H

#include "ByteBuffer.h"
#include "Guid.h"
#include "ClassicOpcodes.h"
#include <string>
#include <vector>

// ResponseCodes (the client's own values): the character-screen results a create, a delete and a
// name check answer with.
constexpr uint8 CLASSIC_CHAR_CREATE_SUCCESS = 24;
constexpr uint8 CLASSIC_CHAR_CREATE_ERROR = 25;
constexpr uint8 CLASSIC_CHAR_CREATE_FAILED = 26;
constexpr uint8 CLASSIC_CHAR_CREATE_NAME_IN_USE = 27;
constexpr uint8 CLASSIC_CHAR_CREATE_SERVER_LIMIT = 30;  // this realm holds no more characters
constexpr uint8 CLASSIC_CHAR_CREATE_ACCOUNT_LIMIT = 31; // this account holds no more
constexpr uint8 CLASSIC_CHAR_DELETE_SUCCESS = 65;
constexpr uint8 CLASSIC_CHAR_DELETE_FAILED = 66;
// Name validation results: CHAR_NAME_FAILURE .. CHAR_NAME_SPACES_DISALLOWED (a create with a bad name
// is answered with one of these in place of a CHAR_CREATE_* code).
constexpr uint8 CLASSIC_CHAR_NAME_SUCCESS = 92;
constexpr uint8 CLASSIC_CHAR_NAME_FAILURE = 93;
constexpr uint8 CLASSIC_CHAR_NAME_NO_NAME = 94;
constexpr uint8 CLASSIC_CHAR_NAME_TOO_SHORT = 95;
constexpr uint8 CLASSIC_CHAR_NAME_TOO_LONG = 96;
constexpr uint8 CLASSIC_CHAR_NAME_INVALID_CHARACTER = 97;
constexpr uint8 CLASSIC_CHAR_NAME_SPACES_DISALLOWED = 109;

inline bool ClassicIsCharNameResult(uint8 result)
{
    return result >= CLASSIC_CHAR_NAME_FAILURE && result <= CLASSIC_CHAR_NAME_SPACES_DISALLOWED;
}

struct ClassicEnumCharacters
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENUM_CHARACTERS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// One entry of SMSG_ENUM_CHARACTERS_RESULT: the fields a bot or scenario wants of a character, out of
// the fixed-layout CharacterInfoBasic block (everything after them -- equipment visuals, tabard,
// professions, ... -- is parsed past and dropped).
struct ClassicCharacterInfo
{
    ClassicGuid Guid;
    uint8 ListPosition = 0;
    uint8 Race = 0;
    uint8 Gender = 0;
    uint8 Class = 0;
    uint8 Level = 0;
    int32 Map = 0;
    int32 Zone = 0;
    float X = 0.0f, Y = 0.0f, Z = 0.0f;
    uint32 CharFlags = 0;
    bool FirstLogin = false;
    std::string Name;
};

// Read parses every character's fixed-layout block into Characters when the body matches it exactly
// (no mail senders, which would add variable-length data after a character), so scenarios can see
// names and guids. When it does not -- a server with a different layout, or a body cut short --
// Read still succeeds with what CharSelect needs, the count and the first guid, and leaves
// Characters empty: the rest of the body (race-unlock list, ...) is real wire shape but not needed,
// and leaving it unread is safe, every packet being length-framed (the same reasoning
// AuthResponse::Read above relies on).
struct ClassicEnumCharactersResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENUM_CHARACTERS_RESULT;
    static constexpr uint32 EQUIPMENT_SLOTS = 19;

    uint32 CharacterCount = 0;
    ClassicGuid FirstCharacterGuid;
    std::vector<ClassicCharacterInfo> Characters; // empty unless every character parsed

    bool Read(ByteBuffer& data)
    {
        // 9 header bits (Success, Realmless, IsDeletedCharacters, IsNewPlayerRestrictionSkipped,
        // IsNewPlayerRestricted, IsNewPlayer, IsTrialAccountRestricted,
        // DisabledClassesMask.HasValue, DontCreateCharacterDisplays), flushed to a byte. Only
        // DisabledClassesMask.HasValue (bit 7) changes how many fields follow.
        bool disabledClassesMaskPresent = false;
        for (int i = 0; i < 9; ++i)
        {
            bool const bit = data.ReadBit();
            if (i == 7)
                disabledClassesMaskPresent = bit;
        }
        data.ResetBitPos();

        // CharacterCount, then 6 more uint32/int32 fields (RegionwideCharacters.Count,
        // MaxCharacterLevel, RaceUnlockData.Count, UnlockedConditionalAppearances.Count,
        // RaceLimitDisables.Count, WarbandGroups.Count), then DisabledClassesMask only if its
        // header bit was set, before the per-character list starts.
        size_t const fixedFields = 7 + (disabledClassesMaskPresent ? 1 : 0);
        if (data.size() - data.rpos() < sizeof(uint32) * fixedFields)
            return false;
        data >> CharacterCount;
        for (int i = 0; i < 6; ++i)
        {
            uint32 skip;
            data >> skip;
        }
        if (disabledClassesMaskPresent)
        {
            uint32 skip;
            data >> skip;
        }

        Characters.clear();
        if (CharacterCount == 0)
            return true;

        // The full per-character parse if the layout matches; else the lenient subset.
        size_t const listStart = data.rpos();
        if (ReadCharacters(data))
        {
            FirstCharacterGuid = Characters[0].Guid;
            return true;
        }
        Characters.clear();
        data.rpos(listStart);
        return FirstCharacterGuid.Read(data);
    }

    // With Characters set: the whole fixed layout for each (unknown fields zero, no mail); without:
    // only the parsed subset (Success set, CharacterCount, the seven other header fields zeroed, the
    // first guid).
    void Write(ByteBuffer& data) const
    {
        for (int i = 0; i < 9; ++i)
            data.WriteBit(i == 0);
        data.FlushBits();

        // DisabledClassesMask.HasValue (bit 7 above) is always false, so its field is omitted here
        // too -- matching Read, which only consumes it when that bit is set.
        data << (Characters.empty() ? CharacterCount : uint32(Characters.size()));
        for (int i = 0; i < 6; ++i)
            data << uint32(0);

        if (Characters.empty())
        {
            if (CharacterCount > 0)
                FirstCharacterGuid.Write(data);
            return;
        }

        for (ClassicCharacterInfo const& c : Characters)
        {
            c.Guid.Write(data);
            data << uint32(0);                 // VirtualRealmAddress
            data << c.ListPosition << c.Race << c.Gender << c.Class;
            data << int16(0);                  // SpecID
            data << uint32(0);                 // customization count
            data << c.Level;
            data << c.Map << c.Zone << c.X << c.Y << c.Z;
            data << uint64(0);                 // GuildClubMemberID
            ClassicGuid().Write(data);         // guild guid
            data << c.CharFlags << uint32(0) << uint32(0); // CharFlags, Flags2, Flags3
            data << uint8(0);                  // unkWod61x
            data << uint32(0) << uint32(0) << uint32(0); // pet display, level, family
            for (uint32 slot = 0; slot < EQUIPMENT_SLOTS; ++slot)
            {
                data << uint32(0) << uint8(0) << uint32(0) << uint8(0) << int32(0) << uint32(0) << uint32(0);
            }
            data << int32(0);                  // Unknown703
            data << uint64(0);                 // LastPlayedTime
            data << int32(0);                  // LastLoginVersion
            for (int i = 0; i < 5; ++i)
                data << int32(-1);             // PersonalTabard
            data << uint32(0) << uint32(0);    // ProfessionIds
            data << int32(0);                  // TimerunningSeasonID
            data << uint32(0);                 // OverrideSelectScreenFileDataID
            data.WriteBits(uint32(c.Name.size()), 6);
            data.WriteBit(c.FirstLogin);
            data.FlushBits();
            data.append(reinterpret_cast<uint8 const*>(c.Name.data()), c.Name.size());
            data.WriteBit(false);              // BoostInProgress
            data.WriteBit(false);              // RpeResetAvailable
            data.WriteBit(false);              // RpeResetQuestClearAvailable
            data.FlushBits();
            data << uint32(0) << uint32(0) << uint32(0); // Flags4, mail senders, mail sender types
        }
    }

    private:
        // Parses CharacterCount characters starting at the current position. False on any shortfall
        // or on a mail-sender count (variable data this does not walk); the caller rewinds.
        bool ReadCharacters(ByteBuffer& data)
        {
            auto const need = [&](size_t bytes) { return data.size() - data.rpos() >= bytes; };
            size_t const equipmentBytes = EQUIPMENT_SLOTS * (4 + 1 + 4 + 1 + 4 + 4 + 4);

            for (uint32 i = 0; i < CharacterCount; ++i)
            {
                ClassicCharacterInfo c;
                if (!c.Guid.Read(data))
                    return false;

                // VirtualRealmAddress(4) ListPosition..Class(4) SpecID(2) customizationCount(4) Level(1)
                // Map(4) Zone(4) X/Y/Z(12) GuildClubMemberID(8)
                if (!need(4 + 4 + 2 + 4 + 1 + 4 + 4 + 12 + 8))
                    return false;
                uint32 virtualRealm; data >> virtualRealm;
                data >> c.ListPosition >> c.Race >> c.Gender >> c.Class;
                int16 spec; data >> spec;
                uint32 customizationCount; data >> customizationCount;
                data >> c.Level >> c.Map >> c.Zone >> c.X >> c.Y >> c.Z;
                uint64 guildClubMember; data >> guildClubMember;

                ClassicGuid guild;
                if (!guild.Read(data))
                    return false;

                // CharFlags, Flags2, Flags3 (12), unkWod61x (1), pet display/level/family (12),
                // the equipment block, Unknown703 (4), LastPlayedTime (8), LastLoginVersion (4),
                // PersonalTabard (20), ProfessionIds (8), TimerunningSeasonID (4),
                // OverrideSelectScreenFileDataID (4)
                if (!need(12 + 1 + 12 + equipmentBytes + 4 + 8 + 4 + 20 + 8 + 4 + 4))
                    return false;
                uint32 flags2, flags3; data >> c.CharFlags >> flags2 >> flags3;
                data.rpos(data.rpos() + 1 + 12 + equipmentBytes + 4 + 8 + 4 + 20 + 8 + 4 + 4);

                if (customizationCount > 1024 || !need(size_t(customizationCount) * 8 + 1))
                    return false;
                data.rpos(data.rpos() + size_t(customizationCount) * 8);

                uint32 nameLength = data.ReadBits(6);
                c.FirstLogin = data.ReadBit();
                data.ResetBitPos();
                if (!need(nameLength + 1 + 12))
                    return false;
                c.Name = data.ReadString(nameLength);

                // Restriction bits (one byte), Flags4, MailSenders.Count, MailSenderTypes.Count.
                data.ReadBit(); data.ReadBit(); data.ReadBit();
                data.ResetBitPos();
                uint32 flags4, mailSenders, mailSenderTypes;
                data >> flags4 >> mailSenders >> mailSenderTypes;
                if (mailSenders != 0 || mailSenderTypes != 0)
                    return false;

                Characters.push_back(std::move(c));
            }
            return true;
        }
};

#endif
