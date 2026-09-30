/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Packets/Classic/GuidMapping.h"
#include "Group.h"
#include "Utilities/Random.h"
#include "SocialMgr.h"
#include "Util.h"

/* differeces from off:
    -you can uninvite yourself - is is useful
    -you can accept invitation even if leader went offline
*/
/* todo:
    -reduce xp gaining when in raid group
    -quest sharing has to be corrected
*/

namespace
{
// Region 1 and battlegroup 1 are what a single-realm setup announces.
constexpr uint32 PARTY_INVITE_REALM_REGION = 1;
constexpr uint32 PARTY_INVITE_REALM_BATTLEGROUP = 1;

ObjectGuid ResolveGuid(ClassicGuid const& guid)
{
    auto const resolved = ClassicGuidMapping::FromClassic(guid);
    return resolved ? *resolved : ObjectGuid();
}

ClassicGuid ToClassic(ObjectGuid const& guid)
{
    return ClassicGuidMapping::ToClassic(guid, realmID);
}
}

void WorldSession::SendPartyResult(PartyOperation operation, std::string const& memberName, PartyResult res)
{
    PartyCommandResult result;
    result.Name = memberName;
    result.Command = uint8(operation);
    result.Result = uint8(res);
    SendClassic(result);
}

void WorldSession::HandlePartyInvite(PartyInviteClient const& packet)
{
    // Attempt add selected player
    std::string name = packet.TargetName;
    Player* player = nullptr;
    if (!name.empty())
    {
        // Cheating
        if (!normalizePlayerName(name))
        {
            SendPartyResult(PARTY_OP_INVITE, name, ERR_BAD_PLAYER_NAME_S);
            return;
        }
        player = sObjectMgr.GetPlayer(name.c_str());
    }
    else
        player = sObjectMgr.GetPlayer(ResolveGuid(packet.TargetGuid));

    // No player, or the inviter themselves
    if (!player || player == GetPlayer())
    {
        SendPartyResult(PARTY_OP_INVITE, name, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    // Can't group with
    if (!GetPlayer()->IsGameMaster() && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        SendPartyResult(PARTY_OP_INVITE, name, ERR_PLAYER_WRONG_FACTION);
        return;
    }

    // Just ignore us
    if (player->GetSocial()->HasIgnore(GetPlayer()->GetObjectGuid()))
    {
        SendPartyResult(PARTY_OP_INVITE, name, ERR_IGNORING_YOU_S);
        return;
    }

    Group* group = GetPlayer()->GetGroup();
    if (group && group->isBGGroup())
        group = GetPlayer()->GetOriginalGroup();

    Group* group2 = player->GetGroup();
    if (group2 && group2->isBGGroup())
        group2 = player->GetOriginalGroup();
    // Player already in another group or invited
    if (group2 || player->GetGroupInvite())
    {
        SendPartyResult(PARTY_OP_INVITE, name, ERR_ALREADY_IN_GROUP_S);
        return;
    }

    if (group)
    {
        // not have permissions for invite
        if (!group->IsLeader(GetPlayer()->GetObjectGuid()) && !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_NOT_LEADER);
            return;
        }
        // not have place
        if (group->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
            return;
        }
    }

    // ok, but group not exist, start a new group
    // but don't create and save the group to the DB until
    // at least one person joins
    if (!group)
    {
        group = new Group;
        // new group: if can't add then delete
        if (!group->AddLeaderInvite(GetPlayer()))
        {
            delete group;
            return;
        }
        if (!group->AddInvite(player))
        {
            delete group;
            return;
        }
    }
    else
    {
        // already existing group: if can't add then just leave
        if (!group->AddInvite(player))
            return;
    }

    // ok, we do it
    PartyInvite invite;
    invite.InviterName = GetPlayer()->GetName();
    invite.InviterRealm.RealmAddress = (PARTY_INVITE_REALM_REGION << 24) | (PARTY_INVITE_REALM_BATTLEGROUP << 16) | uint16(realmID);
    invite.InviterRealm.RealmNameActual = realmName;
    invite.InviterRealm.RealmNameNormalized = realmName;
    invite.InviterGuid = ToClassic(GetPlayer()->GetObjectGuid());
    invite.InviterBNetAccountId = ClassicGuidMapping::MakeBnetAccount(GetAccountId());
    player->GetSession()->SendClassic(invite);

    SendPartyResult(PARTY_OP_INVITE, player->GetName(), ERR_PARTY_RESULT_OK);
}

void WorldSession::HandlePartyInviteResponse(PartyInviteResponse const& packet)
{
    Group* group = GetPlayer()->GetGroupInvite();
    if (!group)
        return;

    if (!packet.Accept)
    {
        // remember leader if online
        Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid());

        // uninvite, group can be deleted
        GetPlayer()->UninviteFromGroup();

        if (!leader || !leader->GetSession())
            return;

        // report
        GroupDecline decline;
        decline.Name = GetPlayer()->GetName();
        leader->GetSession()->SendClassic(decline);
        return;
    }

    if (group->GetLeaderGuid() == GetPlayer()->GetObjectGuid())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePartyInviteResponse: %s tried to accept an invite to his own group",
                 GetPlayer()->GetGuidStr().c_str());
        return;
    }

    // remove in from invites in any case
    group->RemoveInvite(GetPlayer());

    /** error handling **/
    /********************/

    // not have place
    if (group->IsFull())
    {
        SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
        return;
    }

    Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid());

    // forming a new group, create it
    if (!group->IsCreated())
    {
        if (leader)
            group->RemoveInvite(leader);
        if (group->Create(group->GetLeaderGuid(), group->GetLeaderName()))
            sObjectMgr.AddGroup(group);
        else
            return;
    }

    // everything is fine, do it, PLAYER'S GROUP IS SET IN ADDMEMBER!!!
    if (!group->AddMember(GetPlayer()->GetObjectGuid(), GetPlayer()->GetName()))
        return;
    group->BroadcastGroupUpdate();
}

void WorldSession::HandlePartyUninvite(PartyUninvite const& packet)
{
    ObjectGuid const guid = ResolveGuid(packet.TargetGuid);

    // can't uninvite yourself
    if (guid == GetPlayer()->GetObjectGuid())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WorldSession::HandlePartyUninvite: leader %s tried to uninvite himself from the group.", GetPlayer()->GetGuidStr().c_str());
        return;
    }

    PartyResult res = GetPlayer()->CanUninviteFromGroup(guid);
    if (res != ERR_PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_UNINVITE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    if (grp->IsMember(guid))
    {
        Player::RemoveFromGroup(grp, guid);
        return;
    }

    if (Player* plr = grp->GetInvited(guid))
    {
        plr->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_UNINVITE, "", ERR_TARGET_NOT_IN_GROUP_S);
}

void WorldSession::HandleSetPartyLeader(SetPartyLeader const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    Player* player = sObjectMgr.GetPlayer(ResolveGuid(packet.TargetGuid));

    /** error handling **/
    if (!player || player == GetPlayer() || !group->IsLeader(GetPlayer()->GetObjectGuid()) || player->GetGroup() != group)
        return;

    // everything is fine, do it
    group->ChangeLeader(player->GetObjectGuid());
}

void WorldSession::HandleLeaveGroup(LeaveGroup const& /*packet*/)
{
    if (!GetPlayer()->GetGroup())
        return;

    if (_player->InBattleGround())
        return;

    /** error handling **/
    /********************/

    // everything is fine, do it
    SendPartyResult(PARTY_OP_LEAVE, GetPlayer()->GetName(), ERR_PARTY_RESULT_OK);

    GetPlayer()->GetGroup()->BroadcastGroupUpdate();
    GetPlayer()->RemoveFromGroup();
}

void WorldSession::HandleSetLootMethod(SetLootMethod const& packet)
{
    // Impossible.
    if (packet.Method > NEED_BEFORE_GREED || packet.Threshold >= MAX_ITEM_QUALITY)
        return;

    Group* group = GetPlayer()->GetGroup();
    if (!group || group->isBGGroup())
        group = GetPlayer()->GetOriginalGroup();

    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetObjectGuid()))
        return;

    // cannot have loot master in other loot methods
    ObjectGuid lootMaster;
    if (packet.Method == MASTER_LOOT)
    {
        lootMaster = ResolveGuid(packet.LootMaster);
        if (!group->IsMember(lootMaster))
            return;
    }

    // everything is fine, do it
    group->SetLootMethod(LootMethod(packet.Method));
    group->SetLooterGuid(lootMaster);
    group->SetLootThreshold(ItemQualities(packet.Threshold));
    group->SendUpdate();
}

void WorldSession::HandleLootRoll(LootRoll const& request)
{
    auto const lootedTarget = ClassicGuidMapping::FromClassic(request.LootObj);
    if (!lootedTarget || request.LootListId == 0)
        return;
    uint32 const itemSlot = request.LootListId - 1;
    uint8 rollType = request.RollType;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (rollType >= MAX_ROLL_FROM_CLIENT)
        return;

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_PLAY_TIME))
    {
        SendPlayTimeWarning(PTF_UNHEALTHY_TIME, 0);
        rollType = ROLL_PASS;
    }

    // everything is fine, do it, if false then some cheating problem found (result not used in pre-3.0)
    group->CountRollVote(GetPlayer(), *lootedTarget, itemSlot, RollVote(rollType));
}

void WorldSession::HandleMinimapPing(MinimapPingClient const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    MinimapPing ping;
    ping.Sender = ToClassic(GetPlayer()->GetObjectGuid());
    ping.PositionX = packet.PositionX;
    ping.PositionY = packet.PositionY;
    WorldPacket data = MakeClassicPacket(ping);
    group->BroadcastPacket(&data, true, -1, GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleRandomRoll(RandomRollClient const& packet)
{
    /** error handling **/
    if (packet.Min < 0 || packet.Min > packet.Max || packet.Max > 1000000)
        return;
    /********************/

    // everything is fine, do it
    RandomRoll roll;
    roll.Roller = ToClassic(GetPlayer()->GetObjectGuid());
    roll.RollerWowAccount = ClassicGuidMapping::MakeAccount(GetAccountId());
    roll.Min = packet.Min;
    roll.Max = packet.Max;
    roll.Result = int32(urand(uint32(packet.Min), uint32(packet.Max)));
    WorldPacket data = MakeClassicPacket(roll);

    // /random goes to the party or raid wherever they are, and to the player alone otherwise
    if (GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->BroadcastPacket(&data, false);
    else
        SendPacket(&data);
}

void WorldSession::HandleUpdateRaidTarget(UpdateRaidTarget const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        group = GetPlayer()->GetOriginalGroup();
    if (!group)
        return;

    /** error handling **/
    /********************/

    // everything is fine, do it
    if (packet.Symbol == -1) // target icon request
        group->SendTargetIconList(this);
    else // target icon update
    {
        if (packet.Symbol < 0 || packet.Symbol >= TARGET_ICON_COUNT)
            return;

        if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
            !group->IsAssistant(GetPlayer()->GetObjectGuid()))
            return;

        group->SetTargetIcon(uint8(packet.Symbol), ResolveGuid(packet.Target), GetPlayer()->GetObjectGuid());
    }
}

void WorldSession::HandleRequestPartyJoinUpdates(RequestPartyJoinUpdates const& /*packet*/)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    group->SendTargetIconList(this);
}

void WorldSession::HandleConvertRaid(ConvertRaid const& packet)
{
    // a raid cannot be turned back into a party
    if (!packet.Raid)
        return;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (_player->InBattleGround())
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) || group->GetMembersCount() < group->GetMembersMinCount())
        return;
    /********************/

    // everything is fine, do it (is it 0 (PARTY_OP_INVITE) correct code)
    SendPartyResult(PARTY_OP_INVITE, "", ERR_PARTY_RESULT_OK);
    group->ConvertToRaid();
}

void WorldSession::HandleChangeSubGroup(ChangeSubGroup const& packet)
{
    if (packet.NewSubGroup >= MAX_RAID_SUBGROUPS)
        return;

    // we will get correct pointer for group here, so we don't have to check if group is BG raid
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
        !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;

    if (!group->HasFreeSlotSubGroup(packet.NewSubGroup))
        return;
    /********************/

    // everything is fine, do it
    ObjectGuid const guid = ResolveGuid(packet.Target);
    if (Player* player = sObjectMgr.GetPlayer(guid))
        group->ChangeMembersGroup(player, packet.NewSubGroup);
    else if (guid)
        group->ChangeMembersGroup(guid, packet.NewSubGroup);
}

void WorldSession::HandleSwapSubGroups(SwapSubGroups const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
        !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    ObjectGuid const first = ResolveGuid(packet.FirstTarget);
    ObjectGuid const second = ResolveGuid(packet.SecondTarget);
    if (!first || !second)
        return;

    // If both players are online do swap with Player objects, else
    // do swap with Guids
    Player* player = sObjectMgr.GetPlayer(first);
    Player* swapPlayer = sObjectMgr.GetPlayer(second);

    if (player && swapPlayer)
        group->SwapMembersGroup(player, swapPlayer);
    else
        group->SwapMembersGroup(first, second);
}

void WorldSession::HandleSetAssistantLeader(SetAssistantLeader const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    Player* player = sObjectMgr.GetPlayer(ResolveGuid(packet.Target));

    /** error handling **/
    if (!player || player == GetPlayer() || !group->IsLeader(GetPlayer()->GetObjectGuid()) || player->GetGroup() != group)
        return;

    // everything is fine, do it
    group->SetAssistant(player->GetObjectGuid(), packet.Apply);
}

void WorldSession::HandleSetPartyAssignment(SetPartyAssignment const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
        !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // taking a mark back only counts for whoever holds it
    ObjectGuid const target = ResolveGuid(packet.Target);
    switch (packet.Assignment)
    {
        case CLASSIC_PARTY_ASSIGN_MAIN_TANK:
            if (packet.Set)
                group->SetMainTank(target);
            else if (group->GetMainTankGuid() == target)
                group->SetMainTank(ObjectGuid());
            break;
        case CLASSIC_PARTY_ASSIGN_MAIN_ASSIST:
            if (packet.Set)
                group->SetMainAssistant(target);
            else if (group->GetMainAssistantGuid() == target)
                group->SetMainAssistant(ObjectGuid());
            break;
        default:
            break;
    }
}

void WorldSession::HandleDoReadyCheck(DoReadyCheck const& /*packet*/)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetObjectGuid()) &&
        !group->IsAssistant(GetPlayer()->GetObjectGuid()))
        return;
    /********************/

    // everything is fine, do it
    group->StartReadyCheck(GetPlayer()->GetObjectGuid());
}

void WorldSession::HandleReadyCheckResponse(ReadyCheckResponseClient const& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    group->SetMemberReadyCheck(GetPlayer()->GetObjectGuid(), packet.IsReady);
}

/*this procedure handles clients CMSG_REQUEST_PARTY_MEMBER_STATS request*/
void WorldSession::HandleRequestPartyMemberStats(RequestPartyMemberStats const& packet)
{
    ObjectGuid const guid = ResolveGuid(packet.Target);
    Player* player = HashMapHolder<Player>::Find(guid);

    PartyMemberFullState state;
    if (!player || !player->IsInSameRaidWith(_player))
    {
        state.MemberGuid = packet.Target;
        state.Status = MEMBER_STATUS_OFFLINE;
    }
    else
        FillPartyMemberFullState(player, state);

    SendClassic(state);
}

void WorldSession::HandleRequestRaidInfoOpcode(NullClientPacket const& /*packet*/)
{
    // every time the player checks the character screen
    _player->SendRaidInfo();
}
