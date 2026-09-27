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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Database/DatabaseImpl.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "Group.h"
#include "World.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Packets/Classic/GuidMapping.h"
#include "ScriptMgr.h"
#include "Compression/ZLib.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "SocialMgr.h"
#include "Packets/Session.h"
#include "Spell.h"
#include "ZoneScript.h"
#include "Conditions.h"
#include "Anticheat.h"
#include "MasterPlayer.h"

void WorldSession::HandleRepopRequestOpcode(RepopRequest const& /*packet*/)
{
    // recv_data.read_skip<uint8>(); client crash

    if (_player->IsAlive() || _player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if (_player->GetDeathState() == JUST_DIED)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", _player->GetName(), _player->GetGUIDLow());
        _player->KillPlayer();
    }

    _player->BuildPlayerRepop();
    _player->ScheduleRepopAtGraveyard();
}

class WhoListClientQueryTask
{
public:
    uint32 accountId;
    uint32 levelMin, levelMax, raceMask, classMask, zonesCount, strCount;
    uint32 zoneIds[10];                                     // 10 is client limit
    std::wstring str[4];                                    // 4 is client limit
    std::wstring wplayerName, wguildName;
    void operator()()
    {
        WorldSession* sess = sWorld.FindSession(accountId);
        if (!sess)
            return;
        sess->SetReceivedWhoRequest(false);
        if (!sess->GetPlayer() || !sess->GetPlayer()->IsInWorld())
            return;

        uint32 clientCount = 0;
        Team const team = sess->GetPlayer()->GetTeam();
        AccountTypes const security = sess->GetSecurity();
        bool const allowTwoSideWhoList = sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_WHO_LIST);
        bool const showBotsInWhoList = sWorld.getConfig(CONFIG_BOOL_PLAYER_BOT_SHOW_IN_WHO_LIST);
        AccountTypes const gmLevelInWhoList = (AccountTypes)sWorld.getConfig(CONFIG_UINT32_GM_LEVEL_IN_WHO_LIST);
        uint32 const zone = sess->GetPlayer()->GetCachedZoneId();
        bool const notInBattleground = !((zone == 2597) || (zone == 3277) || (zone == 3358));

        WorldPacket data(SMSG_WHO, 50);                         // guess size
        data << uint32(clientCount);                            // clientCount place holder, listed count
        data << uint32(clientCount);                            // clientCount place holder, online count

        // TODO: Guard Player map
        HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
        for (const auto& itr : m)
        {
            Player* pPlayer = itr.second;

            if (security == SEC_PLAYER)
            {
                // player can see member of other team only if CONFIG_BOOL_ALLOW_TWO_SIDE_WHO_LIST
                if (pPlayer->GetTeam() != team && !allowTwoSideWhoList)
                    continue;

                // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
                if (pPlayer->GetSession()->GetSecurity() > gmLevelInWhoList)
                    continue;
            }

            // skip bots
            if (!showBotsInWhoList && pPlayer->IsBot())
                continue;

            // do not process players which are not in world
            if (!pPlayer->IsInWorld())
                continue;

            // check if target's level is in level range
            uint32 lvl = pPlayer->GetLevel();
            if (lvl < levelMin || lvl > levelMax)
                continue;

            // check if target is globally visible for player
            if (!pPlayer->IsVisibleGloballyFor(sess->GetPlayer()))
                continue;

            // check if class matches classmask
            uint32 classId = pPlayer->GetClass();
            if (!(classMask & (1 << classId)))
                continue;

            // check if race matches racemask
            uint32 raceId = pPlayer->GetRace();
            if (!(raceMask & (1 << raceId)))
                continue;

            std::string pname = pPlayer->GetName();
            std::wstring wpname;
            if (!Utf8toWStr(pname, wpname))
                continue;
            wstrToLower(wpname);

            if (!(wplayerName.empty() || wpname.find(wplayerName) != std::wstring::npos))
                continue;

            std::string gname = sGuildMgr.GetGuildNameById(pPlayer->GetGuildId());
            std::wstring wgname;
            if (!Utf8toWStr(gname, wgname))
                continue;
            wstrToLower(wgname);

            if (!(wguildName.empty() || wgname.find(wguildName) != std::wstring::npos))
                continue;

            uint32 pzoneId = pPlayer->GetCachedZoneId();

            bool zShow = true;
            for (uint32 i = 0; i < zonesCount; ++i)
            {
                if (zoneIds[i] == pzoneId)
                {
                    // World of Warcraft Client Patch 1.7.0 (2005-09-13)
                    // Using the / who command while in a Battleground instance will now only display players in your instance.
                    zShow = (zone != pzoneId) || notInBattleground || (sess->GetPlayer()->GetInstanceId() == pPlayer->GetInstanceId());
                    break;
                }

                zShow = false;
            }
            if (!zShow)
                continue;

            std::string aName;
            if (const auto *areaEntry = AreaEntry::GetById(pzoneId))
            {
                aName = areaEntry->Name;
                sObjectMgr.GetAreaLocaleString(areaEntry->Id, sess->GetSessionDbLocaleIndex(), &aName);
            }

            bool sShow = true;
            for (uint32 i = 0; i < strCount; ++i)
            {
                if (!str[i].empty())
                {
                    if (wgname.find(str[i]) != std::wstring::npos ||
                        wpname.find(str[i]) != std::wstring::npos ||
                        Utf8FitTo(aName, str[i]))
                    {
                        sShow = true;
                        break;
                    }
                    sShow = false;
                }
            }
            if (!sShow)
                continue;

            data << pname;                                      // player name
            data << gname;                                      // guild name
            data << uint32(lvl);                                // player level
            data << uint32(classId);                            // player class
            data << uint32(raceId);                             // player race
            data << uint32(pzoneId);                            // player zone id

#if SUPPORTED_CLIENT_BUILD <= CLIENT_BUILD_1_8_4
            data << uint32(pPlayer->GetWhoListPartyStatus());   // not actually displayed anywhere
#endif
            // 50 is maximum player count sent to client
            if ((++clientCount) == 49)
                break;
        }

        uint32 count = m.size();
        data.put(0, clientCount);                               // insert right count, listed count
        data.put(4, count > 49 ? count : clientCount);          // insert right count, online count

        sess->SendPacket(&data);
    }
};

void WorldSession::HandleWhoOpcode(WorldPackets::Misc::Who const& packet)
{
    if (ReceivedWhoRequest())
        return;

    if (packet.zoneIds.size() > 10 || packet.searchTerms.size() > 4) // client limits: 10 zones, 4 search strings
        return;                                                // can't be received from real client or broken packet

    WhoListClientQueryTask task;
    task.accountId = GetAccountId();
    task.levelMin = packet.levelMin;
    task.levelMax = packet.levelMax;
    task.raceMask = packet.raceMask;
    task.classMask = packet.classMask;
    task.zonesCount = static_cast<uint32>(packet.zoneIds.size());
    task.strCount   = static_cast<uint32>(packet.searchTerms.size());

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", task.levelMin, task.levelMax, packet.playerName.c_str(), packet.guildName.c_str(), task.raceMask, task.classMask, task.zonesCount, task.strCount);

    for (uint32 i = 0; i < task.zonesCount; ++i)
    {
        task.zoneIds[i] = packet.zoneIds[i];
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Zone %u: %u", i, task.zoneIds[i]);
    }

    for (uint32 i = 0; i < task.strCount; ++i)
    {
        if (!Utf8toWStr(packet.searchTerms[i], task.str[i]))
            continue;

        wstrToLower(task.str[i]);
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "String %u: %s", i, packet.searchTerms[i].c_str());
    }

    if (!(Utf8toWStr(packet.playerName, task.wplayerName) && Utf8toWStr(packet.guildName, task.wguildName)))
        return;

    wstrToLower(task.wplayerName);
    wstrToLower(task.wguildName);

    // client send in case not set max level value 100 but mangos support 255 max level,
    // update it to show GMs with characters after 100 level
    if (task.levelMax >= MAX_LEVEL)
        task.levelMax = PLAYER_STRONG_MAX_LEVEL;

    SetReceivedWhoRequest(true);
    sWorld.AddAsyncTask(std::move(task));
}

void WorldSession::HandleLFGOpcode(NullClientPacket const& /*packet*/)
{
    WorldPacket data(MSG_LOOKING_FOR_GROUP, 4);
    data << uint32(0);
    SendPacket(&data);
}

void WorldSession::HandleLogoutRequestOpcode(NullClientPacket const& /*packet*/)
{
    if (ObjectGuid lootGuid = GetPlayer()->GetLootGuid())
        DoLootRelease(lootGuid);

    uint8 reason = 0;

    if (GetPlayer()->IsInCombat())
        reason = 1;
    else if (GetPlayer()->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)))
        reason = 3;                      // is jumping or falling
    else if (GetPlayer()->HasAura(9454)) // frozen by GM via freeze command
        reason = 2;                      // FIXME - Need the correct value

    if (reason)
    {
        auto packet = std::make_unique<WorldPackets::Misc::LogoutResponse>();
        packet->reason = reason;
        packet->instant = 0;
        SendPacket(std::move(packet));
        LogoutRequest(0);
        return;
    }

    // instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ||
        GetPlayer()->IsTaxiFlying() ||
        GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))
    {
        auto packet = std::make_unique<WorldPackets::Misc::LogoutResponse>();
        packet->reason = 0;
        packet->instant = 1;
        SendPacket(std::move(packet));
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        if (GetPlayer()->GetStandState() == UNIT_STAND_STATE_STAND &&
           !GetPlayer()->IsMounted() &&
           !(GetPlayer()->GetUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_SPLINE_ENABLED)))
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        GetPlayer()->SetRooted(true);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    GetPlayer()->ApplyModByteFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_FLAGS, PLAYER_FIELD_BYTE_LOGGING_OUT, true);

    auto logoutPacket = std::make_unique<WorldPackets::Misc::LogoutResponse>();
    logoutPacket->reason = 0;
    logoutPacket->instant = 0;
    SendPacket(std::move(logoutPacket));
    LogoutRequest(time(nullptr));
}

void WorldSession::HandlePlayerLogoutOpcode(NullClientPacket const& /*packet*/)
{
}

void WorldSession::HandleLogoutCancelOpcode(NullClientPacket const& /*packet*/)
{
    LogoutRequest(0);

    SendPacket(std::make_unique<WorldPackets::Misc::LogoutCancelAck>());

    // not remove flags if can't free move - its not set in Logout request code.
    if (GetPlayer()->CanFreeMove())
    {
        //!we can move again
        GetPlayer()->SetRooted(false);

        //! Stand Up
        if (GetPlayer()->GetStandState() == UNIT_STAND_STATE_SIT)
            GetPlayer()->SetStandState(UNIT_STAND_STATE_STAND);

        //! DISABLE_ROTATE
        GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    GetPlayer()->ApplyModByteFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_FLAGS, PLAYER_FIELD_BYTE_LOGGING_OUT, false);
}

void WorldSession::HandleSetPvP(SetPvP const& packet)
{
    GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_DESIRED, packet.EnablePvP);

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_DESIRED))
        GetPlayer()->UpdatePvP(true);
}

void WorldSession::HandleZoneUpdateOpcode(WorldPackets::Misc::ZoneUpdate const& /*packet*/)
{
    // use server size data
    uint32 newzone, newarea;
    GetPlayer()->GetZoneAndAreaId(newzone, newarea);
    GetPlayer()->UpdateZone(newzone, newarea);

    // Trigger a client camera reset by sending an `SMSG_STANDSTATE_UPDATE'
    // event. See `WorldSession::HandleMoveWorldportAckOpcode'.
    // Note: There might be a better place to perform this trigger
    if (m_clientOS == CLIENT_OS_MAC && GetPlayer()->m_movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        StandStateUpdate packet;
        packet.StandState = GetPlayer()->GetStandState();
        SendClassic(packet);
    }
}

void WorldSession::HandleSetSelectionOpcode(SetSelection const& packet)
{
    ObjectGuid const targetGuid = ClassicGuidMapping::FromClassic(packet.Target).value_or(ObjectGuid());
    _player->SetSelectionGuid(targetGuid);

    // update reputation list if need
    Unit* unit = ObjectAccessor::GetUnit(*_player, targetGuid);   // can select group members at diff maps

    if (unit)
        if (FactionTemplateEntry const* factionTemplateEntry = unit->GetFactionTemplateEntry())
            if (!factionTemplateEntry->IsHostileToPlayerTeam(*_player->GetFactionTemplateEntry()))
                _player->GetReputationMgr().SetVisible(factionTemplateEntry);

    // Drop combo points only for rogues and druids
    // Warriors use combo points internally, do no reset for everyone
    if ((_player->GetClass() == CLASS_ROGUE || _player->GetClass() == CLASS_DRUID) && unit && targetGuid != _player->GetComboTargetGuid())
        _player->ClearComboPoints();

    // Update autoshot if need
    if (Spell* pSpell = _player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
    {
        if (!unit || !_player->IsValidAttackTarget(unit))
        {
            pSpell->m_targets.setUnitTarget(nullptr);
            pSpell->cancel();
            return;
        }

        if (!unit->IsInWorld() || unit->GetMap() != _player->GetMap())
            return;

        pSpell->m_targets.setUnitTarget(unit);
    }
}

void WorldSession::HandleSetMouseOverTargetOpcode(WorldPackets::Misc::SetTarget const& packet)
{
}

void WorldSession::HandleStandStateChangeOpcode(StandStateChange const& packet)
{
    switch (packet.StandState)
    {
        case UNIT_STAND_STATE_STAND:
        case UNIT_STAND_STATE_SIT:
        case UNIT_STAND_STATE_SLEEP:
        case UNIT_STAND_STATE_KNEEL:
            break;
        default:
            return;
    }

    if (_player->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREVENT_ANIM))
        return;

    _player->InterruptSpellsWithChannelFlags(AURA_INTERRUPT_ANIM_CANCELS);
    _player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_ANIM_CANCELS);
    _player->SetStandState(packet.StandState);
}

void WorldSession::HandleFriendListOpcode(SendContactList const& packet)
{
    ASSERT(GetMasterPlayer());
    GetMasterPlayer()->GetSocial()->SendContactList(packet.Flags);
}

void WorldSession::HandleSetContactNotesOpcode(SetContactNotes const& packet)
{
    ASSERT(GetMasterPlayer());

    nonstd::optional<ObjectGuid> friendGuid = ClassicGuidMapping::FromClassic(packet.Player.Guid);
    if (!friendGuid)
        return;

    GetMasterPlayer()->GetSocial()->SetFriendNote(*friendGuid, packet.Notes);
}

void WorldSession::HandleAddFriendOpcode(AddFriend const& packet)
{
    ASSERT(GetMasterPlayer());

    std::string friendName = packet.FriendName;
    if (!normalizePlayerName(friendName))
        return;

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: %s asked to add friend : '%s'",
              GetMasterPlayer()->GetName(), friendName.c_str());

    PlayerCacheData const* pData = sObjectMgr.GetPlayerDataByName(friendName);
    if (!pData)
        return;

    uint32 friendLowGuid = pData->uiGuid;
    ObjectGuid friendGuid = ObjectGuid(HIGHGUID_PLAYER, friendLowGuid);

    uint32 team = Player::TeamForRace(pData->uiRace);

    FriendsResult friendResult = FRIEND_NOT_FOUND;
    if (friendGuid)
    {
        if (friendGuid == GetMasterPlayer()->GetObjectGuid())
            friendResult = FRIEND_SELF;
        else if (GetMasterPlayer()->GetTeam() != team && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_ADD_FRIEND) && GetSecurity() < SEC_MODERATOR)
            friendResult = FRIEND_ENEMY;
        else if (GetMasterPlayer()->GetSocial()->HasFriend(friendGuid))
            friendResult = FRIEND_ALREADY;
        else
        {
            MasterPlayer* pFriend = ObjectAccessor::FindMasterPlayer(friendGuid);
            if (pFriend && pFriend->IsVisibleGloballyFor(GetMasterPlayer()))
                friendResult = FRIEND_ADDED_ONLINE;
            else
                friendResult = FRIEND_ADDED_OFFLINE;

            if (!GetMasterPlayer()->GetSocial()->AddToSocialList(friendGuid, false))
            {
                friendResult = FRIEND_LIST_FULL;
                sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: %s's friend list is full.", GetMasterPlayer()->GetName());
            }
            else
                GetMasterPlayer()->GetSocial()->SetFriendNote(friendGuid, packet.Notes);
        }
    }

    sSocialMgr.SendFriendStatus(GetMasterPlayer(), friendResult, friendGuid, false);
}

void WorldSession::HandleDelFriendOpcode(DelFriend const& packet)
{
    ASSERT(GetMasterPlayer());

    nonstd::optional<ObjectGuid> friendGuid = ClassicGuidMapping::FromClassic(packet.Player.Guid);
    if (!friendGuid)
        return;

    GetMasterPlayer()->GetSocial()->RemoveFromSocialList(*friendGuid, false);

    sSocialMgr.SendFriendStatus(GetMasterPlayer(), FRIEND_REMOVED, *friendGuid, false);
}

void WorldSession::HandleAddIgnoreOpcode(AddIgnore const& packet)
{
    ASSERT(GetMasterPlayer());

    std::string ignoreName = packet.IgnoreName;
    if (!normalizePlayerName(ignoreName))
        return;

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: %s asked to Ignore: '%s'",
              GetMasterPlayer()->GetName(), ignoreName.c_str());

    PlayerCacheData const* pData = sObjectMgr.GetPlayerDataByName(ignoreName);
    if (!pData)
        return;

    uint32 ignoreLowGuid = pData->uiGuid;
    ObjectGuid ignoreGuid = ObjectGuid(HIGHGUID_PLAYER, ignoreLowGuid);

    FriendsResult ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    if (ignoreGuid)
    {
        if (ignoreGuid == GetMasterPlayer()->GetObjectGuid())
            ignoreResult = FRIEND_IGNORE_SELF;
        else if (GetMasterPlayer()->GetSocial()->HasIgnore(ignoreGuid))
            ignoreResult = FRIEND_IGNORE_ALREADY;
        else
        {
            ignoreResult = FRIEND_IGNORE_ADDED;

            // ignore list full
            if (!GetMasterPlayer()->GetSocial()->AddToSocialList(ignoreGuid, true))
                ignoreResult = FRIEND_IGNORE_FULL;
        }
    }

    sSocialMgr.SendFriendStatus(GetMasterPlayer(), ignoreResult, ignoreGuid, false);
}

void WorldSession::HandleDelIgnoreOpcode(DelIgnore const& packet)
{
    ASSERT(GetMasterPlayer());

    nonstd::optional<ObjectGuid> ignoreGuid = ClassicGuidMapping::FromClassic(packet.Player.Guid);
    if (!ignoreGuid)
        return;

    GetMasterPlayer()->GetSocial()->RemoveFromSocialList(*ignoreGuid, true);

    sSocialMgr.SendFriendStatus(GetMasterPlayer(), FRIEND_IGNORE_REMOVED, *ignoreGuid, false);
}

void WorldSession::HandleBugOpcode(WorldPackets::Misc::Bug const& packet)
{
    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WorldSession::HandleBugOpcode: Player %s (AccountId: %u) reported a %s: [%s] %s",
        GetPlayer() ? GetPlayer()->GetName() : "(none)", GetAccountId(),
        packet.suggestion ? "suggestion" : "bug", packet.type.c_str(), packet.content.c_str());
}

void WorldSession::HandleReclaimCorpseOpcode(ReclaimCorpse const& /*packet*/)
{
    if (GetPlayer()->IsAlive())
        return;

    // body not released yet
    if (!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse* corpse = GetPlayer()->GetCorpse();

    if (!corpse)
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    if (corpse->GetGhostTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP) > time(nullptr))
        return;

    if (!corpse->IsWithinDistInMap(GetPlayer(), CORPSE_RECLAIM_RADIUS, true))
        return;

    // Prevent exploit: die with hellfire during battleground preparation, and resurrect after the door.
    if (BattleGround const* bg = GetPlayer()->GetBattleGround())
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
            return;
    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleGround() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();
}

void WorldSession::HandleResurrectResponseOpcode(ResurrectResponse const& packet)
{
    if (GetPlayer()->IsAlive())
        return;

    if (packet.Response != 0)
    {
        GetPlayer()->ClearResurrectRequestData(); // player denied rezz attempt
        return;
    }

    nonstd::optional<ObjectGuid> const resurrecter = ClassicGuidMapping::FromClassic(packet.Resurrecter);
    if (!resurrecter || !GetPlayer()->IsRessurectRequestedBy(*resurrecter))
        return;

    GetPlayer()->ResurrectUsingRequestData();     // will call SpawnCorpseBones
}

void WorldSession::HandleAreaTriggerOpcode(AreaTrigger const& packet)
{
    Player* const pPlayer = GetPlayer();

    if (pPlayer->HasCheatOption(PLAYER_CHEAT_IGNORE_TRIGGERS))
        return;

    if (pPlayer->IsTaxiFlying())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player '%s' (GUID: %u) in flight, ignore Area Trigger ID: %u", pPlayer->GetName(), pPlayer->GetGUIDLow(), packet.TriggerId);
        return;
    }

    AreaTriggerEntry const* pTrigger = sObjectMgr.GetAreaTrigger(packet.TriggerId);
    if (!pTrigger)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID: %u", pPlayer->GetName(), pPlayer->GetGUIDLow(), packet.TriggerId);
        return;
    }

    // check if player in the range of areatrigger
    if (!IsPointInAreaTriggerZone(pTrigger, pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 5.0f))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player '%s' (GUID: %u) too far, ignore Area Trigger ID: %u", pPlayer->GetName(), pPlayer->GetGUIDLow(), packet.TriggerId);
        return;
    }

    if (pTrigger->script_id || pTrigger->script_name)
        pPlayer->GetMap()->StartAreaTriggerScript(pTrigger, pPlayer);

    uint32 quest_id = sObjectMgr.GetQuestForAreaTrigger(packet.TriggerId);
    if (quest_id && pPlayer->IsAlive() && pPlayer->IsActiveQuest(quest_id))
    {
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (pQuest)
        {
            if (pPlayer->GetQuestStatus(quest_id) == QUEST_STATUS_INCOMPLETE)
                pPlayer->AreaExploredOrEventHappens(quest_id);
        }
    }

    // enter to tavern, not overwrite city rest
    if (sObjectMgr.IsTavernAreaTrigger(packet.TriggerId))
    {
        // set resting flag we are in the inn
        if (pPlayer->GetRestType() != REST_TYPE_IN_CITY)
            pPlayer->SetRestType(REST_TYPE_IN_TAVERN, packet.TriggerId);
        return;
    }

    if (BattlegroundEntranceTrigger const* pBgEntrance = sObjectMgr.GetBattlegroundEntranceTrigger(packet.TriggerId))
    {
        BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(pBgEntrance->bgTypeId);
        if (!bg)
            return;

        if ((pPlayer->GetLevel() < bg->GetMinLevel() || pPlayer->GetLevel() > bg->GetMaxLevel()) ||
            (pPlayer->GetTeam() != pBgEntrance->team))
        {
            SendAreaTriggerMessage("You must be in the %s and at least %u%s level to enter.", pPlayer->GetTeam() == ALLIANCE ? "Horde" : "Alliance", bg->GetMinLevel(), bg->GetMinLevel() % 2 ? "st" : "th");
            return;
        }

        pPlayer->SetBattleGroundEntryPoint(pBgEntrance->exit_mapId, pBgEntrance->exit_X, pBgEntrance->exit_Y, pBgEntrance->exit_Z, pBgEntrance->exit_Orientation);
        SendBattleGroundList(pPlayer->GetObjectGuid(), pBgEntrance->bgTypeId);
        return;
    }

    if (pPlayer->InBattleGround())
    {
        if (BattleGround* bg = pPlayer->GetBattleGround())
            if (bg->HandleAreaTrigger(pPlayer, packet.TriggerId))
                return;
    }

    if (ZoneScript* pZoneScript = pPlayer->GetZoneScript())
    {
        if (pZoneScript->HandleAreaTrigger(_player, packet.TriggerId))
            return;
    }

    // nullptr if all values default (non teleport trigger)
    AreaTriggerTeleport const* pTeleTrigger = sObjectMgr.GetAreaTriggerTeleport(packet.TriggerId);
    if (!pTeleTrigger)
        return;

    MapEntry const* pTargetMap = sMapStorage.LookupEntry<MapEntry>(pTeleTrigger->destination.mapId);
    if (!pTargetMap)
        return;

    // ghost resurrected at enter attempt to dungeon with corpse (including fail enter cases)
    if (!pPlayer->IsAlive() && pTargetMap->IsDungeon())
    {
        int32 corpseMapId = 0;
        if (Corpse* corpse = pPlayer->GetCorpse())
            corpseMapId = corpse->GetMapId();

        // Special case prior Patch 1.3 to revive your corpse if dead in Molten Core
        if (sWorld.GetWowPatch() <= WOW_PATCH_102)
        {
            if (corpseMapId == MAP_MOLTEN_CORE && packet.TriggerId == 1466)
            {
                pPlayer->ResurrectPlayer(0.5f);
                pPlayer->SpawnCorpseBones();
                pPlayer->TeleportTo(MAP_BLACKROCK_DEPTHS, 458.32f, 26.52f, -70.67f, 4.95f); // Blackrock Depths
                // pPlayer->TeleportTo(MAP_MOLTEN_CORE, 1082.04f, -474.596f, -107.762f, 5.02623f); // Molten Core
                return;
            }
        }

        // check back way from corpse to entrance
        uint32 instanceMap = corpseMapId;
        do
        {
            // most often fast case
            if (instanceMap == pTargetMap->id)
                break;

            MapEntry const* instance = sMapStorage.LookupEntry<MapEntry>(instanceMap);
            instanceMap = instance && instance->IsDungeon() ? instance->parent : 0;
        }
        while (instanceMap);

        // corpse not in dungeon or some linked deep dungeons
        if (!instanceMap)
        {
            pPlayer->GetSession()->SendAreaTriggerMessage("You cannot enter %s while in ghost form.", pTargetMap->name);
            return;
        }

        // need find areatrigger to inner dungeon for landing point
        if (pTeleTrigger->destination.mapId != corpseMapId)
            if (AreaTriggerTeleport const* corpseAt = sObjectMgr.GetMapEntranceTrigger(corpseMapId))
                pTeleTrigger = corpseAt;
    }

    if (!pPlayer->IsGameMaster() && !pPlayer->HasCheatOption(PLAYER_CHEAT_TRIGGER_PASS))
    {
        // World of Warcraft Client Patch 1.4.1 (2005-05-03)
        // - Added minimum level requirements to all instances to prevent
        //   exploitive behavior.The minimum levels are very generous and should
        //   not affect the normal course of gameplay.
        bool const bLevelCheck = pPlayer->GetLevel() < pTeleTrigger->requiredLevel && !sWorld.getConfig(CONFIG_BOOL_INSTANCE_IGNORE_LEVEL) && sWorld.GetWowPatch() >= WOW_PATCH_104;
        bool const bConditionCheck = pTeleTrigger->requiredCondition && !IsConditionSatisfied(pTeleTrigger->requiredCondition, pPlayer, pPlayer->GetMap(), pPlayer, CONDITION_FROM_AREATRIGGER);

        if (bLevelCheck || bConditionCheck)
        {
            if (pTeleTrigger->message.empty())
            {
                if (bLevelCheck)
                    SendAreaTriggerMessage(GetMangosString(LANG_LEVEL_MINREQUIRED), pTeleTrigger->requiredLevel);
            }
            else
            {
                char const* message = pTeleTrigger->message.c_str();

                int locIdx = GetSessionDbLocaleIndex();
                if (locIdx >= 0)
                {
                    AreaTriggerLocale const* locale = sObjectMgr.GetAreaTriggerLocale(packet.TriggerId);
                    if (locale)
                    {
                        if (locale->message.size() > size_t(locIdx) && !locale->message[locIdx].empty())
                            message = locale->message[locIdx].c_str();
                    }
                }

                SendAreaTriggerMessage(message);
            }
            return;
        }
    }

    pPlayer->TeleportTo(pTeleTrigger->destination);
}

void WorldSession::HandleUpdateAccountData(AccountDataUpdate const& packet)
{
    NewAccountData::AccountDataType dataType;
    if (GetGameBuild() <= CLIENT_BUILD_1_8_4)
        dataType = ConvertOldAccountDataToNew(packet.DataType);
    else
        dataType = static_cast<NewAccountData::AccountDataType>(packet.DataType);

    if (dataType >= NewAccountData::NUM_ACCOUNT_DATA_TYPES)
    {
        std::stringstream oss;
        oss << "Client sent invalid account data type " << packet.DataType << " in CLASSIC_CMSG_UPDATE_ACCOUNT_DATA.";
        ProcessAnticheatAction("PassiveAnticheat", oss.str().c_str(), CHEAT_ACTION_LOG);
        return;
    }

    if (packet.DecompressedSize == 0)                       // erase
    {
        SetAccountData(dataType, "");
        return;
    }

    if (packet.DecompressedSize > 0xFFFF)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "UAD: Account data packet too big, size %u", packet.DecompressedSize);
        return;
    }

    nonstd::optional<std::vector<uint8>> uncompressedData = Compression::ZLib::Decompress(
        packet.CompressedData,
        packet.DecompressedSize,
        Compression::ZLib::ChecksumOption::IgnoreChecksum // client might not provide ADLER32 checksum
    );

    if (!uncompressedData)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "UAD: Failed to decompress account data");
        return;
    }

    std::string uncompressedString(reinterpret_cast<char const*>(uncompressedData->data()), uncompressedData->size());
    SetAccountData(dataType, uncompressedString);
}

void WorldSession::HandleRequestAccountData(AccountDataRequest const& packet)
{
    NewAccountData::AccountDataType dataType;
    if (GetGameBuild() <= CLIENT_BUILD_1_8_4)
        dataType = ConvertOldAccountDataToNew(packet.DataType);
    else
        dataType = (NewAccountData::AccountDataType)packet.DataType;

    if (dataType >= NewAccountData::NUM_ACCOUNT_DATA_TYPES)
    {
        std::stringstream oss;
        oss << "Client requested invalid account data type " << packet.DataType << " in CLASSIC_CMSG_REQUEST_ACCOUNT_DATA.";
        ProcessAnticheatAction("PassiveAnticheat", oss.str().c_str(), CHEAT_ACTION_LOG);
        return;
    }

    AccountData* adata = GetAccountData(dataType);

    AccountDataUpdateResult response;
    response.Timestamp = time(nullptr);
    response.PlayerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
    response.DataType = packet.DataType; // use the original type sent by client

    uint32 size = adata->data.size();
    if (size)
    {
        nonstd::optional<std::vector<uint8>> compressedData = Compression::ZLib::Compress(reinterpret_cast<uint8 const*>(adata->data.data()), adata->data.size());
        if (!compressedData)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "CLASSIC_SMSG_UPDATE_ACCOUNT_DATA: Failed to compress account data");
            return;
        }

        response.DecompressedLength = size;
        response.CompressedData = std::move(*compressedData);
    }

    SendClassic(response);
}

void WorldSession::HandleSetActionButtonOpcode(SetActionButton const& packet)
{
    uint32 action = ACTION_BUTTON_ACTION(packet.Action);
    uint8  type   = ACTION_BUTTON_TYPE(packet.Action);

    if (!packet.Action)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "MISC: Remove action from button %u", packet.Index);
        GetMasterPlayer()->removeActionButton(packet.Index);
    }
    else
    {
        switch (type)
        {
            case ACTION_BUTTON_MACRO:
            case ACTION_BUTTON_CMACRO:
            case ACTION_BUTTON_SPELL:
            case ACTION_BUTTON_ITEM:
                break;
            default:
                sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "MISC: Unknown action button type %u for action %u into button %u", type, action, packet.Index);
                return;
        }
        if (!Player::IsActionButtonDataValid(packet.Index, action, type, GetPlayer()))
            return;
        GetMasterPlayer()->addActionButton(packet.Index, action, type);
    }
}

void WorldSession::HandleCompleteCinematic(NullClientPacket const& /*packet*/)
{
    GetPlayer()->CinematicEnd();
}

void WorldSession::HandleNextCinematicCamera(NullClientPacket const& /*packet*/)
{
}

void WorldSession::HandleSetActionBarTogglesOpcode(SetActionBarToggles const& packet)
{
    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_ACTION_BARS, packet.Mask);
}

void WorldSession::HandlePlayedTime(RequestPlayedTime const& packet)
{
    PlayedTimeResult result;
    result.TotalTime = int32(_player->GetTotalPlayedTime());
    result.LevelTime = int32(_player->GetLevelPlayedTime());
    result.TriggerEvent = packet.TriggerScriptEvent;
    SendClassic(result);
}

void WorldSession::HandleInspectOpcode(Inspect const& packet)
{
    nonstd::optional<ObjectGuid> const targetGuid = ClassicGuidMapping::FromClassic(packet.TargetGuid);
    if (!targetGuid)
        return;

    _player->SetSelectionGuid(*targetGuid);

    Player* pTarget = sObjectMgr.GetPlayer(*targetGuid);
    if (!pTarget)
        return;

    if (_player->GetDistance3dToCenter(pTarget) > INSPECT_DISTANCE)
        return;

    if (_player->IsValidAttackTarget(pTarget))
        return;

    // The 1.12 client reconstructed the inspect window from the target's visible-item update
    // fields and a separate honor-stats round trip; the 1.14 client expects this one packet with
    // equipment and honor both up front (TrinityCore 1.14's HandleInspectOpcode confirms the
    // same wire shape).
    InspectResult response;
    InspectDisplayInfo& displayInfo = response.DisplayInfo;
    displayInfo.PlayerGuid = packet.TargetGuid;
    displayInfo.Name = pTarget->GetName();
    displayInfo.Gender = pTarget->GetGender();
    displayInfo.Race = pTarget->GetRace();
    displayInfo.ClassId = pTarget->GetClass();

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* item = pTarget->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!item)
            continue;

        InspectItemEntry itemData;
        itemData.Index = i;
        itemData.Item.ItemId = int32(item->GetProto()->ItemId);
        itemData.Item.RandomPropertiesSeed = int32(item->GetItemSuffixFactor());
        itemData.Item.RandomPropertiesId = int32(item->GetItemRandomPropertyId());
        itemData.CreatorGuid = ClassicGuidMapping::ToClassic(item->GetGuidValue(ITEM_FIELD_CREATOR), realmID);

        // Permanent + temporary enchant slots only -- vanilla's socket/use-enchant slots occupy
        // the same field indices and must not be shown here.
        for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
        {
            if (uint32 enchantId = item->GetEnchantmentId(EnchantmentSlot(j)))
            {
                InspectEnchantEntry enchant;
                enchant.Id = enchantId;
                enchant.Index = j;
                itemData.Enchants.push_back(enchant);
            }
        }
        displayInfo.Items.push_back(itemData);
    }

    response.LifetimeMaxRank = pTarget->GetHonorMgr().GetHighestRank().rank;
    response.TodayHK = uint16(pTarget->GetUInt32Value(PLAYER_FIELD_SESSION_KILLS));
    response.YesterdayHK = pTarget->GetUInt16Value(PLAYER_FIELD_YESTERDAY_KILLS, 0);
    response.LifetimeHK = pTarget->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS);

    if (Guild* guild = sGuildMgr.GetGuildById(pTarget->GetGuildId()))
    {
        response.HasGuildData = true;
        response.GuildData.GuildGuid = ClassicGuidMapping::MakeGuild(guild->GetId());
        response.GuildData.NumMembers = int32(guild->GetMemberSize());
    }

    SendClassic(response);
}

void WorldSession::HandleInspectPvpOpcode(InspectPvp const& packet)
{
    // Vanilla has no arenas or rated brackets, so the response carries an empty bracket list and
    // the three team slots (2v2/3v3/5v5) empty -- the client still needs the three slots present
    // to render the tab.
    InspectPvpResult response;
    response.PlayerGuid = packet.TargetGuid;
    response.ArenaTeams.resize(3);
    SendClassic(response);
}

void WorldSession::HandleQueryInspectAchievementsOpcode(QueryInspectAchievements const& /*packet*/)
{
    // No achievement system in vanilla (1.12 predates achievements) -- consumed no-op so the
    // opcode stops being logged as unhandled on every inspect. The client tolerates the missing
    // reply by simply not drawing the achievements tab.
}

void WorldSession::HandleTeleportToUnitOpcode(WorldPackets::Misc::TeleportToUnit const& packet)
{
    if (packet.playerName.length() > MAX_PLAYER_NAME)
        return;

    std::string command = ".goname " + packet.playerName;
    SanitizeChatMessageAndProcessCommand(command, LANG_UNIVERSAL, CHAT_MSG_SYSTEM);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPackets::Misc::WorldTeleport const& packet)
{
    // write in client console: worldport 1 16226 16257 13.2 90 or /console worldport 1 16226 16257 13.2 90
    // Received opcode CMSG_WORLD_TELEPORT
    // Time is ***, map=1, x=16226.000000, y=16257.000000, z=13.200000, orient=1.57079637

    //sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Received opcode CMSG_WORLD_TELEPORT");

    if (GetPlayer()->IsTaxiFlying())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player '%s' (GUID: %u) in flight, ignore worldport command.", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    if (GetSecurity() >= SEC_ADMINISTRATOR)
        GetPlayer()->TeleportTo(packet.location);
    else
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Received worldport command from player %s", GetPlayer()->GetName());
}

void WorldSession::HandleWhoisOpcode(WhoIsRequest const& packet)
{
    if (GetSecurity() < SEC_ADMINISTRATOR)
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    std::string charName = packet.CharacterName;
    if (charName.empty() || !normalizePlayerName(charName))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player* plr = sObjectMgr.GetPlayer(charName.c_str());

    if (!plr)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charName.c_str());
        return;
    }

    uint32 accId = plr->GetSession()->GetAccountId();

    std::unique_ptr<QueryResult> result = LoginDatabase.PQuery("SELECT `username`, `email`, `last_ip` FROM `account` WHERE `id`=%u", accId);
    if (!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charName.c_str());
        return;
    }

    Field* fields = result->Fetch();
    std::string acc = fields[0].GetCppString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetCppString();
    if (email.empty())
        email = "Unknown";
    std::string lastIp = fields[2].GetCppString();
    if (lastIp.empty())
        lastIp = "Unknown";

    WhoIsResult response;
    response.Message = charName + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastIp;
    _player->GetSession()->SendClassic(response);
}

void WorldSession::HandleFarSightOpcode(FarSightRequest const& packet)
{
    WorldObject* obj = _player->GetMap()->GetWorldObject(_player->GetFarSightGuid());
    if (!obj)
        return;

    if (packet.Enable)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Added FarSight %s to %s", _player->GetFarSightGuid().GetString().c_str(), _player->GetGuidStr().c_str());
        _player->GetCamera().SetView(obj, false);
    }
    else
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Removed FarSight from %s", _player->GetGuidStr().c_str());
        _player->GetCamera().ResetView(false);
    }
}

void WorldSession::HandleResetInstancesOpcode(NullClientPacket const& /*packet*/)
{
    if (Group* pGroup = _player->GetGroup())
    {
        if (pGroup->IsLeader(_player->GetObjectGuid()))
            pGroup->ResetInstances(INSTANCE_RESET_ALL, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL);
}

void WorldSession::HandleRequestPetInfoOpcode(RequestPetInfo const& /*packet */)
{
    if (_player->GetPet())
        _player->PetSpellInitialize();
    else if (_player->GetCharm())
        _player->CharmSpellInitialize();
}

void WorldSession::HandleClassicLogoutRequestOpcode(ClassicLogoutRequest const& /*packet*/)
{
    if (ObjectGuid lootGuid = GetPlayer()->GetLootGuid())
        DoLootRelease(lootGuid);

    uint8 reason = 0;

    if (GetPlayer()->IsInCombat())
        reason = 1;
    else if (GetPlayer()->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR)))
        reason = 3;                      // is jumping or falling
    else if (GetPlayer()->HasAura(9454)) // frozen by GM via freeze command
        reason = 2;                      // FIXME - Need the correct value

    if (reason)
    {
        SendClassic(LogoutResponse{ int32(reason), false });
        LogoutRequest(0);
        return;
    }

    // instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ||
        GetPlayer()->IsTaxiFlying() ||
        GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))
    {
        SendClassic(LogoutResponse{ 0, true });
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        if (GetPlayer()->GetStandState() == UNIT_STAND_STATE_STAND &&
           !GetPlayer()->IsMounted() &&
           !(GetPlayer()->GetUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_SPLINE_ENABLED)))
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        GetPlayer()->SetRooted(true);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
    }

    GetPlayer()->ApplyModByteFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_FLAGS, PLAYER_FIELD_BYTE_LOGGING_OUT, true);

    SendClassic(LogoutResponse{ 0, false });
    LogoutRequest(time(nullptr));
}

void WorldSession::HandleClassicTimeSyncResponseOpcode(TimeSyncResponse const& packet)
{
    auto it = m_pendingTimeSyncRequests.find(packet.SequenceIndex);
    if (it == m_pendingTimeSyncRequests.end())
        return;

    uint32 const roundTripMs = WorldTimer::getMSTimeDiff(it->second, WorldTimer::getMSTime());
    m_pendingTimeSyncRequests.erase(it);

    // Half the round trip approximates the one-way delay to the client, so adding it to ClientTime
    // gives the server's own clock at the moment the client actually sent the response.
    m_timeSyncClockDeltaMs = int32(WorldTimer::getMSTime()) - int32(packet.ClientTime + roundTripMs / 2);
}

