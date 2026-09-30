// A GM founds a guild, is joined by a plain bot, promotes and finally kicks it. Exercises query
// (both the guild-guid and player-guid lookup paths), roster, ranks, permissions, MOTD and the
// typed SMSG_GUILD_EVENT_* broadcasts between a GM bot and a plain bot.
#include "Packets/GuildMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("guild: a GM founds a guild, invites, promotes and kicks a member", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot founder(TestBotOptions().Gm());
        TestBot member;
        REQUIRE(co_await founder.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await member.Stage(BotStage::InWorld, 30000));

        // A run that was cut short leaves the pooled accounts in a guild from before; leaving is a
        // harmless no-op for an account that isn't in one.
        GuildLeave leaveStale;
        founder.Send(leaveStale);
        member.Send(leaveStale);
        co_await TestDelay(500);

        ClassicGuid const founderGuid = founder.GetCharacterGuid();
        ClassicGuid const memberGuid = member.GetCharacterGuid();

        SayCommand(founder, ".guild create \"Realbots Test Guild\"");
        co_await TestDelay(500);

        // Player-guid lookup path: no guild guid known yet, resolves through the founder's own
        // membership.
        GuildQuery queryByPlayer;
        queryByPlayer.PlayerGuid = founderGuid;
        founder.Send(queryByPlayer);
        auto const infoByPlayer = co_await founder.Packet<QueryGuildInfoResponse>(5000);
        REQUIRE(infoByPlayer);
        CHECK(infoByPlayer->Valid);
        CHECK(infoByPlayer->GuildName == "Realbots Test Guild");
        REQUIRE(infoByPlayer->RankNames.size() == 5);
        CHECK(!infoByPlayer->RankNames[0].empty());
        ClassicGuid const guildGuid = infoByPlayer->ResolvedGuildGuid;
        REQUIRE(!guildGuid.IsEmpty());

        // Guild-guid lookup path.
        GuildQuery queryByGuild;
        queryByGuild.GuildGuid = guildGuid;
        founder.Send(queryByGuild);
        auto const infoByGuild = co_await founder.Packet<QueryGuildInfoResponse>(5000);
        REQUIRE(infoByGuild);
        CHECK(infoByGuild->Valid);
        CHECK(infoByGuild->GuildName == "Realbots Test Guild");

        GuildGetRoster getRoster;
        founder.Send(getRoster);
        auto const soloRoster = co_await founder.Packet<GuildRoster>(5000);
        REQUIRE(soloRoster);
        REQUIRE(soloRoster->RosterMembers.size() == 1);
        CHECK(soloRoster->RosterMembers[0].Guid == founderGuid);
        CHECK(soloRoster->RosterMembers[0].RankId == 0);

        GuildGetRanks getRanks;
        founder.Send(getRanks);
        auto const ranks = co_await founder.Packet<GuildRanksResponse>(5000);
        REQUIRE(ranks);
        REQUIRE(ranks->Ranks.size() == 5);
        CHECK(ranks->Ranks[0].RankId == 0);
        CHECK(ranks->Ranks[0].Flags == 0x000FF1FF); // GR_RIGHT_ALL (Guild.h, not linked here)

        GuildPermissionsQuery permissionsQuery;
        founder.Send(permissionsQuery);
        auto const permissions = co_await founder.Packet<GuildPermissionsQueryResults>(5000);
        REQUIRE(permissions);
        CHECK(permissions->RankId == 0);
        CHECK(permissions->Flags == 0x000FF1FF); // GR_RIGHT_ALL

        GuildUpdateMotdText motd;
        motd.Motd = "Welcome to Realbots Test Guild";
        founder.Send(motd);
        auto const motdEvent = co_await founder.Packet<GuildEventMotd>(5000);
        REQUIRE(motdEvent);
        CHECK(motdEvent->MotdText == motd.Motd);

        GuildInviteByName invite;
        invite.InvitedName = member.GetCharacterName();
        founder.Send(invite);
        auto const inviteNotice = co_await member.Packet<GuildInviteNotification>(5000);
        REQUIRE(inviteNotice);
        CHECK(inviteNotice->InviterName == founder.GetCharacterName());
        CHECK(inviteNotice->GuildName == "Realbots Test Guild");

        AcceptGuildInvite accept;
        member.Send(accept);
        REQUIRE(co_await founder.WaitReceived<GuildEventPlayerJoined>(5000, [&](GuildEventPlayerJoined const& p) { return p.Guid == memberGuid; }));
        REQUIRE(co_await member.WaitReceived<GuildEventMotd>(5000, [&](GuildEventMotd const& p) { return p.MotdText == motd.Motd; }));

        GuildPromoteMember promote;
        promote.PromoteeGuid = memberGuid;
        founder.Send(promote);
        REQUIRE(co_await member.WaitReceived<GuildSendRankChange>(5000, [&](GuildSendRankChange const& p)
        {
            return p.TargetGuid == memberGuid && p.Promote;
        }));

        GuildGetRoster getRosterWithMember;
        founder.Send(getRosterWithMember);
        auto const fullRoster = co_await founder.Packet<GuildRoster>(5000);
        REQUIRE(fullRoster);
        CHECK(fullRoster->RosterMembers.size() == 2);

        GuildOfficerRemoveMember kick;
        kick.RemoveeGuid = memberGuid;
        founder.Send(kick);
        REQUIRE(co_await founder.WaitReceived<GuildEventPlayerLeft>(5000, [&](GuildEventPlayerLeft const& p)
        {
            return p.Removed && p.LeaverGuid == memberGuid;
        }));

        GuildLeave leave;
        founder.Send(leave);
    });
}
