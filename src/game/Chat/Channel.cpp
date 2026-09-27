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

#include "Channel.h"
#include "ObjectMgr.h"
#include "World.h"
#include "SocialMgr.h"
#include "MasterPlayer.h"
#include "Chat.h"
#include "Packets/ChannelMessages.h"
#include "Packets/Classic/GuidMapping.h"

// The modern client associates a channel by a synthetic guid keyed on the joining player's
// current map and zone (like a creature's guid, with the channel's own dbc id -- or 0 for a
// custom channel -- standing in for the counter). Only meaningful for a real in-world player.
static ClassicGuid MakeChannelGuid(Player* player, uint32 channelId)
{
    return ClassicGuidMapping::MakeChannel(realmID, player->GetMapId(), player->GetZoneId(), channelId);
}

Channel::Channel(std::string const& name)
    : m_area_dependant(true), m_announce(true), m_levelRestricted(true),
      m_name(name), m_flags(0), m_securityLevel(0), m_channelId(0)
{
    // set special flags if built-in channel
    ChatChannelsEntry const* ch = GetChannelEntryFor(name);
    if (ch)                                                 // it's built-in channel
    {
        m_channelId = ch->ChannelID;                        // only built-in channel have channel id != 0
        m_announce = false;                                 // no join/leave announces

        // Wire ChannelFlags == ChatChannels.dbc Flags column; forward the relevant bits directly
        // instead of re-deriving a vanilla-scheme value (see the header comment).
        m_flags = ch->flags & (CHANNEL_DBC_FLAG_AUTO_JOIN | CHANNEL_DBC_FLAG_ZONE_BASED | CHANNEL_DBC_FLAG_READ_ONLY |
                                CHANNEL_DBC_FLAG_ALLOW_ITEM_LINKS | CHANNEL_DBC_FLAG_ONLY_IN_CITIES | CHANNEL_DBC_FLAG_LINKED_CHANNEL |
                                CHANNEL_DBC_FLAG_ZONE_ATTACK_ALERTS | CHANNEL_DBC_FLAG_GUILD_RECRUITMENT);

        m_generalModerated = true;                          // for all built-in channels

        if (ch->flags & CHANNEL_DBC_FLAG_ZONE_BASED)        // Determine if handled in Master or Node
            m_area_dependant = true;

        m_isLFGChannel = (ch->flags & CHANNEL_DBC_FLAG_LFG) != 0;

        // old code's `GetFlags() == 0x38` (CITY|GENERAL|NOT_LFG) gate for skipping guild-recruitment
        // auto-join, expressed independently of the wire m_flags value
        m_skipGuildAutoJoin = (ch->flags & CHANNEL_DBC_FLAG_LINKED_CHANNEL) && !m_isLFGChannel;
    }
    else                                                    // it's custom channel
    {
        if (!normalizePlayerName(m_name, 128)) // Capitalizes the first letter
        {
            m_name = "INVALIDCHANNEL";
            m_announce = false;
        }

        if (m_name == "World")
        {
            m_generalModerated = true;
            m_announce = false;
        }
        else if (m_name == "China" || m_name == "\xE4\xB8\xAD\xE5\x9B\xBD") // The hex-sequence is the utf8 version of "China" in Mandarin
        {
            m_allowOwnership = true;
            m_announce = false;
        }
        else
        {
            m_allowOwnership = true;
            m_levelRestricted = false;
        }
    }
}

void Channel::Join(ObjectGuid guid, char const* password)
{
    WorldPacket data;
    if (IsOn(guid))
    {
        if (!IsConstant())                                  // non send error message for built-in channels
        {
            MakePlayerAlreadyMember(&data, guid);
            SendToOne(&data, guid);
        }
        return;
    }

    if (IsBanned(guid))
    {
        MakeBanned(&data);
        SendToOne(&data, guid);
        return;
    }

    if (m_password.length() > 0 && strcmp(password, m_password.c_str()) != 0)
    {
        MakeWrongPassword(&data);
        SendToOne(&data, guid);
        return;
    }

    PlayerPointer pPlayer = GetPlayer(guid);

    if (m_securityLevel && (!pPlayer.get() || pPlayer->GetSession()->GetSecurity() < m_securityLevel))
    {
        MakeWrongPassword(&data);
        SendToOne(&data, guid);
        return;
    }

    if (pPlayer && pPlayer->ToPlayer())
    {
        if (pPlayer->GetGuildId() && m_skipGuildAutoJoin)
            return;

        pPlayer->ToPlayer()->JoinedChannel(this);
    }

    if (m_announce && (!pPlayer.get() || pPlayer->GetSession()->GetSecurity() < SEC_GAMEMASTER || !sWorld.getConfig(CONFIG_BOOL_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        MakeJoined(&data, guid);
        SendToAll(&data);
    }

    data.clear();

    PlayerInfo& pinfo = m_players[guid];
    pinfo.player = guid;
    pinfo.flags = MEMBER_FLAG_NONE;

    // A real player gets the dedicated join notice (carries the channel guid the client keys its
    // UI and future chat lines on); a MasterPlayer falls back to the legacy generic notify.
    if (pPlayer && pPlayer->ToPlayer())
    {
        ChannelNotifyJoined joined;
        joined.ChannelName = m_name;
        joined.ChannelFlags = GetFlags();
        joined.ChatChannelId = int32(m_channelId);
        joined.ChannelGuid = MakeChannelGuid(pPlayer->ToPlayer(), m_channelId);
        pPlayer->GetSession()->SendClassic(joined);
    }
    else
    {
        MakeYouJoined(&data);
        SendToOne(&data, guid);
    }

    JoinNotify(guid);

    // if no owner first logged will become
    if (m_allowOwnership && !m_ownerGuid)
    {
        SetOwner(guid, (m_players.size() > 1));
        m_players[guid].SetModerator(true);
    }
}

void Channel::Leave(ObjectGuid guid, bool send)
{
    if (!IsOn(guid))
    {
        if (send)
        {
            WorldPacket data;
            MakeNotMember(&data);
            SendToOne(&data, guid);
        }
        return;
    }

    PlayerPointer pPlayer = GetPlayer(guid);

    // leave channel
    if (send)
    {
        // A real player gets the dedicated leave notice; a MasterPlayer falls back to the legacy
        // generic notify.
        if (pPlayer && pPlayer->ToPlayer())
        {
            ChannelNotifyLeft left;
            left.ChannelName = m_name;
            left.ChatChannelId = int32(m_channelId);
            left.Suspended = false;
            pPlayer->GetSession()->SendClassic(left);
        }
        else
        {
            WorldPacket data;
            MakeYouLeft(&data);
            SendToOne(&data, guid);
        }
        pPlayer->LeftChannel(this);
    }

    bool changeowner = m_players[guid].IsOwner();

    m_players.erase(guid);
    if (m_announce && (!pPlayer.get() || pPlayer->GetSession()->GetSecurity() < SEC_GAMEMASTER || !sWorld.getConfig(CONFIG_BOOL_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        WorldPacket data;
        MakeLeft(&data, guid);
        SendToAll(&data);
    }

    LeaveNotify(guid);

    if (changeowner)
    {
        ObjectGuid newowner = !m_players.empty() ? m_players.begin()->second.player : ObjectGuid();
        SetOwner(newowner);
    }
}

void Channel::KickOrBan(ObjectGuid guid, char const* targetName, bool ban)
{
    AccountTypes sec = SEC_PLAYER;
    PlayerPointer gplr = GetPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (!m_players[guid].IsModerator() && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    Player* pTarget = sObjectMgr.GetPlayer(targetName);
    if (!pTarget)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    ObjectGuid targetGuid = pTarget->GetObjectGuid();
    if (!IsOn(targetGuid))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    bool changeowner = (m_ownerGuid == targetGuid);

    if (sec < SEC_GAMEMASTER && changeowner && guid != m_ownerGuid)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, guid);
        return;
    }

    // kick or ban player
    WorldPacket data;

    if (ban && !IsBanned(targetGuid))
    {
        m_banned.insert(targetGuid);
        MakePlayerBanned(&data, targetGuid, guid);
    }
    else
        MakePlayerKicked(&data, targetGuid, guid);

    SendToAll(&data);
    m_players.erase(targetGuid);
    pTarget->LeftChannel(this);

    if (changeowner)
    {
        ObjectGuid newowner = !m_players.empty() ? guid : ObjectGuid();
        SetOwner(newowner);
    }
}

void Channel::UnBan(ObjectGuid guid, char const* targetName)
{
    uint32 sec = 0;
    PlayerPointer gplr = GetPlayer(guid);
    if (gplr)
        sec = gplr->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (!m_players[guid].IsModerator() && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    Player* pTarget = sObjectMgr.GetPlayer(targetName);
    if (!pTarget)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    ObjectGuid targetGuid = pTarget->GetObjectGuid();
    if (!IsBanned(targetGuid))
    {
        WorldPacket data;
        MakePlayerNotBanned(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    // unban player
    m_banned.erase(targetGuid);

    WorldPacket data;
    MakePlayerUnbanned(&data, targetGuid, guid);
    SendToAll(&data);
}

void Channel::Password(ObjectGuid guid, char const* password)
{
    uint32 sec = 0;
    PlayerPointer pPlayer = GetPlayer(guid);
    if (pPlayer)
        sec = pPlayer->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (!m_players[guid].IsModerator() && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    // set channel password
    m_password = password;

    WorldPacket data;
    MakePasswordChanged(&data, guid);
    SendToAll(&data);
}

void Channel::SetMode(ObjectGuid guid, char const* targetName, bool moderator, bool set)
{
    PlayerPointer pPlayer = GetPlayer(guid);
    if (!pPlayer.get())
        return;

    uint32 sec = pPlayer->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }
    if (!m_players[guid].IsModerator() && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    PlayerPointer pTarget = GetPlayer(targetName);
    if (!pTarget.get())
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    ObjectGuid targetGuid = pTarget->GetObjectGuid();
    if (moderator && guid == m_ownerGuid && targetGuid == m_ownerGuid)
        return;

    if (!IsOn(targetGuid))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    // allow make moderator from another team only if both are GMs
    // at this moment this only way to show channel post for GM from another team
    if ((pPlayer->GetSession()->GetSecurity() < SEC_GAMEMASTER || pTarget->GetSession()->GetSecurity() < SEC_GAMEMASTER) &&
        pPlayer->GetTeam() != pTarget->GetTeam() && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    if (m_ownerGuid == targetGuid && m_ownerGuid != guid)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, guid);
        return;
    }

    // set channel moderator
    if (moderator)
    {
        if (m_generalModerated && pTarget->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            return;

        SetModerator(targetGuid, set);
    }
    else
        SetMute(targetGuid, set);
}

void Channel::SetOwner(ObjectGuid guid, char const* targetName)
{
    PlayerPointer pPlayer = GetPlayer(guid);
    if (!pPlayer.get())
        return;

    uint32 sec = pPlayer->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (sec < SEC_GAMEMASTER && guid != m_ownerGuid)
    {
        WorldPacket data;
        MakeNotOwner(&data);
        SendToOne(&data, guid);
        return;
    }

    Player* pTarget = sObjectMgr.GetPlayer(targetName);
    if (!pTarget)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    ObjectGuid targetGuid = pTarget->GetObjectGuid();
    if (!IsOn(targetGuid))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    if (pTarget->GetTeam() != pPlayer->GetTeam() && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    if (m_generalModerated && pTarget->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        return;

    // set channel owner
    m_players[targetGuid].SetModerator(true);
    SetOwner(targetGuid);
}

void Channel::SendWhoOwner(ObjectGuid guid)
{
    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    // send channel owner
    WorldPacket data;
    MakeChannelOwner(&data);
    SendToOne(&data, guid);
}

void Channel::List(PlayerPointer player)
{
    ObjectGuid guid = player->GetObjectGuid();

    MasterPlayer* masterPlayer = player->ToMasterPlayer();
    Player* asPlayer = player->ToPlayer();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    ChannelListResponse list;
    list.Display = true; // a real reference server always sets this
    list.ChannelName = GetName();
    list.ChannelFlags = GetFlags();

    AccountTypes gmLevelInWhoList = (AccountTypes)sWorld.getConfig(CONFIG_UINT32_GM_LEVEL_IN_WHO_LIST);

    for (const auto& itr : m_players)
    {
        if (masterPlayer)
        {
            MasterPlayer* pPlayer = sObjectAccessor.FindMasterPlayer(itr.first);
            // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
            // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
            if (pPlayer && (masterPlayer->GetSession()->GetSecurity() > SEC_PLAYER || pPlayer->GetSession()->GetSecurity() <= gmLevelInWhoList) &&
                pPlayer->IsVisibleGloballyFor(masterPlayer))
            {
                list.Members.push_back({ ClassicGuidMapping::ToClassic(itr.first, realmID), 0, itr.second.flags });
            }
        }
        else
        {
            Player* pPlayer = sObjectMgr.GetPlayer(itr.first);

            // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters
            // MODERATOR, GAME MASTER, ADMINISTRATOR can see all
            if (pPlayer && (asPlayer->GetSession()->GetSecurity() > SEC_PLAYER || pPlayer->GetSession()->GetSecurity() <= gmLevelInWhoList) &&
                pPlayer->IsVisibleGloballyFor(asPlayer))
            {
                list.Members.push_back({ ClassicGuidMapping::ToClassic(itr.first, realmID), 0, itr.second.flags });
            }
        }
    }

    player->GetSession()->SendClassic(list);
}

void Channel::Announce(ObjectGuid guid)
{
    uint32 sec = 0;
    PlayerPointer pPlayer = GetPlayer(guid);
    if (pPlayer)
        sec = pPlayer->GetSession()->GetSecurity();

    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    if (!m_players[guid].IsModerator() && sec < SEC_GAMEMASTER)
    {
        WorldPacket data;
        MakeNotModerator(&data);
        SendToOne(&data, guid);
        return;
    }

    // toggle channel announcement
    m_announce = !m_announce;

    WorldPacket data;
    if (m_announce)
        MakeAnnouncementsOn(&data, guid);
    else
        MakeAnnouncementsOff(&data, guid);

    SendToAll(&data);
}

void Channel::Say(ObjectGuid guid, char const* text, uint32 lang, bool skipCheck)
{
    if (!text)
        return;

    PlayerPointer pPlayer = GetPlayer(guid);
    uint32 const sec = pPlayer ? pPlayer->GetSession()->GetSecurity() : 0;
    uint8  const honor_rank = pPlayer ? pPlayer->ToPlayer()->GetHonorMgr().GetCurrentHonorRank() : 0;

    if (!skipCheck)
    {
        if (!IsOn(guid))
        {
            WorldPacket data;
            MakeNotMember(&data);
            SendToOne(&data, guid);
            return;
        }

        if (m_players[guid].IsMuted() || ((GetChannelId() == CHANNEL_ID_WORLD_DEFENSE) && (honor_rank < 15)))
        {
            WorldPacket data;
            MakeMuted(&data);
            SendToOne(&data, guid);
            return;
        }
    }

    // send channel message
    if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    WorldPacket data;
    // Pass the member's name as senderName -- see BuildChatPacket: the modern client suppresses
    // chat lines whose sender name it cannot resolve from its own (empty on fresh login) name
    // cache. Also pass the channel guid (the same value the join notice used): the modern client
    // associates a CHAT_MSG_CHANNEL line with the channel by that guid and drops it otherwise.
    ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, text, Language(lang), pPlayer ? pPlayer->GetChatTag() : 0, guid, pPlayer ? pPlayer->GetName() : nullptr, ObjectGuid(), "", m_name.c_str(), honor_rank,
        (pPlayer && pPlayer->ToPlayer()) ? MakeChannelGuid(pPlayer->ToPlayer(), m_channelId) : ClassicGuid());

    if (!skipCheck && pPlayer &&
        pPlayer->GetSession()->GetAccountFlags() & ACCOUNT_FLAG_MUTED_FROM_PUBLIC_CHANNELS &&
        pPlayer->GetSession()->GetAccountMaxLevel() < sWorld.getConfig(CONFIG_UINT32_PUB_CHANS_MUTE_VANISH_LEVEL))
        pPlayer->GetSession()->SendPacket(&data);
    else
        SendToAll(&data, (!skipCheck && !m_players[guid].IsModerator()) ? guid : ObjectGuid());
}

void Channel::Invite(ObjectGuid guid, char const* targetName)
{
    if (!IsOn(guid))
    {
        WorldPacket data;
        MakeNotMember(&data);
        SendToOne(&data, guid);
        return;
    }

    PlayerPointer pTarget = GetPlayer(targetName);
    if (!pTarget)
    {
        WorldPacket data;
        MakePlayerNotFound(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    ObjectGuid targetGuid = pTarget->GetObjectGuid();
    if (IsOn(targetGuid))
    {
        WorldPacket data;
        MakePlayerAlreadyMember(&data, targetGuid);
        SendToOne(&data, guid);
        return;
    }

    if (IsBanned(targetGuid))
    {
        WorldPacket data;
        MakePlayerInviteBanned(&data, targetName);
        SendToOne(&data, guid);
        return;
    }

    PlayerPointer pPlayer = GetPlayer(guid);
    if (!pPlayer.get())
        return;

    if (pTarget->GetTeam() != pPlayer->GetTeam() && !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        WorldPacket data;
        MakeInviteWrongFaction(&data);
        SendToOne(&data, guid);
        return;
    }

    // invite player
    WorldPacket data;
    if (!pTarget->GetSocial()->HasIgnore(guid))
    {
        MakeInvite(&data, guid);
        SendToOne(&data, targetGuid);
        data.clear();
    }

    MakePlayerInvited(&data, pTarget->GetName());
    SendToOne(&data, guid);
}

void Channel::SetOwner(ObjectGuid guid, bool exclaim)
{
    PlayerPointer pPlayer = GetPlayer(guid);
    if (!pPlayer)
        return;

    if (m_generalModerated && pPlayer->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        return;

    if (m_ownerGuid)
    {
        // [] will re-add player after it possible removed
        PlayerList::iterator p_itr = m_players.find(m_ownerGuid);
        if (p_itr != m_players.end())
            p_itr->second.SetOwner(false);
    }

    m_ownerGuid = guid;

    if (m_ownerGuid)
    {
        uint8 oldFlag = GetPlayerFlags(m_ownerGuid);
        m_players[m_ownerGuid].SetOwner(true);

        WorldPacket data;
        MakeModeChange(&data, m_ownerGuid, oldFlag);
        SendToAll(&data);

        if (exclaim)
        {
            MakeOwnerChanged(&data, m_ownerGuid);
            SendToAll(&data);
        }
    }
}

void Channel::SendToAll(WorldPacket* data, ObjectGuid guid)
{
    for (const auto& itr : m_players)
    {
        if (PlayerPointer pPlayer = GetPlayer(itr.first))
            if (!pPlayer->GetSocial()->HasIgnore(guid))
                pPlayer->GetSession()->SendPacket(data);
    }
}

void Channel::SendToOne(WorldPacket* data, ObjectGuid who)
{
    PlayerPointer pPlayer = GetPlayer(who);
    if (pPlayer)
        pPlayer->GetSession()->SendPacket(data);
}

void Channel::MakeNotifyPacket(WorldPacket* data, uint8 notify_type, ObjectGuid senderGuid, ObjectGuid targetGuid, std::string const& sender, uint8 oldFlags, uint8 newFlags)
{
    ChannelNotify notify;
    notify.Type = notify_type;
    notify.ChannelName = m_name;
    notify.Sender = sender;
    notify.SenderGuid = ClassicGuidMapping::ToClassic(senderGuid, realmID);
    notify.TargetGuid = ClassicGuidMapping::ToClassic(targetGuid, realmID);
    notify.ChatChannelId = int32(m_channelId);
    notify.OldFlags = oldFlags;
    notify.NewFlags = newFlags;

    data->Initialize(CLASSIC_SMSG_CHANNEL_NOTIFY);
    notify.Write(*data);
}

void Channel::MakeJoined(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_JOINED_NOTICE, guid);
}

void Channel::MakeLeft(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_LEFT_NOTICE, guid);
}

void Channel::MakeYouJoined(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_YOU_JOINED_NOTICE);
}

void Channel::MakeYouLeft(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_YOU_LEFT_NOTICE);
}

void Channel::MakeWrongPassword(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_WRONG_PASSWORD_NOTICE);
}

void Channel::MakeNotMember(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_MEMBER_NOTICE);
}

void Channel::MakeNotOnPacket(WorldPacket* data, std::string const& name)
{
    ChannelNotify notify;
    notify.Type = CHAT_NOT_MEMBER_NOTICE;
    notify.ChannelName = name;

    data->Initialize(CLASSIC_SMSG_CHANNEL_NOTIFY);
    notify.Write(*data);
}

void Channel::MakeNotModerator(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_MODERATOR_NOTICE);
}

void Channel::MakePasswordChanged(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_PASSWORD_CHANGED_NOTICE, guid);
}

void Channel::MakeOwnerChanged(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_OWNER_CHANGED_NOTICE, guid);
}

void Channel::MakePlayerNotFound(WorldPacket* data, std::string const& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_FOUND_NOTICE, ObjectGuid(), ObjectGuid(), name);
}

void Channel::MakeNotOwner(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_NOT_OWNER_NOTICE);
}

void Channel::MakeChannelOwner(WorldPacket* data)
{
    std::string name;

    if (!sObjectMgr.GetPlayerNameByGUID(m_ownerGuid, name) || name.empty())
        name = "PLAYER_NOT_FOUND";

    MakeNotifyPacket(data, CHAT_CHANNEL_OWNER_NOTICE, ObjectGuid(), ObjectGuid(), (IsConstant() || !m_ownerGuid) ? "Nobody" : name);
}

void Channel::MakeModeChange(WorldPacket* data, ObjectGuid guid, uint8 oldflags)
{
    MakeNotifyPacket(data, CHAT_MODE_CHANGE_NOTICE, guid, ObjectGuid(), "", oldflags, GetPlayerFlags(guid));
}

void Channel::MakeAnnouncementsOn(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_ON_NOTICE, guid);
}

void Channel::MakeAnnouncementsOff(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_ANNOUNCEMENTS_OFF_NOTICE, guid);
}

void Channel::MakeMuted(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_MUTED_NOTICE);
}

void Channel::MakePlayerKicked(WorldPacket* data, ObjectGuid target, ObjectGuid source)
{
    MakeNotifyPacket(data, CHAT_PLAYER_KICKED_NOTICE, source, target);
}

void Channel::MakeBanned(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_BANNED_NOTICE);
}

void Channel::MakePlayerBanned(WorldPacket* data, ObjectGuid target, ObjectGuid source)
{
    MakeNotifyPacket(data, CHAT_PLAYER_BANNED_NOTICE, source, target);
}

void Channel::MakePlayerUnbanned(WorldPacket* data, ObjectGuid target, ObjectGuid source)
{
    MakeNotifyPacket(data, CHAT_PLAYER_UNBANNED_NOTICE, source, target);
}

void Channel::MakePlayerNotBanned(WorldPacket* data, std::string const& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_NOT_BANNED_NOTICE, ObjectGuid(), ObjectGuid(), name);
}

void Channel::MakePlayerAlreadyMember(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_PLAYER_ALREADY_MEMBER_NOTICE, guid);
}

void Channel::MakeInvite(WorldPacket* data, ObjectGuid guid)
{
    MakeNotifyPacket(data, CHAT_INVITE_NOTICE, guid);
}

void Channel::MakeInviteWrongFaction(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_INVITE_WRONG_FACTION_NOTICE);
}

void Channel::MakeWrongFaction(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_WRONG_FACTION_NOTICE);
}

void Channel::MakeInvalidName(WorldPacket* data)
{
    MakeNotifyPacket(data, CHAT_INVALID_NAME_NOTICE);
}

void Channel::MakePlayerInvited(WorldPacket* data, std::string const& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITED_NOTICE, ObjectGuid(), ObjectGuid(), name);
}

void Channel::MakePlayerInviteBanned(WorldPacket* data, std::string const& name)
{
    MakeNotifyPacket(data, CHAT_PLAYER_INVITE_BANNED_NOTICE, ObjectGuid(), ObjectGuid(), name);
}

void Channel::JoinNotify(ObjectGuid guid)
{
    // Incremental roster update, mirroring TrinityCore's own JoinNotify()/LeaveNotify(). Built-in
    // channels announce the newcomer to everyone else via USERLIST_ADD; custom channels broadcast
    // a USERLIST_UPDATE (same packet, also reaching the newcomer so its own roster includes itself).
    if (IsConstant())
    {
        UserlistAdd userlistAdd;
        userlistAdd.AddedUserGuid = ClassicGuidMapping::ToClassic(guid, realmID);
        userlistAdd.ChannelFlags = GetFlags();
        userlistAdd.UserFlags = GetPlayerFlags(guid);
        userlistAdd.ChannelId = m_channelId;
        userlistAdd.ChannelName = m_name;

        WorldPacket data = MakeClassicPacket(userlistAdd);
        for (const auto& itr : m_players)
        {
            if (itr.first == guid)                      // everyone except the newcomer
                continue;
            if (PlayerPointer pPlayer = GetPlayer(itr.first))
                pPlayer->GetSession()->SendPacket(&data);
        }
    }
    else
    {
        UserlistUpdate userlistUpdate;
        userlistUpdate.UpdatedUserGuid = ClassicGuidMapping::ToClassic(guid, realmID);
        userlistUpdate.ChannelFlags = GetFlags();
        userlistUpdate.UserFlags = GetPlayerFlags(guid);
        userlistUpdate.ChannelId = m_channelId;
        userlistUpdate.ChannelName = m_name;

        WorldPacket data = MakeClassicPacket(userlistUpdate);
        for (const auto& itr : m_players)
        {
            if (PlayerPointer pPlayer = GetPlayer(itr.first))
                pPlayer->GetSession()->SendPacket(&data);
        }
    }
}

void Channel::LeaveNotify(ObjectGuid guid)
{
    // The leaver is already erased from m_players by the caller (Channel::Leave), so this reaches
    // exactly the remaining members -- the leaver's own client drops the channel on the
    // SMSG_CHANNEL_NOTIFY_LEFT it got before the erase.
    UserlistRemove userlistRemove;
    userlistRemove.RemovedUserGuid = ClassicGuidMapping::ToClassic(guid, realmID);
    userlistRemove.ChannelFlags = GetFlags();
    userlistRemove.ChannelId = m_channelId;
    userlistRemove.ChannelName = m_name;

    WorldPacket data = MakeClassicPacket(userlistRemove);
    for (const auto& itr : m_players)
    {
        if (PlayerPointer pPlayer = GetPlayer(itr.first))
            pPlayer->GetSession()->SendPacket(&data);
    }
}

PlayerPointer Channel::GetPlayer(ObjectGuid guid)
{
    if (m_area_dependant)
    {
        Player* pPlayer = sObjectAccessor.FindPlayerNotInWorld(guid);
        return PlayerPointer(pPlayer ? new PlayerWrapper<Player>(pPlayer) : nullptr);
    }
    MasterPlayer* pPlayer = sObjectAccessor.FindMasterPlayer(guid);
    return PlayerPointer(pPlayer ? new PlayerWrapper<MasterPlayer>(pPlayer) : nullptr);
}

PlayerPointer Channel::GetPlayer(char const* name)
{
    if (m_area_dependant)
    {
        Player* pPlayer = sObjectAccessor.FindPlayerByNameNotInWorld(name);
        return PlayerPointer(pPlayer ? new PlayerWrapper<Player>(pPlayer) : nullptr);
    }
    MasterPlayer* pPlayer = sObjectAccessor.FindMasterPlayer(name);
    return PlayerPointer(pPlayer ? new PlayerWrapper<MasterPlayer>(pPlayer) : nullptr);
}
