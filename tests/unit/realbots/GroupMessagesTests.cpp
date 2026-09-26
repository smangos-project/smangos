// Round trips for the modern group wire.
#include "Packets/GroupMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid Player(uint64 counter) { return { counter, 0x0800000000000000ULL }; }
ClassicGuid Party() { return { 1000, 0x6C00000000000000ULL }; }

void CheckOptionalIndex(std::optional<uint8> const& a, std::optional<uint8> const& b)
{
    REQUIRE(a.has_value() == b.has_value());
    if (a)
        CHECK(*a == *b);
}
}

TEST_CASE("PartyInviteClient round-trips with and without a party index", "[packets]")
{
    PartyInviteClient invite;
    invite.PartyIndex = 1;
    invite.ProposedRoles = 4;
    invite.TargetGuid = Player(7);
    invite.TargetName = "Anton";
    invite.TargetRealm = "Realm";
    auto check = [](PartyInviteClient const& a, PartyInviteClient const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(a.ProposedRoles == b.ProposedRoles);
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.TargetName == b.TargetName);
        CHECK(a.TargetRealm == b.TargetRealm);
    };
    RoundTrip(invite, check);
    RejectsEveryTruncation(invite);

    invite.PartyIndex.reset();
    invite.TargetRealm.clear();
    RoundTrip(invite, check);
    RejectsEveryTruncation(invite);
}

TEST_CASE("PartyInviteResponse, PartyUninvite and LeaveGroup round-trip", "[packets]")
{
    PartyInviteResponse response;
    response.Accept = true;
    response.RolesDesired = 2;
    RoundTrip(response, [](PartyInviteResponse const& a, PartyInviteResponse const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(b.Accept);
        REQUIRE(b.RolesDesired.has_value());
        CHECK(*b.RolesDesired == *a.RolesDesired);
    });
    RejectsEveryTruncation(response);

    response.PartyIndex = 0;
    response.Accept = false;
    response.RolesDesired.reset();
    RoundTrip(response, [](PartyInviteResponse const& a, PartyInviteResponse const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK_FALSE(b.Accept);
        CHECK_FALSE(b.RolesDesired.has_value());
    });
    RejectsEveryTruncation(response);

    PartyUninvite uninvite;
    uninvite.PartyIndex = 0;
    uninvite.TargetGuid = Player(9);
    uninvite.Reason = "afk";
    RoundTrip(uninvite, [](PartyUninvite const& a, PartyUninvite const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(uninvite);

    LeaveGroup leave;
    leave.PartyIndex = 1;
    RoundTrip(leave, [](LeaveGroup const& a, LeaveGroup const& b) { CheckOptionalIndex(a.PartyIndex, b.PartyIndex); });
    RejectsEveryTruncation(leave);
}

TEST_CASE("Leader, assistant and assignment requests round-trip", "[packets]")
{
    SetPartyLeader leader;
    leader.TargetGuid = Player(3);
    RoundTrip(leader, [](SetPartyLeader const& a, SetPartyLeader const& b)
    {
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK_FALSE(b.PartyIndex.has_value());
    });
    RejectsEveryTruncation(leader);

    SetAssistantLeader assistant;
    assistant.PartyIndex = 0;
    assistant.Apply = true;
    assistant.Target = Player(4);
    RoundTrip(assistant, [](SetAssistantLeader const& a, SetAssistantLeader const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(b.Apply);
        CHECK(a.Target == b.Target);
    });
    RejectsEveryTruncation(assistant);

    SetPartyAssignment assignment;
    assignment.Set = true;
    assignment.Assignment = CLASSIC_PARTY_ASSIGN_MAIN_ASSIST;
    assignment.Target = Player(5);
    RoundTrip(assignment, [](SetPartyAssignment const& a, SetPartyAssignment const& b)
    {
        CHECK(b.Set);
        CHECK(a.Assignment == b.Assignment);
        CHECK(a.Target == b.Target);
    });
    RejectsEveryTruncation(assignment);
}

TEST_CASE("Loot method, ping, roll and marker requests round-trip", "[packets]")
{
    SetLootMethod method;
    method.PartyIndex = 0;
    method.Method = 2;
    method.LootMaster = Player(6);
    method.Threshold = 3;
    RoundTrip(method, [](SetLootMethod const& a, SetLootMethod const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(a.Method == b.Method);
        CHECK(a.LootMaster == b.LootMaster);
        CHECK(a.Threshold == b.Threshold);
    });
    RejectsEveryTruncation(method);

    MinimapPingClient ping;
    ping.PositionX = 1.5f;
    ping.PositionY = -2.5f;
    RoundTrip(ping, [](MinimapPingClient const& a, MinimapPingClient const& b)
    {
        CHECK(a.PositionX == b.PositionX);
        CHECK(a.PositionY == b.PositionY);
    });
    RejectsEveryTruncation(ping);

    RandomRollClient roll;
    roll.PartyIndex = 0;
    roll.Min = 1;
    roll.Max = 100;
    RoundTrip(roll, [](RandomRollClient const& a, RandomRollClient const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
        CHECK(a.Min == b.Min);
        CHECK(a.Max == b.Max);
    });
    RejectsEveryTruncation(roll);

    UpdateRaidTarget marker;
    marker.Target = Player(8);
    marker.Symbol = -1;
    RoundTrip(marker, [](UpdateRaidTarget const& a, UpdateRaidTarget const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(b.Symbol == -1);
    });
    RejectsEveryTruncation(marker);
}

TEST_CASE("Raid and ready check requests round-trip", "[packets]")
{
    ConvertRaid convert;
    convert.Raid = true;
    RoundTrip(convert, [](ConvertRaid const&, ConvertRaid const& b) { CHECK(b.Raid); });
    RejectsEveryTruncation(convert);

    RequestPartyJoinUpdates joinUpdates;
    joinUpdates.PartyIndex = 1;
    RoundTrip(joinUpdates, [](RequestPartyJoinUpdates const& a, RequestPartyJoinUpdates const& b)
    {
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
    });
    RejectsEveryTruncation(joinUpdates);

    ChangeSubGroup change;
    change.Target = Player(2);
    change.NewSubGroup = 3;
    change.PartyIndex = 0;
    RoundTrip(change, [](ChangeSubGroup const& a, ChangeSubGroup const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.NewSubGroup == b.NewSubGroup);
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
    });
    RejectsEveryTruncation(change);

    SwapSubGroups swap;
    swap.FirstTarget = Player(2);
    swap.SecondTarget = Player(3);
    RoundTrip(swap, [](SwapSubGroups const& a, SwapSubGroups const& b)
    {
        CHECK(a.FirstTarget == b.FirstTarget);
        CHECK(a.SecondTarget == b.SecondTarget);
    });
    RejectsEveryTruncation(swap);

    DoReadyCheck check;
    check.PartyIndex = 0;
    RoundTrip(check, [](DoReadyCheck const& a, DoReadyCheck const& b) { CheckOptionalIndex(a.PartyIndex, b.PartyIndex); });
    RejectsEveryTruncation(check);

    ReadyCheckResponseClient answer;
    answer.IsReady = true;
    answer.PartyIndex = 1;
    RoundTrip(answer, [](ReadyCheckResponseClient const& a, ReadyCheckResponseClient const& b)
    {
        CHECK(b.IsReady);
        CheckOptionalIndex(a.PartyIndex, b.PartyIndex);
    });
    RejectsEveryTruncation(answer);

    RequestPartyMemberStats stats;
    stats.Target = Player(12);
    RoundTrip(stats, [](RequestPartyMemberStats const& a, RequestPartyMemberStats const& b) { CHECK(a.Target == b.Target); });
    RejectsEveryTruncation(stats);
}

TEST_CASE("PartyCommandResult round-trips", "[packets]")
{
    PartyCommandResult result;
    result.Name = "Anton";
    result.Command = 2;
    result.Result = 5;
    result.ResultData = 9;
    result.ResultGuid = Player(1);
    RoundTrip(result, [](PartyCommandResult const& a, PartyCommandResult const& b)
    {
        CHECK(a.Name == b.Name);
        CHECK(a.Command == b.Command);
        CHECK(a.Result == b.Result);
        CHECK(a.ResultData == b.ResultData);
        CHECK(a.ResultGuid == b.ResultGuid);
    });
    RejectsEveryTruncation(result);
}

TEST_CASE("PartyInvite round-trips and refuses dungeon finder slots", "[packets]")
{
    PartyInvite invite;
    invite.CanAccept = true;
    invite.InviterName = "Anton";
    invite.InviterRealm.RealmAddress = 0x01000001;
    invite.InviterRealm.RealmNameActual = "Test Realm";
    invite.InviterRealm.RealmNameNormalized = "testrealm";
    invite.InviterGuid = Player(7);
    invite.InviterBNetAccountId = { 3, 0x7800000000000000ULL };
    invite.Unk1 = 4904;
    invite.ProposedRoles = 2;
    RoundTrip(invite, [](PartyInvite const& a, PartyInvite const& b)
    {
        CHECK(b.CanAccept);
        CHECK(a.InviterName == b.InviterName);
        CHECK(a.InviterRealm.RealmAddress == b.InviterRealm.RealmAddress);
        CHECK(b.InviterRealm.IsLocal);
        CHECK(a.InviterRealm.RealmNameActual == b.InviterRealm.RealmNameActual);
        CHECK(a.InviterRealm.RealmNameNormalized == b.InviterRealm.RealmNameNormalized);
        CHECK(a.InviterGuid == b.InviterGuid);
        CHECK(a.InviterBNetAccountId == b.InviterBNetAccountId);
        CHECK(a.Unk1 == b.Unk1);
        CHECK(a.ProposedRoles == b.ProposedRoles);
    });
    RejectsEveryTruncation(invite);

    ByteBuffer data;
    invite.Write(data);
    // the slot count sits 4 bytes before the mask and the name
    ByteBuffer withSlot;
    withSlot.append(data.contents(), data.size() - invite.InviterName.size() - 2 * sizeof(uint32));
    withSlot << uint32(1) << uint32(0);
    withSlot.WriteString(invite.InviterName);
    PartyInvite in;
    CHECK_FALSE(in.Read(withSlot));
}

TEST_CASE("Decline, uninvite, destroyed and new leader round-trip", "[packets]")
{
    GroupDecline decline;
    decline.Name = "Anton";
    RoundTrip(decline, [](GroupDecline const& a, GroupDecline const& b) { CHECK(a.Name == b.Name); });
    RejectsEveryTruncation(decline);

    RoundTrip(GroupUninvite(), [](GroupUninvite const&, GroupUninvite const&) {});
    RoundTrip(GroupDestroyed(), [](GroupDestroyed const&, GroupDestroyed const&) {});

    GroupNewLeader leader;
    leader.Name = "Anton";
    RoundTrip(leader, [](GroupNewLeader const& a, GroupNewLeader const& b)
    {
        CHECK(a.PartyIndex == b.PartyIndex);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(leader);
}

TEST_CASE("PartyUpdate round-trips a roster and a destroyed party", "[packets]")
{
    PartyUpdate update;
    update.PartyFlags = CLASSIC_PARTY_FLAG_RAID;
    update.PartyType = CLASSIC_PARTY_TYPE_NORMAL;
    update.MyIndex = 1;
    update.PartyGuid = Party();
    update.SequenceNum = 5;
    update.LeaderGuid = Player(1);
    update.LeaderFactionGroup = 3;
    for (uint64 i = 1; i <= 2; ++i)
    {
        PartyPlayerInfo member;
        member.Guid = Player(i);
        member.Name = i == 1 ? "Anton" : "Bertil";
        member.Subgroup = uint8(i);
        member.Flags = i == 2 ? CLASSIC_PARTY_MEMBER_FLAG_ASSISTANT : 0;
        member.ClassId = 1;
        member.FactionGroup = 3;
        member.Connected = i == 1;
        update.Players.push_back(member);
    }
    update.LootSettings.emplace();
    update.LootSettings->Method = 2;
    update.LootSettings->LootMaster = Player(1);
    update.LootSettings->Threshold = 3;
    update.DifficultySettings.emplace();
    update.DifficultySettings->DungeonDifficultyId = 1;
    update.DifficultySettings->RaidDifficultyId = 9;
    RoundTrip(update, [](PartyUpdate const& a, PartyUpdate const& b)
    {
        CHECK(a.PartyFlags == b.PartyFlags);
        CHECK(a.MyIndex == b.MyIndex);
        CHECK(a.PartyGuid == b.PartyGuid);
        CHECK(a.SequenceNum == b.SequenceNum);
        CHECK(a.LeaderGuid == b.LeaderGuid);
        CHECK(a.LeaderFactionGroup == b.LeaderFactionGroup);
        REQUIRE(a.Players.size() == b.Players.size());
        for (size_t i = 0; i < a.Players.size(); ++i)
        {
            CHECK(a.Players[i].Guid == b.Players[i].Guid);
            CHECK(a.Players[i].Name == b.Players[i].Name);
            CHECK(a.Players[i].Subgroup == b.Players[i].Subgroup);
            CHECK(a.Players[i].Flags == b.Players[i].Flags);
            CHECK(a.Players[i].ClassId == b.Players[i].ClassId);
            CHECK(a.Players[i].FactionGroup == b.Players[i].FactionGroup);
            CHECK(a.Players[i].Connected == b.Players[i].Connected);
        }
        REQUIRE(b.LootSettings.has_value());
        CHECK(a.LootSettings->Method == b.LootSettings->Method);
        CHECK(a.LootSettings->LootMaster == b.LootSettings->LootMaster);
        CHECK(a.LootSettings->Threshold == b.LootSettings->Threshold);
        REQUIRE(b.DifficultySettings.has_value());
        CHECK(a.DifficultySettings->RaidDifficultyId == b.DifficultySettings->RaidDifficultyId);
    });
    RejectsEveryTruncation(update);

    PartyUpdate destroyed;
    destroyed.PartyFlags = CLASSIC_PARTY_FLAG_DESTROYED;
    destroyed.PartyType = CLASSIC_PARTY_TYPE_NONE;
    destroyed.PartyGuid = Party();
    RoundTrip(destroyed, [](PartyUpdate const& a, PartyUpdate const& b)
    {
        CHECK(b.MyIndex == -1);
        CHECK(b.Players.empty());
        CHECK_FALSE(b.LootSettings.has_value());
        CHECK(a.PartyFlags == b.PartyFlags);
    });
    RejectsEveryTruncation(destroyed);
}

TEST_CASE("PartyMemberFullState round-trips with auras and a pet", "[packets]")
{
    PartyMemberFullState state;
    state.Status = 1;
    state.PowerType = 1;
    state.CurrentHealth = 500;
    state.MaxHealth = 600;
    state.CurrentPower = 20;
    state.MaxPower = 100;
    state.Level = 12;
    state.ZoneId = 12;
    state.PositionX = -8900;
    state.PositionY = -120;
    state.PositionZ = 80;
    state.Auras = { { 1459, 0x100, 1 }, { 5, 0x10, 3 } };
    state.MemberGuid = Player(2);
    auto check = [](PartyMemberFullState const& a, PartyMemberFullState const& b)
    {
        CHECK(a.Status == b.Status);
        CHECK(a.PowerType == b.PowerType);
        CHECK(a.CurrentHealth == b.CurrentHealth);
        CHECK(a.MaxHealth == b.MaxHealth);
        CHECK(a.CurrentPower == b.CurrentPower);
        CHECK(a.MaxPower == b.MaxPower);
        CHECK(a.Level == b.Level);
        CHECK(a.ZoneId == b.ZoneId);
        CHECK(a.PositionX == b.PositionX);
        CHECK(a.PositionY == b.PositionY);
        CHECK(a.PositionZ == b.PositionZ);
        REQUIRE(a.Auras.size() == b.Auras.size());
        for (size_t i = 0; i < a.Auras.size(); ++i)
        {
            CHECK(a.Auras[i].SpellId == b.Auras[i].SpellId);
            CHECK(a.Auras[i].Flags == b.Auras[i].Flags);
            CHECK(a.Auras[i].ActiveFlags == b.Auras[i].ActiveFlags);
        }
        CHECK(a.Pet.has_value() == b.Pet.has_value());
        if (a.Pet)
        {
            CHECK(a.Pet->Guid == b.Pet->Guid);
            CHECK(a.Pet->Name == b.Pet->Name);
            CHECK(a.Pet->CurrentHealth == b.Pet->CurrentHealth);
            CHECK(a.Pet->Auras.size() == b.Pet->Auras.size());
        }
        CHECK(a.MemberGuid == b.MemberGuid);
    };
    RoundTrip(state, check);
    RejectsEveryTruncation(state);

    state.Pet.emplace();
    state.Pet->Guid = { 4, 0x2800000000000000ULL };
    state.Pet->ModelId = 850;
    state.Pet->CurrentHealth = 90;
    state.Pet->MaxHealth = 100;
    state.Pet->Auras = { { 2, 0x100, 1 } };
    state.Pet->Name = "Wrathguard";
    RoundTrip(state, check);
    RejectsEveryTruncation(state);
}

TEST_CASE("Ready check messages round-trip", "[packets]")
{
    ReadyCheckStarted started;
    started.PartyGuid = Party();
    started.InitiatorGuid = Player(1);
    started.Duration = 35000;
    RoundTrip(started, [](ReadyCheckStarted const& a, ReadyCheckStarted const& b)
    {
        CHECK(a.PartyGuid == b.PartyGuid);
        CHECK(a.InitiatorGuid == b.InitiatorGuid);
        CHECK(a.Duration == b.Duration);
    });
    RejectsEveryTruncation(started);

    ReadyCheckResponse response;
    response.PartyGuid = Party();
    response.Player = Player(2);
    response.IsReady = true;
    RoundTrip(response, [](ReadyCheckResponse const& a, ReadyCheckResponse const& b)
    {
        CHECK(a.PartyGuid == b.PartyGuid);
        CHECK(a.Player == b.Player);
        CHECK(b.IsReady);
    });
    RejectsEveryTruncation(response);

    ReadyCheckCompleted completed;
    completed.PartyGuid = Party();
    RoundTrip(completed, [](ReadyCheckCompleted const& a, ReadyCheckCompleted const& b) { CHECK(a.PartyGuid == b.PartyGuid); });
    RejectsEveryTruncation(completed);
}

TEST_CASE("Raid marker messages round-trip and cap the marker count", "[packets]")
{
    RaidTargetUpdateSingle single;
    single.Symbol = 7;
    single.Target = { 5, 0x2000000000000000ULL };
    single.ChangedBy = Player(1);
    RoundTrip(single, [](RaidTargetUpdateSingle const& a, RaidTargetUpdateSingle const& b)
    {
        CHECK(a.Symbol == b.Symbol);
        CHECK(a.Target == b.Target);
        CHECK(a.ChangedBy == b.ChangedBy);
    });
    RejectsEveryTruncation(single);

    RaidTargetUpdateAll all;
    all.Targets = { { Player(1), 0 }, { Player(2), 7 } };
    RoundTrip(all, [](RaidTargetUpdateAll const& a, RaidTargetUpdateAll const& b)
    {
        REQUIRE(a.Targets.size() == b.Targets.size());
        for (size_t i = 0; i < a.Targets.size(); ++i)
        {
            CHECK(a.Targets[i].Target == b.Targets[i].Target);
            CHECK(a.Targets[i].Symbol == b.Targets[i].Symbol);
        }
    });
    RejectsEveryTruncation(all);

    ByteBuffer tooMany;
    tooMany << uint8(0) << uint32(9);
    RaidTargetUpdateAll in;
    CHECK_FALSE(in.Read(tooMany));
}

TEST_CASE("Ping, roll and master loot list round-trip", "[packets]")
{
    MinimapPing ping;
    ping.Sender = Player(1);
    ping.PositionX = 10.0f;
    ping.PositionY = 20.0f;
    RoundTrip(ping, [](MinimapPing const& a, MinimapPing const& b)
    {
        CHECK(a.Sender == b.Sender);
        CHECK(a.PositionX == b.PositionX);
        CHECK(a.PositionY == b.PositionY);
    });
    RejectsEveryTruncation(ping);

    RandomRoll roll;
    roll.Roller = Player(1);
    roll.RollerWowAccount = { 2, 0x7400000000000000ULL };
    roll.Min = 1;
    roll.Max = 100;
    roll.Result = 42;
    RoundTrip(roll, [](RandomRoll const& a, RandomRoll const& b)
    {
        CHECK(a.Roller == b.Roller);
        CHECK(a.RollerWowAccount == b.RollerWowAccount);
        CHECK(a.Min == b.Min);
        CHECK(a.Max == b.Max);
        CHECK(a.Result == b.Result);
    });
    RejectsEveryTruncation(roll);

    MasterLootCandidateList list;
    list.LootObj = { 6, 0x2000000000000000ULL };
    list.Players = { Player(1), Player(2) };
    RoundTrip(list, [](MasterLootCandidateList const& a, MasterLootCandidateList const& b)
    {
        CHECK(a.LootObj == b.LootObj);
        CHECK(a.Players == b.Players);
    });
    RejectsEveryTruncation(list);
}
