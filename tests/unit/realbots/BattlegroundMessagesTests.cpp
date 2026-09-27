// Round trips for the modern battleground wire.
#include "Packets/BattlegroundMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("RideTicket and BattlefieldStatusHeader round-trip", "[packets]")
{
    RideTicket ticket;
    ticket.RequesterGuid = { 0x1234, 0x0400000000000000 };
    ticket.Id = 42;
    ticket.Time = 1700000000;
    RoundTrip(ticket, [](RideTicket const& a, RideTicket const& b)
    {
        CHECK(a.RequesterGuid == b.RequesterGuid);
        CHECK(a.Id == b.Id);
        CHECK(a.Type == b.Type);
        CHECK(a.Time == b.Time);
    });
    RejectsEveryTruncation(ticket);

    BattlefieldStatusHeader header;
    header.Ticket = ticket;
    header.QueueId = 0x1F10000000000001ULL;
    header.RangeMin = 10;
    header.RangeMax = 60;
    header.TeamSize = 0;
    header.InstanceId = 7;
    header.RegisteredMatch = false;
    header.TournamentRules = false;
    RoundTrip(header, [](BattlefieldStatusHeader const& a, BattlefieldStatusHeader const& b)
    {
        CHECK(a.Ticket.Id == b.Ticket.Id);
        CHECK(a.QueueId == b.QueueId);
        CHECK(a.RangeMin == b.RangeMin);
        CHECK(a.RangeMax == b.RangeMax);
        CHECK(a.TeamSize == b.TeamSize);
        CHECK(a.InstanceId == b.InstanceId);
    });
    RejectsEveryTruncation(header);
}

TEST_CASE("Battleground client requests round-trip", "[packets]")
{
    AreaSpiritHealerQuery query;
    query.Guid = { 0x11, 0x0400000000000000 };
    RoundTrip(query, [](AreaSpiritHealerQuery const& a, AreaSpiritHealerQuery const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(query);

    AreaSpiritHealerQueue queue;
    queue.Guid = { 0x12, 0x0400000000000000 };
    RoundTrip(queue, [](AreaSpiritHealerQueue const& a, AreaSpiritHealerQueue const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(queue);

    BattlemasterHello hello;
    hello.Guid = { 0x13, 0x0300000000000000 };
    RoundTrip(hello, [](BattlemasterHello const& a, BattlemasterHello const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(hello);

    BattlemasterJoin join;
    join.QueueId = 0x1F10000000000002ULL;
    join.Roles = 0;
    join.BlacklistMap[0] = -1;
    join.BlacklistMap[1] = -1;
    join.BattlemasterGuid = { 0x14, 0x0300000000000000 };
    join.Verification = 0;
    join.BattlefieldInstanceId = 5;
    join.AsGroup = true;
    RoundTrip(join, [](BattlemasterJoin const& a, BattlemasterJoin const& b)
    {
        CHECK(a.QueueId == b.QueueId);
        CHECK(a.Roles == b.Roles);
        CHECK(a.BlacklistMap[0] == b.BlacklistMap[0]);
        CHECK(a.BlacklistMap[1] == b.BlacklistMap[1]);
        CHECK(a.BattlemasterGuid == b.BattlemasterGuid);
        CHECK(a.BattlefieldInstanceId == b.BattlefieldInstanceId);
        CHECK(a.AsGroup == b.AsGroup);
    });
    RejectsEveryTruncation(join);

    BattleFieldPort port;
    port.Ticket.RequesterGuid = { 0x15, 0x0400000000000000 };
    port.Ticket.Id = 9;
    port.Ticket.Time = 1234;
    port.AcceptedInvite = true;
    RoundTrip(port, [](BattleFieldPort const& a, BattleFieldPort const& b)
    {
        CHECK(a.Ticket.Id == b.Ticket.Id);
        CHECK(a.AcceptedInvite == b.AcceptedInvite);
    });
    RejectsEveryTruncation(port);

    LeaveBattlefield leave;
    RoundTrip(leave, [](LeaveBattlefield const&, LeaveBattlefield const&) {});

    BattlefieldListRequest listRequest;
    listRequest.ListId = 2;
    RoundTrip(listRequest, [](BattlefieldListRequest const& a, BattlefieldListRequest const& b) { CHECK(a.ListId == b.ListId); });
    RejectsEveryTruncation(listRequest);

    PVPLogDataRequest logRequest;
    RoundTrip(logRequest, [](PVPLogDataRequest const&, PVPLogDataRequest const&) {});

    RequestBattlefieldStatus statusRequest;
    RoundTrip(statusRequest, [](RequestBattlefieldStatus const&, RequestBattlefieldStatus const&) {});

    GetPVPOptionsEnabled optionsRequest;
    RoundTrip(optionsRequest, [](GetPVPOptionsEnabled const&, GetPVPOptionsEnabled const&) {});

    RequestRatedPvpInfo ratedRequest;
    RoundTrip(ratedRequest, [](RequestRatedPvpInfo const&, RequestRatedPvpInfo const&) {});

    ReportPvPPlayerAFK afkReport;
    afkReport.Offender = { 0x16, 0x0400000000000000 };
    RoundTrip(afkReport, [](ReportPvPPlayerAFK const& a, ReportPvPPlayerAFK const& b) { CHECK(a.Offender == b.Offender); });
    RejectsEveryTruncation(afkReport);
}

TEST_CASE("SMSG_BATTLEFIELD_STATUS_* family round-trips for every state", "[packets]")
{
    RideTicket ticket;
    ticket.RequesterGuid = { 0x21, 0x0400000000000000 };
    ticket.Id = 3;
    ticket.Time = 99;

    BattlefieldStatusNone none;
    none.Ticket = ticket;
    RoundTrip(none, [](BattlefieldStatusNone const& a, BattlefieldStatusNone const& b) { CHECK(a.Ticket.Id == b.Ticket.Id); });
    RejectsEveryTruncation(none);

    BattlefieldStatusQueued queued;
    queued.Header.Ticket = ticket;
    queued.Header.QueueId = 5;
    queued.Header.RangeMax = 60;
    queued.Header.InstanceId = 1;
    queued.AverageWaitTime = 12000;
    queued.WaitTime = 500;
    queued.AsGroup = true;
    queued.EligibleForMatchmaking = false;
    queued.SuspendedQueue = true;
    RoundTrip(queued, [](BattlefieldStatusQueued const& a, BattlefieldStatusQueued const& b)
    {
        CHECK(a.AverageWaitTime == b.AverageWaitTime);
        CHECK(a.WaitTime == b.WaitTime);
        CHECK(a.AsGroup == b.AsGroup);
        CHECK(a.EligibleForMatchmaking == b.EligibleForMatchmaking);
        CHECK(a.SuspendedQueue == b.SuspendedQueue);
    });
    RejectsEveryTruncation(queued);

    BattlefieldStatusNeedConfirmation confirm;
    confirm.Header.Ticket = ticket;
    confirm.MapId = 489;
    confirm.Timeout = 80000;
    confirm.Role = 0;
    RoundTrip(confirm, [](BattlefieldStatusNeedConfirmation const& a, BattlefieldStatusNeedConfirmation const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.Timeout == b.Timeout);
    });
    RejectsEveryTruncation(confirm);

    BattlefieldStatusActive active;
    active.Header.Ticket = ticket;
    active.MapId = 489;
    active.ShutdownTimer = 120000;
    active.StartTimer = 45000;
    active.ArenaFaction = false;
    active.LeftEarly = false;
    RoundTrip(active, [](BattlefieldStatusActive const& a, BattlefieldStatusActive const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.ShutdownTimer == b.ShutdownTimer);
        CHECK(a.StartTimer == b.StartTimer);
    });
    RejectsEveryTruncation(active);

    BattlefieldStatusFailed failed;
    failed.Ticket = ticket;
    failed.QueueId = 5;
    failed.Reason = 2;
    failed.ClientId = { 0x22, 0x0400000000000000 };
    RoundTrip(failed, [](BattlefieldStatusFailed const& a, BattlefieldStatusFailed const& b)
    {
        CHECK(a.QueueId == b.QueueId);
        CHECK(a.Reason == b.Reason);
        CHECK(a.ClientId == b.ClientId);
    });
    RejectsEveryTruncation(failed);
}

TEST_CASE("BattlefieldList round-trips with and without instances", "[packets]")
{
    BattlefieldList list;
    list.BattlemasterGuid = { 0x31, 0x0300000000000000 };
    list.BattlemasterListId = 2;
    list.MinLevel = 10;
    list.MaxLevel = 60;
    list.Battlefields = { 1, 2, 3 };
    list.PvpAnywhere = false;
    list.HasRandomWinToday = true;
    RoundTrip(list, [](BattlefieldList const& a, BattlefieldList const& b)
    {
        CHECK(a.BattlemasterGuid == b.BattlemasterGuid);
        CHECK(a.BattlemasterListId == b.BattlemasterListId);
        CHECK(a.MinLevel == b.MinLevel);
        CHECK(a.MaxLevel == b.MaxLevel);
        CHECK(b.Battlefields == std::vector<int32>{1, 2, 3});
        CHECK(a.HasRandomWinToday == b.HasRandomWinToday);
    });
    RejectsEveryTruncation(list);

    BattlefieldList anywhere;
    anywhere.BattlemasterListId = 1;
    anywhere.PvpAnywhere = true;
    RoundTrip(anywhere, [](BattlefieldList const& a, BattlefieldList const& b)
    {
        CHECK(b.Battlefields.empty());
        CHECK(a.PvpAnywhere == b.PvpAnywhere);
    });
    RejectsEveryTruncation(anywhere);
}

TEST_CASE("Battleground join/leave and flag-carrier position notices round-trip", "[packets]")
{
    BattlegroundPlayerJoined joined;
    joined.PlayerGuid = { 0x41, 0x0400000000000000 };
    RoundTrip(joined, [](BattlegroundPlayerJoined const& a, BattlegroundPlayerJoined const& b) { CHECK(a.PlayerGuid == b.PlayerGuid); });
    RejectsEveryTruncation(joined);

    BattlegroundPlayerLeft left;
    left.PlayerGuid = { 0x42, 0x0400000000000000 };
    RoundTrip(left, [](BattlegroundPlayerLeft const& a, BattlegroundPlayerLeft const& b) { CHECK(a.PlayerGuid == b.PlayerGuid); });
    RejectsEveryTruncation(left);

    BattlegroundPlayerPositions positions;
    BattlegroundFlagCarrier horde;
    horde.Guid = { 0x43, 0x0400000000000000 };
    horde.X = 100.5f;
    horde.Y = -50.25f;
    horde.IconId = BattlegroundFlagCarrier::IconHordeFlag;
    positions.FlagCarriers.push_back(horde);
    BattlegroundFlagCarrier alliance;
    alliance.Guid = { 0x44, 0x0400000000000000 };
    alliance.X = 10.0f;
    alliance.Y = 20.0f;
    alliance.IconId = BattlegroundFlagCarrier::IconAllianceFlag;
    positions.FlagCarriers.push_back(alliance);
    RoundTrip(positions, [](BattlegroundPlayerPositions const& a, BattlegroundPlayerPositions const& b)
    {
        REQUIRE(b.FlagCarriers.size() == 2);
        CHECK(b.FlagCarriers[0].Guid == a.FlagCarriers[0].Guid);
        CHECK(b.FlagCarriers[0].X == a.FlagCarriers[0].X);
        CHECK(b.FlagCarriers[0].IconId == BattlegroundFlagCarrier::IconHordeFlag);
        CHECK(b.FlagCarriers[1].IconId == BattlegroundFlagCarrier::IconAllianceFlag);
        CHECK(b.FlagCarriers[1].ArenaSlot == -1);
    });
    RejectsEveryTruncation(positions);

    BattlegroundPlayerPositions empty;
    RoundTrip(empty, [](BattlegroundPlayerPositions const&, BattlegroundPlayerPositions const& b) { CHECK(b.FlagCarriers.empty()); });
    RejectsEveryTruncation(empty);
}

TEST_CASE("PvpMatchStatistics round-trips with and without a winner", "[packets]")
{
    PvpMatchStatisticsMessage message;
    message.Data.HasWinner = true;
    message.Data.Winner = CLASSIC_PVP_TEAM_ALLIANCE;
    message.Data.PlayerCount[0] = 4;
    message.Data.PlayerCount[1] = 5;

    PvpMatchPlayerStatistics withHonor;
    withHonor.PlayerGuid = { 0x51, 0x0400000000000000 };
    withHonor.Kills = 3;
    withHonor.Stats = { 2, 1 };
    withHonor.Faction = CLASSIC_PVP_TEAM_ALLIANCE;
    withHonor.IsInWorld = true;
    withHonor.HasHonorData = true;
    withHonor.Honor.HonorKills = 5;
    withHonor.Honor.Deaths = 1;
    withHonor.Honor.ContributionPoints = 200;
    message.Data.Statistics.push_back(withHonor);

    PvpMatchPlayerStatistics noHonor;
    noHonor.PlayerGuid = { 0x52, 0x0400000000000000 };
    noHonor.Faction = CLASSIC_PVP_TEAM_HORDE;
    noHonor.HasHonorData = false;
    message.Data.Statistics.push_back(noHonor);

    RoundTrip(message, [](PvpMatchStatisticsMessage const& a, PvpMatchStatisticsMessage const& b)
    {
        CHECK(b.Data.HasWinner);
        CHECK(b.Data.Winner == CLASSIC_PVP_TEAM_ALLIANCE);
        CHECK(b.Data.PlayerCount[0] == 4);
        CHECK(b.Data.PlayerCount[1] == 5);
        REQUIRE(b.Data.Statistics.size() == 2);

        auto const& first = b.Data.Statistics[0];
        CHECK(first.PlayerGuid == a.Data.Statistics[0].PlayerGuid);
        CHECK(first.Kills == 3);
        CHECK(first.Stats == std::vector<int32>{2, 1});
        CHECK(first.Faction == CLASSIC_PVP_TEAM_ALLIANCE);
        CHECK(first.IsInWorld);
        REQUIRE(first.HasHonorData);
        CHECK(first.Honor.HonorKills == 5);
        CHECK(first.Honor.Deaths == 1);
        CHECK(first.Honor.ContributionPoints == 200);

        auto const& second = b.Data.Statistics[1];
        CHECK(second.Faction == CLASSIC_PVP_TEAM_HORDE);
        CHECK_FALSE(second.HasHonorData);
    });
    RejectsEveryTruncation(message);

    PvpMatchStatisticsMessage noWinner;
    RoundTrip(noWinner, [](PvpMatchStatisticsMessage const&, PvpMatchStatisticsMessage const& b)
    {
        CHECK_FALSE(b.Data.HasWinner);
        CHECK(b.Data.Statistics.empty());
    });
    RejectsEveryTruncation(noWinner);
}

TEST_CASE("PVPOptionsEnabled, RatedPvpInfo and ReportPvPPlayerAFKResult round-trip", "[packets]")
{
    PVPOptionsEnabled options;
    options.PugBattlegrounds = true;
    RoundTrip(options, [](PVPOptionsEnabled const& a, PVPOptionsEnabled const& b)
    {
        CHECK(a.RatedBattlegrounds == b.RatedBattlegrounds);
        CHECK(a.PugBattlegrounds == b.PugBattlegrounds);
        CHECK(a.WargameBattlegrounds == b.WargameBattlegrounds);
        CHECK(a.WargameArenas == b.WargameArenas);
        CHECK(a.RatedArenas == b.RatedArenas);
        CHECK(a.ArenaSkirmish == b.ArenaSkirmish);
    });
    RejectsEveryTruncation(options);

    RatedPvpInfo rated;
    rated.Bracket[0].PersonalRating = 1500;
    rated.Bracket[0].Disqualified = true;
    RoundTrip(rated, [](RatedPvpInfo const& a, RatedPvpInfo const& b)
    {
        CHECK(b.Bracket[0].PersonalRating == 1500);
        CHECK(b.Bracket[0].Disqualified);
        CHECK(b.Bracket[1].PersonalRating == a.Bracket[1].PersonalRating);
    });
    RejectsEveryTruncation(rated);

    ReportPvPPlayerAFKResult afkResult;
    afkResult.Offender = { 0x61, 0x0400000000000000 };
    afkResult.Result = ReportPvPPlayerAFKResult::SystemDisabled;
    afkResult.NumBlackMarksOnOffender = 2;
    afkResult.NumPlayersIHaveReported = 1;
    RoundTrip(afkResult, [](ReportPvPPlayerAFKResult const& a, ReportPvPPlayerAFKResult const& b)
    {
        CHECK(a.Offender == b.Offender);
        CHECK(b.Result == ReportPvPPlayerAFKResult::SystemDisabled);
        CHECK(a.NumBlackMarksOnOffender == b.NumBlackMarksOnOffender);
        CHECK(a.NumPlayersIHaveReported == b.NumPlayersIHaveReported);
    });
    RejectsEveryTruncation(afkResult);
}

TEST_CASE("AreaSpiritHealerTime round-trips", "[packets]")
{
    AreaSpiritHealerTime time;
    time.SpiritHealerGuid = { 0x71, 0x0400000000000000 };
    time.TimeLeft = 15000;
    RoundTrip(time, [](AreaSpiritHealerTime const& a, AreaSpiritHealerTime const& b)
    {
        CHECK(a.SpiritHealerGuid == b.SpiritHealerGuid);
        CHECK(a.TimeLeft == b.TimeLeft);
    });
    RejectsEveryTruncation(time);
}
