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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "ScriptMgr.h"
#include "Group.h"
#include "Packets/Classic/GuidMapping.h"

uint8 WorldSession::GetQuestGiverStatus(Object* questgiver)
{
    uint8 dialogStatus = DIALOG_STATUS_NONE;

    switch (questgiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            Creature* cr_questgiver = (Creature*)questgiver;

            if (!cr_questgiver->IsHostileTo(_player))       // not show quest status to enemies
            {
                dialogStatus = sScriptMgr.GetDialogStatus(_player, cr_questgiver);

                if (dialogStatus > 6)
                    dialogStatus = GetDialogStatus(_player, cr_questgiver, DIALOG_STATUS_NONE);
            }
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            GameObject* go_questgiver = (GameObject*)questgiver;
            dialogStatus = sScriptMgr.GetDialogStatus(_player, go_questgiver);
            if (dialogStatus > 6)
                dialogStatus = GetDialogStatus(_player, go_questgiver, DIALOG_STATUS_NONE);
            break;
        }
        default:
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "QuestGiver called for unexpected type %u", questgiver->GetTypeId());
            break;
    }

    return dialogStatus;
}

void WorldSession::HandleQuestgiverStatusQueryOpcode(QuestgiverStatusQuery const& packet)
{
    auto const guid = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guid)
        return;

    Object* questgiver = _player->GetObjectByTypeMask(*guid, TYPEMASK_CREATURE_OR_GAMEOBJECT);
    if (!questgiver)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "Error in CLASSIC_CMSG_QUEST_GIVER_STATUS_QUERY, called for not found questgiver %s", guid->GetString().c_str());
        return;
    }

    //inform client about status of quest
    _player->PlayerTalkClass->SendQuestGiverStatus(GetQuestGiverStatus(questgiver), *guid);
}

void WorldSession::HandleQuestgiverStatusMultipleQuery(QuestGiverStatusMultipleQuery const& /*packet*/)
{
    std::vector<ObjectGuid> visible;
    {
        std::shared_lock<std::shared_timed_mutex> lock(_player->m_visibleGUIDs_lock);
        for (ObjectGuid const& guid : _player->m_visibleGUIDs)
            if (guid.IsCreature() || guid.IsGameObject())
                visible.push_back(guid);
    }

    std::vector<std::pair<ObjectGuid, uint8>> statuses;
    for (ObjectGuid const& guid : visible)
    {
        Object* questgiver = _player->GetObjectByTypeMask(guid, TYPEMASK_CREATURE_OR_GAMEOBJECT);
        if (!questgiver)
            continue;

        uint8 const status = GetQuestGiverStatus(questgiver);
        if (status != DIALOG_STATUS_NONE)
            statuses.emplace_back(guid, status);
    }

    _player->PlayerTalkClass->SendQuestGiverStatusMultiple(statuses);
}

void WorldSession::HandleQuestgiverHelloOpcode(QuestgiverHello const& packet)
{
    auto const guid = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guid)
        return;

    Creature* pCreature = GetPlayer()->GetNPCIfCanInteractWith(*guid, UNIT_NPC_FLAG_NONE);

    if (!pCreature)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandleQuestgiverHelloOpcode - %s not found or you can't interact with him.", guid->GetString().c_str());
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // Stop the npc if moving
    if (!pCreature->HasExtraFlag(CREATURE_FLAG_EXTRA_NO_MOVEMENT_PAUSE))
        pCreature->PauseOutOfCombatMovement();

    GetPlayer()->InterruptSpellsWithChannelFlags(AURA_INTERRUPT_INTERACTING_CANCELS);
    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_INTERACTING_CANCELS);

    if (sScriptMgr.OnGossipHello(_player, pCreature))
        return;

    _player->PrepareGossipMenu(pCreature, pCreature->GetDefaultGossipMenuId());
    _player->SendPreparedGossip(pCreature);
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode(QuestgiverAcceptQuest const& packet)
{
    auto const guidOpt = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guidOpt)
        return;
    ObjectGuid const guid = *guidOpt;

    Object* pObject = _player->GetObjectByTypeMask(guid, TYPEMASK_CREATURE_GAMEOBJECT_PLAYER_OR_ITEM);

    // no or incorrect quest giver
    if (!pObject
            || (pObject->GetTypeId() != TYPEID_PLAYER && !pObject->HasQuest(packet.QuestId))
            || (pObject->GetTypeId() == TYPEID_PLAYER && !((Player*)pObject)->CanShareQuest(packet.QuestId))
       )
    {
        _player->PlayerTalkClass->CloseGossip();
        _player->ClearQuestShareInfo();
        return;
    }

    if (!_player->CanInteractWithQuestGiver(pObject))
    {
        _player->PlayerTalkClass->CloseGossip();
        _player->ClearQuestShareInfo();
        return;
    }

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(packet.QuestId);
    if (qInfo)
    {
        // prevent cheating
        if (!GetPlayer()->CanTakeQuest(qInfo, true))
        {
            _player->PlayerTalkClass->CloseGossip();
            _player->ClearQuestShareInfo();
            return;
        }

        if (auto const& questShareInfo = _player->GetQuestShareInfo())
        {
            if (questShareInfo->QuestId == qInfo->GetQuestId())
            {
                Player* sharer = _player->GetMap()->GetPlayer(questShareInfo->PlayerGuid);
                if (!sharer)
                {
                    _player->PlayerTalkClass->CloseGossip();
                    _player->ClearQuestShareInfo();
                    return;
                }

                if (!_player->IsWithinDist(sharer, QUEST_SHARE_DISTANCE, true, SizeFactor::None))
                {
                    _player->PlayerTalkClass->CloseGossip();
                    _player->ClearQuestShareInfo();
                    sharer->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_TOO_FAR, qInfo);
                    return;
                }

                sharer->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_ACCEPT_QUEST, qInfo);
                _player->ClearQuestShareInfo();
            }
            else
            {
                _player->ClearQuestShareInfo();
            }
        }

        if (_player->CanAddQuest(qInfo, true))
        {
            _player->AddQuest(qInfo, pObject);              // pObject (if it item) can be destroyed at call

            if (qInfo->HasQuestFlag(QUEST_FLAGS_PARTY_ACCEPT))
            {
                if (Group* pGroup = _player->GetGroup())
                {
                    for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                    {
                        Player* pPlayer = itr->getSource();

                        if (!pPlayer || pPlayer == _player || !pPlayer->IsInMap(_player))     // not self and in same map
                            continue;

                        if (pPlayer->CanTakeQuest(qInfo, true))
                        {
                            pPlayer->SetQuestShareInfo(_player->GetObjectGuid(), qInfo->GetQuestId());

                            //need confirmation that any gossip window will close
                            pPlayer->PlayerTalkClass->CloseGossip();

                            _player->SendQuestConfirmAccept(qInfo, pPlayer);
                        }
                    }
                }
            }

            if (_player->CanCompleteQuest(packet.QuestId))
                _player->CompleteQuest(packet.QuestId);

            // Push the updated quest-giver status so the client's map marker (!/?) refreshes
            // immediately instead of staying stale until the next status query. Only for real
            // creature/gameobject givers -- GetDialogStatus has no player/item branch (the quest
            // share path closes the window anyway).
            if (pObject->GetTypeId() == TYPEID_UNIT || pObject->GetTypeId() == TYPEID_GAMEOBJECT)
            {
                uint32 const newStatus = GetDialogStatus(_player, pObject, DIALOG_STATUS_NONE);
                _player->PlayerTalkClass->SendQuestGiverStatus(uint8(newStatus), guid);
            }

            _player->PlayerTalkClass->CloseGossip();

            if (qInfo->GetSrcSpell() > 0)
                _player->CastSpell(_player, qInfo->GetSrcSpell(), true);

            return;
        }
    }

    _player->PlayerTalkClass->CloseGossip();
    _player->ClearQuestShareInfo();
}

void WorldSession::HandleQueryQuestInfoOpcode(QueryQuestInfo const& packet)
{
    _player->PlayerTalkClass->SendQuestInfo(uint32(packet.QuestId));
}

void WorldSession::HandleQuestgiverQueryQuestOpcode(QuestgiverQueryQuest const& packet)
{
    auto const guidOpt = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guidOpt)
        return;

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* pObject = _player->GetObjectByTypeMask(*guidOpt, TYPEMASK_CREATURE_GAMEOBJECT_OR_ITEM);
    if (!pObject || (!pObject->HasQuest(packet.QuestId) && !pObject->HasInvolvedQuest(packet.QuestId)))
    {
        _player->PlayerTalkClass->CloseGossip();
        return;
    }

    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
        _player->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, pObject->GetObjectGuid(), true);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(QuestgiverChooseReward const& packet)
{
    auto const guidOpt = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guidOpt)
        return;
    ObjectGuid const guid = *guidOpt;

    // The modern client identifies its chosen reward by item id (packet.RewardItemId, parsed from
    // an embedded ItemInstance), not by a bare slot index. Resolve the slot here, now that the
    // Quest template is available.
    //
    // Most quests have zero choice-reward items (a single fixed reward, or none at all) --
    // CLASSIC_CMSG_QUEST_GIVER_CHOOSE_REWARD is still the client's universal "turn in" trigger
    // regardless, so it arrives even when there's nothing to actually choose between. Treat
    // "no choice items exist" as slot 0 unconditionally, matching the old vanilla client's own
    // behavior of always sending index 0 in this case.
    uint32 reward = 0;
    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
    {
        if (pQuest->GetRewChoiceItemsCount() > 0)
        {
            reward = QUEST_REWARD_CHOICES_COUNT;
            for (uint32 i = 0; i < pQuest->GetRewChoiceItemsCount(); ++i)
            {
                if (pQuest->RewChoiceItemId[i] == packet.RewardItemId)
                {
                    reward = i;
                    break;
                }
            }
        }
    }

    if (reward >= QUEST_REWARD_CHOICES_COUNT)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Error in CLASSIC_CMSG_QUEST_GIVER_CHOOSE_REWARD: player %s (guid %d) tried to get invalid reward item %u for quest %u (probably packet hacking)", _player->GetName(), _player->GetGUIDLow(), packet.RewardItemId, packet.QuestId);
        return;
    }

    WorldObject* pObject = (WorldObject*)_player->GetObjectByTypeMask(guid, TYPEMASK_CREATURE_OR_GAMEOBJECT);
    if (!pObject)
        return;

    if (!pObject->HasInvolvedQuest(packet.QuestId))
        return;

    if (!GetPlayer()->IsAlive())
    {
        // Some quest can be rewarded while dead (cf q3912 [Meet at the Grave])
        if (Creature* crea = pObject->ToCreature())
        {
            if (!crea->IsVisibleForDead())
                return;
        }
        else
            return;
    }

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId);
    if (pQuest)
    {
        if (_player->CanRewardQuest(pQuest, reward, true))
        {
            _player->RewardQuest(pQuest, reward, pObject);

            // Push the updated quest-giver status so the client's map marker (!/?) refreshes
            // immediately after turn-in.
            uint32 const newStatus = GetDialogStatus(_player, pObject, DIALOG_STATUS_NONE);
            _player->PlayerTalkClass->SendQuestGiverStatus(uint8(newStatus), guid);

            // Send next quest
            if (Quest const* nextquest = _player->GetNextQuest(guid, pQuest))
                _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest, guid, true);
        }
        else
        {
            _player->PlayerTalkClass->SendQuestGiverOfferReward(pQuest, guid, true);
        }
    }
}

void WorldSession::HandleQuestgiverRequestRewardOpcode(QuestgiverRequestReward const& packet)
{
    auto const guidOpt = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guidOpt)
        return;

    Object* pObject = _player->GetObjectByTypeMask(*guidOpt, TYPEMASK_CREATURE_OR_GAMEOBJECT);
    if (!pObject || !pObject->HasInvolvedQuest(packet.QuestId))
        return;

    if (!GetPlayer()->IsAlive())
    {
        // Some quest can be rewarded while dead (cf q3912 [Meet at the Grave])
        if (Creature* crea = pObject->ToCreature())
        {
            if (!crea->IsInvisibleForAlive())
                return;
        }
        else
            return;
    }

    if (_player->CanCompleteQuest(packet.QuestId))
        _player->CompleteQuest(packet.QuestId);

    if (_player->GetQuestStatus(packet.QuestId) != QUEST_STATUS_COMPLETE)
        return;

    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
        _player->PlayerTalkClass->SendQuestGiverOfferReward(pQuest, *guidOpt, true);
}

void WorldSession::HandleQuestgiverCancel(QuestGiverCloseQuest const& /*packet*/)
{
    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestLogRemoveQuest(QuestLogRemoveQuest const& packet)
{
    _player->RemoveQuestAtSlot(packet.Slot);
}

void WorldSession::HandleQuestConfirmAccept(QuestConfirmAccept const& packet)
{
    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
    {
        if (!pQuest->HasQuestFlag(QUEST_FLAGS_PARTY_ACCEPT))
            return;

        auto const& questShareInfo = _player->GetQuestShareInfo();
        if (!questShareInfo)
            return;

        if (questShareInfo->QuestId != packet.QuestId)
            return;

        Player* pOriginalPlayer = ObjectAccessor::FindPlayer(questShareInfo->PlayerGuid);
        if (!pOriginalPlayer)
        {
            _player->ClearQuestShareInfo();
            return;
        }

        if (pQuest->IsAllowedInRaid())
        {
            if (!_player->IsInSameRaidWith(pOriginalPlayer))
                return;
        }
        else
        {
            if (!_player->IsInSameGroupWith(pOriginalPlayer))
                return;
        }

        if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(packet.QuestId))
        {
            if (qInfo->HasQuestFlag(QUEST_FLAGS_SHARABLE))
                if (!pOriginalPlayer->IsCurrentQuest(packet.QuestId))
                    return;
            if (!qInfo->HasQuestFlag(QuestFlags(QUEST_FLAGS_SHARABLE | QUEST_FLAGS_PARTY_ACCEPT)))
                return;
        }

        if (!_player->CanTakeQuest(pQuest, true))
            return;

        if (_player->CanAddQuest(pQuest, true))
            _player->AddQuest(pQuest, nullptr);                // nullptr, this prevent DB script from duplicate running

        _player->ClearQuestShareInfo();
    }
}

void WorldSession::HandleQuestgiverCompleteQuest(QuestgiverCompleteQuest const& packet)
{
    auto const guidOpt = ClassicGuidMapping::FromClassic(packet.Guid);
    if (!guidOpt)
        return;

    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
    {
        if (_player->GetQuestStatus(packet.QuestId) != QUEST_STATUS_COMPLETE)
        {
            if (pQuest->IsRepeatable())
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, *guidOpt, _player->CanCompleteRepeatableQuest(pQuest), false);
            else
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, *guidOpt, _player->CanRewardQuest(pQuest, false), false);
        }
        else
            _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, *guidOpt, _player->CanRewardQuest(pQuest, false), false);
    }
}

void WorldSession::HandlePushQuestToParty(PushQuestToParty const& packet)
{
    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(packet.QuestId))
    {
        if (Group* pGroup = _player->GetGroup())
        {
            for (GroupReference* itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* pPlayer = itr->getSource();

                if (!pPlayer || pPlayer == _player)         // skip self
                    continue;

                _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_SHARING_QUEST, pQuest);

                if (!_player->IsWithinDist(pPlayer, QUEST_SHARE_DISTANCE, true, SizeFactor::None))
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_TOO_FAR, pQuest);
                    continue;
                }

                if (pPlayer->GetQuestStatus(packet.QuestId) == QUEST_STATUS_COMPLETE)
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_FINISH_QUEST, pQuest);
                    continue;
                }

                if (!pPlayer->SatisfyQuestStatus(pQuest, false))
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_HAVE_QUEST, pQuest);
                    continue;
                }

                if (!pPlayer->CanTakeQuest(pQuest, false))
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_CANT_TAKE_QUEST, pQuest);
                    continue;
                }

                if (!pPlayer->SatisfyQuestLog(false))
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_LOG_FULL, pQuest);
                    continue;
                }

                if (pPlayer->GetQuestShareInfo())
                {
                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_BUSY, pQuest);
                    continue;
                }

                pPlayer->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, _player->GetObjectGuid(), true);
                pPlayer->SetQuestShareInfo(_player->GetObjectGuid(), pQuest->GetQuestId());
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(QuestPushResult const& packet)
{
    auto const& questShareInfo = _player->GetQuestShareInfo();
    if (!questShareInfo)
        return;

    if (Player* pPlayer = ObjectAccessor::FindPlayer(questShareInfo->PlayerGuid))
    {
        QuestPushResultResponse response;
        response.SenderGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
        response.Result = packet.Msg;  // enum QuestShareMessages
        if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(questShareInfo->QuestId))
            response.QuestTitle = pQuest->GetTitle();
        pPlayer->GetSession()->SendClassic(response);
    }

    _player->ClearQuestShareInfo();
}

/**
* What - if any - kind of explanation mark or question-mark should a quest-giver display for a player
* @param pPlayer - for whom
* @param questgiver - from whom
* @param defstatus - initial set status (usually it will be called with DIALOG_STATUS_NONE) - must not be DIALOG_STATUS_UNDEFINED
*/
uint32 WorldSession::GetDialogStatus(Player* pPlayer, Object* questgiver, uint32 defstatus)
{
    uint32 dialogStatus = defstatus;

    QuestRelationsMapBounds rbounds;                        // QuestRelations (quest-giver)
    QuestRelationsMapBounds irbounds;                       // InvolvedRelations (quest-finisher)

    switch (questgiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            rbounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(questgiver->GetEntry());
            irbounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(questgiver->GetEntry());
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            rbounds = sObjectMgr.GetGOQuestRelationsMapBounds(questgiver->GetEntry());
            irbounds = sObjectMgr.GetGOQuestInvolvedRelationsMapBounds(questgiver->GetEntry());
            break;
        }
        default:
            //it's impossible, but check ^)
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Warning: GetDialogStatus called for unexpected type %u", questgiver->GetTypeId());
            return DIALOG_STATUS_NONE;
    }

    // Check markings for quest-finisher
    for (QuestRelationsMap::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        uint32 dialogStatusNew = DIALOG_STATUS_NONE;
        uint32 quest_id = itr->second;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);

        if (!pQuest || !pQuest->IsActive())
            continue;

        QuestStatus status = pPlayer->GetQuestStatus(quest_id);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(quest_id)) ||
                (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            else
                dialogStatusNew = DIALOG_STATUS_REWARD2;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;

        if (dialogStatusNew > dialogStatus)
            dialogStatus = dialogStatusNew;
    }

    // check markings for quest-giver
    for (QuestRelationsMap::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        uint32 dialogStatusNew = DIALOG_STATUS_NONE;
        uint32 quest_id = itr->second;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);

        if (!pQuest || !pQuest->IsActive())
            continue;

        QuestStatus status = pPlayer->GetQuestStatus(quest_id);

        if (status == QUEST_STATUS_NONE) // For all other cases the mark is handled either at some place else, or with involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld.getConfig(CONFIG_INT32_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && !pQuest->HasQuestFlag(QUEST_FLAGS_UNK2) && pPlayer->GetQuestStatusMap()[quest_id].m_rewarded))
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    else if (lowLevelDiff < 0 || pPlayer->GetLevel() <= pPlayer->GetQuestLevelForPlayer(pQuest) + uint32(lowLevelDiff))
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    else
                        dialogStatusNew = DIALOG_STATUS_CHAT;
                }
                else
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
            }
        }

        if (dialogStatusNew > dialogStatus)
            dialogStatus = dialogStatusNew;
    }

    return dialogStatus;
}
