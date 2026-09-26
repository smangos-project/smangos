// Forming a party, changing its rules and marking targets, asking for a ready check, pinging and
// rolling, and breaking it up again, between two bots.
#include "Packets/GroupMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
PartyPlayerInfo const* FindMember(PartyUpdate const& update, ClassicGuid const& guid)
{
    for (PartyPlayerInfo const& member : update.Players)
        if (member.Guid == guid)
            return &member;
    return nullptr;
}

bool HasBothMembers(PartyUpdate const& update, ClassicGuid const& first, ClassicGuid const& second)
{
    return update.Players.size() == 2 && FindMember(update, first) && FindMember(update, second);
}

}

TEST_CASE("group: an invite that is declined tells the inviter", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot leader;
        TestBot member;
        REQUIRE(co_await leader.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await member.Stage(BotStage::InWorld, 30000));
        // Characters are saved in their party, so a run that was cut short leaves one behind.
        leader.Send(LeaveGroup());
        member.Send(LeaveGroup());
        co_await TestDelay(1500);

        ClassicGuid const leaderGuid = leader.GetCharacterGuid();
        std::string const memberName = member.GetCharacterName();

        PartyInviteClient invite;
        invite.TargetName = memberName;
        leader.Send(invite);
        REQUIRE(co_await member.WaitReceived<PartyInvite>(10000, [&](PartyInvite const& p) { return p.InviterGuid == leaderGuid; }));

        PartyInviteResponse decline;
        decline.Accept = false;
        member.Send(decline);
        REQUIRE(co_await leader.WaitReceived<GroupDecline>(10000, [&](GroupDecline const& p) { return p.Name == memberName; }));
    });
}

TEST_CASE("group: a party forms, changes its rules and breaks up", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot leader;
        TestBot member;
        REQUIRE(co_await leader.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await member.Stage(BotStage::InWorld, 30000));
        // Characters are saved in their party, so a run that was cut short leaves one behind.
        leader.Send(LeaveGroup());
        member.Send(LeaveGroup());
        co_await TestDelay(1500);

        ClassicGuid const leaderGuid = leader.GetCharacterGuid();
        ClassicGuid const memberGuid = member.GetCharacterGuid();

        // The leader invites the other bot and it accepts; both have the roster afterwards.
        std::string const memberName = member.GetCharacterName();
        PartyInviteClient invite;
        invite.TargetName = memberName;
        leader.Send(invite);
        REQUIRE(co_await leader.WaitReceived<PartyCommandResult>(10000, [&](PartyCommandResult const& p) { return p.Name == memberName; }));
        CHECK(leader.FindReceived<PartyCommandResult>([&](PartyCommandResult const& p) { return p.Name == memberName; })->Result == 0);
        REQUIRE(co_await member.WaitReceived<PartyInvite>(10000, [&](PartyInvite const& p) { return p.InviterGuid == leaderGuid; }));

        PartyInviteResponse accept;
        accept.Accept = true;
        member.Send(accept);
        auto joined = [&](PartyUpdate const& p) { return HasBothMembers(p, leaderGuid, memberGuid); };
        REQUIRE(co_await leader.WaitReceived<PartyUpdate>(10000, joined));
        REQUIRE(co_await member.WaitReceived<PartyUpdate>(10000, joined));

        auto rosterOfMember = member.FindReceived<PartyUpdate>([&](PartyUpdate const& p) { return HasBothMembers(p, leaderGuid, memberGuid); });
        REQUIRE(rosterOfMember);
        CHECK(rosterOfMember->LeaderGuid == leaderGuid);
        REQUIRE(rosterOfMember->MyIndex >= 0);
        CHECK(rosterOfMember->Players[size_t(rosterOfMember->MyIndex)].Guid == memberGuid);
        CHECK(rosterOfMember->PartyType == CLASSIC_PARTY_TYPE_NORMAL);
        CHECK_FALSE(rosterOfMember->PartyGuid.IsEmpty());
        REQUIRE(rosterOfMember->LootSettings.has_value());
        for (PartyPlayerInfo const& info : rosterOfMember->Players)
        {
            CHECK(info.Connected);
            CHECK(info.ClassId != 0);
            CHECK(info.FactionGroup != 0);
        }

        // The leader picks group loot at rare quality.
        SetLootMethod groupLoot;
        groupLoot.Method = 3;
        groupLoot.Threshold = 3;
        leader.Send(groupLoot);
        auto isGroupLoot = [](PartyUpdate const& p) { return p.LootSettings && p.LootSettings->Method == 3 && p.LootSettings->Threshold == 3; };
        REQUIRE(co_await leader.WaitReceived<PartyUpdate>(10000, isGroupLoot));
        REQUIRE(co_await member.WaitReceived<PartyUpdate>(10000, isGroupLoot));

        // Then master loot with themselves as the master.
        SetLootMethod masterLoot;
        masterLoot.Method = 2;
        masterLoot.LootMaster = leaderGuid;
        masterLoot.Threshold = 2;
        leader.Send(masterLoot);
        REQUIRE(co_await member.WaitReceived<PartyUpdate>(10000, [&](PartyUpdate const& p)
        {
            return p.LootSettings && p.LootSettings->Method == 2 && p.LootSettings->LootMaster == leaderGuid;
        }));

        // A raid can make assistants.
        ConvertRaid convert;
        convert.Raid = true;
        leader.Send(convert);
        REQUIRE(co_await member.WaitReceived<PartyUpdate>(10000, [](PartyUpdate const& p) { return (p.PartyFlags & CLASSIC_PARTY_FLAG_RAID) != 0; }));

        SetAssistantLeader assistant;
        assistant.Apply = true;
        assistant.Target = memberGuid;
        leader.Send(assistant);
        REQUIRE(co_await leader.WaitReceived<PartyUpdate>(10000, [&](PartyUpdate const& p)
        {
            PartyPlayerInfo const* info = FindMember(p, memberGuid);
            return info && (info->Flags & CLASSIC_PARTY_MEMBER_FLAG_ASSISTANT) != 0;
        }));

        // Raid markers, on the other member, seen by both.
        UpdateRaidTarget mark;
        mark.Target = memberGuid;
        mark.Symbol = 3;
        leader.Send(mark);
        auto isMark = [&](RaidTargetUpdateSingle const& p) { return p.Symbol == 3 && p.Target == memberGuid && p.ChangedBy == leaderGuid; };
        REQUIRE(co_await leader.WaitReceived<RaidTargetUpdateSingle>(10000, isMark));
        REQUIRE(co_await member.WaitReceived<RaidTargetUpdateSingle>(10000, isMark));

        UpdateRaidTarget request;
        request.Symbol = -1;
        member.Send(request);
        REQUIRE(co_await member.WaitReceived<RaidTargetUpdateAll>(10000, [&](RaidTargetUpdateAll const& p)
        {
            return p.Targets.size() == 1 && p.Targets[0].Symbol == 3 && p.Targets[0].Target == memberGuid;
        }));

        // A ready check: the asker counts as ready, the other answers, and it completes.
        leader.Send(DoReadyCheck());
        REQUIRE(co_await member.WaitReceived<ReadyCheckStarted>(10000, [&](ReadyCheckStarted const& p) { return p.InitiatorGuid == leaderGuid; }));
        REQUIRE(co_await member.WaitReceived<ReadyCheckResponse>(10000, [&](ReadyCheckResponse const& p) { return p.Player == leaderGuid && p.IsReady; }));

        ReadyCheckResponseClient ready;
        ready.IsReady = true;
        member.Send(ready);
        REQUIRE(co_await leader.WaitReceived<ReadyCheckResponse>(10000, [&](ReadyCheckResponse const& p) { return p.Player == memberGuid && p.IsReady; }));
        REQUIRE(co_await leader.WaitReceived<ReadyCheckCompleted>(10000));

        // A minimap ping reaches the others but not the sender.
        MinimapPingClient ping;
        ping.PositionX = 12.5f;
        ping.PositionY = -30.0f;
        member.Send(ping);
        REQUIRE(co_await leader.WaitReceived<MinimapPing>(10000, [&](MinimapPing const& p)
        {
            return p.Sender == memberGuid && p.PositionX == 12.5f && p.PositionY == -30.0f;
        }));

        // A roll goes to everyone.
        RandomRollClient roll;
        roll.Min = 1;
        roll.Max = 100;
        member.Send(roll);
        auto isRoll = [&](RandomRoll const& p) { return p.Roller == memberGuid && p.Min == 1 && p.Max == 100 && p.Result >= 1 && p.Result <= 100; };
        REQUIRE(co_await leader.WaitReceived<RandomRoll>(10000, isRoll));
        REQUIRE(co_await member.WaitReceived<RandomRoll>(10000, isRoll));

        // The leader asks how the other is doing.
        RequestPartyMemberStats stats;
        stats.Target = memberGuid;
        leader.Send(stats);
        REQUIRE(co_await leader.WaitReceived<PartyMemberFullState>(10000, [&](PartyMemberFullState const& p) { return p.MemberGuid == memberGuid; }));
        auto state = leader.FindReceived<PartyMemberFullState>([&](PartyMemberFullState const& p) { return p.MemberGuid == memberGuid; });
        CHECK((state->Status & 1) != 0);
        CHECK(state->Level >= 1);
        CHECK(state->MaxHealth > 0);

        // The leader removes the member; with two left, the party is gone for both.
        PartyUninvite kick;
        kick.TargetGuid = memberGuid;
        leader.Send(kick);
        auto isDestroyed = [](PartyUpdate const& p) { return (p.PartyFlags & CLASSIC_PARTY_FLAG_DESTROYED) != 0 && p.MyIndex == -1 && p.Players.empty(); };
        REQUIRE(co_await leader.WaitReceived<PartyUpdate>(10000, isDestroyed));
        REQUIRE(co_await member.WaitReceived<PartyUpdate>(10000, isDestroyed));
    });
}
