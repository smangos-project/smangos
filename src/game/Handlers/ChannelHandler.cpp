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

#include "ObjectMgr.h"                                      // for normalizePlayerName
#include "ChannelMgr.h"
#include "Chat.h"
#include "World.h"

void WorldSession::HandleJoinChannelOpcode(JoinChannel const& packet)
{
    // Channel name must begin with a letter.
    if (packet.ChannelName.empty() || (uint8(packet.ChannelName[0]) <= 127 && !isalpha(packet.ChannelName[0])))
    {
        ChannelNotify notify;
        notify.Type = CHAT_INVALID_NAME_NOTICE;
        notify.ChannelName = packet.ChannelName;
        SendClassic(notify);
        return;
    }

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetJoinChannel(packet.ChannelName))
            chn->Join(player->GetObjectGuid(), packet.Password.c_str());
    }

    if (player->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_JOIN_OPPOSITE_FACTION_CHANNELS))
        if (ChannelMgr* cMgr = channelMgr(_player->GetTeam() == ALLIANCE ? HORDE : ALLIANCE))
            if (Channel *chn = cMgr->GetJoinChannel(packet.ChannelName))
                if (!chn->GetSecurityLevel()) // Special both factions channel
                    chn->Join(player->GetObjectGuid(), packet.Password.c_str());
}

void WorldSession::HandleLeaveChannelOpcode(LeaveChannel const& packet)
{
    if (packet.ChannelName.empty())
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Leave(player->GetObjectGuid(), true);

        cMgr->LeftChannel(packet.ChannelName);
    }
    if (player->GetSession()->GetSecurity() > SEC_PLAYER && sWorld.getConfig(CONFIG_BOOL_GM_JOIN_OPPOSITE_FACTION_CHANNELS))
        if (ChannelMgr* cMgr = channelMgr(player->GetTeam() == ALLIANCE ? HORDE : ALLIANCE))
        {
            if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
                chn->Leave(player->GetObjectGuid(), true);
            cMgr->LeftChannel(packet.ChannelName);
        }
}

void WorldSession::HandleChannelListOpcode(ChannelList const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->List(player);
    }
}

// CLASSIC_CMSG_CHAT_CHANNEL_DISPLAY_LIST -- TrinityCore's own channel-command handler treats
// DISPLAY_LIST identically to LIST (both list the channel's members).
void WorldSession::HandleChannelDisplayListOpcode(ChannelDisplayList const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->List(player);
    }
}

void WorldSession::HandleChannelPasswordOpcode(ChannelPassword const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Password(player->GetObjectGuid(), packet.Password.c_str());
    }
}

void WorldSession::HandleChannelSetOwnerOpcode(ChannelSetOwner const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->SetOwner(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelOwnerOpcode(ChannelOwner const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->SendWhoOwner(player->GetObjectGuid());
    }
}

void WorldSession::HandleChannelModeratorOpcode(ChannelModerator const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->SetModerator(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelUnmoderatorOpcode(ChannelUnmoderator const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->UnsetModerator(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelInviteOpcode(ChannelInvite const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (player->GetLevel() < sWorld.getConfig(CONFIG_UINT32_CHANNEL_INVITE_MIN_LEVEL))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Invite(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelKickOpcode(ChannelKick const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Kick(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelBanOpcode(ChannelBan const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Ban(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelUnbanOpcode(ChannelUnban const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->UnBan(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelAnnouncementsOpcode(ChannelAnnouncements const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->Announce(player->GetObjectGuid());
    }
}

void WorldSession::HandleChannelDeclineInviteOpcode(ChannelDeclineInvite const& packet)
{
    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->DeclineInvite(player->GetObjectGuid());
    }
}

void WorldSession::HandleChannelSilenceAllOpcode(ChannelSilenceAll const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->SilenceAll(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}

void WorldSession::HandleChannelUnsilenceAllOpcode(ChannelUnsilenceAll const& packet)
{
    if (!normalizePlayerName(const_cast<std::string&>(packet.PlayerName)))
        return;

    PlayerPointer player = GetPlayerPointer();
    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(packet.ChannelName, player))
            chn->UnsilenceAll(player->GetObjectGuid(), packet.PlayerName.c_str());
    }
}
