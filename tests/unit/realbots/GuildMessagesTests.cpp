// Round trips for the modern guild wire.
#include "Packets/GuildMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("GuildQuery round-trips", "[packets]")
{
    GuildQuery packet;
    packet.GuildGuid = { 0x1111, 0 };
    packet.PlayerGuid = { 0x2222, 0 };
    RoundTrip(packet, [](GuildQuery const& a, GuildQuery const& b)
    {
        CHECK(a.GuildGuid == b.GuildGuid);
        CHECK(a.PlayerGuid == b.PlayerGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildInviteByName round-trips", "[packets]")
{
    GuildInviteByName packet;
    packet.InvitedName = "Realbot";
    RoundTrip(packet, [](GuildInviteByName const& a, GuildInviteByName const& b) { CHECK(a.InvitedName == b.InvitedName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildOfficerRemoveMember round-trips", "[packets]")
{
    GuildOfficerRemoveMember packet;
    packet.RemoveeGuid = { 0x3333, 0 };
    RoundTrip(packet, [](GuildOfficerRemoveMember const& a, GuildOfficerRemoveMember const& b) { CHECK(a.RemoveeGuid == b.RemoveeGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AcceptGuildInvite round-trips", "[packets]")
{
    AcceptGuildInvite packet;
    packet.GuildGuid = { 0x4444, 0 };
    RoundTrip(packet, [](AcceptGuildInvite const& a, AcceptGuildInvite const& b) { CHECK(a.GuildGuid == b.GuildGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildDeclineInvitation round-trips", "[packets]")
{
    GuildDeclineInvitation packet;
    packet.GuildGuid = { 0x5555, 0 };
    packet.IsAuto = true;
    RoundTrip(packet, [](GuildDeclineInvitation const& a, GuildDeclineInvitation const& b)
    {
        CHECK(a.GuildGuid == b.GuildGuid);
        CHECK(a.IsAuto == b.IsAuto);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildGetRoster round-trips", "[packets]")
{
    GuildGetRoster packet;
    RoundTrip(packet, [](GuildGetRoster const&, GuildGetRoster const&) {});
}

TEST_CASE("GuildGetRanks round-trips", "[packets]")
{
    GuildGetRanks packet;
    packet.GuildGuid = { 0x6666, 0 };
    RoundTrip(packet, [](GuildGetRanks const& a, GuildGetRanks const& b) { CHECK(a.GuildGuid == b.GuildGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildPermissionsQuery round-trips", "[packets]")
{
    GuildPermissionsQuery packet;
    RoundTrip(packet, [](GuildPermissionsQuery const&, GuildPermissionsQuery const&) {});
}

TEST_CASE("GuildPromoteMember round-trips", "[packets]")
{
    GuildPromoteMember packet;
    packet.PromoteeGuid = { 0x7777, 0 };
    RoundTrip(packet, [](GuildPromoteMember const& a, GuildPromoteMember const& b) { CHECK(a.PromoteeGuid == b.PromoteeGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildDemoteMember round-trips", "[packets]")
{
    GuildDemoteMember packet;
    packet.DemoteeGuid = { 0x8888, 0 };
    RoundTrip(packet, [](GuildDemoteMember const& a, GuildDemoteMember const& b) { CHECK(a.DemoteeGuid == b.DemoteeGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildLeave round-trips", "[packets]")
{
    GuildLeave packet;
    RoundTrip(packet, [](GuildLeave const&, GuildLeave const&) {});
}

TEST_CASE("GuildDelete round-trips", "[packets]")
{
    GuildDelete packet;
    RoundTrip(packet, [](GuildDelete const&, GuildDelete const&) {});
}

TEST_CASE("GuildSetGuildMaster round-trips", "[packets]")
{
    GuildSetGuildMaster packet;
    packet.NewMasterName = "Newleader";
    RoundTrip(packet, [](GuildSetGuildMaster const& a, GuildSetGuildMaster const& b) { CHECK(a.NewMasterName == b.NewMasterName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildUpdateMotdText round-trips", "[packets]")
{
    GuildUpdateMotdText packet;
    packet.Motd = "Welcome to the guild";
    RoundTrip(packet, [](GuildUpdateMotdText const& a, GuildUpdateMotdText const& b) { CHECK(a.Motd == b.Motd); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildUpdateInfoText round-trips", "[packets]")
{
    GuildUpdateInfoText packet;
    packet.InfoText = "A guild for testing.";
    RoundTrip(packet, [](GuildUpdateInfoText const& a, GuildUpdateInfoText const& b) { CHECK(a.InfoText == b.InfoText); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SaveGuildEmblem round-trips", "[packets]")
{
    SaveGuildEmblem packet;
    packet.VendorGuid = { 0x9999, 0 };
    packet.EmblemStyle = 1;
    packet.EmblemColor = 2;
    packet.BorderStyle = 3;
    packet.BorderColor = 4;
    packet.BackgroundColor = 5;
    RoundTrip(packet, [](SaveGuildEmblem const& a, SaveGuildEmblem const& b)
    {
        CHECK(a.VendorGuid == b.VendorGuid);
        CHECK(a.EmblemStyle == b.EmblemStyle);
        CHECK(a.EmblemColor == b.EmblemColor);
        CHECK(a.BorderStyle == b.BorderStyle);
        CHECK(a.BorderColor == b.BorderColor);
        CHECK(a.BackgroundColor == b.BackgroundColor);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildSetMemberNote round-trips for public and officer notes", "[packets]")
{
    GuildSetMemberNote packet;
    packet.NoteeGuid = { 0xaaaa, 0 };
    packet.IsPublic = true;
    packet.Note = "A public note";
    RoundTrip(packet, [](GuildSetMemberNote const& a, GuildSetMemberNote const& b)
    {
        CHECK(a.NoteeGuid == b.NoteeGuid);
        CHECK(a.IsPublic == b.IsPublic);
        CHECK(a.Note == b.Note);
    });
    RejectsEveryTruncation(packet);

    packet.IsPublic = false;
    packet.Note = "An officer note";
    RoundTrip(packet, [](GuildSetMemberNote const& a, GuildSetMemberNote const& b)
    {
        CHECK(a.IsPublic == b.IsPublic);
        CHECK(a.Note == b.Note);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildAddRank round-trips", "[packets]")
{
    GuildAddRank packet;
    packet.RankName = "Recruit";
    packet.RankOrder = 4;
    RoundTrip(packet, [](GuildAddRank const& a, GuildAddRank const& b)
    {
        CHECK(a.RankName == b.RankName);
        CHECK(a.RankOrder == b.RankOrder);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildDeleteRank round-trips", "[packets]")
{
    GuildDeleteRank packet;
    packet.RankOrder = 4;
    RoundTrip(packet, [](GuildDeleteRank const& a, GuildDeleteRank const& b) { CHECK(a.RankOrder == b.RankOrder); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildSetRankPermissions round-trips", "[packets]")
{
    GuildSetRankPermissions packet;
    packet.RankId = 2;
    packet.RankOrder = 2;
    packet.Flags = 0x41;
    packet.WithdrawGoldLimit = 0;
    packet.RankName = "Veteran";
    RoundTrip(packet, [](GuildSetRankPermissions const& a, GuildSetRankPermissions const& b)
    {
        CHECK(a.RankId == b.RankId);
        CHECK(a.Flags == b.Flags);
        CHECK(a.RankName == b.RankName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildCommandResult round-trips", "[packets]")
{
    GuildCommandResult packet;
    packet.Command = 1;
    packet.Str = "Realbot";
    packet.Result = 0x0B;
    RoundTrip(packet, [](GuildCommandResult const& a, GuildCommandResult const& b)
    {
        CHECK(a.Command == b.Command);
        CHECK(a.Str == b.Str);
        CHECK(a.Result == b.Result);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildInviteNotification round-trips", "[packets]")
{
    GuildInviteNotification packet;
    packet.InviterName = "Leader";
    packet.GuildName = "Realbots Test Guild";
    packet.GuildGuid = { 0x7777, 0 };
    RoundTrip(packet, [](GuildInviteNotification const& a, GuildInviteNotification const& b)
    {
        CHECK(a.InviterName == b.InviterName);
        CHECK(a.GuildName == b.GuildName);
        CHECK(a.GuildGuid == b.GuildGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildDeclineNotification round-trips", "[packets]")
{
    GuildDeclineNotification packet;
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](GuildDeclineNotification const& a, GuildDeclineNotification const& b) { CHECK(a.PlayerName == b.PlayerName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("QueryGuildInfoResponse round-trips when invalid and when valid", "[packets]")
{
    QueryGuildInfoResponse invalid;
    invalid.GuildGuid = { 0xbbbb, 0 };
    invalid.Valid = false;
    RoundTrip(invalid, [](QueryGuildInfoResponse const& a, QueryGuildInfoResponse const& b)
    {
        CHECK(a.GuildGuid == b.GuildGuid);
        CHECK(a.Valid == b.Valid);
    });
    RejectsEveryTruncation(invalid);

    QueryGuildInfoResponse valid;
    valid.GuildGuid = { 0xbbbb, 0 };
    valid.Valid = true;
    valid.ResolvedGuildGuid = { 0xcccc, 0 };
    valid.GuildId = 7;
    valid.GuildName = "Realbots Test Guild";
    valid.RankNames = { "Guild Master", "Officer", "Veteran", "Member", "Initiate" };
    valid.EmblemStyle = 1;
    valid.EmblemColor = 2;
    valid.BorderStyle = 3;
    valid.BorderColor = 4;
    valid.BackgroundColor = 5;
    RoundTrip(valid, [](QueryGuildInfoResponse const& a, QueryGuildInfoResponse const& b)
    {
        CHECK(a.GuildGuid == b.GuildGuid);
        CHECK(a.ResolvedGuildGuid == b.ResolvedGuildGuid);
        CHECK(a.GuildName == b.GuildName);
        CHECK(a.RankNames == b.RankNames);
        CHECK(a.EmblemStyle == b.EmblemStyle);
        CHECK(a.EmblemColor == b.EmblemColor);
        CHECK(a.BorderStyle == b.BorderStyle);
        CHECK(a.BorderColor == b.BorderColor);
        CHECK(a.BackgroundColor == b.BackgroundColor);
    });
    RejectsEveryTruncation(valid);
}

TEST_CASE("GuildRoster round-trips with no and with several members", "[packets]")
{
    GuildRoster packet;
    packet.Motd = "Welcome";
    packet.GuildInfo = "Info text";
    packet.CreatedDay = 15;
    packet.CreatedMonth = 6;
    packet.CreatedYear = 2026;
    packet.AccountCount = 3;
    RoundTrip(packet, [](GuildRoster const& a, GuildRoster const& b)
    {
        CHECK(a.Motd == b.Motd);
        CHECK(a.GuildInfo == b.GuildInfo);
        CHECK(a.CreatedDay == b.CreatedDay);
        CHECK(a.CreatedMonth == b.CreatedMonth);
        CHECK(a.CreatedYear == b.CreatedYear);
        CHECK(a.AccountCount == b.AccountCount);
        CHECK(a.RosterMembers.size() == b.RosterMembers.size());
    });
    RejectsEveryTruncation(packet);

    GuildRosterMember online;
    online.Guid = { 0x1111, 0 };
    online.PresenceFlags = 1;
    online.Name = "Realbot";
    online.RankId = 3;
    online.Level = 10;
    online.ClassId = 1;
    online.RaceId = 1;
    online.ZoneId = 12;
    online.LastOnlineTime = 0.0f;
    online.PublicNote = "public";
    online.OfficerNote = "officer";

    GuildRosterMember offline;
    offline.Guid = { 0x2222, 0 };
    offline.PresenceFlags = 0;
    offline.Name = "Offlinebot";
    offline.RankId = 4;
    offline.Level = 5;
    offline.ClassId = 2;
    offline.RaceId = 0;
    offline.ZoneId = 0;
    offline.LastOnlineTime = 2.5f;
    offline.PublicNote = "";
    offline.OfficerNote = "";

    packet.RosterMembers = { online, offline };
    RoundTrip(packet, [](GuildRoster const& a, GuildRoster const& b)
    {
        REQUIRE(a.RosterMembers.size() == b.RosterMembers.size());
        for (size_t i = 0; i < a.RosterMembers.size(); ++i)
        {
            CHECK(a.RosterMembers[i].Guid == b.RosterMembers[i].Guid);
            CHECK(a.RosterMembers[i].PresenceFlags == b.RosterMembers[i].PresenceFlags);
            CHECK(a.RosterMembers[i].Name == b.RosterMembers[i].Name);
            CHECK(a.RosterMembers[i].RankId == b.RosterMembers[i].RankId);
            CHECK(a.RosterMembers[i].Level == b.RosterMembers[i].Level);
            CHECK(a.RosterMembers[i].ClassId == b.RosterMembers[i].ClassId);
            CHECK(a.RosterMembers[i].RaceId == b.RosterMembers[i].RaceId);
            CHECK(a.RosterMembers[i].ZoneId == b.RosterMembers[i].ZoneId);
            CHECK(a.RosterMembers[i].LastOnlineTime == b.RosterMembers[i].LastOnlineTime);
            CHECK(a.RosterMembers[i].PublicNote == b.RosterMembers[i].PublicNote);
            CHECK(a.RosterMembers[i].OfficerNote == b.RosterMembers[i].OfficerNote);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildPermissionsQueryResults round-trips", "[packets]")
{
    GuildPermissionsQueryResults packet;
    packet.RankId = 1;
    packet.Flags = 0x41;
    packet.WithdrawGoldLimit = 0;
    packet.NumTabs = 0;
    RoundTrip(packet, [](GuildPermissionsQueryResults const& a, GuildPermissionsQueryResults const& b)
    {
        CHECK(a.RankId == b.RankId);
        CHECK(a.Flags == b.Flags);
        CHECK(a.WithdrawGoldLimit == b.WithdrawGoldLimit);
        CHECK(a.NumTabs == b.NumTabs);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildRanksResponse round-trips with no and with several ranks", "[packets]")
{
    GuildRanksResponse packet;
    RoundTrip(packet, [](GuildRanksResponse const& a, GuildRanksResponse const& b) { CHECK(a.Ranks.size() == b.Ranks.size()); });
    RejectsEveryTruncation(packet);

    GuildRankEntry master;
    master.RankId = 0;
    master.RankOrder = 0;
    master.Flags = 0xFF;
    master.WithdrawGoldLimit = 0;
    master.RankName = "Guild Master";

    GuildRankEntry officer;
    officer.RankId = 1;
    officer.RankOrder = 1;
    officer.Flags = 0x41;
    officer.WithdrawGoldLimit = 0;
    officer.RankName = "Officer";

    packet.Ranks = { master, officer };
    RoundTrip(packet, [](GuildRanksResponse const& a, GuildRanksResponse const& b)
    {
        REQUIRE(a.Ranks.size() == b.Ranks.size());
        for (size_t i = 0; i < a.Ranks.size(); ++i)
        {
            CHECK(a.Ranks[i].RankId == b.Ranks[i].RankId);
            CHECK(a.Ranks[i].RankOrder == b.Ranks[i].RankOrder);
            CHECK(a.Ranks[i].Flags == b.Ranks[i].Flags);
            CHECK(a.Ranks[i].RankName == b.Ranks[i].RankName);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventMotd round-trips", "[packets]")
{
    GuildEventMotd packet;
    packet.MotdText = "Welcome to the guild";
    RoundTrip(packet, [](GuildEventMotd const& a, GuildEventMotd const& b) { CHECK(a.MotdText == b.MotdText); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventPresenceChange round-trips", "[packets]")
{
    GuildEventPresenceChange packet;
    packet.Guid = { 0x3333, 0 };
    packet.Name = "Realbot";
    packet.LoggedOn = true;
    RoundTrip(packet, [](GuildEventPresenceChange const& a, GuildEventPresenceChange const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.Name == b.Name);
        CHECK(a.LoggedOn == b.LoggedOn);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventPlayerJoined round-trips", "[packets]")
{
    GuildEventPlayerJoined packet;
    packet.Guid = { 0x4444, 0 };
    packet.Name = "Realbot";
    RoundTrip(packet, [](GuildEventPlayerJoined const& a, GuildEventPlayerJoined const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventPlayerLeft round-trips for a voluntary leave and a kick", "[packets]")
{
    GuildEventPlayerLeft left;
    left.LeaverGuid = { 0x5555, 0 };
    left.LeaverName = "Realbot";
    left.Removed = false;
    RoundTrip(left, [](GuildEventPlayerLeft const& a, GuildEventPlayerLeft const& b)
    {
        CHECK(a.LeaverGuid == b.LeaverGuid);
        CHECK(a.LeaverName == b.LeaverName);
        CHECK(a.Removed == b.Removed);
    });
    RejectsEveryTruncation(left);

    GuildEventPlayerLeft kicked;
    kicked.LeaverGuid = { 0x6666, 0 };
    kicked.LeaverName = "Kickedbot";
    kicked.RemoverGuid = { 0x7777, 0 };
    kicked.RemoverName = "Officerbot";
    kicked.Removed = true;
    RoundTrip(kicked, [](GuildEventPlayerLeft const& a, GuildEventPlayerLeft const& b)
    {
        CHECK(a.LeaverGuid == b.LeaverGuid);
        CHECK(a.LeaverName == b.LeaverName);
        CHECK(a.RemoverGuid == b.RemoverGuid);
        CHECK(a.RemoverName == b.RemoverName);
        CHECK(a.Removed == b.Removed);
    });
    RejectsEveryTruncation(kicked);
}

TEST_CASE("GuildEventNewLeader round-trips", "[packets]")
{
    GuildEventNewLeader packet;
    packet.NewLeaderGuid = { 0x8888, 0 };
    packet.NewLeaderName = "Newleader";
    packet.OldLeaderGuid = { 0x9999, 0 };
    packet.OldLeaderName = "Oldleader";
    packet.SelfPromoted = false;
    RoundTrip(packet, [](GuildEventNewLeader const& a, GuildEventNewLeader const& b)
    {
        CHECK(a.NewLeaderGuid == b.NewLeaderGuid);
        CHECK(a.NewLeaderName == b.NewLeaderName);
        CHECK(a.OldLeaderGuid == b.OldLeaderGuid);
        CHECK(a.OldLeaderName == b.OldLeaderName);
        CHECK(a.SelfPromoted == b.SelfPromoted);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildSendRankChange round-trips", "[packets]")
{
    GuildSendRankChange packet;
    packet.OfficerGuid = { 0xaaaa, 0 };
    packet.TargetGuid = { 0xbbbb, 0 };
    packet.RankId = 2;
    packet.Promote = true;
    RoundTrip(packet, [](GuildSendRankChange const& a, GuildSendRankChange const& b)
    {
        CHECK(a.OfficerGuid == b.OfficerGuid);
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.RankId == b.RankId);
        CHECK(a.Promote == b.Promote);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventRankChanged round-trips", "[packets]")
{
    GuildEventRankChanged packet;
    packet.RankId = 2;
    RoundTrip(packet, [](GuildEventRankChanged const& a, GuildEventRankChanged const& b) { CHECK(a.RankId == b.RankId); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GuildEventRanksUpdated round-trips", "[packets]")
{
    GuildEventRanksUpdated packet;
    RoundTrip(packet, [](GuildEventRanksUpdated const&, GuildEventRanksUpdated const&) {});
}

TEST_CASE("GuildEventDisbanded round-trips", "[packets]")
{
    GuildEventDisbanded packet;
    RoundTrip(packet, [](GuildEventDisbanded const&, GuildEventDisbanded const&) {});
}

TEST_CASE("SaveGuildEmblemResult round-trips", "[packets]")
{
    SaveGuildEmblemResult packet;
    packet.Error = 5;
    RoundTrip(packet, [](SaveGuildEmblemResult const& a, SaveGuildEmblemResult const& b) { CHECK(a.Error == b.Error); });
    RejectsEveryTruncation(packet);
}
