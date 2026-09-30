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
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "GossipDef.h"
#include "SocialMgr.h"
#include "Language.h"
#include "Anticheat.h"
#include "Packets/GuildMessages.h"
#include "Packets/Classic/GuidMapping.h"

// CMSG_GUILD_CREATE has no modern-protocol equivalent at all: a real client only creates guilds
// through the petition/charter system (buy a charter from an NPC, collect signatures, turn it in
// at CLASSIC_CMSG_TURN_IN_PETITION) -- see PetitionMessages.h/PetitionsHandler.cpp, which already
// creates the Guild object directly on a successful turn-in.
void WorldSession::HandleGuildQueryOpcode(GuildQuery const& packet)
{
    QueryGuildInfoResponse response;
    response.GuildGuid = packet.GuildGuid;

    // Falls back to the queried player's own guild if no guild guid was supplied -- matches how
    // the client asks when inspecting a player's guild tab.
    nonstd::optional<uint32> const guildId = ClassicGuidMapping::GuildIdFromClassic(packet.GuildGuid);
    Guild* guild = guildId ? sGuildMgr.GetGuildById(*guildId) : nullptr;
    if (!guild)
    {
        nonstd::optional<ObjectGuid> const playerGuid = ClassicGuidMapping::FromClassic(packet.PlayerGuid);
        Player* target = playerGuid ? ObjectAccessor::FindPlayer(*playerGuid) : nullptr;
        if (target)
            guild = sGuildMgr.GetGuildById(target->GetGuildId());
    }

    if (guild)
    {
        response.Valid = true;
        response.ResolvedGuildGuid = ClassicGuidMapping::MakeGuild(guild->GetId());
        response.GuildId = guild->GetId();
        response.GuildName = guild->GetName();
        for (uint32 i = 0; i < guild->GetRanksSize(); ++i)
            response.RankNames.push_back(guild->GetRankName(i));
        response.EmblemStyle = guild->GetEmblemStyle();
        response.EmblemColor = guild->GetEmblemColor();
        response.BorderStyle = guild->GetBorderStyle();
        response.BorderColor = guild->GetBorderColor();
        response.BackgroundColor = guild->GetBackgroundColor();
    }

    SendClassic(response);
}

void WorldSession::HandleGuildInviteOpcode(GuildInviteByName const& packet)
{
    std::string invitedName = packet.InvitedName;
    Player* player = nullptr;
    if (normalizePlayerName(invitedName))
        player = ObjectAccessor::FindPlayerByName(invitedName.c_str());

    if (!player)
    {
        SendGuildCommandResult(GUILD_INVITE_S, invitedName, ERR_GUILD_PLAYER_NOT_FOUND_S);
        return;
    }

    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (player->GetSession()->HasTrialRestrictions())
    {
        SendNotification(LANG_RESTRICTED_ACCOUNT);
        return;
    }

    // OK result but not send invite
    if (player->GetSocial()->HasIgnore(GetPlayer()->GetObjectGuid()))
    {
        SendGuildCommandResult(GUILD_INVITE_S, invitedName, ERR_GUILD_IGNORING_YOU_S);
        return;
    }

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != GetPlayer()->GetTeam())
    {
        SendGuildCommandResult(GUILD_INVITE_S, invitedName, ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (player->GetGuildId())
    {
        SendGuildCommandResult(GUILD_INVITE_S, invitedName, ERR_ALREADY_IN_GUILD_S);
        return;
    }

    if (player->GetGuildIdInvited())
    {
        SendGuildCommandResult(GUILD_INVITE_S, invitedName, ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_INVITE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Player %s Invited %s to Join his Guild", GetPlayer()->GetName(), invitedName.c_str());

    player->SetGuildIdInvited(GetPlayer()->GetGuildId());
    // Put record into guildlog
    guild->LogGuildEvent(GUILD_EVENT_LOG_INVITE_PLAYER, GetPlayer()->GetObjectGuid(), player->GetObjectGuid());

    GuildInviteNotification invite;
    invite.InviterName = GetPlayer()->GetName();
    invite.GuildName = guild->GetName();
    invite.GuildGuid = ClassicGuidMapping::MakeGuild(guild->GetId());
    player->GetSession()->SendClassic(invite);
}

void WorldSession::HandleGuildRemoveOpcode(GuildOfficerRemoveMember const& packet)
{
    Player* player = GetPlayer();
    if (!player->IsInWorld())
        return;

    Guild* guild = sGuildMgr.GetGuildById(player->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_REMOVE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    nonstd::optional<ObjectGuid> removeeGuid = ClassicGuidMapping::FromClassic(packet.RemoveeGuid);
    MemberSlot* slot = removeeGuid ? guild->GetMemberSlot(*removeeGuid) : nullptr;
    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if (slot->RankId == GR_GUILDMASTER)
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", ERR_GUILD_LEADER_LEAVE);
        return;
    }

    // do not allow to kick player with same or higher rights
    if (GetPlayer()->GetRank() >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_QUIT_S, slot->Name, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    ObjectGuid memberGuid = slot->guid; // $slot will get removed in 'DelMember', but we need it for logging.
    std::string removeeName = slot->Name;
    // possible last member removed, do cleanup, and no need events
    if (guild->DelMember(memberGuid))
    {
        guild->Disband();
        delete guild;
        return;
    }

    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_UNINVITE_PLAYER, player->GetObjectGuid(), memberGuid);

    guild->BroadcastPlayerKickedEvent(memberGuid, removeeName, player->GetObjectGuid(), player->GetName());
}

void WorldSession::HandleGuildAcceptOpcode(AcceptGuildInvite const& /*packet*/)
{
    // packet.GuildGuid is parsed but unused: this engine tracks only one pending invite per
    // player (Player::GetGuildIdInvited), the same scope vanilla's bodyless request already had.
    Guild* guild;
    Player* player = GetPlayer();

    guild = sGuildMgr.GetGuildById(player->GetGuildIdInvited());
    if (!guild || player->GetGuildId())
        return;

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && player->GetTeam() != sObjectMgr.GetPlayerTeamByGUID(guild->GetLeaderGuid()))
        return;

    if (guild->AddMember(GetPlayer()->GetObjectGuid(), guild->GetLowestRank()) != GuildAddStatus::OK)
        return;
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_JOIN_GUILD, GetPlayer()->GetObjectGuid());

    guild->BroadcastPlayerJoinedEvent(player->GetObjectGuid(), player->GetName());

    // Matches the MOTD push already done on login (CharacterHandler.cpp) -- without this the new
    // member's own client never gets the MOTD popup until their next relog.
    GuildEventMotd motd;
    motd.MotdText = guild->GetMOTD();
    SendClassic(motd);
}

void WorldSession::HandleGuildDeclineOpcode(GuildDeclineInvitation const& /*packet*/)
{
    // packet.GuildGuid/IsAuto are parsed but unused, same scope as AcceptGuildInvite above.
    if (_player->GetGuildId() || !_player->GetGuildIdInvited())
        return;

    if (Guild* guild = sGuildMgr.GetGuildById(_player->GetGuildIdInvited()))
    {
        if (ObjectGuid inviterGuid = guild->GetGuildInviter(_player->GetObjectGuid()))
        {
            if (Player const* pInviter = ObjectAccessor::FindPlayer(inviterGuid))
            {
                GuildDeclineNotification decline;
                decline.PlayerName = _player->GetName();
                pInviter->GetSession()->SendClassic(decline);
            }
        }
    }

    _player->SetGuildIdInvited(0);
}

// Replaces vanilla's no-opcode-at-all "guild info" request; GuildRoster now carries the
// created-date/account-count fields that used to be their own SMSG_GUILD_INFO response.
void WorldSession::HandleGuildRosterOpcode(GuildGetRoster const& /*packet*/)
{
    if (Guild* guild = sGuildMgr.GetGuildById(_player->GetGuildId()))
        guild->SendGuildRoster(this);
    else
        SendGuildCommandResult(GUILD_COMMAND_GET_ROSTER, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
}

void WorldSession::HandleGuildGetRanksOpcode(GuildGetRanks const& /*packet*/)
{
    // packet.GuildGuid is parsed but unused: this engine only ever answers for the requester's
    // own guild, matching every other guild handler's scope.
    if (Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId()))
        guild->SendRanksResponse(this);
}

void WorldSession::HandleGuildPermissionsQueryOpcode(GuildPermissionsQuery const& /*packet*/)
{
    if (Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId()))
        guild->SendPermissions(this);
}

void WorldSession::HandleGuildPromoteOpcode(GuildPromoteMember const& packet)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_PROMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    nonstd::optional<ObjectGuid> promoteeGuid = ClassicGuidMapping::FromClassic(packet.PromoteeGuid);
    MemberSlot* slot = promoteeGuid ? guild->GetMemberSlot(*promoteeGuid) : nullptr;
    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if (slot->guid == GetPlayer()->GetObjectGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_NAME_INVALID);
        return;
    }

    // allow to promote only to lower rank than member's rank
    // guildmaster's rank = 0
    // GetPlayer()->GetRank() + 1 is highest rank that current player can promote to
    if (GetPlayer()->GetRank() + 1 >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, slot->Name, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    uint32 newRankId = slot->RankId - 1;                    //when promoting player, rank is decreased
    ObjectGuid promoteeGuidValue = slot->guid;

    slot->ChangeRank(newRankId);
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_PROMOTE_PLAYER, GetPlayer()->GetObjectGuid(), promoteeGuidValue, newRankId);

    guild->BroadcastRankChangeEvent(promoteeGuidValue, GetPlayer()->GetObjectGuid(), true, newRankId);
}

void WorldSession::HandleGuildDemoteOpcode(GuildDemoteMember const& packet)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_DEMOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    nonstd::optional<ObjectGuid> demoteeGuid = ClassicGuidMapping::FromClassic(packet.DemoteeGuid);
    MemberSlot* slot = demoteeGuid ? guild->GetMemberSlot(*demoteeGuid) : nullptr;

    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if (slot->guid == GetPlayer()->GetObjectGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_NAME_INVALID);
        return;
    }

    // do not allow to demote same or higher rank
    if (GetPlayer()->GetRank() >= slot->RankId)
    {
        SendGuildCommandResult(GUILD_INVITE_S, slot->Name, ERR_GUILD_RANK_TOO_HIGH_S);
        return;
    }

    // do not allow to demote lowest rank
    if (slot->RankId >= guild->GetLowestRank())
    {
        SendGuildCommandResult(GUILD_INVITE_S, slot->Name, ERR_GUILD_RANK_TOO_LOW_S);
        return;
    }

    uint32 newRankId = slot->RankId + 1;                    //when demoting player, rank is increased
    ObjectGuid demoteeGuidValue = slot->guid;

    slot->ChangeRank(newRankId);
    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_DEMOTE_PLAYER, GetPlayer()->GetObjectGuid(), demoteeGuidValue, newRankId);

    guild->BroadcastRankChangeEvent(demoteeGuidValue, GetPlayer()->GetObjectGuid(), false, newRankId);
}

void WorldSession::HandleGuildLeaveOpcode(GuildLeave const& /*packet*/)
{
    Guild* guild = sGuildMgr.GetGuildById(_player->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (_player->GetObjectGuid() == guild->GetLeaderGuid() && guild->GetMemberSize() > 1)
    {
        SendGuildCommandResult(GUILD_QUIT_S, "", ERR_GUILD_LEADER_LEAVE);
        return;
    }

    if (_player->GetObjectGuid() == guild->GetLeaderGuid())
    {
        guild->Disband();
        delete guild;
        return;
    }

    SendGuildCommandResult(GUILD_QUIT_S, guild->GetName(), ERR_PLAYER_NO_MORE_IN_GUILD);

    if (guild->DelMember(_player->GetObjectGuid()))
    {
        guild->Disband();
        delete guild;
        return;
    }

    // Put record into guild log
    guild->LogGuildEvent(GUILD_EVENT_LOG_LEAVE_GUILD, _player->GetObjectGuid());

    guild->BroadcastPlayerLeftEvent(_player->GetObjectGuid(), _player->GetName());
}

// TrinityCore's HandleDelete() sends no command result on failure at all (no dedicated
// GuildCommandType exists for disband) -- it just silently no-ops for a non-leader/no-guild caller.
void WorldSession::HandleGuildDisbandOpcode(GuildDelete const& /*packet*/)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
        return;

    if (GetPlayer()->GetObjectGuid() != guild->GetLeaderGuid())
        return;

    guild->Disband();
    delete guild;
}

void WorldSession::HandleGuildLeaderOpcode(GuildSetGuildMaster const& packet)
{
    Player* oldLeader = GetPlayer();

    std::string newMasterName = packet.NewMasterName;
    if (!normalizePlayerName(newMasterName))
        return;

    Guild* guild = sGuildMgr.GetGuildById(oldLeader->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (oldLeader->GetObjectGuid() != guild->GetLeaderGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    MemberSlot* oldSlot = guild->GetMemberSlot(oldLeader->GetObjectGuid());
    if (!oldSlot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    MemberSlot* slot = guild->GetMemberSlot(newMasterName);
    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, newMasterName, ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    ObjectGuid newLeaderGuid = slot->guid;
    std::string newLeaderName = slot->Name;

    guild->SetLeader(newLeaderGuid);
    oldSlot->ChangeRank(GR_OFFICER);

    guild->BroadcastNewLeaderEvent(oldLeader->GetObjectGuid(), oldLeader->GetName(), newLeaderGuid, newLeaderName);
}

void WorldSession::HandleGuildMOTDOpcode(GuildUpdateMotdText const& packet)
{
    if (utf8length(packet.Motd).value_or(GUILD_MOTD_MAX_LENGTH + 1) > GUILD_MOTD_MAX_LENGTH)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to set guild motd to string longer than client limit.", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS | CHEAT_ACTION_KICK);
        return;
    }

    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }
    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_SETMOTD))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->SetMOTD(packet.Motd);
    guild->BroadcastMotdEvent();
}

void WorldSession::HandleGuildSetMemberNoteOpcode(GuildSetMemberNote const& packet)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());

    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), packet.IsPublic ? GR_RIGHT_EPNOTE : GR_RIGHT_EOFFNOTE))
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    nonstd::optional<ObjectGuid> noteeGuid = ClassicGuidMapping::FromClassic(packet.NoteeGuid);
    MemberSlot* slot = noteeGuid ? guild->GetMemberSlot(*noteeGuid) : nullptr;
    if (!slot)
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD_S);
        return;
    }

    if (utf8length(packet.Note).value_or(GUILD_NOTE_MAX_LENGTH + 1) > GUILD_NOTE_MAX_LENGTH)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to set guild player note to string longer than client limit.", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS | CHEAT_ACTION_KICK);
        return;
    }

    if (packet.IsPublic)
        slot->SetPNOTE(packet.Note);
    else
        slot->SetOFFNOTE(packet.Note);

    guild->SendGuildRoster(this);
}

void WorldSession::HandleGuildRankOpcode(GuildSetRankPermissions const& packet)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (GetPlayer()->GetObjectGuid() != guild->GetLeaderGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    if (utf8length(packet.RankName).value_or(GUILD_RANK_MAX_LENGTH + 1) > GUILD_RANK_MAX_LENGTH)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to set guild rank name to string longer than client limit.", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS | CHEAT_ACTION_KICK);
        return;
    }

    guild->SetRankName(packet.RankId, packet.RankName);

    uint32 newRights = packet.Flags;
    if (packet.RankId == GR_GUILDMASTER)                    // prevent loss leader rights
        newRights = GR_RIGHT_ALL;

    guild->SetRankRights(packet.RankId, newRights);

    // Matches TrinityCore's Guild::HandleSetRankInfo(): only this one event, no roster/query
    // resend -- resending those too forces every online member's guild panel, including their own
    // nameplate tag, to redundantly redraw on every rank edit.
    guild->BroadcastRankInfoChangedEvent(packet.RankId);
}

void WorldSession::HandleGuildAddRankOpcode(GuildAddRank const& packet)
{
    if (utf8length(packet.RankName).value_or(GUILD_RANK_MAX_LENGTH + 1) > GUILD_RANK_MAX_LENGTH)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to set guild rank name to string longer than client limit.", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS | CHEAT_ACTION_KICK);
        return;
    }

    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (GetPlayer()->GetObjectGuid() != guild->GetLeaderGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    if (guild->GetRanksSize() >= GUILD_RANKS_MAX_COUNT)     // client not let create more 10 than ranks
        return;

    // packet.RankOrder is parsed but unused: this engine's ranks are always appended
    // (Guild::CreateRank has no positional-insert support), matching the old client's own
    // behavior.
    guild->CreateRank(packet.RankName, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);

    // Matches TrinityCore's Guild::HandleAddNewRank(): only this one event, no roster/query
    // resend (see the comment in HandleGuildRankOpcode above).
    guild->BroadcastRanksUpdatedEvent();
}

void WorldSession::HandleGuildDelRankOpcode(GuildDeleteRank const& /*packet*/)
{
    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (GetPlayer()->GetObjectGuid() != guild->GetLeaderGuid())
    {
        SendGuildCommandResult(GUILD_INVITE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    // packet.RankOrder is parsed but unused: the real client only ever offers deleting the last
    // (lowest) rank, matching Guild::DelRank()'s no-parameter always-remove-last behavior.
    guild->DelRank();

    // Matches TrinityCore's Guild::HandleRemoveRank(): only this one event, no roster/query
    // resend (see the comment in HandleGuildRankOpcode above).
    guild->BroadcastRanksUpdatedEvent();
}

void WorldSession::SendGuildCommandResult(uint32 typecmd, std::string const& str, uint32 cmdresult)
{
    GuildCommandResult packet;
    packet.Command = typecmd;
    packet.Str = str;
    packet.Result = cmdresult;
    SendClassic(packet);
}

void WorldSession::HandleGuildChangeInfoTextOpcode(GuildUpdateInfoText const& packet)
{
    if (utf8length(packet.InfoText).value_or(GUILD_INFO_MAX_LENGTH + 1) > GUILD_INFO_MAX_LENGTH)
    {
        ProcessAnticheatAction("PassiveAnticheat", "Attempt to set guild info to string longer than client limit.", CHEAT_ACTION_LOG | CHEAT_ACTION_REPORT_GMS | CHEAT_ACTION_KICK);
        return;
    }

    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PLAYER_NOT_IN_GUILD);
        return;
    }

    if (!guild->HasRankRight(GetPlayer()->GetRank(), GR_RIGHT_MODIFY_GUILD_INFO))
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_PERMISSIONS);
        return;
    }

    guild->SetGINFO(packet.InfoText);
}

void WorldSession::HandleSaveGuildEmblemOpcode(SaveGuildEmblem const& packet)
{
    nonstd::optional<ObjectGuid> vendorGuid = ClassicGuidMapping::FromClassic(packet.VendorGuid);
    Creature* pCreature = vendorGuid ? GetPlayer()->GetNPCIfCanInteractWith(*vendorGuid, UNIT_NPC_FLAG_TABARDDESIGNER) : nullptr;
    if (!pCreature)
    {
        //[-ZERO] fails silently, not "That's not an emblem vendor!"
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_FAIL_NO_MESSAGE);
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandleSaveGuildEmblemOpcode - vendor not found or you can't interact with him.");
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    Guild* guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
    if (!guild)
    {
        //"You are not part of a guild!";
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOGUILD);
        return;
    }

    if (guild->GetLeaderGuid() != GetPlayer()->GetObjectGuid())
    {
        //"Only guild leaders can create emblems."
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOTGUILDMASTER);
        return;
    }

    if (GetPlayer()->GetMoney() < 10 * GOLD)
    {
        //"You can't afford to do that."
        SendSaveGuildEmblem(ERR_GUILDEMBLEM_NOTENOUGHMONEY);
        return;
    }

    GetPlayer()->ModifyMoney(-10 * GOLD);
    guild->SetEmblem(packet.EmblemStyle, packet.EmblemColor, packet.BorderStyle, packet.BorderColor, packet.BackgroundColor);

    //"Guild Emblem saved."
    SendSaveGuildEmblem(ERR_GUILDEMBLEM_SUCCESS);

    // Refresh the emblem in every online member's guild cache so tabards update instantly
    // (TrinityCore only refreshes the saving player).
    guild->BroadcastQueryResponse();
}

void WorldSession::SendSaveGuildEmblem(uint32 msg)
{
    SaveGuildEmblemResult packet;
    packet.Error = int32(msg);
    SendClassic(packet);
}
