#ifndef MANGOS_PACKETS_GUILDMESSAGES_H
#define MANGOS_PACKETS_GUILDMESSAGES_H

// The modern guild wire: query, roster, ranks and permissions, invite/accept/decline/leave/kick,
// promote/demote/leader, MOTD/info text, member notes, the tabard emblem, and the typed
// SMSG_GUILD_EVENT_* broadcasts that replace vanilla's one generic SMSG_GUILD_EVENT. Light like
// Packets/PetitionMessages.h, so RealBots can include it. Guild creation has no modern CMSG at
// all (see PetitionMessages.h's TurnInPetition) and the guild bank is not implemented by this
// engine at all, so neither is covered here.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>
#include <vector>

// Real client body width for CMSG_GUILD_SET_RANK_PERMISSIONS / SMSG_GUILD_RANKS's per-tab arrays.
// This engine has no guild bank, so every tab entry is parsed and discarded, or written empty --
// the count itself is fixed by the wire, not a choice this engine makes.
constexpr uint8 GuildBankMaxTabs = 6;

// --- Client packets ---

struct GuildQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_GUILD_INFO;

    ClassicGuid GuildGuid;
    ClassicGuid PlayerGuid;

    void Write(ByteBuffer& data) const
    {
        GuildGuid.Write(data);
        PlayerGuid.Write(data);
    }

    bool Read(ByteBuffer& data) { return GuildGuid.Read(data) && PlayerGuid.Read(data); }
};

// Replaces vanilla's name-based GuildInvite.
struct GuildInviteByName
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_INVITE_BY_NAME;

    std::string InvitedName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(InvitedName.length()), 9);
        data.WriteBit(false); // Unused910
        data.FlushBits();
        data.WriteString(InvitedName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 9 + 1 bits
            return false;
        uint32 const nameLength = data.ReadBits(9);
        bool const hasUnused910 = data.ReadBit();
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        InvitedName = data.ReadString(nameLength);
        if (hasUnused910)
        {
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            data.read_skip<int32>();
        }
        return true;
    }
};

// Replaces vanilla's name-based GuildRemove.
struct GuildOfficerRemoveMember
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_OFFICER_REMOVE_MEMBER;

    ClassicGuid RemoveeGuid;

    void Write(ByteBuffer& data) const { RemoveeGuid.Write(data); }
    bool Read(ByteBuffer& data) { return RemoveeGuid.Read(data); }
};

// The guild guid is parsed but unused: this engine tracks only one pending invite per player
// (Player::GetGuildIdInvited), the same scope vanilla's bodyless CMSG_GUILD_ACCEPT already had.
struct AcceptGuildInvite
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ACCEPT_GUILD_INVITE;

    ClassicGuid GuildGuid;

    void Write(ByteBuffer& data) const { GuildGuid.Write(data); }
    bool Read(ByteBuffer& data) { return GuildGuid.Read(data); }
};

// The guild guid and IsAuto are parsed but unused, same scope as AcceptGuildInvite above.
struct GuildDeclineInvitation
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_DECLINE_INVITATION;

    ClassicGuid GuildGuid;
    bool IsAuto = false;

    void Write(ByteBuffer& data) const
    {
        GuildGuid.Write(data);
        data.WriteBit(IsAuto);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!GuildGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        IsAuto = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Genuinely empty -- replaces vanilla's no-opcode-at-all "guild info" request; GuildRoster (below)
// now carries the created-date/account-count fields that used to be their own SMSG_GUILD_INFO.
struct GuildGetRoster
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_GET_ROSTER;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The guild guid is parsed but unused: this engine only ever answers for the requester's own
// guild, matching every other guild handler's scope.
struct GuildGetRanks
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_GET_RANKS;

    ClassicGuid GuildGuid;

    void Write(ByteBuffer& data) const { GuildGuid.Write(data); }
    bool Read(ByteBuffer& data) { return GuildGuid.Read(data); }
};

struct GuildPermissionsQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_PERMISSIONS_QUERY;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct GuildPromoteMember
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_PROMOTE_MEMBER;

    ClassicGuid PromoteeGuid;

    void Write(ByteBuffer& data) const { PromoteeGuid.Write(data); }
    bool Read(ByteBuffer& data) { return PromoteeGuid.Read(data); }
};

struct GuildDemoteMember
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_DEMOTE_MEMBER;

    ClassicGuid DemoteeGuid;

    void Write(ByteBuffer& data) const { DemoteeGuid.Write(data); }
    bool Read(ByteBuffer& data) { return DemoteeGuid.Read(data); }
};

struct GuildLeave
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_LEAVE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Disbands the caller's guild -- replaces vanilla's CMSG_GUILD_DISBAND (same body, new opcode).
struct GuildDelete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_DELETE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Replaces vanilla's GuildLeader (same name-based shape, new opcode).
struct GuildSetGuildMaster
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_SET_GUILD_MASTER;

    std::string NewMasterName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(NewMasterName.length()), 9);
        data.FlushBits();
        data.WriteString(NewMasterName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const nameLength = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        NewMasterName = data.ReadString(nameLength);
        return true;
    }
};

// Replaces vanilla's GuildMOTD, which read a plain null-terminated string.
struct GuildUpdateMotdText
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_UPDATE_MOTD_TEXT;

    std::string Motd;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Motd.length()), 11);
        data.FlushBits();
        data.WriteString(Motd);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const textLength = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < textLength)
            return false;
        Motd = data.ReadString(textLength);
        return true;
    }
};

// Replaces vanilla's GuildChangeInfoText.
struct GuildUpdateInfoText
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_UPDATE_INFO_TEXT;

    std::string InfoText;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(InfoText.length()), 11);
        data.FlushBits();
        data.WriteString(InfoText);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const textLength = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < textLength)
            return false;
        InfoText = data.ReadString(textLength);
        return true;
    }
};

struct SaveGuildEmblem
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_SAVE_GUILD_EMBLEM;

    ClassicGuid VendorGuid;
    int32 EmblemStyle = 0;
    int32 EmblemColor = 0;
    int32 BorderStyle = 0;
    int32 BorderColor = 0;
    int32 BackgroundColor = 0;

    void Write(ByteBuffer& data) const
    {
        VendorGuid.Write(data);
        data << EmblemStyle << EmblemColor << BorderStyle << BorderColor << BackgroundColor;
    }

    bool Read(ByteBuffer& data)
    {
        if (!VendorGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5 * sizeof(int32))
            return false;
        data >> EmblemStyle >> EmblemColor >> BorderStyle >> BorderColor >> BackgroundColor;
        return true;
    }
};

// Replaces vanilla's separate GuildSetPublicNote/GuildSetOfficerNote with one opcode; IsPublic
// selects which note (false == officer, true == public).
struct GuildSetMemberNote
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_SET_MEMBER_NOTE;

    ClassicGuid NoteeGuid;
    bool IsPublic = false;
    std::string Note;

    void Write(ByteBuffer& data) const
    {
        NoteeGuid.Write(data);
        data.WriteBits(uint32(Note.length()), 8);
        data.WriteBit(IsPublic);
        data.FlushBits();
        data.WriteString(Note);
    }

    bool Read(ByteBuffer& data)
    {
        if (!NoteeGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const noteLength = data.ReadBits(8);
        IsPublic = data.ReadBit();
        data.ResetBitPos();
        if (data.size() - data.rpos() < noteLength)
            return false;
        Note = data.ReadString(noteLength);
        return true;
    }
};

// RankOrder is parsed but unused: this engine's ranks are always appended (Guild::CreateRank has
// no positional-insert support), matching the old client's own behavior.
struct GuildAddRank
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_ADD_RANK;

    std::string RankName;
    int32 RankOrder = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(RankName.length()), 7);
        data.FlushBits();
        data << RankOrder;
        data.WriteString(RankName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> RankOrder;
        if (data.size() - data.rpos() < nameLength)
            return false;
        RankName = data.ReadString(nameLength);
        return true;
    }
};

// RankOrder is parsed but unused: the real client only ever offers deleting the last (lowest)
// rank, matching Guild::DelRank()'s no-parameter always-remove-last behavior.
struct GuildDeleteRank
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_DELETE_RANK;

    int32 RankOrder = 0;

    void Write(ByteBuffer& data) const { data << RankOrder; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> RankOrder;
        return true;
    }
};

// Replaces vanilla's name+rights-based GuildRank (rename + rights change combined). RankOrder,
// the per-tab bank arrays and OldFlags are parsed but unused -- this engine has no guild bank and
// no separate rank id/order distinction.
struct GuildSetRankPermissions
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GUILD_SET_RANK_PERMISSIONS;

    uint32 RankId = 0;
    uint32 RankOrder = 0;
    uint32 Flags = 0;
    uint32 WithdrawGoldLimit = 0;
    std::string RankName;

    void Write(ByteBuffer& data) const
    {
        data << RankId << RankOrder << Flags << WithdrawGoldLimit;
        for (uint8 i = 0; i < GuildBankMaxTabs; ++i)
            data << uint32(0) << uint32(0); // TabFlags[i], TabWithdrawItemLimit[i]
        data << uint32(0); // OldFlags
        data.WriteBits(uint32(RankName.length()), 7);
        data.FlushBits();
        data.WriteString(RankName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4 * sizeof(uint32) + GuildBankMaxTabs * 2 * sizeof(uint32) + sizeof(uint32))
            return false;
        data >> RankId >> RankOrder >> Flags >> WithdrawGoldLimit;
        for (uint8 i = 0; i < GuildBankMaxTabs; ++i)
        {
            data.read_skip<uint32>();
            data.read_skip<uint32>();
        }
        uint32 oldFlags;
        data >> oldFlags;
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        RankName = data.ReadString(nameLength);
        return true;
    }
};

// --- Server packets ---

struct GuildCommandResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_COMMAND_RESULT;

    uint32 Command = 0;
    std::string Str;
    uint32 Result = 0;

    void Write(ByteBuffer& data) const
    {
        data << int32(Result);
        data << int32(Command);
        data.WriteBits(uint32(Str.length()), 8);
        data.FlushBits();
        data.WriteString(Str);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        int32 result, command;
        data >> result >> command;
        Result = uint32(result);
        Command = uint32(command);
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const strLength = data.ReadBits(8);
        data.ResetBitPos();
        if (data.size() - data.rpos() < strLength)
            return false;
        Str = data.ReadString(strLength);
        return true;
    }
};

struct GuildInviteNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_INVITE;

    std::string InviterName;
    std::string GuildName;
    ClassicGuid GuildGuid; // see ClassicGuidMapping::MakeGuild

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(InviterName.length()), 6);
        data.WriteBits(uint32(GuildName.length()), 7);
        data.WriteBits(0, 7); // OldGuildName length -- no cross-guild-transfer flow
        data.FlushBits();

        data << uint32(0); // InviterVirtualRealmAddress -- single realm
        data << uint32(0); // GuildVirtualRealmAddress
        GuildGuid.Write(data);
        data << uint32(0); // OldGuildVirtualRealmAddress
        ClassicGuid().Write(data); // OldGuildGUID
        data << uint32(0); // EmblemStyle
        data << uint32(0); // EmblemColor
        data << uint32(0); // BorderStyle
        data << uint32(0); // BorderColor
        data << uint32(0); // Background
        data << int32(0);  // AchievementPoints -- no achievements in this engine

        data.WriteString(InviterName);
        data.WriteString(GuildName);
        data.WriteString(std::string());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3)
            return false;
        uint32 const inviterLength = data.ReadBits(6);
        uint32 const guildLength = data.ReadBits(7);
        uint32 const oldGuildLength = data.ReadBits(7);
        data.ResetBitPos();

        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        uint32 unused32;
        data >> unused32 >> unused32; // InviterVirtualRealmAddress, GuildVirtualRealmAddress

        if (!GuildGuid.Read(data))
            return false;

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> unused32; // OldGuildVirtualRealmAddress

        ClassicGuid unusedGuid;
        if (!unusedGuid.Read(data)) // OldGuildGUID
            return false;

        if (data.size() - data.rpos() < 5 * sizeof(uint32) + sizeof(int32))
            return false;
        data >> unused32 >> unused32 >> unused32 >> unused32 >> unused32; // emblem fields
        int32 unused;
        data >> unused; // AchievementPoints

        if (data.size() - data.rpos() < inviterLength + guildLength + oldGuildLength)
            return false;
        InviterName = data.ReadString(inviterLength);
        GuildName = data.ReadString(guildLength);
        data.ReadString(oldGuildLength);
        return true;
    }
};

struct GuildDeclineNotification
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_INVITE_DECLINED;

    std::string PlayerName;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(PlayerName.length()), 6);
        data.FlushBits();
        data.WriteString(PlayerName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(6);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        PlayerName = data.ReadString(nameLength);
        return true;
    }
};

// GuildGuid echoes the request's target guid; ResolvedGuildGuid (only sent when Valid) is the
// answering guild's own guid, built fresh from its numeric id -- a real client capture confirmed
// no player guid rides this response at all, unlike the request.
struct QueryGuildInfoResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_GUILD_INFO_RESPONSE;

    ClassicGuid GuildGuid;
    bool Valid = false;
    ClassicGuid ResolvedGuildGuid;
    uint32 GuildId = 0;
    std::string GuildName;
    std::vector<std::string> RankNames; // index == rank order
    int32 EmblemStyle = 0;
    int32 EmblemColor = 0;
    int32 BorderStyle = 0;
    int32 BorderColor = 0;
    int32 BackgroundColor = 0;

    void Write(ByteBuffer& data) const
    {
        GuildGuid.Write(data);
        data.WriteBit(Valid);
        data.FlushBits();
        if (!Valid)
            return;

        ResolvedGuildGuid.Write(data);
        data << uint32(0); // VirtualRealmAddress -- single realm
        data << uint32(RankNames.size());
        data << uint32(EmblemStyle);
        data << uint32(EmblemColor);
        data << uint32(BorderStyle);
        data << uint32(BorderColor);
        data << uint32(BackgroundColor);
        data.WriteBits(uint32(GuildName.length()), 7);
        data.FlushBits();

        for (size_t i = 0; i < RankNames.size(); ++i)
        {
            data << uint32(i); // RankID
            data << uint32(i); // RankOrder -- this engine has no separate id/order distinction
            data.WriteBits(uint32(RankNames[i].length()), 7);
            data.FlushBits();
            data.WriteString(RankNames[i]);
        }

        data.WriteString(GuildName);
    }

    bool Read(ByteBuffer& data)
    {
        if (!GuildGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Valid = data.ReadBit();
        data.ResetBitPos();
        if (!Valid)
            return true;

        if (!ResolvedGuildGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 7 * sizeof(uint32))
            return false;
        uint32 realmAddress, rankCount;
        data >> realmAddress >> rankCount;
        uint32 emblemStyle, emblemColor, borderStyle, borderColor, backgroundColor;
        data >> emblemStyle >> emblemColor >> borderStyle >> borderColor >> backgroundColor;
        EmblemStyle = int32(emblemStyle);
        EmblemColor = int32(emblemColor);
        BorderStyle = int32(borderStyle);
        BorderColor = int32(borderColor);
        BackgroundColor = int32(backgroundColor);

        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const nameLength = data.ReadBits(7);
        data.ResetBitPos();

        RankNames.clear();
        for (uint32 i = 0; i < rankCount; ++i)
        {
            if (data.size() - data.rpos() < 2 * sizeof(uint32) + 1)
                return false;
            uint32 rankId, rankOrder;
            data >> rankId >> rankOrder;
            uint32 const rankNameLength = data.ReadBits(7);
            data.ResetBitPos();
            if (data.size() - data.rpos() < rankNameLength)
                return false;
            RankNames.push_back(data.ReadString(rankNameLength));
        }

        if (data.size() - data.rpos() < nameLength)
            return false;
        GuildName = data.ReadString(nameLength);
        return true;
    }
};

struct GuildRosterMember
{
    ClassicGuid Guid;
    uint8 PresenceFlags = 0; // offline = 0, online = GuildRosterOnline, ...Afk, ...Dnd
    std::string Name;
    uint32 RankId = 0;
    uint8 Level = 0;
    uint8 ClassId = 0;
    uint8 RaceId = 0; // 0 for an offline member -- this engine's roster cache doesn't track race
    uint32 ZoneId = 0;
    float LastOnlineTime = 0.0f; // time in days since last login; 0 while online
    std::string PublicNote;
    std::string OfficerNote; // empty if the viewer has no permission
};

// Full rewrite from vanilla's flat shape. Fields with no equivalent in this engine (weekly/total
// XP, personal achievement points, guild reputation, dungeon score, cross-realm virtual realm
// address, Battle.net club id) are hardcoded -- the client still expects them present even for a
// Classic-era guild. Follows TrinityCore's exact per-member layout, including GuildClubMemberID
// and RaceID and the trailing (always-empty) mythic-plus dungeon score summary: an earlier
// reference implementation of this struct (vibemangos) dropped both fields and added a bit with
// no counterpart in the real struct at all, which would desync every byte of the roster after the
// first member. See the RealBots README "vibemangos vs TrinityCore" section.
struct GuildRoster
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_ROSTER;

    std::string Motd;
    std::string GuildInfo;
    std::vector<GuildRosterMember> RosterMembers;
    uint32 CreatedDay = 0;
    uint32 CreatedMonth = 0;
    uint32 CreatedYear = 0;
    int32 AccountCount = 0;

    static void WriteMember(ByteBuffer& data, GuildRosterMember const& member)
    {
        member.Guid.Write(data);
        data << int32(member.RankId);
        data << int32(member.ZoneId); // AreaID
        data << int32(-1); // PersonalAchievementPoints -- no achievements in this engine
        data << int32(-1); // GuildReputation -- not tracked
        data << float(member.LastOnlineTime);

        for (uint8 i = 0; i < 2; ++i)
        {
            data << int32(0); // Profession[i].DbID
            data << int32(0); // Profession[i].Rank
            data << int32(0); // Profession[i].Step
        }

        data << uint32(0); // VirtualRealmAddress -- single realm
        data << uint8(member.PresenceFlags);
        data << uint8(member.Level);
        data << uint8(member.ClassId);
        data << uint8(0); // Gender -- not tracked
        data << uint64(0); // GuildClubMemberID -- no Battle.net club in this engine
        data << uint8(member.RaceId);

        data.WriteBits(uint32(member.Name.length()), 6);
        data.WriteBits(uint32(member.PublicNote.length()), 8);
        data.WriteBits(uint32(member.OfficerNote.length()), 8);
        data.WriteBit(true); // Authenticated -- no character-copy/undelete pending state
        data.FlushBits();

        data << float(0.0f); // DungeonScoreSummary::OverallScoreCurrentSeason
        data << float(0.0f); // DungeonScoreSummary::LadderScoreCurrentSeason
        data << uint32(0);   // DungeonScoreSummary::Runs count -- no mythic+ in this engine

        data.WriteString(member.Name);
        data.WriteString(member.PublicNote);
        data.WriteString(member.OfficerNote);
    }

    static bool ReadMember(ByteBuffer& data, GuildRosterMember& member)
    {
        if (!member.Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(int32) + sizeof(float) + 2 * 3 * sizeof(int32) + sizeof(uint32) + 4 * sizeof(uint8) + sizeof(uint64) + sizeof(uint8))
            return false;
        int32 rankId, zoneId, unusedAchievementPoints, unusedReputation;
        data >> rankId >> zoneId >> unusedAchievementPoints >> unusedReputation;
        member.RankId = uint32(rankId);
        member.ZoneId = uint32(zoneId);
        float lastOnlineTime;
        data >> lastOnlineTime;
        member.LastOnlineTime = lastOnlineTime;

        for (uint8 i = 0; i < 2; ++i)
        {
            int32 unused;
            data >> unused >> unused >> unused;
        }

        uint32 unusedRealmAddress;
        data >> unusedRealmAddress;
        uint8 presenceFlags, level, classId, unusedGender;
        data >> presenceFlags >> level >> classId >> unusedGender;
        member.PresenceFlags = presenceFlags;
        member.Level = level;
        member.ClassId = classId;
        uint64 unusedClubId;
        data >> unusedClubId;
        uint8 raceId;
        data >> raceId;
        member.RaceId = raceId;

        if (data.size() - data.rpos() < 3) // 6 + 8 + 8 + 1 bits
            return false;
        uint32 const nameLength = data.ReadBits(6);
        uint32 const publicNoteLength = data.ReadBits(8);
        uint32 const officerNoteLength = data.ReadBits(8);
        data.ReadBit(); // Authenticated
        data.ResetBitPos();

        if (data.size() - data.rpos() < 2 * sizeof(float) + sizeof(uint32))
            return false;
        float unusedScore;
        uint32 unusedRunCount;
        data >> unusedScore >> unusedScore >> unusedRunCount;
        if (unusedRunCount != 0)
            return false; // this engine never sends a non-empty dungeon score

        if (data.size() - data.rpos() < nameLength + publicNoteLength + officerNoteLength)
            return false;
        member.Name = data.ReadString(nameLength);
        member.PublicNote = data.ReadString(publicNoteLength);
        member.OfficerNote = data.ReadString(officerNoteLength);
        return true;
    }

    void Write(ByteBuffer& data) const
    {
        data << int32(AccountCount);
        // WowTime CreateDate -- packed date; this engine tracks day/month/year separately rather
        // than a WowTime object, so this hand-encodes just the date bits WowTime::GetPackedTime()
        // would produce: minute:6(0) | hour:5(6) | weekDay:3(11) | monthDay:6(14) | month:4(20) |
        // year%100:5(24) | flags:2(29). Time-of-day/weekday/flags are 0 -- only the creation date
        // is tracked.
        {
            uint32 const packedYear = (CreatedYear % 100) & 0x1F;
            uint32 const packedMonth = (CreatedMonth ? CreatedMonth - 1 : 0) & 0xF;
            uint32 const packedDay = (CreatedDay ? CreatedDay - 1 : 0) & 0x3F;
            data << uint32((packedDay << 14) | (packedMonth << 20) | (packedYear << 24));
        }
        data << int32(2); // GuildFlags -- matches a real client capture of a working roster response
        data << uint32(RosterMembers.size());
        data.WriteBits(uint32(Motd.length()), 11);
        data.WriteBits(uint32(GuildInfo.length()), 11);
        data.FlushBits();

        for (GuildRosterMember const& member : RosterMembers)
            WriteMember(data, member);

        data.WriteString(Motd);
        data.WriteString(GuildInfo);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32) + 2 * sizeof(uint32))
            return false;
        int32 accountCount;
        data >> accountCount;
        AccountCount = accountCount;
        uint32 packedTime;
        data >> packedTime;
        CreatedDay = ((packedTime >> 14) & 0x3F) + 1;
        CreatedMonth = ((packedTime >> 20) & 0xF) + 1;
        CreatedYear = ((packedTime >> 24) & 0x1F) + 2000;
        int32 guildFlags;
        data >> guildFlags;
        uint32 memberCount;
        data >> memberCount;

        if (data.size() - data.rpos() < 3)
            return false;
        uint32 const motdLength = data.ReadBits(11);
        uint32 const guildInfoLength = data.ReadBits(11);
        data.ResetBitPos();

        RosterMembers.clear();
        for (uint32 i = 0; i < memberCount; ++i)
        {
            GuildRosterMember member;
            if (!ReadMember(data, member))
                return false;
            RosterMembers.push_back(std::move(member));
        }

        if (data.size() - data.rpos() < motdLength + guildInfoLength)
            return false;
        Motd = data.ReadString(motdLength);
        GuildInfo = data.ReadString(guildInfoLength);
        return true;
    }
};

struct GuildPermissionsQueryResults
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_PERMISSIONS_QUERY_RESULTS;

    uint32 RankId = 0;
    int32 Flags = 0;
    int32 WithdrawGoldLimit = 0; // no guild bank in this engine
    int32 NumTabs = 0;           // no guild bank in this engine

    void Write(ByteBuffer& data) const
    {
        data << RankId;
        data << Flags;
        data << WithdrawGoldLimit;
        data << NumTabs;
        data << uint32(0); // Tab count -- no guild bank in this engine
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4 * sizeof(int32) + sizeof(uint32))
            return false;
        data >> RankId >> Flags >> WithdrawGoldLimit >> NumTabs;
        uint32 tabCount;
        data >> tabCount;
        for (uint32 i = 0; i < tabCount; ++i)
        {
            if (data.size() - data.rpos() < 2 * sizeof(int32))
                return false;
            data.read_skip<int32>();
            data.read_skip<int32>();
        }
        return true;
    }
};

struct GuildRankEntry
{
    uint8 RankId = 0;
    int32 RankOrder = 0;
    uint32 Flags = 0;
    uint32 WithdrawGoldLimit = 0; // no guild bank in this engine
    std::string RankName;
};

struct GuildRanksResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_RANKS;

    std::vector<GuildRankEntry> Ranks;

    void Write(ByteBuffer& data) const
    {
        data << uint32(Ranks.size());
        for (GuildRankEntry const& rank : Ranks)
        {
            data << uint8(rank.RankId);
            data << int32(rank.RankOrder);
            data << uint32(rank.Flags);
            data << uint32(rank.WithdrawGoldLimit);
            for (uint8 i = 0; i < GuildBankMaxTabs; ++i)
                data << uint32(0) << uint32(0); // TabFlags[i], TabWithdrawItemLimit[i]
            data.WriteBits(uint32(rank.RankName.length()), 7);
            data.FlushBits();
            data.WriteString(rank.RankName);
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        Ranks.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            if (data.size() - data.rpos() < sizeof(uint8) + sizeof(int32) + 2 * sizeof(uint32) + GuildBankMaxTabs * 2 * sizeof(uint32))
                return false;
            GuildRankEntry rank;
            uint8 rankId;
            data >> rankId;
            rank.RankId = rankId;
            data >> rank.RankOrder >> rank.Flags >> rank.WithdrawGoldLimit;
            for (uint8 t = 0; t < GuildBankMaxTabs; ++t)
            {
                data.read_skip<uint32>();
                data.read_skip<uint32>();
            }
            if (data.size() - data.rpos() < 1)
                return false;
            uint32 const nameLength = data.ReadBits(7);
            data.ResetBitPos();
            if (data.size() - data.rpos() < nameLength)
                return false;
            rank.RankName = data.ReadString(nameLength);
            Ranks.push_back(std::move(rank));
        }
        return true;
    }
};

struct GuildEventMotd
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_MOTD;

    std::string MotdText;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(MotdText.length()), 11);
        data.FlushBits();
        data.WriteString(MotdText);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(11);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        MotdText = data.ReadString(length);
        return true;
    }
};

// Sent to the whole guild when a member logs on or off.
struct GuildEventPresenceChange
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_PRESENCE_CHANGE;

    ClassicGuid Guid;
    std::string Name;
    bool LoggedOn = false;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << uint32(0); // VirtualRealmAddress -- single realm
        data.WriteBits(uint32(Name.length()), 6);
        data.WriteBit(LoggedOn);
        data.WriteBit(false); // Mobile -- no mobile companion app in this engine
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        uint32 unusedRealmAddress;
        data >> unusedRealmAddress;
        uint32 const nameLength = data.ReadBits(6);
        LoggedOn = data.ReadBit();
        data.ReadBit(); // Mobile
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

struct GuildEventPlayerJoined
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_PLAYER_JOINED;

    ClassicGuid Guid;
    std::string Name;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << uint32(0); // VirtualRealmAddress -- single realm
        data.WriteBits(uint32(Name.length()), 6);
        data.FlushBits();
        data.WriteString(Name);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        uint32 unusedRealmAddress;
        data >> unusedRealmAddress;
        uint32 const nameLength = data.ReadBits(6);
        data.ResetBitPos();
        if (data.size() - data.rpos() < nameLength)
            return false;
        Name = data.ReadString(nameLength);
        return true;
    }
};

// Covers both a voluntary leave (Removed = false) and a kick (Removed = true, remover fields
// set) -- there is no separate modern opcode for "kicked" vs "left".
struct GuildEventPlayerLeft
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_PLAYER_LEFT;

    ClassicGuid LeaverGuid;
    std::string LeaverName;
    ClassicGuid RemoverGuid;
    std::string RemoverName;
    bool Removed = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Removed);
        data.WriteBits(uint32(LeaverName.length()), 6);
        if (Removed)
            data.WriteBits(uint32(RemoverName.length()), 6);
        data.FlushBits();

        if (Removed)
        {
            RemoverGuid.Write(data);
            data << uint32(0); // RemoverVirtualRealmAddress -- single realm
            data.WriteString(RemoverName);
        }

        LeaverGuid.Write(data);
        data << uint32(0); // LeaverVirtualRealmAddress -- single realm
        data.WriteString(LeaverName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Removed = data.ReadBit();
        uint32 const leaverLength = data.ReadBits(6);
        uint32 removerLength = 0;
        if (Removed)
        {
            // The 7 bits read so far leave 1 bit of the current byte unused; the 6-bit
            // RemoverName length can still spill into a fresh byte, so check before reading it.
            if (data.size() - data.rpos() < 1)
                return false;
            removerLength = data.ReadBits(6);
        }
        data.ResetBitPos();

        if (Removed)
        {
            if (!RemoverGuid.Read(data))
                return false;
            if (data.size() - data.rpos() < sizeof(uint32))
                return false;
            uint32 unusedRealmAddress;
            data >> unusedRealmAddress;
            if (data.size() - data.rpos() < removerLength)
                return false;
            RemoverName = data.ReadString(removerLength);
        }

        if (!LeaverGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 unusedRealmAddress;
        data >> unusedRealmAddress;
        if (data.size() - data.rpos() < leaverLength)
            return false;
        LeaverName = data.ReadString(leaverLength);
        return true;
    }
};

struct GuildEventNewLeader
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_NEW_LEADER;

    ClassicGuid NewLeaderGuid;
    std::string NewLeaderName;
    ClassicGuid OldLeaderGuid;
    std::string OldLeaderName;
    bool SelfPromoted = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(SelfPromoted);
        data.WriteBits(uint32(OldLeaderName.length()), 6);
        data.WriteBits(uint32(NewLeaderName.length()), 6);
        data.FlushBits();

        OldLeaderGuid.Write(data);
        data << uint32(0); // OldLeaderVirtualRealmAddress -- single realm
        NewLeaderGuid.Write(data);
        data << uint32(0); // NewLeaderVirtualRealmAddress

        data.WriteString(OldLeaderName);
        data.WriteString(NewLeaderName);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 1 + 6 + 6 bits
            return false;
        SelfPromoted = data.ReadBit();
        uint32 const oldLength = data.ReadBits(6);
        uint32 const newLength = data.ReadBits(6);
        data.ResetBitPos();

        if (!OldLeaderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 unusedRealmAddress;
        data >> unusedRealmAddress;
        if (!NewLeaderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> unusedRealmAddress;

        if (data.size() - data.rpos() < oldLength + newLength)
            return false;
        OldLeaderName = data.ReadString(oldLength);
        NewLeaderName = data.ReadString(newLength);
        return true;
    }
};

// Sent to the whole guild: "X has been promoted/demoted to Y by Z". GuildEventRanksUpdated below
// is the separate "the rank list changed, refresh it" signal.
struct GuildSendRankChange
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_SEND_RANK_CHANGE;

    ClassicGuid OfficerGuid;
    ClassicGuid TargetGuid;
    uint32 RankId = 0;
    bool Promote = false;

    void Write(ByteBuffer& data) const
    {
        OfficerGuid.Write(data);
        TargetGuid.Write(data);
        data << RankId;
        data.WriteBit(Promote);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!OfficerGuid.Read(data) || !TargetGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> RankId;
        Promote = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Broadcast after a single rank's name/rights are edited (not the rank list itself).
struct GuildEventRankChanged
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_RANK_CHANGED;

    int32 RankId = 0;

    void Write(ByteBuffer& data) const { data << RankId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> RankId;
        return true;
    }
};

// Broadcast after AddRank/DelRank (the rank list changed) -- the client relies on this to know to
// re-request rank/roster data itself; resending the roster or query response unsolicited does not
// refresh an already-open ranks UI.
struct GuildEventRanksUpdated
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_RANKS_UPDATED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct GuildEventDisbanded
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GUILD_EVENT_DISBANDED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct SaveGuildEmblemResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PLAYER_SAVE_GUILD_EMBLEM;

    int32 Error = 0;

    void Write(ByteBuffer& data) const { data << Error; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> Error;
        return true;
    }
};

#endif
