#ifndef MANGOS_PACKETS_BATTLEGROUNDMESSAGES_H
#define MANGOS_PACKETS_BATTLEGROUNDMESSAGES_H

// The modern battleground wire: queueing, status, port, the scoreboard and the flag-carrier
// minimap blips. Light like Packets/DuelMessages.h. The battleground simulation itself (BattleGround,
// BattleGroundMgr, the AV/AB/WS scripts) is unchanged -- only the packets it talks through are new.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <vector>

// Wire-level team id for the scoreboard, confirmed against TrinityCore's PvPTeamId: Horde=0,
// Alliance=1, Neutral=2. Deliberately not this engine's own Team enum (ALLIANCE/HORDE), whose
// values are unrelated faction-template ids, not wire values.
constexpr uint8 CLASSIC_PVP_TEAM_HORDE = 0;
constexpr uint8 CLASSIC_PVP_TEAM_ALLIANCE = 1;
constexpr uint8 CLASSIC_PVP_TEAM_NEUTRAL = 2;

// Shared with TrinityCore's LFG::RideTicket -- reused by both the LFG system (which this engine
// doesn't have) and the battleground queue on the modern client. Since there's no LFG system to
// unify with, BattleGroundMgr mints its own simple incrementing ticket id per queued player; the
// client only ever echoes back whatever ticket it was given, it never validates uniqueness itself.
struct RideTicket
{
    static constexpr uint32 TypeBattlegrounds = 1;

    ClassicGuid RequesterGuid;
    uint32 Id = 0;
    uint32 Type = TypeBattlegrounds;
    int64 Time = 0;

    void Write(ByteBuffer& data) const
    {
        RequesterGuid.Write(data);
        data << Id << Type << Time;
    }

    bool Read(ByteBuffer& data)
    {
        if (!RequesterGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32) + sizeof(int64))
            return false;
        data >> Id >> Type >> Time;
        return true;
    }
};

// Shared header for the SMSG_BATTLEFIELD_STATUS_* family. TrinityCore's own header carries a list
// of queue ids (multiple simultaneous rated/arena queues); this engine only ever has exactly one
// queue id per status push, so a single-element list is always serialized.
struct BattlefieldStatusHeader
{
    RideTicket Ticket;
    uint64 QueueId = 0;
    uint8 RangeMin = 0;
    uint8 RangeMax = 0;
    uint8 TeamSize = 0;
    uint32 InstanceId = 0;
    bool RegisteredMatch = false;
    bool TournamentRules = false;

    void Write(ByteBuffer& data) const
    {
        Ticket.Write(data);
        data << uint32(1); // queue id count -- this engine only ever queues one battleground at a time
        data << RangeMin << RangeMax << TeamSize << InstanceId << QueueId;
        data.WriteBit(RegisteredMatch);
        data.WriteBit(TournamentRules);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ticket.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 queueIdCount;
        data >> queueIdCount;
        if (queueIdCount != 1)
            return false;
        if (data.size() - data.rpos() < 3 + sizeof(uint32) + sizeof(uint64))
            return false;
        data >> RangeMin >> RangeMax >> TeamSize >> InstanceId >> QueueId;
        if (data.size() - data.rpos() < 1)
            return false;
        RegisteredMatch = data.ReadBit();
        TournamentRules = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// --- Client packets ---

struct AreaSpiritHealerQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AREA_SPIRIT_HEALER_QUERY;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

struct AreaSpiritHealerQueue
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AREA_SPIRIT_HEALER_QUEUE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

struct BattlemasterHello
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BATTLEMASTER_HELLO;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// QueueId is the packed BattlegroundQueueTypeId (TrinityCore's BattlegroundQueueTypeId::GetPacked):
// the low 16 bits are the BattlemasterListId, numerically identical to BattleGroundTypeId for this
// server (AV=1, WS=2, AB=3), decoded directly with no lookup table needed. Roles/BlacklistMap/
// Verification are read but otherwise unused: they only mean something to TrinityCore's LFG
// role-check and cross-realm blacklist systems, neither of which this engine has.
struct BattlemasterJoin
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BATTLEMASTER_JOIN;

    uint64 QueueId = 0;
    uint8 Roles = 0;
    int32 BlacklistMap[2] = {};
    ClassicGuid BattlemasterGuid; // battlemaster guid, or the player's own guid if queueing from a portal
    int32 Verification = 0;
    int32 BattlefieldInstanceId = 0;
    bool AsGroup = false;

    void Write(ByteBuffer& data) const
    {
        data << QueueId << Roles << BlacklistMap[0] << BlacklistMap[1];
        BattlemasterGuid.Write(data);
        data << Verification << BattlefieldInstanceId;
        data.WriteBit(AsGroup);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint64) + 1 + 2 * sizeof(int32))
            return false;
        data >> QueueId >> Roles >> BlacklistMap[0] >> BlacklistMap[1];
        if (!BattlemasterGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32) + 1)
            return false;
        data >> Verification >> BattlefieldInstanceId;
        AsGroup = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Replaces the old mapId+action shape entirely: the ticket identifies which queued battleground
// this refers to (matched against the ticket id stashed on the player's queue slot), and a single
// bit says whether the invite was accepted or declined.
struct BattleFieldPort
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BATTLEFIELD_PORT;

    RideTicket Ticket;
    bool AcceptedInvite = false;

    void Write(ByteBuffer& data) const
    {
        Ticket.Write(data);
        data.WriteBit(AcceptedInvite);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ticket.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        AcceptedInvite = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Empty body -- the modern client sends no mapId (the pre-1.14.2 field), so leaving a battleground
// trusts server-side battleground/map state instead.
struct LeaveBattlefield
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BATTLEFIELD_LEAVE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// ListId is the BattlemasterListId, numerically identical to BattleGroundTypeId here (see
// BattlemasterJoin's comment) -- replaces the old raw mapId field.
struct BattlefieldListRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_BATTLEFIELD_LIST;

    int32 ListId = 0;

    void Write(ByteBuffer& data) const { data << ListId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> ListId;
        return true;
    }
};

// Empty body -- the scoreboard request itself carries nothing; the reply is whichever battleground
// the requesting player is currently in.
struct PVPLogDataRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PVP_LOG_DATA;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Replaces the old empty CMSG_BATTLEFIELD_STATUS: still an empty body, still means "resend the
// status of every queue slot I'm in".
struct RequestBattlefieldStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_BATTLEFIELD_STATUS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Purely a UI feature-gate query, irrelevant to gameplay. Empty body.
struct GetPVPOptionsEnabled
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_GET_PVP_OPTIONS_ENABLED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Empty body. This engine has no rated PvP/arena system, so the reply is always all-zero.
struct RequestRatedPvpInfo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REQUEST_RATED_PVP_INFO;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct ReportPvPPlayerAFK
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_REPORT_PVP_PLAYER_AFK;

    ClassicGuid Offender;

    void Write(ByteBuffer& data) const { Offender.Write(data); }
    bool Read(ByteBuffer& data) { return Offender.Read(data); }
};

// --- Server packets ---

struct BattlefieldStatusNone
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_STATUS_NONE;

    RideTicket Ticket;

    void Write(ByteBuffer& data) const { Ticket.Write(data); }
    bool Read(ByteBuffer& data) { return Ticket.Read(data); }
};

struct BattlefieldStatusQueued
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_STATUS_QUEUED;

    BattlefieldStatusHeader Header;
    uint32 AverageWaitTime = 0;
    uint32 WaitTime = 0;
    bool AsGroup = false;
    bool EligibleForMatchmaking = true;
    bool SuspendedQueue = false;

    void Write(ByteBuffer& data) const
    {
        Header.Write(data);
        data << AverageWaitTime << WaitTime;
        data.WriteBit(AsGroup);
        data.WriteBit(EligibleForMatchmaking);
        data.WriteBit(SuspendedQueue);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Header.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        data >> AverageWaitTime >> WaitTime;
        if (data.size() - data.rpos() < 1)
            return false;
        AsGroup = data.ReadBit();
        EligibleForMatchmaking = data.ReadBit();
        SuspendedQueue = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct BattlefieldStatusNeedConfirmation
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_STATUS_NEED_CONFIRMATION;

    BattlefieldStatusHeader Header;
    uint32 MapId = 0;
    uint32 Timeout = 0;
    uint8 Role = 0;

    void Write(ByteBuffer& data) const
    {
        Header.Write(data);
        data << MapId << Timeout << Role;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Header.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(uint32) + 1)
            return false;
        data >> MapId >> Timeout >> Role;
        return true;
    }
};

struct BattlefieldStatusActive
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_STATUS_ACTIVE;

    BattlefieldStatusHeader Header;
    uint32 MapId = 0;
    uint32 ShutdownTimer = 0; // ms until auto-leave (0 while the match is still running)
    uint32 StartTimer = 0;    // ms elapsed since the match started
    bool ArenaFaction = false;
    bool LeftEarly = false;

    void Write(ByteBuffer& data) const
    {
        Header.Write(data);
        data << MapId << ShutdownTimer << StartTimer;
        data.WriteBit(ArenaFaction);
        data.WriteBit(LeftEarly);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Header.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> MapId >> ShutdownTimer >> StartTimer;
        if (data.size() - data.rpos() < 1)
            return false;
        ArenaFaction = data.ReadBit();
        LeftEarly = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Join/queue attempt failed (deserter, queue full, ...). Reason uses GroupJoinBattlegroundResult
// (BattleGroundDefines.h), TrinityCore's own wire-level codes.
struct BattlefieldStatusFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_STATUS_FAILED;

    RideTicket Ticket;
    uint64 QueueId = 0;
    int32 Reason = 0;
    ClassicGuid ClientId;

    void Write(ByteBuffer& data) const
    {
        Ticket.Write(data);
        data << QueueId << Reason;
        ClientId.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Ticket.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint64) + sizeof(int32))
            return false;
        data >> QueueId >> Reason;
        return ClientId.Read(data);
    }
};

struct BattlefieldList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEFIELD_LIST;

    ClassicGuid BattlemasterGuid;
    int32 BattlemasterListId = 0; // BattlemasterListId, numerically == BattleGroundTypeId here
    uint8 MinLevel = 0;
    uint8 MaxLevel = 0;
    // Retail dropped per-instance picking, but Classic Era preserves the original vanilla UX of
    // listing/choosing among active instances -- the same data the pre-modern packet sent.
    std::vector<int32> Battlefields;
    bool PvpAnywhere = false;
    bool HasRandomWinToday = false;

    void Write(ByteBuffer& data) const
    {
        int32 const verification = 121761856; // HARDCODE: matches the client's own hardcoded value (purpose unclear even upstream)
        BattlemasterGuid.Write(data);
        data << verification << BattlemasterListId << MinLevel << MaxLevel;
        data << uint32(Battlefields.size());
        for (int32 id : Battlefields)
            data << id;
        data.WriteBit(PvpAnywhere);
        data.WriteBit(HasRandomWinToday);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!BattlemasterGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32) * 2 + 2 + sizeof(uint32))
            return false;
        int32 verification;
        uint32 count;
        data >> verification >> BattlemasterListId >> MinLevel >> MaxLevel >> count;
        if (data.size() - data.rpos() < count * sizeof(int32))
            return false;
        Battlefields.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            int32 id;
            data >> id;
            Battlefields.push_back(id);
        }
        if (data.size() - data.rpos() < 1)
            return false;
        PvpAnywhere = data.ReadBit();
        HasRandomWinToday = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct BattlegroundPlayerJoined
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEGROUND_PLAYER_JOINED;

    ClassicGuid PlayerGuid;

    void Write(ByteBuffer& data) const { PlayerGuid.Write(data); }
    bool Read(ByteBuffer& data) { return PlayerGuid.Read(data); }
};

struct BattlegroundPlayerLeft
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEGROUND_PLAYER_LEFT;

    ClassicGuid PlayerGuid;

    void Write(ByteBuffer& data) const { PlayerGuid.Write(data); }
    bool Read(ByteBuffer& data) { return PlayerGuid.Read(data); }
};

struct BattlegroundFlagCarrier
{
    static constexpr int8 IconHordeFlag = 1;
    static constexpr int8 IconAllianceFlag = 2;

    ClassicGuid Guid;
    float X = 0.0f;
    float Y = 0.0f;
    int8 IconId = 0;
    int8 ArenaSlot = -1; // HARDCODE: -1, no arenas in this engine

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << X << Y << IconId << ArenaSlot;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(float) + 2)
            return false;
        data >> X >> Y >> IconId >> ArenaSlot;
        return true;
    }
};

// Flag-carrier minimap blips only (confirmed against TrinityCore), not every teammate's position
// like the old 1.12 MSG_BATTLEGROUND_PLAYER_POSITIONS did -- that behavior has no modern
// equivalent. Server push only: no client request opcode exists for the modern client.
struct BattlegroundPlayerPositions
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_BATTLEGROUND_PLAYER_POSITIONS;

    std::vector<BattlegroundFlagCarrier> FlagCarriers;

    void Write(ByteBuffer& data) const
    {
        data << uint32(FlagCarriers.size());
        for (BattlegroundFlagCarrier const& carrier : FlagCarriers)
            carrier.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 count;
        data >> count;
        FlagCarriers.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            BattlegroundFlagCarrier carrier;
            if (!carrier.Read(data))
                return false;
            FlagCarriers.push_back(carrier);
        }
        return true;
    }
};

struct PvpMatchHonorData
{
    uint32 HonorKills = 0;
    uint32 Deaths = 0;
    uint32 ContributionPoints = 0;

    void Write(ByteBuffer& data) const { data << HonorKills << Deaths << ContributionPoints; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> HonorKills >> Deaths >> ContributionPoints;
        return true;
    }
};

// Field-by-field match against TrinityCore's BattlegroundScore::BuildPvPLogPlayerDataPacket --
// notably TrinityCore's own production code leaves PrimaryTalentTree/Sex/Race/Class/CreatureId/
// HonorLevel/Role/IsInWorld at their zero/false defaults; it never populates them from the player
// object either, so this matches confirmed real behavior rather than guessing richer data would help.
struct PvpMatchPlayerStatistics
{
    ClassicGuid PlayerGuid;
    uint32 Kills = 0;
    uint32 DamageDone = 0;  // HARDCODE: not tracked by this engine's scoring system
    uint32 HealingDone = 0; // HARDCODE: not tracked by this engine's scoring system
    std::vector<int32> Stats; // per-BG stat values (towers/bases/flags); the id itself is never sent, only the value in insertion order -- the client matches slot position to a label via its own per-BG table
    int32 PrimaryTalentTree = 0;
    int32 Sex = 0;
    int32 Race = 0;
    int32 Class = 0;
    int32 CreatureId = 0;
    int32 HonorLevel = 0;
    int32 Role = 0;
    uint8 Faction = CLASSIC_PVP_TEAM_HORDE; // wire bit is `Faction != Horde`, so Alliance and Neutral are indistinguishable on the wire
    bool IsInWorld = false;
    bool HasHonorData = false;
    PvpMatchHonorData Honor;

    void Write(ByteBuffer& data) const
    {
        PlayerGuid.Write(data);
        data << Kills << DamageDone << HealingDone << uint32(Stats.size());
        data << PrimaryTalentTree << Sex << Race << Class << CreatureId << HonorLevel << Role;
        for (int32 stat : Stats)
            data << stat;
        data.WriteBit(Faction != CLASSIC_PVP_TEAM_HORDE);
        data.WriteBit(IsInWorld);
        data.WriteBit(HasHonorData);
        data.WriteBit(false); // PreMatchRating -- HARDCODE: no rated system here
        data.WriteBit(false); // RatingChange
        data.WriteBit(false); // PreMatchMMR
        data.WriteBit(false); // MmrChange
        data.FlushBits();
        if (HasHonorData)
            Honor.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (!PlayerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(uint32))
            return false;
        uint32 statCount;
        data >> Kills >> DamageDone >> HealingDone >> statCount;
        if (data.size() - data.rpos() < 7 * sizeof(int32))
            return false;
        data >> PrimaryTalentTree >> Sex >> Race >> Class >> CreatureId >> HonorLevel >> Role;
        if (data.size() - data.rpos() < uint64(statCount) * sizeof(int32))
            return false;
        Stats.clear();
        for (uint32 i = 0; i < statCount; ++i)
        {
            int32 stat;
            data >> stat;
            Stats.push_back(stat);
        }
        if (data.size() - data.rpos() < 1)
            return false;
        bool const notHorde = data.ReadBit();
        IsInWorld = data.ReadBit();
        HasHonorData = data.ReadBit();
        data.ReadBit(); // PreMatchRating
        data.ReadBit(); // RatingChange
        data.ReadBit(); // PreMatchMMR
        data.ReadBit(); // MmrChange
        data.ResetBitPos();
        Faction = notHorde ? CLASSIC_PVP_TEAM_ALLIANCE : CLASSIC_PVP_TEAM_HORDE;
        if (HasHonorData && !Honor.Read(data))
            return false;
        return true;
    }
};

// The modern per-match scoreboard payload, replacing the old flat MSG_PVP_LOG_DATA "extraFields"
// vector.
struct PvpMatchStatistics
{
    std::vector<PvpMatchPlayerStatistics> Statistics;
    bool HasWinner = false;
    uint8 Winner = CLASSIC_PVP_TEAM_HORDE;
    int8 PlayerCount[2] = {};

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(false); // Ratings -- HARDCODE: no rated system here
        data.WriteBit(false); // arena teams flag -- unused upstream too
        data.WriteBit(HasWinner);
        data.FlushBits();

        data << uint32(Statistics.size());
        data << PlayerCount[0] << PlayerCount[1];
        if (HasWinner)
            data << Winner;
        for (PvpMatchPlayerStatistics const& player : Statistics)
            player.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data.ReadBit(); // Ratings
        data.ReadBit(); // arena teams flag
        HasWinner = data.ReadBit();
        data.ResetBitPos();

        if (data.size() - data.rpos() < sizeof(uint32) + 2)
            return false;
        uint32 count;
        data >> count >> PlayerCount[0] >> PlayerCount[1];
        if (HasWinner)
        {
            if (data.size() - data.rpos() < 1)
                return false;
            data >> Winner;
        }
        Statistics.clear();
        for (uint32 i = 0; i < count; ++i)
        {
            PvpMatchPlayerStatistics player;
            if (!player.Read(data))
                return false;
            Statistics.push_back(std::move(player));
        }
        return true;
    }
};

// The scoreboard reply, sent both on request (PVPLogDataRequest) and whenever a player is added to
// a battleground already in progress.
struct PvpMatchStatisticsMessage
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PVP_LOG_DATA;

    PvpMatchStatistics Data;

    void Write(ByteBuffer& data) const { Data.Write(data); }
    bool Read(ByteBuffer& data) { return Data.Read(data); }
};

// Classic has none of these systems except pug battlegrounds, so every other field is hardcoded false.
struct PVPOptionsEnabled
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PVP_OPTIONS_ENABLED;

    bool RatedBattlegrounds = false;
    bool PugBattlegrounds = true;
    bool WargameBattlegrounds = false;
    bool WargameArenas = false;
    bool RatedArenas = false;
    bool ArenaSkirmish = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(RatedBattlegrounds);
        data.WriteBit(PugBattlegrounds);
        data.WriteBit(WargameBattlegrounds);
        data.WriteBit(WargameArenas);
        data.WriteBit(RatedArenas);
        data.WriteBit(ArenaSkirmish);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        RatedBattlegrounds = data.ReadBit();
        PugBattlegrounds = data.ReadBit();
        WargameBattlegrounds = data.ReadBit();
        WargameArenas = data.ReadBit();
        RatedArenas = data.ReadBit();
        ArenaSkirmish = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct PvpBracketInfo
{
    int32 PersonalRating = 0;
    int32 Ranking = 0;
    int32 SeasonPlayed = 0;
    int32 SeasonWon = 0;
    int32 WeeklyPlayed = 0;
    int32 WeeklyWon = 0;
    int32 BestWeeklyRating = 0;
    int32 BestSeasonRating = 0;
    int32 LastWeeksBestRating = 0;
    int32 PvpTierId = 0;
    int32 Unused1 = 0;
    int32 Unused2 = 0;
    int32 Unused3 = 0;
    int32 Unused4 = 0;
    int32 Unused5 = 0;
    bool Disqualified = false;

    void Write(ByteBuffer& data) const
    {
        data << PersonalRating << Ranking << SeasonPlayed << SeasonWon << WeeklyPlayed << WeeklyWon;
        data << BestWeeklyRating << BestSeasonRating << LastWeeksBestRating << PvpTierId;
        data << Unused1 << Unused2 << Unused3 << Unused4 << Unused5;
        data.WriteBit(Disqualified);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 15 * sizeof(int32))
            return false;
        data >> PersonalRating >> Ranking >> SeasonPlayed >> SeasonWon >> WeeklyPlayed >> WeeklyWon;
        data >> BestWeeklyRating >> BestSeasonRating >> LastWeeksBestRating >> PvpTierId;
        data >> Unused1 >> Unused2 >> Unused3 >> Unused4 >> Unused5;
        if (data.size() - data.rpos() < 1)
            return false;
        Disqualified = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

constexpr uint32 CLASSIC_PVP_BRACKET_COUNT = 6;

// Classic has no rated PvP/arenas at all, so every bracket is sent all-zero/not-disqualified -- the
// client renders that as "no rating data", which is factually correct here.
struct RatedPvpInfo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_RATED_PVP_INFO;

    PvpBracketInfo Bracket[CLASSIC_PVP_BRACKET_COUNT];

    void Write(ByteBuffer& data) const
    {
        for (PvpBracketInfo const& bracket : Bracket)
            bracket.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        for (PvpBracketInfo& bracket : Bracket)
            if (!bracket.Read(data))
                return false;
        return true;
    }
};

// This engine has no AFK-report/vote-kick tracking system, so the response honestly reports the
// system itself as disabled rather than fabricating a success/failure the server can't back up.
struct ReportPvPPlayerAFKResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_REPORT_PVP_PLAYER_AFK_RESULT;
    static constexpr uint8 SystemDisabled = 6;

    ClassicGuid Offender;
    uint8 Result = SystemDisabled;
    uint8 NumBlackMarksOnOffender = 0;
    uint8 NumPlayersIHaveReported = 0;

    void Write(ByteBuffer& data) const
    {
        Offender.Write(data);
        data << Result << NumBlackMarksOnOffender << NumPlayersIHaveReported;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Offender.Read(data))
            return false;
        if (data.size() - data.rpos() < 3)
            return false;
        data >> Result >> NumBlackMarksOnOffender >> NumPlayersIHaveReported;
        return true;
    }
};

struct AreaSpiritHealerTime
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AREA_SPIRIT_HEALER_TIME;

    ClassicGuid SpiritHealerGuid;
    int32 TimeLeft = 0; // ms until the healer can next resurrect

    void Write(ByteBuffer& data) const
    {
        SpiritHealerGuid.Write(data);
        data << TimeLeft;
    }

    bool Read(ByteBuffer& data)
    {
        if (!SpiritHealerGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> TimeLeft;
        return true;
    }
};

#endif
