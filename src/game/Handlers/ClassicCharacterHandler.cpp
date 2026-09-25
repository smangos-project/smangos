/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 * Copyright (C) 2017-2024 VMaNGOS Project <https://github.com/vmangos>
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
#include "WorldSession.h"
#include "SharedDefines.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "Packets/Classic/GuidMapping.h"

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
    // HandleClassicCharCreateOpcode/HandleClassicCharDeleteOpcode update synchronously, so a create
    // or delete is always reflected in the very next enum -- a DB round trip on a worker thread can
    // lag behind the transaction that made the change.
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

void WorldSession::HandleClassicCharCreateOpcode(ClassicCreateCharacter const& packet)
{
    auto sendResponse = [this](uint8 result, ClassicGuid const& guid = ClassicGuid{})
    {
        ClassicCreateCharResponse response;
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

    // The bots never send customization choices (ClassicCreateCharacter::CustomizationCount is
    // always 0), which maps to a random valid vanilla appearance rather than always the same one --
    // a real client's own customization choices aren't converted to vanilla appearance bytes yet.
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

void WorldSession::HandleClassicCharDeleteOpcode(ClassicCharDelete const& packet)
{
    auto sendResponse = [this](uint8 result)
    {
        ClassicDeleteCharResponse response;
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

void WorldSession::HandleClassicPlayerLoginOpcode(ClassicPlayerLogin const& packet)
{
    auto sendFailed = [this](uint8 reason)
    {
        ClassicCharacterLoginFailed failed;
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

    // World entry (SMSG_CONNECT_TO, the Instance connection, SMSG_LOGIN_VERIFY_WORLD) is
    // port-plan.md step 5, not built yet -- a valid login is acknowledged with nothing further.
    sLog.Out(LOG_NETWORK, LOG_LVL_DEBUG, "[%s] WorldSession: CMSG_PLAYER_LOGIN for guid %u validated; world entry not ported yet",
        GetRemoteAddress().c_str(), guid.GetCounter());
}
