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
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Object.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGround.h"
#include "Language.h"
#include "ScriptMgr.h"
#include "World.h"
#include "Anticheat.h"
#include "Packets/Classic/GuidMapping.h"

void WorldSession::HandleBattlemasterHelloOpcode(BattlemasterHello const& packet)
{
    nonstd::optional<ObjectGuid> guid = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guid)
        return;

    Creature* pCreature = GetPlayer()->GetMap()->GetCreature(*guid);

    if (!pCreature)
        return;

    if (!pCreature->IsBattleMaster())                       // it's not battlemaster
        return;

    // Stop the npc if moving
    if (!pCreature->HasExtraFlag(CREATURE_FLAG_EXTRA_NO_MOVEMENT_PAUSE))
        pCreature->PauseOutOfCombatMovement();

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(pCreature->GetEntry());

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
        return;

    if (!_player->GetBGAccessByLevel(bgTypeId))
    {
        // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    _player->InterruptSpellsWithChannelFlags(AURA_INTERRUPT_INTERACTING_CANCELS);
    _player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_INTERACTING_CANCELS);
    SendBattleGroundList(*guid, bgTypeId);
}

void WorldSession::SendBattleGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId)
{
    SendClassic(sBattleGroundMgr.BuildBattleGroundListPacket(guid, _player, bgTypeId));
}

// QueueId carries a packed BattlegroundQueueTypeId (see BattlemasterJoin's comment in
// Packets/BattlegroundMessages.h) instead of the old raw mapId. Its low 16 bits are the
// BattlemasterListId, numerically identical to BattleGroundTypeId for this server.
void WorldSession::HandleBattlemasterJoinOpcode(BattlemasterJoin const& packet)
{
    BattleGroundTypeId bgTypeId = static_cast<BattleGroundTypeId>(packet.QueueId & 0xFFFF);
    nonstd::optional<ObjectGuid> battlemasterGuid = ClassicGuidMapping::FromClassic(packet.BattlemasterGuid);
    RequestBgJoinQueue(battlemasterGuid.value_or(ObjectGuid()), uint32(packet.BattlefieldInstanceId), bgTypeId, packet.AsGroup);
}

void WorldSession::RequestBgJoinQueue(ObjectGuid battlemaster, uint32 instanceId, BattleGroundTypeId bgTypeId, bool joinAsGroup)
{
    bool queuedAtBGPortal = false;
    bool isPremade = false;
    Group* grp;

    if (battlemaster == GetPlayer()->GetObjectGuid())
        queuedAtBGPortal = true;

    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to queue for invalid BG type", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS);
        return;
    }
    if (bgTypeId == BATTLEGROUND_AV && joinAsGroup)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to queue for AV as group", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS);
        return;
    }

    if (queuedAtBGPortal)
    {
        auto const& bgQueuePos = _player->GetBattleGroundEntryPoint();
        if (_player->GetMapId() != bgQueuePos.mapId || !_player->IsWithinDist3d(bgQueuePos, 50.0f))
        {
            ProcessAnticheatAction("PassiveAnticheat", "Attempt to queue for BG through out of range portal", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS);
            return;
        }
    }
    else
    {
        if (!_player->GetNPCIfCanInteractWith(battlemaster, UNIT_NPC_FLAG_BATTLEMASTER))
        {
            ProcessAnticheatAction("PassiveAnticheat", "Attempt to queue for BG through invalid creature", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS);
            return;
        }
    }

    // can do this, since it's battleground, not arena
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BgQueueTypeId(bgTypeId);

    // ignore if player is already in BG
    if (_player->InBattleGround())
    {
        if (!queuedAtBGPortal)
            _player->PlayerTalkClass->CloseGossip();
        return;
    }

    // get bg instance or bg template if instance not found
    BattleGround *bg = nullptr;
    if (instanceId)
        bg = sBattleGroundMgr.GetBattleGroundThroughClientInstance(instanceId, bgTypeId);

    if (!bg && !(bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId)))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground: no available bg / template found");
        return;
    }

    BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel(bgTypeId);

    if (bgBracketId == BG_BRACKET_ID_NONE)
    {
        if (!queuedAtBGPortal)
            _player->PlayerTalkClass->CloseGossip();
        return;
    }

    // check queue conditions
    if (!joinAsGroup)
    {
        // check Deserter debuff
        if (!_player->CanJoinToBattleground())
        {
            _player->GetSession()->SendClassic(sBattleGroundMgr.BuildBattlefieldStatusFailed(_player, bgTypeId, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS));
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }
        // check if already in queue -- re-echo the current status (matches what the client already
        // has) instead of leaving the interaction hanging with zero response, which would give the
        // client nothing telling it the interaction concluded, so it would refuse to let the NPC be
        // re-interacted with afterward.
        uint32 existingSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
        if (existingSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            BattleGroundQueue& existingQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];
            uint32 avgTime = 0;
            GroupQueueInfo groupInfo;
            if (existingQueue.GetPlayerGroupInfoData(_player->GetObjectGuid(), &groupInfo))
                avgTime = existingQueue.GetAverageQueueWaitTime(&groupInfo, bgBracketId);
            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, existingSlot, STATUS_WAIT_QUEUE, avgTime, 0);
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }
        // check if has free queue slots
        if (!_player->HasFreeBattleGroundQueueId())
        {
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }

        // if we're here, then the conditions to join a bg are met. We can proceed in joining.
        // _player->GetGroup() was already checked, grp is already initialized
        BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];

        GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, nullptr, bgTypeId, bgBracketId, isPremade, instanceId, nullptr);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bgBracketId);
        // already checked if queueSlot is valid, now just get it
        PlayerQueueInfo& selfQueueInfo = bgQueue.m_queuedPlayers[_player->GetObjectGuid()];
        uint32 queueSlot = _player->AddBattleGroundQueueId(bgQueueTypeId, selfQueueInfo.ticketId, selfQueueInfo.ticketTime);
        // store entry point coords
        _player->SetBattleGroundEntryPoint(_player, queuedAtBGPortal);

        // send status packet (in queue)
        sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, _player->GetGUIDLow(), _player->GetName());
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if (!grp)
        {
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }

        std::vector<uint32> excludedMembers;
        uint32 err = grp->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), _player, &excludedMembers);
        isPremade = sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH) &&
                    (grp->GetMembersCount() >= sWorld.getConfig(CONFIG_UINT32_BATTLEGROUND_PREMADE_QUEUE_GROUP_MIN_SIZE));

        if (err == BG_JOIN_ERR_GROUP_DESERTER)
        {
            _player->GetSession()->SendClassic(sBattleGroundMgr.BuildBattlefieldStatusFailed(_player, bgTypeId, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS));
            SendBattleGroundJoinError(err);
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }
        else if (err != BG_JOIN_ERR_OK)
        {
            SendBattleGroundJoinError(err);
            if (!queuedAtBGPortal)
                _player->PlayerTalkClass->CloseGossip();
            return;
        }

        // if we're here, then the conditions to join a bg are met. We can proceed in joining.

        // _player->GetGroup() was already checked, grp is already initialized
        BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: the following players are joining as group:");
        GroupQueueInfo* ginfo = bgQueue.AddGroup(_player, grp, bgTypeId, bgBracketId, isPremade, instanceId, &excludedMembers);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bgBracketId);
        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member) continue;  // this should never happen

            if (std::find(excludedMembers.begin(), excludedMembers.end(), member->GetGUIDLow()) != excludedMembers.end())
            {
                member->GetSession()->SendClassic(sBattleGroundMgr.BuildBattlefieldStatusFailed(member, bgTypeId, ERR_BATTLEGROUND_JOIN_FAILED));
                SendBattleGroundJoinError(err);
                continue;
            }

            PlayerQueueInfo& memberQueueInfo = bgQueue.m_queuedPlayers[member->GetObjectGuid()];
            uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId, memberQueueInfo.ticketId, memberQueueInfo.ticketTime); // add to queue
            member->SetBattleGroundEntryPoint(_player, queuedAtBGPortal);               // store entry point coords

            // send status packet (in queue) -- this alone covers the "you joined the queue"
            // notification the old SMSG_GROUP_JOINED_BATTLEGROUND success case used to send
            // separately; that packet has no modern equivalent.
            sBattleGroundMgr.SendBattleGroundStatusPacket(member, bg, queueSlot, STATUS_WAIT_QUEUE, avgTime, 0);
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: player joined queue for bg queue type %u bg type %u: GUID %u, NAME %s", bgQueueTypeId, bgTypeId, member->GetGUIDLow(), member->GetName());
        }
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: group end");
    }

    // Close the battlemaster's interaction window now that the queue attempt actually concluded
    // (matches e.g. accepting a quest closing the gossip window). Only for players who were
    // actually talking to an NPC -- queuing from a portal (queuedAtBGPortal) never opened any
    // interaction window to begin with.
    if (!queuedAtBGPortal)
        _player->PlayerTalkClass->CloseGossip();

    sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, bgBracketId);
}

// The modern request opcode for the scoreboard; responds with SMSG_PVP_LOG_DATA instead of the old
// bidirectional MSG_PVP_LOG_DATA.
void WorldSession::HandlePVPLogDataOpcode(PVPLogDataRequest const& /*packet*/)
{
    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    if (bg->GetStatus() != STATUS_WAIT_LEAVE)
        SendClassic(sBattleGroundMgr.BuildPvpLogDataPacket(bg));
    else
        SendPacket(bg->GetFinalScorePacket());
}

void WorldSession::HandleBattlefieldListOpcode(BattlefieldListRequest const& packet)
{
    // ListId is the BattlemasterListId, numerically == BattleGroundTypeId here (see
    // BattlefieldListRequest's comment, Packets/BattlegroundMessages.h).
    BattleGroundTypeId bgTypeId = static_cast<BattleGroundTypeId>(packet.ListId);

    if (bgTypeId == BATTLEGROUND_TYPE_NONE || bgTypeId >= MAX_BATTLEGROUND_TYPE_ID)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground: invalid bgtype received.");
        return;
    }

    // Empty guid, not the player's own -- this is the "queue from anywhere" request path (no
    // battlemaster involved), which is also how BuildBattleGroundListPacket decides PvpAnywhere.
    SendClassic(sBattleGroundMgr.BuildBattleGroundListPacket(ObjectGuid(), _player, bgTypeId));
}

// The ticket the client echoes back identifies which queued battleground this refers to (matched
// against the ticket id stashed on the player's queue slot), replacing the old mapId lookup.
void WorldSession::HandleBattleFieldPortOpcode(BattleFieldPort const& packet)
{
    uint8 action = packet.AcceptedInvite ? 1 : 0; // enter battle 0x1, leave queue 0x0
    if (!_player->InBattleGroundQueue())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "BattlegroundHandler: Invalid CLASSIC_CMSG_BATTLEFIELD_PORT received from player (%u), he is not in bg_queue.", _player->GetGUIDLow());
        return;
    }

    // Match against the ticket id stashed on the player itself rather than
    // BattleGroundQueue::m_queuedPlayers -- that map only holds an entry while still literally
    // queued, so a retried CLASSIC_CMSG_BATTLEFIELD_PORT sent after being invited/ported (exactly
    // when the client resends unanswered port requests) would otherwise miss here and get silently
    // dropped with no response, so the client would just keep retrying forever.
    BattleGroundQueueTypeId bgQueueTypeId = BATTLEGROUND_QUEUE_NONE;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattleGroundQueueTypeId candidate = _player->GetBattleGroundQueueTypeId(i);
        if (candidate == BATTLEGROUND_QUEUE_NONE)
            continue;

        if (_player->GetBattleGroundQueueTicketId(candidate) == packet.Ticket.Id)
        {
            bgQueueTypeId = candidate;
            break;
        }
    }

    if (bgQueueTypeId == BATTLEGROUND_QUEUE_NONE)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "BattlegroundHandler: CLASSIC_CMSG_BATTLEFIELD_PORT ticket doesn't match any of player (%u)'s queue slots.", _player->GetGUIDLow());
        // Echo a bare NONE back for this exact ticket so the client can retire it instead of
        // retrying forever -- can happen for a ticket left over from before a relog.
        BattlefieldStatusNone none;
        none.Ticket = packet.Ticket;
        SendClassic(none);
        return;
    }

    BattleGroundTypeId bgTypeId = BattleGroundMgr::BgTemplateId(bgQueueTypeId);

    //get GroupQueueInfo from BattleGroundQueue
    BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];
    //we must use temporary variable, because GroupQueueInfo pointer can be deleted in BattleGroundQueue::RemovePlayer() function
    GroupQueueInfo ginfo;
    if (!bgQueue.GetPlayerGroupInfoData(_player->GetObjectGuid(), &ginfo))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "BattlegroundHandler: itrplayerstatus not found.");
        return;
    }
    // if action == 1, then instanceId is required
    if (!ginfo.isInvitedToBgInstanceGuid && action == 1)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "BattlegroundHandler: instance not found.");
        return;
    }

    BattleGround *bg = sBattleGroundMgr.GetBattleGround(ginfo.isInvitedToBgInstanceGuid, bgTypeId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if (!bg && action == 0)
        bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);

    if (!bg)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "BattlegroundHandler: bgTemplate not found for type id %u.", bgTypeId);
        return;
    }

    //some checks if player isn't cheating - it is not exactly cheating, but we cannot allow it
    if (action == 1)
    {
        //if player is trying to enter battleground and he has deserter debuff, we must just remove him from queue
        if (!_player->CanJoinToBattleground())
        {
            //send bg command result to show nice message
            _player->GetSession()->SendClassic(sBattleGroundMgr.BuildBattlefieldStatusFailed(_player, bgTypeId, ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS));
            action = 0;
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: player %s (%u) has a deserter debuff, do not port him to battleground!", _player->GetName(), _player->GetGUIDLow());
        }

        //if player don't match battleground max level, then do not allow him to enter! (this might happen when player leveled up during his waiting in queue
        if (_player->GetLevel() > bg->GetMaxLevel())
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground: Player %s (%u) has level (%u) higher than maxlevel (%u) of battleground (%u)! Do not port him to battleground!",
                          _player->GetName(), _player->GetGUIDLow(), _player->GetLevel(), bg->GetMaxLevel(), bg->GetTypeID());
            action = 0;
        }

        // do not allow entering bg which is already over
        if (bg->GetStatus() == STATUS_WAIT_LEAVE)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground: Player %s (%u) is attempting to enter battleground (%u) which has already ended!",
                _player->GetName(), _player->GetGUIDLow(), bg->GetTypeID());
            action = 0;
        }
    }

    uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
    switch (action)
    {
        case 1:                                         // port to battleground
            if (!_player->IsInvitedForBattleGroundQueueType(bgQueueTypeId))
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground: Player %s (%u) is not invited in queue type %u !", _player->GetName(), _player->GetGUIDLow(), bgQueueTypeId);
                return;                                 // cheating?
            }

            // resurrect the player
            if (!_player->IsAlive())
            {
                _player->ResurrectPlayer(1.0f);
                _player->SpawnCorpseBones();
            }
            // stop taxi flight at port
            if (_player->IsTaxiFlying())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->m_taxi.ClearTaxiDestinations();
            }

            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
            // remove battleground queue status from BGmgr
            bgQueue.RemovePlayer(_player->GetObjectGuid(), false);
            // this is still needed here if battleground "jumping" shouldn't add deserter debuff
            // also this is required to prevent stuck at old battleground after SetBattleGroundId set to new
            if (BattleGround *currentBg = _player->GetBattleGround())
                currentBg->RemovePlayerAtLeave(_player->GetObjectGuid(), false, true);

            // set the destination instance id
            _player->SetBattleGroundId(bg->GetInstanceID(), bgTypeId);
            // set the destination team
            _player->SetBGTeam(ginfo.groupTeam);
            // bg->HandleBeforeTeleportToBattleGround(_player);
            sBattleGroundMgr.SendToBattleGround(_player, ginfo.isInvitedToBgInstanceGuid, bgTypeId);
            // add only in HandleMoveWorldPortAck()
            // bg->AddPlayer(_player,team);
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: player %s (%u) joined battle for bg %u, bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetInstanceID(), bg->GetTypeID(), bgQueueTypeId);
            sLog.Out(LOG_BG, LOG_LVL_DETAIL, "[%u,%u]: %s:%u [%u:%s] enters",
                     bg->GetMapId(), bg->GetInstanceID(),
                     _player->GetName(),
                     _player->GetGUIDLow(),
                     GetAccountId(), GetRemoteAddress().c_str());
            break;
        case 0:                                         // leave queue
            _player->RemoveBattleGroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
            // Build+send the status packet BEFORE bgQueue.RemovePlayer(): the RideTicket it echoes
            // back is looked up from bgQueue.m_queuedPlayers, which RemovePlayer erases. Sending
            // after removal would mint a fresh, unrelated ticket id the client can't match to the
            // queue slot it's tracking, so it would never clear the "in queue" icon even though the
            // server-side state is correct.
            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, queueSlot, STATUS_NONE, 0, 0);
            bgQueue.RemovePlayer(_player->GetObjectGuid(), true);
            // player left queue, we should update it
            sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Battleground: player %s (%u) left queue for bgtype %u, queue type %u.", _player->GetName(), _player->GetGUIDLow(), bg->GetTypeID(), bgQueueTypeId);
            sLog.Out(LOG_BG, LOG_LVL_DETAIL, "[%u,%u] %s:%u [%u:%s] leaves queue, bgTypeId: %u",
                     bg->GetMapId(), bg->GetInstanceID(),
                     _player->GetName(),
                     _player->GetGUIDLow(),
                     GetAccountId(), GetRemoteAddress().c_str(),
                     bgTypeId);
            break;
        default:
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Battleground port: unknown action %u", action);
            break;
    }
}

void WorldSession::HandleLeaveBattlefieldOpcode(LeaveBattlefield const& /*packet*/)
{
    // The modern client sends no mapId (see LeaveBattlefield's comment, Packets/BattlegroundMessages.h)
    // -- trust server-side battleground/map state instead.

    // not allow leave battleground in combat
    if (_player->IsInCombat())
        if (BattleGround* bg = _player->GetBattleGround())
            if (bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

    _player->LeaveBattleground();
}

void WorldSession::HandleRequestBattlefieldStatusOpcode(RequestBattlefieldStatus const& /*packet*/)
{
    // empty opcode
    // we must update all queues here
    BattleGround *bg = nullptr;
    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;

        BattleGroundTypeId bgTypeId = BattleGroundMgr::BgTemplateId(bgQueueTypeId);
        if (bgTypeId == _player->GetBattleGroundTypeId())
        {
            bg = _player->GetBattleGround();
            //so i must use bg pointer to get that information
            if (bg)
            {
                // this line is checked, i only don't know if GetStartTime is changing itself after bg end!
                // send status in BattleGround
                sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, i, STATUS_IN_PROGRESS, bg->GetEndTime(), bg->GetStartTime());
                continue;
            }
        }
        //we are sending update to player about queue - he can be invited there!
        //get GroupQueueInfo for queue status
        BattleGroundQueue& bgQueue = sBattleGroundMgr.m_battleGroundQueues[bgQueueTypeId];
        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(_player->GetObjectGuid(), &ginfo))
            continue;
        if (ginfo.isInvitedToBgInstanceGuid)
        {
            bg = sBattleGroundMgr.GetBattleGround(ginfo.isInvitedToBgInstanceGuid, bgTypeId);
            if (!bg)
                continue;
            uint32 remainingTime = WorldTimer::getMSTimeDiff(WorldTimer::getMSTime(), ginfo.removeInviteTime);
            // send status invited to BattleGround
            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, i, STATUS_WAIT_JOIN, remainingTime, 0);
        }
        else
        {
            bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            if (!bg)
                continue;
            uint32 avgTime = bgQueue.GetAverageQueueWaitTime(&ginfo, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
            // send status in BattleGround Queue
            sBattleGroundMgr.SendBattleGroundStatusPacket(_player, bg, i, STATUS_WAIT_QUEUE, avgTime, WorldTimer::getMSTimeDiff(ginfo.joinTime, WorldTimer::getMSTime()));
        }
    }
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode(AreaSpiritHealerQuery const& packet)
{
    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    nonstd::optional<ObjectGuid> guid = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guid)
        return;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(*guid);
    if (!unit)
        return;

    if (!unit->IsSpiritService())                           // it's not spirit service
        return;

    unit->SendAreaSpiritHealerQueryOpcode(GetPlayer());
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(AreaSpiritHealerQueue const& packet)
{
    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    nonstd::optional<ObjectGuid> guid = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guid)
        return;

    Creature* unit = GetPlayer()->GetMap()->GetCreature(*guid);
    if (!unit)
        return;

    if (!unit->IsSpiritService())                           // it's not spirit service
        return;

    sScriptMgr.OnGossipHello(GetPlayer(), unit);
}

// Purely a UI feature-gate query, irrelevant to gameplay. All fields hardcoded per
// PVPOptionsEnabled's defaults (Packets/BattlegroundMessages.h) since Classic has none of these
// systems except pug battlegrounds.
void WorldSession::HandleGetPVPOptionsEnabledOpcode(GetPVPOptionsEnabled const& /*packet*/)
{
    SendClassic(PVPOptionsEnabled());
}

// This engine has no rated PvP/arena system, so every bracket is sent all-zero (RatedPvpInfo's
// default field values).
void WorldSession::HandleRequestRatedPvpInfoOpcode(RequestRatedPvpInfo const& /*packet*/)
{
    SendClassic(RatedPvpInfo());
}

// This engine has no AFK-report/vote-kick tracking system, so honestly report the system itself as
// disabled rather than fabricate a success/failure the server can't back up.
void WorldSession::HandleReportPvPAFKOpcode(ReportPvPPlayerAFK const& packet)
{
    ReportPvPPlayerAFKResult response;
    response.Offender = packet.Offender;
    SendClassic(response);
}

void WorldSession::SendBattleGroundJoinError(uint8 err)
{
    WorldPacket data;
    int32 msg;
    switch (err)
    {
        case BG_JOIN_ERR_OFFLINE_MEMBER:
            msg = LANG_BG_GROUP_OFFLINE_MEMBER;
            break;
        case BG_JOIN_ERR_GROUP_TOO_MANY:
            msg = LANG_BG_GROUP_TOO_LARGE;
            break;
        case BG_JOIN_ERR_MIXED_FACTION:
            msg = LANG_BG_GROUP_MIXED_FACTION;
            break;
        case BG_JOIN_ERR_MIXED_LEVELS:
            msg = LANG_BG_GROUP_MIXED_LEVELS;
            break;
        case BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE:
            msg = LANG_BG_GROUP_MEMBER_ALREADY_IN_QUEUE;
            break;
        case BG_JOIN_ERR_GROUP_DESERTER:
            msg = LANG_BG_GROUP_MEMBER_DESERTER;
            break;
        case BG_JOIN_ERR_ALL_QUEUES_USED:
            msg = LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS;
            break;
        case BG_JOIN_ERR_GROUP_NOT_ENOUGH:
        //case BG_JOIN_ERR_MIXED_ARENATEAM:
        default:
            return;
            break;
    }
    ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, GetMangosString(msg), LANG_UNIVERSAL);
    SendPacket(&data);
}
