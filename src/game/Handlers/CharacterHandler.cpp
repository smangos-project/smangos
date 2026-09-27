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
#include "WorldPacket.h"
#include "SharedDefines.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "Database/DatabaseImpl.h"
#include "SocialMgr.h"
#include "Util.h"
#include "Language.h"
#include "Anticheat.h"
#include "MasterPlayer.h"
#include "PlayerBroadcaster.h"
#include "PlayerBotMgr.h"
#include "MapManager.h"
#include "AccountMgr.h"
#include "Packets/Auth.h"
#include "Packets/Classic/GuidMapping.h"
#include "Packets/GuildMessages.h"

class LoginQueryHolder : public SqlQueryHolder
{
private:
    uint32 m_accountId;
    ObjectGuid m_guid;
public:
    LoginQueryHolder(uint32 accountId, ObjectGuid guid)
        : SqlQueryHolder(guid.GetCounter()), m_accountId(accountId), m_guid(guid) { }
    ~LoginQueryHolder()
    {
        // Queries should NOT be deleted by user
        DeleteAllResults();
    }
    ObjectGuid GetGuid() const
    {
        return m_guid;
    }
    uint32 GetAccountId() const
    {
        return m_accountId;
    }
    bool Initialize();
};

bool LoginQueryHolder::Initialize()
{
    SetSize(MAX_PLAYER_LOGIN_QUERY);

    bool res = true;

    // NOTE: all fields in `characters` must be read to prevent lost character data at next save in case wrong DB structure.
    // !!! NOTE: including unused `zone`,`online`
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADFROM,            "SELECT `guid`, `account`, `name`, `race`, `class`, `gender`, `level`, `xp`, `money`, `skin`, `face`, `hair_style`, `hair_color`, `facial_hair`, `bank_bag_slots`, `character_flags`, "
                     "`position_x`, `position_y`, `position_z`, `map`, `orientation`, `known_taxi_mask`, `played_time_total`, `played_time_level`, `rest_bonus`, `logout_time`, `reset_talents_multiplier`, "
                     "`reset_talents_time`, `transport_guid`, `transport_x`, `transport_y`, `transport_z`, `transport_o`, `extra_flags`, `stable_slots`, `death_expire_time`, `current_taxi_path`, "
                     "`honor_rank_points`, `honor_highest_rank`, `honor_standing`, `honor_last_week_hk`, `honor_last_week_cp`, `honor_stored_hk`, `honor_stored_dk`, "
                     "`watched_faction`, `drunk`, `health`, `power1`, `power2`, `power3`, `power4`, `power5`, `explored_zones`, `ammo_id`, `action_bars`, "
                     "`world_phase_mask`, `create_time`, `instance` FROM `characters` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADGROUP,           "SELECT `group_id` FROM `group_member` WHERE `member_guid` ='%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES,  "SELECT `id`, `permanent`, `map`, `reset_time` FROM `character_instance` LEFT JOIN `instance` ON `instance` = `id` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADAURAS,           "SELECT `caster_guid`, `item_guid`, `spell`, `stacks`, `charges`, `base_points0`, `base_points1`, `base_points2`, `periodic_time0`, `periodic_time1`, `periodic_time2`, `max_duration`, `duration`, `effect_index_mask` FROM `character_aura` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSPELLS,          "SELECT `spell`, `active`, `disabled` FROM `character_spell` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADQUESTSTATUS,     "SELECT `quest`, `status`, `rewarded`, `explored`, `timer`, `mob_count1`, `mob_count2`, `mob_count3`, `mob_count4`, `item_count1`, `item_count2`, `item_count3`, `item_count4`, `reward_choice` FROM `character_queststatus` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADHONORCP,         "SELECT `victim_type`, `victim_id`, `cp`, `date`, `type` FROM `character_honor_cp` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADREPUTATION,      "SELECT `faction`, `standing`, `flags` FROM `character_reputation` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADINVENTORY,       "SELECT * FROM (SELECT `creator_guid`, `gift_creator_guid`, `count`, `duration`, `charges`, `flags`, `enchantments`, `random_property_id`, `durability`, `text`, `bag`, `slot`, `item_guid`, `item_instance`.`item_id`, `generated_loot` FROM `character_inventory` JOIN `item_instance` ON `character_inventory`.`item_guid` = `item_instance`.`guid` WHERE `character_inventory`.`guid` = '%u') as t ORDER BY `bag`, `slot`", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADITEMLOOT,        "SELECT `guid`, `item_id`, `amount`, `property` FROM `item_loot` WHERE `owner_guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADACTIONS,         "SELECT `button`, `action`, `type` FROM `character_action` WHERE `guid` = '%u' ORDER BY `button`", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSOCIALLIST,      "SELECT `friend`, `flags` FROM `character_social` WHERE `guid` = '%u' LIMIT 255", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADHOMEBIND,        "SELECT `map`, `zone`, `position_x`, `position_y`, `position_z` FROM `character_homebind` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS,  "SELECT `spell`, `spell_expire_time`, `category`, `category_expire_time`, `item_id` FROM `character_spell_cooldown` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADGUILD,           "SELECT `guild_id`, `rank` FROM `guild_member` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADBGDATA,          "SELECT `instance_id`, `team`, `join_x`, `join_y`, `join_z`, `join_o`, `join_map` FROM `character_battleground_data` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADACCOUNTDATA,     "SELECT `type`, `time`, `data` FROM `character_account_data` WHERE `guid`='%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSKILLS,          "SELECT `skill`, `value`, `max` FROM `character_skills` WHERE `guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADMAILS,           "SELECT `id`, `message_type`, `sender_guid`, `receiver_guid`, `subject`, `item_text_id`, `expire_time`, `deliver_time`, `money`, `cod`, `checked`, `stationery`, `mail_template_id`, `has_items` FROM `mail` WHERE `receiver_guid` = '%u' ORDER BY `id` DESC", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADMAILEDITEMS,     "SELECT `creator_guid`, `gift_creator_guid`, `count`, `duration`, `charges`, `flags`, `enchantments`, `random_property_id`, `durability`, `text`, `mail_id`, `item_guid`, `item_instance`.`item_id`, `generated_loot` FROM `mail_items` JOIN `item_instance` ON `item_guid` = `guid` WHERE `receiver_guid` = '%u'", m_guid.GetCounter());
    res &= SetPQuery(PLAYER_LOGIN_QUERY_FORGOTTEN_SKILLS,    "SELECT `skill`, `value` FROM `character_forgotten_skills` WHERE `guid` = '%u'", m_guid.GetCounter());

    return res;
}

// don't call WorldSession directly
// it may get deleted before the query callbacks get executed
// instead pass an account id to this handler
class CharacterHandler
{
public:
    void HandleCharEnumCallback(std::unique_ptr<QueryResult> result, uint32 account)
    {
        WorldSession* session = sWorld.FindSession(account);
        if (!session)
        {
            return;
        }
        session->HandleCharEnum(std::move(result));
    }
    void HandlePlayerLoginCallback(std::unique_ptr<QueryResult> /*dummy*/, SqlQueryHolder* holder)
    {
        if (!holder)
            return;

        uint32 const accountId = ((LoginQueryHolder*)holder)->GetAccountId();
        WorldSession* session = sWorld.FindSession(accountId);
        if (!session)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "HandlePlayerLoginCallback: no session for account %u, login result dropped", accountId);
            delete holder;
            return;
        }
        session->HandlePlayerLogin((LoginQueryHolder*)holder);
    }
} chrHandler;

void WorldSession::HandleCharEnum(std::unique_ptr<QueryResult> result)
{
    WorldPacket data(SMSG_CHAR_ENUM, 100);                  // we guess size

    uint8 num = 0;
    data << num;

    if (result)
    {
        do
        {
            uint32 guidlow = (*result)[0].GetUInt32();
            uint32 level   = (*result)[10].GetUInt32();
            if (m_characterMaxLevel < level)
                m_characterMaxLevel = level;

            sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "Build enum data for char guid %u from account %u.", guidlow, GetAccountId());
            if (Player::BuildEnumData(result, &data))
                ++num;
        }
        while (result->NextRow());
    }

    data.put<uint8>(0, num);
    m_charactersCount = num;

    SendPacket(&data);
}

void WorldSession::HandleCharEnumOpcode(NullClientPacket const& /*packet*/)
{
    // get all the data necessary for loading all characters (along with their pets) on the account
    CharacterDatabase.AsyncPQuery(&chrHandler, &CharacterHandler::HandleCharEnumCallback, GetAccountId(),
                                  //           0                    1                    2                    3                     4                      5                    6                    7                          8                          9                           10
                                  "SELECT `characters`.`guid`, `characters`.`name`, `characters`.`race`, `characters`.`class`, `characters`.`gender`, `characters`.`skin`, `characters`.`face`, `characters`.`hair_style`, `characters`.`hair_color`, `characters`.`facial_hair`, `characters`.`level`, "
                                  //    11                   12                  13                         14                         15                         16                         17
                                  "`characters`.`zone`, `characters`.`map`, `characters`.`position_x`, `characters`.`position_y`, `characters`.`position_z`, `guild_member`.`guild_id`, `characters`.`character_flags`, "
                                  //    18                                19                       20                            21                       22
                                  "`characters`.`played_time_total`, `character_pet`.`entry`, `character_pet`.`display_id`, `character_pet`.`level`, `characters`.`equipment_cache` "
                                  "FROM `characters` LEFT JOIN `character_pet` ON `characters`.`guid`=`character_pet`.`owner_guid` AND `character_pet`.`slot`='%u' "
                                  "LEFT JOIN `guild_member` ON `characters`.`guid` = `guild_member`.`guid` "
                                  "WHERE `characters`.`account` = '%u' ORDER BY `characters`.`create_time`, `characters`.`guid` "
                                  "LIMIT 0,10",
                                  PET_SAVE_AS_CURRENT, GetAccountId());
}

void WorldSession::LoginPlayer(ObjectGuid loginPlayerGuid)
{
    ASSERT(loginPlayerGuid.IsPlayer());
    LoginQueryHolder* holder = new LoginQueryHolder(GetAccountId(), loginPlayerGuid);
    if (!holder->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return;
    }
    m_playerLoading = true;
    CharacterDatabase.DelayQueryHolderUnsafe(&chrHandler, &CharacterHandler::HandlePlayerLoginCallback, holder);
}

void WorldSession::HandlePlayerLogin(LoginQueryHolder *holder)
{
    // The following fixes a crash. Use case:
    // Session1 created, requests login, kicked.
    // Session2 created, requests login, and receives 2 login callback.
    if (GetPlayer() || !m_playerLoading)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "[CRASH] HandlePlayerLogin on session %u with player %s [loading=%u]", GetAccountId(), GetPlayerName(), m_playerLoading);
        delete holder;
        m_playerLoading = false;
        return;
    }

    ObjectGuid playerGuid = holder->GetGuid();
    ASSERT(playerGuid.IsPlayer());
    m_currentPlayerGuid = playerGuid;

    // If the character is online (ALT-F4 logout for example)
    Player* pCurrChar = sObjectAccessor.FindPlayer(playerGuid);
    MasterPlayer* pCurrMasterPlayer = sObjectAccessor.FindMasterPlayer(playerGuid);
    bool alreadyOnline = false;
    if (pCurrChar)
    {
        // Hacking attempt
        if (pCurrChar->GetSession()->GetAccountId() != GetAccountId())
        {
            ProcessAnticheatAction("PassiveAnticheat", "Attempt to login to character on different account", CHEAT_ACTION_LOG);
            KickPlayer();
            delete holder;
            m_playerLoading = false;
            return;
        }

        if (pCurrChar->FindMap() != sMapMgr.FindMap(pCurrChar->GetMapId(), pCurrChar->GetInstanceId()))
        {
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "[CRASH] Dangling map pointer during login on character guid %u", playerGuid.GetCounter());
            KickPlayer();
            delete holder;
            m_playerLoading = false;
            return;
        }

        alreadyOnline = true;
        pCurrChar->GetSession()->SetPlayer(nullptr);
        pCurrChar->SetSession(this);

        // Need to attach packet bcaster to the new socket
        if (pCurrChar->m_broadcaster)
            pCurrChar->m_broadcaster->ChangeSocket(GetSocket());
        else
            pCurrChar->CreatePacketBroadcaster();

        // If the character had a logout request, then he is articifially stunned (in CMSG_LOGOUT_REQUEST handler). Fix it here.
        if (pCurrChar->CanFreeMove())
        {
            pCurrChar->SetRootedReal(false);
            pCurrChar->SetStandState(UNIT_STAND_STATE_STAND);
            pCurrChar->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else
    {
        // Character found online but not in world ?
        if (HashMapHolder<Player>::Find(playerGuid))
        {
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "[CRASH] Trying to login already ingame character guid %u", playerGuid.GetCounter());
            KickPlayer();
            delete holder;
            m_playerLoading = false;
            return;
        }
        pCurrChar = new Player(this);
        pCurrChar->GetMotionMaster()->Initialize();

        if (GetBot() && !sPlayerBotMgr.IsSavingAllowed())
            pCurrChar->m_saveDisabled = true;
    }

    // "GetAccountId()==db stored account id" checked in LoadFromDB (prevent login not own character using cheating tools)
    if (alreadyOnline)
        pCurrChar->SendPacketsAtRelogin();
    else if (!pCurrChar->LoadFromDB(playerGuid, holder))
    {
        KickPlayer();                                       // disconnect client, player no set to session and it will not deleted or saved at kick
        delete pCurrChar;                                   // delete it manually
        delete holder;                                      // delete all unprocessed queries
        m_playerLoading = false;
        return;
    }

    ASSERT(pCurrChar->GetSession() == this);
    SetPlayer(pCurrChar);
    if (pCurrMasterPlayer)
    {
        pCurrMasterPlayer->GetSession()->SetMasterPlayer(nullptr);
        pCurrMasterPlayer->SetSession(this);
        m_masterPlayer = pCurrMasterPlayer;
    }
    else
    {
        m_masterPlayer = new MasterPlayer(this);
        m_masterPlayer->LoadPlayer(GetPlayer());
        m_masterPlayer->LoadActions(holder->TakeResult(PLAYER_LOGIN_QUERY_LOADACTIONS));
        m_masterPlayer->LoadSocial(holder->TakeResult(PLAYER_LOGIN_QUERY_LOADSOCIALLIST));
        m_masterPlayer->LoadMails(holder->TakeResult(PLAYER_LOGIN_QUERY_LOADMAILS));
        m_masterPlayer->LoadMailedItems(holder->TakeResult(PLAYER_LOGIN_QUERY_LOADMAILEDITEMS));
    }
    m_masterPlayer->UpdateNextMailTimeAndUnreads();

    sObjectAccessor.AddObject(m_masterPlayer);

    Position const& position = pCurrChar->GetTransport()
                             ? pCurrChar->m_movementInfo.GetTransportPos()
                             : pCurrChar->GetPosition();

    LoginVerifyWorld loginVerifyWorld;
    loginVerifyWorld.MapId = pCurrChar->GetMapId();
    loginVerifyWorld.X = position.x;
    loginVerifyWorld.Y = position.y;
    loginVerifyWorld.Z = position.z;
    loginVerifyWorld.O = position.o;
    SendClassic(loginVerifyWorld);
    SendTimeSync();

    // load player specific part before send times
    LoadAccountData(holder->TakeResult(PLAYER_LOGIN_QUERY_LOADACCOUNTDATA), NewAccountData::PER_CHARACTER_CACHE_MASK);
    SendTutorialsData();
    SendAccountDataTimes();

    pCurrChar->GetSocial()->SendContactList(SOCIAL_FLAG_FRIEND | SOCIAL_FLAG_IGNORED);

    // Send MOTD (1.12.1 not have SMSG_MOTD, so do it in another way)
    {
        uint32 linecount = 0;
        std::string str_motd = sWorld.GetMotd();
        std::string::size_type pos, nextpos;
        std::string motd;

        pos = 0;
        while ((nextpos = str_motd.find('@', pos)) != std::string::npos)
        {
            if (nextpos != pos)
            {
                pCurrChar->PSendSysMessage(str_motd.substr(pos, nextpos - pos).c_str());
                ++linecount;
            }
            pos = nextpos + 1;
        }

        if (pos < str_motd.length())
        {
            pCurrChar->PSendSysMessage(str_motd.substr(pos).c_str());
            ++linecount;
        }
    }

    if (Guild* guild = sGuildMgr.GetGuildById(pCurrChar->GetGuildId()))
    {
        GuildEventMotd motd;
        motd.MotdText = guild->GetMOTD();
        SendClassic(motd);

        guild->BroadcastPresenceChange(pCurrChar->GetObjectGuid(), pCurrChar->GetName(), true);
    }

    if (char const* warning = sAccountMgr.GetWarningText(GetAccountId()))
    {
        pCurrChar->PSendSysMessage(LANG_ACCOUNT_WARNED, warning);
        SendNotification("WARNING: %s", warning);
    }

    if (!pCurrChar->IsAlive())
        pCurrChar->SendCorpseReclaimDelay(true);

    pCurrChar->SendInitialPacketsBeforeAddToMap();

    // Show cinematic at the first time that player login
    if (pCurrChar->m_playedTime[PLAYED_TIME_TOTAL] == 0 && !sWorld.getConfig(CONFIG_BOOL_SKIP_CINEMATICS))
    {
        if (ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(pCurrChar->GetRace()))
            pCurrChar->SendCinematicStart(rEntry->CinematicSequence);
    }

    if (!alreadyOnline && !pCurrChar->GetMap()->Add(pCurrChar))
    {
        // normal delayed teleport protection not applied (and this correct) for this case (Player object just created)
        AreaTriggerTeleport const* at = sObjectMgr.GetGoBackTrigger(pCurrChar->GetMapId());
        if (at)
            pCurrChar->TeleportTo(at->destination);
        else if (pCurrChar->GetMapId() == MAP_NAXXRAMAS)
            pCurrChar->TeleportTo(MAP_EASTERN_KINGDOMS, 3362.15f, -3379.35f, 144.782f, 6.28319f); // Naxxramas has no exit trigger
        else
            pCurrChar->TeleportToHomebind();

        sMapMgr.ExecuteSingleDelayedTeleport(pCurrChar);
    }

    if (alreadyOnline)
        pCurrChar->GetMap()->ExistingPlayerLogin(pCurrChar); // SendInitSelf ...
    else
        sObjectAccessor.AddObject(pCurrChar);

    //sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player %s added to Map.",pCurrChar->GetName());

    pCurrChar->SendInitialPacketsAfterAddToMap();
    if (alreadyOnline)
        pCurrChar->SendInitWorldStates(pCurrChar->GetCachedZoneId(), pCurrChar->GetCachedAreaId());

    static SqlStatementID updChars;
    static SqlStatementID updAccount;

    SqlStatement stmt = CharacterDatabase.CreateStatement(updChars, "UPDATE `characters` SET `online` = 1 WHERE `guid` = ?");
    stmt.PExecute(pCurrChar->GetGUIDLow());

    stmt = LoginDatabase.CreateStatement(updAccount, "UPDATE `account` SET current_realm = ?, online = 1 WHERE id = ?");
    stmt.PExecute(realmID, GetAccountId());

    pCurrChar->SetInGameTime(WorldTimer::getMSTime());

    // announce group about member online (must be after add to player list to receive announce to self)
    if (Group* group = pCurrChar->GetGroup())
        group->UpdatePlayerOnlineStatus(pCurrChar);

    // friend status
    // TODO: Call it when node finished loading also
    if (GetMasterPlayer())
    {
        GetMasterPlayer()->areaId = pCurrChar->GetCachedAreaId();
        GetMasterPlayer()->zoneId = pCurrChar->GetCachedZoneId();
        sSocialMgr.SendFriendStatus(GetMasterPlayer(), FRIEND_ONLINE, GetMasterPlayer()->GetObjectGuid(), true);
    }

    if (!alreadyOnline)
    {
        // Place character in world (and load zone) before some object loading
        pCurrChar->LoadCorpse();

        // setting Ghost+speed if dead
        if (pCurrChar->m_deathState != ALIVE)
        {
            // not blizz like, we must correctly save and load player instead...
            pCurrChar->ApplyGhostForm();
        }
    }

    // Load pet if any (if player not alive and in taxi flight or another then pet will remember as temporary unsummoned)
    if (alreadyOnline)
    {
        pCurrChar->PetSpellInitialize();
        pCurrChar->SendMirrorTimers(true);
    }
    else
    {
        pCurrChar->ContinueTaxiFlight();
        pCurrChar->LoadPet();
    }

    if (pCurrChar->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
        pCurrChar->UpdatePvPContested(true);

    if (pCurrChar->HasCharacterFlag(CHARACTER_FLAG_RESET_TALENTS_ON_LOGIN))
    {
        pCurrChar->ResetTalents(true);
        SendNotification(LANG_RESET_TALENTS);               // we can use SMSG_TALENTS_INVOLUNTARILY_RESET here
    }

    // show time before shutdown if shutdown planned.
    if (sWorld.IsShutdowning())
        sWorld.ShutdownMsg(true, pCurrChar);

    if (sWorld.getConfig(CONFIG_BOOL_ALL_TAXI_PATHS))
        pCurrChar->SetTaxiCheater(true);

    if (pCurrChar->IsGameMaster())
        SendNotification(LANG_GM_ON);

    if (!pCurrChar->IsGMVisible())
        SendNotification(LANG_INVISIBLE_INVISIBLE, pCurrChar->GetGMInvisibilityLevel());

    std::string IP_str = GetRemoteAddress();

    sLog.Player(this, LOG_CHAR, "Login", LOG_LVL_DETAIL, alreadyOnline ? "Player was already online" : "");

    if (!alreadyOnline && !pCurrChar->IsStandingUp() && !pCurrChar->HasUnitState(UNIT_STATE_STUNNED))
        pCurrChar->SetStandState(UNIT_STAND_STATE_STAND);

    m_playerLoading = false;
    delete holder;

    if (alreadyOnline)
    {
        pCurrChar->UpdateControl();
        // Send "Release spirit" timer, etc ...
        if (pCurrChar->GetDeathState() == CORPSE)
            pCurrChar->KillPlayer();
    }
    pCurrChar->RestorePendingTeleport();

    sObjectMgr.UpdatePlayerCachedPosition(pCurrChar);

    if (sWorld.getConfig(CONFIG_BOOL_SEND_LOOT_ROLL_UPON_RECONNECT) && alreadyOnline)
        if (Group* pGroup = pCurrChar->GetGroup())
            pGroup->SendLootStartRollsForPlayer(pCurrChar);
}

void WorldSession::HandleSetFactionAtWarOpcode(WorldPackets::Misc::SetFactionAtWar const& packet)
{
    Player* pPlayer = GetPlayer();

    if (pPlayer->IsInCombat())
        return;

    pPlayer->GetReputationMgr().SetAtWar(packet.repListId, packet.flag);
}

// The modern client folds the legacy set/clear/reset opcode triplet into one CLASSIC_CMSG_TUTORIAL
// with a 2-bit Action field.
void WorldSession::HandleTutorialFlagOpcode(TutorialRequest const& packet)
{
    switch (packet.Action)
    {
        case TutorialRequest::ActionUpdate:
        {
            uint32 wInt = packet.TutorialBit >> 5;
            if (wInt >= ACCOUNT_TUTORIALS_COUNT)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "CLASSIC_CMSG_TUTORIAL received bad TutorialBit %u.", packet.TutorialBit);
                return;
            }
            uint32 tutflag = GetTutorialInt(wInt);
            tutflag |= (1 << (packet.TutorialBit & 0x1F));
            SetTutorialInt(wInt, tutflag);
            break;
        }
        case TutorialRequest::ActionClear:
            for (uint32 iI = 0; iI < ACCOUNT_TUTORIALS_COUNT; ++iI)
                SetTutorialInt(iI, 0xFFFFFFFF);
            break;
        case TutorialRequest::ActionReset:
            for (uint32 iI = 0; iI < ACCOUNT_TUTORIALS_COUNT; ++iI)
                SetTutorialInt(iI, 0x00000000);
            break;
        default:
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "CLASSIC_CMSG_TUTORIAL received unknown Action %u.", packet.Action);
            break;
    }
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
void WorldSession::HandleSetWatchedFactionOpcode(WorldPackets::Misc::SetWatchedFaction const& packet)
{
    GetPlayer()->SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, packet.repId);
}

void WorldSession::HandleSetFactionInactiveOpcode(WorldPackets::Misc::SetFactionInactive const& packet)
{
    _player->GetReputationMgr().SetInactive(packet.replistid, packet.inactive);
}
#endif

void WorldSession::HandleShowingHelmOpcode(NullClientPacket const& /*packet*/)
{
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM);
}

void WorldSession::HandleShowingCloakOpcode(NullClientPacket const& /*packet*/)
{
    _player->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK);
}

void WorldSession::HandleCharRenameOpcode(WorldPackets::Character::CharRename const& packet)
{
    // prevent character rename to invalid name
    if (!normalizePlayerName(const_cast<std::string&>(packet.newname)))
    {
        auto response = std::make_unique<WorldPackets::Character::CharRenameResponse>();
        response->result = CHAR_NAME_NO_NAME;
        SendPacket(std::move(response));
        return;
    }

    uint8 res = ObjectMgr::CheckPlayerName(packet.newname, true);
    if (res != CHAR_NAME_SUCCESS)
    {
        auto response = std::make_unique<WorldPackets::Character::CharRenameResponse>();
        response->result = res;
        SendPacket(std::move(response));
        return;
    }

    // check name limitations
    if (GetSecurity() == SEC_PLAYER && sObjectMgr.IsReservedName(packet.newname))
    {
        auto response = std::make_unique<WorldPackets::Character::CharRenameResponse>();
        response->result = CHAR_NAME_RESERVED;
        SendPacket(std::move(response));
        return;
    }

    std::string escaped_newname = packet.newname;
    CharacterDatabase.escape_string(escaped_newname);

    // make sure that the character belongs to the current account, that rename at login is enabled
    // and that there is no character with the desired new name
    CharacterDatabase.AsyncPQuery(&WorldSession::HandleChangePlayerNameOpcodeCallBack,
                                  GetAccountId(), packet.newname,
                                  "SELECT `guid`, `name` FROM `characters` WHERE `guid` = %u AND `account` = %u AND (`character_flags` & %u) = %u AND NOT EXISTS (SELECT NULL FROM `characters` WHERE `name` = '%s')",
                                  packet.guid.GetCounter(), GetAccountId(), CHARACTER_FLAG_RENAME, CHARACTER_FLAG_RENAME, escaped_newname.c_str()
                                 );
}

void WorldSession::HandleChangePlayerNameOpcodeCallBack(std::unique_ptr<QueryResult> result, uint32 accountId, std::string newname)
{
    WorldSession* session = sWorld.FindSession(accountId);
    if (!session)
    {
        return;
    }

    if (!result)
    {
        auto response = std::make_unique<WorldPackets::Character::CharRenameResponse>();
        response->result = CHAR_CREATE_ERROR;
        session->SendPacket(std::move(response));
        return;
    }

    uint32 guidLow = result->Fetch()[0].GetUInt32();
    ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, guidLow);
    std::string oldname = result->Fetch()[1].GetCppString();

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("UPDATE `characters` SET `name` = '%s', `character_flags` = (`character_flags` & ~ %u), `character_flags` = (`character_flags` | %u) WHERE `guid` ='%u'", newname.c_str(), uint32(CHARACTER_FLAG_RENAME), uint32(CHARACTER_FLAG_RENAME_NEEDS_GM_REVIEW), guidLow);
    CharacterDatabase.CommitTransaction();

    sLog.Player(session->GetAccountId(), LOG_CHAR, LOG_LVL_BASIC,
        "Account: %d (IP: %s) Character:[%s] (guid:%u) Changed name to: %s",
        session->GetAccountId(), session->GetRemoteAddress().c_str(), oldname.c_str(), guidLow, newname.c_str());

    auto response = std::make_unique<WorldPackets::Character::CharRenameResponse>();
    response->result = RESPONSE_SUCCESS;
    response->renamedCharacterGuid = guid;
    response->newName = newname;
    session->SendPacket(std::move(response));

    sObjectMgr.ChangePlayerNameInCache(guidLow, oldname, newname);
    sWorld.InvalidatePlayerDataToAllClient(guid);
}

namespace
{
// Maps the vanilla ResponseCodes this tree's name/create checks still return onto the modern
// wire's numbering (Packets/Classic/Character.h), since the two are not numerically related. Only
// the values CheckPlayerName/normalizePlayerName actually produce are translated; anything else
// (a reserved name, a disabled race, ...) falls back to the closest generic failure -- their exact
// modern codes aren't confirmed yet.
uint8 ClassicNameResult(uint8 vanillaResult)
{
    switch (vanillaResult)
    {
        case CHAR_NAME_SUCCESS:           return CLASSIC_CHAR_NAME_SUCCESS;
        case CHAR_NAME_NO_NAME:           return CLASSIC_CHAR_NAME_NO_NAME;
        case CHAR_NAME_TOO_SHORT:         return CLASSIC_CHAR_NAME_TOO_SHORT;
        case CHAR_NAME_TOO_LONG:          return CLASSIC_CHAR_NAME_TOO_LONG;
        case CHAR_NAME_INVALID_CHARACTER: return CLASSIC_CHAR_NAME_INVALID_CHARACTER;
        default:                          return CLASSIC_CHAR_NAME_FAILURE;
    }
}

// Looks up an account's character by its classic wire guid, refusing anything that doesn't map to
// a player guid this account owns. Used by both delete and player-login.
PlayerCacheData const* FindOwnedCharacter(WorldSession& session, ClassicGuid const& wireGuid, ObjectGuid& outGuid)
{
    nonstd::optional<ObjectGuid> guid = ClassicGuidMapping::FromClassic(wireGuid);
    if (!guid || !guid->IsPlayer())
        return nullptr;

    PlayerCacheData const* cacheData = sObjectMgr.GetPlayerDataByGUID(guid->GetCounter());
    if (!cacheData || cacheData->uiAccount != session.GetAccountId())
        return nullptr;

    outGuid = *guid;
    return cacheData;
}

}

void WorldSession::HandleClassicCharEnumOpcode(ClassicEnumCharacters const& /*packet*/)
{
    // Built from ObjectMgr's in-memory player cache, not a fresh DB query: the cache is what
    // HandleCharCreateOpcode/HandleCharDeleteOpcode update synchronously, so a create or delete is
    // always reflected in the very next enum -- a DB round trip on a worker thread can lag behind
    // the transaction that made the change.
    std::vector<PlayerCacheData const*> accountCharacters;
    sObjectMgr.GetPlayerDataForAccount(GetAccountId(), accountCharacters);

    ClassicEnumCharactersResult response;
    uint8 listPosition = 1; // 0 is treated as unset by the client; hand out 1-based positions
    for (PlayerCacheData const* cacheData : accountCharacters)
    {
        ClassicCharacterInfo info;
        info.Guid = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, cacheData->uiGuid), realmID);
        info.Name = cacheData->sName;
        info.Race = uint8(cacheData->uiRace);
        info.Class = uint8(cacheData->uiClass);
        info.Gender = uint8(cacheData->uiGender);
        info.Level = uint8(cacheData->uiLevel);
        info.Zone = int32(cacheData->uiZoneId);
        info.Map = int32(cacheData->uiMapId);
        info.X = cacheData->fPosX;
        info.Y = cacheData->fPosY;
        info.Z = cacheData->fPosZ;
        info.ListPosition = listPosition++;

        if (info.Level > m_characterMaxLevel)
            m_characterMaxLevel = info.Level;

        response.Characters.push_back(std::move(info));
    }

    response.CharacterCount = uint32(response.Characters.size());
    m_charactersCount = uint8(response.Characters.size());

    SendClassic(response);
}

void WorldSession::HandleCharCreateOpcode(CharCreate const& packet)
{
    auto sendResponse = [this](uint8 result, ClassicGuid const& guid = ClassicGuid{})
    {
        CharCreateResponse response;
        response.Result = result;
        response.Guid = guid;
        SendClassic(response);
    };

    if (GetSecurity() == SEC_PLAYER)
    {
        if (uint32 mask = sWorld.getConfig(CONFIG_UINT32_CHARACTERS_CREATING_DISABLED))
        {
            bool disabled = false;
            switch (Player::TeamForRace(packet.Race))
            {
                case ALLIANCE: disabled = mask & (1 << 0); break;
                case HORDE:    disabled = mask & (1 << 1); break;
            }
            if (disabled)
            {
                sendResponse(CLASSIC_CHAR_CREATE_FAILED);
                return;
            }
        }
    }

    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(packet.Class);
    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(packet.Race);
    if (!classEntry || !raceEntry || raceEntry->HasFlag(CHRRACES_FLAGS_NOT_PLAYABLE))
    {
        sendResponse(CLASSIC_CHAR_CREATE_FAILED);
        return;
    }

    // The bots never send customization choices (CharCreate::CustomizationCount is always 0),
    // which maps to a random valid vanilla appearance rather than always the same one -- a real
    // client's own customization choices aren't converted to vanilla appearance bytes yet.
    uint8 skin = 0, face = 0, hairStyle = 0, hairColor = 0, facialHair = 0;
    Player::SelectRandomAppearance(packet.Race, packet.Gender, hairStyle, hairColor, face, facialHair, skin);
    if (!Player::ValidateAppearance(packet.Race, packet.Gender, hairStyle, hairColor, face, facialHair, skin))
    {
        sendResponse(CLASSIC_CHAR_CREATE_FAILED);
        return;
    }

    std::string safeName = packet.Name;
    if (!normalizePlayerName(safeName))
    {
        sendResponse(CLASSIC_CHAR_NAME_NO_NAME);
        return;
    }

    uint8 const nameResult = ObjectMgr::CheckPlayerName(safeName, true);
    if (nameResult != CHAR_NAME_SUCCESS)
    {
        sendResponse(ClassicNameResult(nameResult));
        return;
    }

    if (GetSecurity() == SEC_PLAYER && sObjectMgr.IsReservedName(safeName))
    {
        sendResponse(CLASSIC_CHAR_NAME_FAILURE);
        return;
    }

    if (sObjectMgr.GetPlayerGuidByName(safeName))
    {
        sendResponse(CLASSIC_CHAR_CREATE_NAME_IN_USE);
        return;
    }

    // The account's live character list, straight from the cache rather than m_charactersCount:
    // that counter only gets a real value once CMSG_ENUM_CHARACTERS runs (it starts at UINT32_MAX
    // precisely so a create can't race ahead of it), and nothing requires the classic client to
    // enumerate before it creates.
    std::vector<PlayerCacheData const*> accountCharacters;
    sObjectMgr.GetPlayerDataForAccount(GetAccountId(), accountCharacters);

    if (accountCharacters.size() >= sWorld.getConfig(CONFIG_UINT32_CHARACTERS_PER_REALM))
    {
        sendResponse(CLASSIC_CHAR_CREATE_SERVER_LIMIT);
        return;
    }

    bool const allowTwoSideAccounts = !sWorld.IsPvPRealm() || sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_ACCOUNTS) || GetSecurity() > SEC_PLAYER;
    if (!allowTwoSideAccounts && !accountCharacters.empty())
    {
        PlayerCacheData const* cData = accountCharacters.front();
        if (cData->uiRace == 0 || Player::TeamForRace(cData->uiRace) != Player::TeamForRace(packet.Race))
        {
            sendResponse(CLASSIC_CHAR_CREATE_FAILED);
            return;
        }
    }

    uint32 const guidLow = sObjectMgr.GeneratePlayerLowGuid();
    if (Player::SaveNewPlayer(this, guidLow, safeName, packet.Race, packet.Class, packet.Gender, skin, face, hairStyle, hairColor, facialHair))
    {
        uint32 const newCount = uint32(accountCharacters.size()) + 1;
        m_charactersCount = newCount;
        LoginDatabase.PExecute("REPLACE INTO `realmcharacters` (`numchars`, `acctid`, `realmid`) VALUES (%u, %u, %u)", newCount, GetAccountId(), realmID);
        sendResponse(CLASSIC_CHAR_CREATE_SUCCESS, ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, guidLow), realmID));
    }
    else
    {
        sendResponse(CLASSIC_CHAR_CREATE_ERROR);
    }
}

void WorldSession::HandleCharDeleteOpcode(CharDelete const& packet)
{
    auto sendResponse = [this](uint8 result)
    {
        CharDeleteResponse response;
        response.Result = result;
        SendClassic(response);
    };

    ObjectGuid guid;
    PlayerCacheData const* cacheData = FindOwnedCharacter(*this, packet.Guid, guid);
    if (!cacheData)
    {
        sendResponse(CLASSIC_CHAR_DELETE_FAILED);
        return;
    }

    // Can't delete a loaded character, or a guild leader; matches the vanilla handler's own
    // silent-ignore and failure responses for the same two cases.
    if (ObjectAccessor::FindPlayerNotInWorld(guid))
        return;

    if (sGuildMgr.GetGuildByLeader(guid))
    {
        sendResponse(CLASSIC_CHAR_DELETE_FAILED);
        return;
    }

    sLog.Player(this, LOG_CHAR, "Delete", LOG_LVL_BASIC, "Character %s guid %u", cacheData->sName.c_str(), guid.GetCounter());

    if (Player* onlinePlayer = sObjectAccessor.FindPlayer(guid))
        onlinePlayer->GetSession()->LogoutPlayer(true);

    Player::DeleteFromDB(guid, GetAccountId());

    sendResponse(CLASSIC_CHAR_DELETE_SUCCESS);
}

void WorldSession::HandlePlayerLoginOpcode(PlayerLogin const& packet)
{
    auto sendFailed = [this](uint8 reason)
    {
        CharacterLoginFailed failed;
        failed.Reason = reason;
        SendClassic(failed);
    };

    if ((!sWorld.getConfig(CONFIG_BOOL_WORLD_AVAILABLE) && GetSecurity() == SEC_PLAYER) || PlayerLoading() || GetPlayer() != nullptr)
    {
        sendFailed(1); // NoWorld
        return;
    }

    ObjectGuid guid;
    if (!FindOwnedCharacter(*this, packet.Guid, guid))
    {
        sendFailed(1); // NoWorld
        return;
    }

    // The redirect to the Instance connection starts in parallel with the player load, not after
    // it: a slow load shouldn't hold up the client's own retries.
    SendConnectToInstance(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1);
    LoginPlayer(guid);
}

void WorldSession::HandleClassicConnectToFailedOpcode(ConnectToFailed const& packet)
{
    if (!m_awaitingInstanceConnect || packet.Serial != m_pendingInstanceConnectSerial)
        return; // stale or unexpected; the pending attempt (if any) is what still matters

    uint32 nextSerial = 0;
    switch (packet.Serial)
    {
        case CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1: nextSerial = CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2; break;
        case CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2: nextSerial = CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3; break;
        case CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3: nextSerial = CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4; break;
        case CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4: nextSerial = CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5; break;
        default: break; // attempt 5 (or unknown) exhausts the retries
    }

    if (nextSerial == 0)
    {
        m_awaitingInstanceConnect = false;
        ClearPendingInstanceConnect();
        CharacterLoginFailed failed;
        failed.Reason = 1; // NoWorld
        SendClassic(failed);
        return;
    }

    SendConnectToInstance(nextSerial);
}
