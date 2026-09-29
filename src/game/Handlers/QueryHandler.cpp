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
#include "Database/DatabaseImpl.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Packets/QueryMessages.h"
#include "Packets/Classic/GuidMapping.h"
#include "NPCHandler.h"

namespace
{
void FillPlayerName(PlayerNameLookup& lookup, ObjectGuid guid, uint32 accountId, std::string const& name, uint32 race, uint32 gender, uint32 playerClass, uint32 level)
{
    lookup.Player = ClassicGuidMapping::ToClassic(guid, realmID);
    lookup.Result = 0;

    PlayerNameData& data = lookup.Data.emplace();
    data.AccountId = ClassicGuidMapping::MakeAccount(accountId);
    data.BnetAccountId = ClassicGuidMapping::MakeBnetAccount(accountId);
    data.PlayerGuid = lookup.Player;
    data.Name = name;
    data.Race = uint8(race);
    data.Sex = uint8(gender);
    data.ClassId = uint8(playerClass);
    data.Level = uint8(level);
}

void FillPlayerName(PlayerNameLookup& lookup, Player const* player)
{
    FillPlayerName(lookup, player->GetObjectGuid(), player->GetSession()->GetAccountId(), player->GetName(),
        player->GetRace(), player->GetGender(), player->GetClass(), player->GetLevel());
}

// A guid the server cannot name still gets an entry, so the client stops asking.
void FillPlayerName(PlayerNameLookup& lookup, ObjectGuid guid)
{
    if (Player const* player = sObjectMgr.GetPlayer(guid))
        FillPlayerName(lookup, player);
    else if (PlayerCacheData const* cached = sObjectMgr.GetPlayerDataByGUID(guid.GetCounter()))
        FillPlayerName(lookup, guid, cached->uiAccount, cached->sName, cached->uiRace, cached->uiGender, cached->uiClass, cached->uiLevel);
    else
    {
        lookup.Player = ClassicGuidMapping::ToClassic(guid, realmID);
        lookup.Result = 1;
    }
}
}

void WorldSession::SendNameQueryOpcode(Player* p)
{
    if (!p)
        return;

    QueryPlayerNamesResponse response;
    FillPlayerName(response.Players.emplace_back(), p);
    SendClassic(response);
}

void WorldSession::HandleQueryPlayerNamesOpcode(QueryPlayerNames const& packet)
{
    QueryPlayerNamesResponse response;
    response.Players.reserve(packet.Players.size());
    for (ClassicGuid const& classicGuid : packet.Players)
    {
        PlayerNameLookup& lookup = response.Players.emplace_back();
        auto const guid = ClassicGuidMapping::FromClassic(classicGuid);
        if (!guid || !guid->IsPlayer())
        {
            lookup.Player = classicGuid;
            lookup.Result = 1;
            continue;
        }
        FillPlayerName(lookup, *guid);
    }
    SendClassic(response);
}

void WorldSession::HandleQueryTimeOpcode(QueryTime const& /*packet*/)
{
    SendQueryTimeResponse();
}

// Only _static_ data send in this packet !!!
void WorldSession::HandleCreatureQueryOpcode(QueryCreature const& packet)
{
    QueryCreatureResponse response;
    response.Entry = packet.Entry;

    CreatureInfo const* ci = sObjectMgr.GetCreatureTemplate(packet.Entry);
    if (!ci)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: CMSG_QUERY_CREATURE - Entry: %u NO CREATURE INFO!", packet.Entry);
        SendClassic(response);
        return;
    }

    response.Allow = true;
    response.Name = ci->name;
    response.Title = ci->subname;

    int const locIdx = GetSessionDbLocaleIndex();
    if (locIdx >= 0)
    {
        if (CreatureLocale const* locale = sObjectMgr.GetCreatureLocale(ci->entry))
        {
            if (locale->Name.size() > size_t(locIdx) && !locale->Name[locIdx].empty())
                response.Name = locale->Name[locIdx];
            if (locale->SubName.size() > size_t(locIdx) && !locale->SubName[locIdx].empty())
                response.Title = locale->SubName[locIdx];
        }
    }

    response.Civilian = ci->civilian;
    response.Leader = ci->racial_leader;
    response.TypeFlags = ci->GetTypeFlags();
    response.CreatureType = int32(ci->type);
    response.Family = int32(ci->pet_family);
    response.Classification = int32(ci->rank);
    response.PetSpellDataId = ci->pet_spell_list_id;
    response.UnitClass = int32(ci->unit_class);
    response.HealthMultiplier = ci->health_multiplier;
    response.EnergyMultiplier = ci->mana_multiplier;
    response.TotalProbability = ci->display_total_probability ? float(ci->display_total_probability) : 1.0f;

    for (uint32 i = 0; i < MAX_DISPLAY_IDS_PER_CREATURE; ++i)
    {
        if (!ci->display_id[i])
            continue;
        CreatureDisplayEntry& display = response.Displays.emplace_back();
        display.DisplayId = ci->display_id[i];
        display.Scale = ci->display_scale[i] > 0.0f ? ci->display_scale[i] : 1.0f;
        display.Probability = ci->display_probability[i] ? float(ci->display_probability[i]) : 1.0f;
    }
    if (response.Displays.empty())
        response.Displays.emplace_back().DisplayId = ci->display_id[0];

    SendClassic(response);
}

// Only _static_ data send in this packet !!!
void WorldSession::HandleGameObjectQueryOpcode(QueryGameObject const& packet)
{
    QueryGameObjectResponse response;
    response.Entry = packet.Entry;
    response.Guid = packet.Guid;

    GameObjectInfo const* info = sObjectMgr.GetGameObjectTemplate(packet.Entry);
    if (!info)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: CMSG_QUERY_GAME_OBJECT - Entry: %u Missing gameobject info!", packet.Entry);
        SendClassic(response);
        return;
    }

    response.Allow = true;
    response.Type = int32(info->type);
    response.DisplayId = int32(info->displayId);
    response.Name = info->name;
    response.IconName = info->icon;
    response.Size = info->size > 0.0f ? info->size : 1.0f;

    int const locIdx = GetSessionDbLocaleIndex();
    if (locIdx >= 0)
    {
        if (GameObjectLocale const* locale = sObjectMgr.GetGameObjectLocale(info->id))
            if (locale->Name.size() > size_t(locIdx) && !locale->Name[locIdx].empty())
                response.Name = locale->Name[locIdx];
    }

    static_assert(sizeof(info->raw.data) <= CLASSIC_MAX_GAMEOBJECT_DATA * sizeof(int32), "template data no longer fits the wire");
    int32 const* rawData = reinterpret_cast<int32 const*>(info->raw.data);
    for (size_t i = 0; i < sizeof(info->raw.data) / sizeof(int32); ++i)
        response.Data[i] = rawData[i];

    SendClassic(response);
}

void WorldSession::HandleCorpseQueryOpcode(QueryCorpseLocationFromClient const& /*packet*/)
{
    CorpseLocation response;
    response.Player = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);

    Corpse* corpse = GetPlayer()->GetCorpse();
    if (!corpse)
    {
        response.Valid = false;
        SendClassic(response);
        return;
    }

    uint32 corpsemapid = corpse->GetMapId();
    float x = corpse->GetPositionX();
    float y = corpse->GetPositionY();
    float z = corpse->GetPositionZ();
    int32 mapid = corpsemapid;

    // if corpse at different map
    if (corpsemapid != _player->GetMapId())
    {
        // search entrance map for proper show entrance
        if (MapEntry const* temp = sMapStorage.LookupEntry<MapEntry>(mapid))
        {
            if (temp->IsDungeon() && temp->ghostEntranceMap >= 0)
            {
                // if corpse map have entrance
                if (TerrainInfo const* entranceMap = sTerrainMgr.LoadTerrain(temp->ghostEntranceMap))
                {
                    mapid = temp->ghostEntranceMap;
                    x = temp->ghostEntranceX;
                    y = temp->ghostEntranceY;
                    z = entranceMap->GetHeightStatic(x, y, MAX_HEIGHT);
                }
            }
        }
    }

    response.Valid = true;
    response.ActualMapId = mapid;
    response.MapId = int32(corpsemapid);
    response.PositionX = x;
    response.PositionY = y;
    response.PositionZ = z;
    SendClassic(response);
}

void WorldSession::HandleNpcTextQueryOpcode(QueryNpcText const& packet)
{
    QueryNpcTextResponse response;
    response.TextId = packet.TextId;

    if (NpcText const* text = sObjectMgr.GetNpcText(packet.TextId))
    {
        response.Allow = true;
        for (uint32 i = 0; i < CLASSIC_MAX_NPC_TEXT_OPTIONS; ++i)
        {
            response.Probabilities[i] = text->Options[i].Probability;
            response.BroadcastTextIds[i] = text->Options[i].BroadcastTextID;
        }
    }

    SendClassic(response);
}

void WorldSession::HandleDbQueryBulkOpcode(DbQueryBulk const& packet)
{
    // The client resolves the broadcast text ids of gossip and creature texts by asking for the
    // rows it lacks; any other table is answered as invalid so the client uses its local data.
    // One reply per requested record, in order.
    constexpr uint32 broadcastTextHash = 0x021826BB;

    for (uint32 recordId : packet.RecordIds)
    {
        DbReply reply;
        reply.TableHash = packet.TableHash;
        reply.RecordId = recordId;
        reply.Timestamp = uint32(time(nullptr));

        if (packet.TableHash == broadcastTextHash)
        {
            if (BroadcastText const* text = sObjectMgr.GetBroadcastTextLocale(recordId))
            {
                int const locIdx = GetSessionDbLocaleIndex();
                reply.Row << text->GetText(locIdx, GENDER_MALE, true);
                reply.Row << text->GetText(locIdx, GENDER_FEMALE, true);
                reply.Row << uint32(text->entry) << uint32(text->languageId);
                reply.Row << uint32(0);                     // ConditionID
                reply.Row << uint16(0);                     // EmotesID
                reply.Row << uint8(0);                      // Flags
                reply.Row << uint32(0);                     // ChatBubbleDurationMs
                reply.Row << int32(0);                      // VoiceOverPriorityID
                reply.Row << uint32(text->soundId) << uint32(0);
                reply.Row << uint16(text->emoteId1) << uint16(text->emoteId2) << uint16(text->emoteId3);
                reply.Row << uint16(text->emoteDelay1) << uint16(text->emoteDelay2) << uint16(text->emoteDelay3);
                reply.Status = CLASSIC_DB_REPLY_VALID;
            }
        }

        SendClassic(reply);
    }
}

void WorldSession::HandlePageTextQueryOpcode(QueryPageText const& packet)
{
    // A book is a chain of pages; one response carries all of it.
    constexpr uint32 maxPages = 64;

    QueryPageTextResponse response;
    response.PageTextId = packet.PageTextId;
    response.Allow = true;

    uint32 pageID = packet.PageTextId;
    while (pageID && response.Pages.size() < maxPages)
    {
        PageTextEntry& page = response.Pages.emplace_back();
        page.Id = pageID;

        PageText const* pPage = sPageTextStore.LookupEntry<PageText>(pageID);
        if (!pPage)
        {
            page.Text = "Item page missing.";
            break;
        }

        char const* text = pPage->text;
        int const locIdx = GetSessionDbLocaleIndex();
        if (locIdx >= 0)
        {
            if (PageTextLocale const* locale = sObjectMgr.GetPageTextLocale(pageID))
                if (locale->text.size() > size_t(locIdx) && !locale->text[locIdx].empty())
                    text = locale->text[locIdx].c_str();
        }

        page.Text = text;
        if (page.Text.size() > QueryPageTextResponse::MaxTextLength)
            page.Text.resize(QueryPageTextResponse::MaxTextLength);
        page.NextPageId = pPage->next_page;
        pageID = pPage->next_page;
    }

    SendClassic(response);
}

void WorldSession::SendQueryTimeResponse()
{
    QueryTimeResponse response;
    response.CurrentTime = uint32(time(nullptr));
    SendClassic(response);
}
