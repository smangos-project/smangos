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
#include "Anticheat.h"
#include "Packets/Classic/GuidMapping.h"

// Charter id in item_template
#define GUILD_CHARTER               5863
#define GUILD_CHARTER_COST          1000                    // 10 S

void WorldSession::HandlePetitionBuyOpcode(PetitionBuy const& packet)
{
    nonstd::optional<ObjectGuid> const npcGuid = ClassicGuidMapping::FromClassic(packet.NpcGuid);
    Creature* pCreature = npcGuid ? GetPlayer()->GetNPCIfCanInteractWith(*npcGuid, UNIT_NPC_FLAG_PETITIONER) : nullptr;
    if (!pCreature)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandlePetitionBuyOpcode - npc not found or you can't interact with him.");
        return;
    }

    if (!pCreature->IsTabardDesigner())
        return;

    if (HasTrialRestrictions())
    {
        SendNotification(LANG_RESTRICTED_ACCOUNT);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // if tabard designer, then trying to buy a guild charter.
    // do not let if already in guild.
    if (_player->GetGuildId())
        return;

    // Cannot buy a petition if the owner already has one
    if (sGuildMgr.GetPetitionByOwnerGuid(_player->GetObjectGuid()))
        return;

    uint32 charterid = GUILD_CHARTER;
    uint32 cost = GUILD_CHARTER_COST;

    if (sGuildMgr.GetGuildByName(packet.Name))
    {
        SendGuildCommandResult(GUILD_CREATE_S, packet.Name, ERR_GUILD_NAME_EXISTS_S);
        return;
    }
    if (sObjectMgr.IsReservedName(packet.Name) || !ObjectMgr::IsValidCharterName(packet.Name))
    {
        SendGuildCommandResult(GUILD_CREATE_S, packet.Name, ERR_GUILD_NAME_INVALID);
        return;
    }

    // Check guild petition name (use whisper type - 6)
    if (AntispamInterface *a = sAnticheatMgr->GetAntispam())
    {
        if (a->filterMessage(packet.Name))
        {
            sWorld.LogChat(this, "Guild", "Attempt to create guild petition with spam name");
            SendGuildCommandResult(GUILD_CREATE_S, packet.Name, ERR_GUILD_NAME_INVALID);
            return;
        }
    }

    ItemPrototype const* pProto = sObjectMgr.GetItemPrototype(charterid);
    if (!pProto)
    {
        _player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, nullptr, charterid, 0);
        return;
    }

    if (_player->GetMoney() < cost)
    {
        //player hasn't got enough money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, charterid, 0);
        return;
    }

    ItemPosCountVec dest;
    InventoryResult msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->BuyCount);
    if (msg != EQUIP_ERR_OK)
    {
        _player->SendEquipError(msg, nullptr, nullptr, 0, charterid);
        return;
    }

    _player->ModifyMoney(-(int32)cost);
    Item *charter = _player->StoreNewItem(dest, charterid, true);
    if (!charter)
        return;

    uint32 petitionId = sObjectMgr.GeneratePetitionID();
    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT, petitionId);
    // ITEM_FIELD_ENCHANTMENT is guild
    // ITEM_FIELD_ENCHANTMENT+1 is current signatures count (showed on item)
    charter->SetState(ITEM_CHANGED, _player);
    _player->SendNewItem(charter, 1, true, false);

    sGuildMgr.CreatePetition(petitionId, _player, charter->GetObjectGuid(), packet.Name);

    _player->SaveInventoryAndGoldToDB();
}

void WorldSession::HandlePetitionShowSignOpcode(PetitionShowSignatures const& packet)
{
    // if guild petition and has guild => error, return;
    if (_player->GetGuildId())
        return;

    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    Item *charter = itemGuid ? _player->GetItemByGuid(*itemGuid) : nullptr;
    if (!charter)
        return;

    uint32 petitionGuid = charter->GetEnchantmentId(EnchantmentSlot(0));
    Petition *petition = sGuildMgr.GetPetitionById(petitionGuid);

    if (!petition)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "[PetitionHandler] No petition exists for petition ID %u, yet charter exists with guid %u for owner %s",
            petitionGuid, charter->GetObjectGuid().GetCounter(), _player->GetGuidStr().c_str());
        return;
    }

    PetitionSignatures response;
    response.ItemGuid = packet.ItemGuid;
    response.OwnerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
    response.OwnerAccountGuid = ClassicGuidMapping::MakeAccount(GetAccountId());
    response.PetitionId = int32(petitionGuid);
    for (PetitionSignature* signature : petition->GetSignatureList())
    {
        PetitionSignatureEntry entry;
        entry.SignerGuid = ClassicGuidMapping::ToClassic(signature->GetSignatureGuid(), realmID);
        response.Signatures.push_back(entry);
    }

    SendClassic(response);
}

void WorldSession::HandlePetitionQueryOpcode(QueryPetition const& packet)
{
    Petition* petition = sGuildMgr.GetPetitionById(packet.PetitionId);

    QueryPetitionResponse response;
    response.PetitionId = packet.PetitionId;
    if (!petition)
    {
        response.Allow = false;
        SendClassic(response);
        return;
    }

    response.Allow = true;
    response.OwnerGuid = ClassicGuidMapping::ToClassic(petition->GetOwnerGuid(), realmID);
    response.Name = petition->GetName();
    response.MinSignatures = sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS);
    response.MaxSignatures = sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS);
    SendClassic(response);
}

void WorldSession::HandlePetitionRenameOpcode(RenamePetition const& packet)
{
    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    Item *charter = itemGuid ? _player->GetItemByGuid(*itemGuid) : nullptr;
    if (!charter)
        return;

    if (sGuildMgr.GetGuildByName(packet.NewName))
    {
        SendGuildCommandResult(GUILD_CREATE_S, packet.NewName, ERR_GUILD_NAME_EXISTS_S);
        return;
    }
    if (sObjectMgr.IsReservedName(packet.NewName) || !ObjectMgr::IsValidCharterName(packet.NewName))
    {
        SendGuildCommandResult(GUILD_CREATE_S, packet.NewName, ERR_GUILD_NAME_INVALID);
        return;
    }

    uint32 petitionGuid = charter->GetEnchantmentId(EnchantmentSlot(0));
    Petition* petition = sGuildMgr.GetPetitionById(petitionGuid);
    if (!petition)
        return;

    if (petition->Rename(packet.NewName))
    {
        RenamePetitionResult renameResult;
        renameResult.ItemGuid = packet.ItemGuid;
        renameResult.NewName = packet.NewName;
        SendClassic(renameResult);
    }
}

void WorldSession::HandlePetitionSignOpcode(SignPetition const& packet)
{
    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    Petition* petition = itemGuid ? sGuildMgr.GetPetitionByCharterGuid(*itemGuid) : nullptr;

    if (!petition)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "[PetitionHandler] No petition exists for charter for signer %s", _player->GetGuidStr().c_str());
        return;
    }

    if (petition->IsComplete()) // reached maximum number of signatures for this petition
        return;

    if (petition->GetOwnerGuid() == _player->GetObjectGuid())
    {
        PetitionSignResult signPacket;
        signPacket.ItemGuid = packet.ItemGuid;
        signPacket.PlayerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
        signPacket.Result = PetitionSignResult::CantSignOwn;
        SendClassic(signPacket);
        return;
    }

    if (HasTrialRestrictions())
    {
        SendNotification(LANG_RESTRICTED_ACCOUNT);
        return;
    }

    // not let enemies sign guild charter
    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) &&
            GetPlayer()->GetTeam() != petition->GetTeam())
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (_player->GetGuildId())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_IN_GUILD_S);
        return;
    }
    if (_player->GetGuildIdInvited())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    uint8 signs = petition->GetSignatureCount();

    // Client hard limit at 9 signatures
    if (signs >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS))
        return;

    //client doesn't allow to sign petition two times by one character, but not check sign by another character from same account
    //not allow sign another player from already sign player account
    if (PetitionSignature* signature = petition->GetSignatureForPlayer(_player))
    {
        PetitionSignResult signPacket;
        signPacket.ItemGuid = packet.ItemGuid;
        signPacket.PlayerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
        signPacket.Result = PetitionSignResult::AlreadySigned;

        // close at signer side
        SendClassic(signPacket);

        // Update for owner if online. Note: Unsure if this is the correct message,
        // but sending SMSG_PETITION_SIGN_RESULTS does nothing for the owner here
        if (Player* owner = sObjectMgr.GetPlayer(petition->GetOwnerGuid()))
            owner->GetSession()->SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    if (petition->AddNewSignature(_player))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "PETITION SIGN: %u by %s", petition->GetId(), _player->GetGuidStr().c_str());

        PetitionSignResult signPacket;
        signPacket.ItemGuid = packet.ItemGuid;
        signPacket.PlayerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
        signPacket.Result = PetitionSignResult::Ok;

        // close at signer side
        SendClassic(signPacket);

        // update for owner if online
        if (Player* owner = sObjectMgr.GetPlayer(petition->GetOwnerGuid()))
            owner->GetSession()->SendClassic(signPacket);
    }
}

void WorldSession::HandlePetitionDeclineOpcode(DeclinePetition const& /*packet*/)
{
    // The modern client never handles this notification, so nothing is sent for it.
}

void WorldSession::HandleOfferPetitionOpcode(OfferPetition const& packet)
{
    nonstd::optional<ObjectGuid> const targetGuid = ClassicGuidMapping::FromClassic(packet.PlayerGuid);
    Player* player = targetGuid ? ObjectAccessor::FindPlayer(*targetGuid) : nullptr;
    if (!player)
        return;

    if (!sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GUILD) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        SendGuildCommandResult(GUILD_CREATE_S, "", ERR_GUILD_NOT_ALLIED);
        return;
    }

    if (player->GetGuildId())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_IN_GUILD_S);
        return;
    }

    if (player->GetGuildIdInvited())
    {
        SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ERR_ALREADY_INVITED_TO_GUILD_S);
        return;
    }

    if (player->GetSession()->HasTrialRestrictions())
    {
        SendNotification(LANG_RESTRICTED_ACCOUNT);
        return;
    }

    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    Item const* charter = itemGuid ? _player->GetItemByGuid(*itemGuid) : nullptr;
    if (!charter)
        return;

    uint32 petitionGuid = charter->GetEnchantmentId(EnchantmentSlot(0));

    Petition const* petition = sGuildMgr.GetPetitionById(petitionGuid);
    if (!petition)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "[PetitionHandler] No petition exists for charter with guid %u for signer %s",
            charter->GetObjectGuid().GetCounter(), _player->GetGuidStr().c_str());
        return;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "OFFER PETITION: petition %u to %s", petitionGuid, player->GetName());

    PetitionSignatures response;
    response.ItemGuid = packet.ItemGuid;
    response.OwnerGuid = ClassicGuidMapping::ToClassic(_player->GetObjectGuid(), realmID);
    response.OwnerAccountGuid = ClassicGuidMapping::MakeAccount(GetAccountId());
    response.PetitionId = int32(petitionGuid);
    for (PetitionSignature* signature : petition->GetSignatureList())
    {
        PetitionSignatureEntry entry;
        entry.SignerGuid = ClassicGuidMapping::ToClassic(signature->GetSignatureGuid(), realmID);
        response.Signatures.push_back(entry);
    }

    player->GetSession()->SendClassic(response);
}

void WorldSession::HandleTurnInPetitionOpcode(TurnInPetition const& packet)
{
    nonstd::optional<ObjectGuid> const itemGuid = ClassicGuidMapping::FromClassic(packet.ItemGuid);
    Item *charter = itemGuid ? _player->GetItemByGuid(*itemGuid) : nullptr;
    if (!charter)
        return;

    uint32 petitionGuid = charter->GetEnchantmentId(EnchantmentSlot(0));
    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Petition %u turned in by %s", petitionGuid, _player->GetGuidStr().c_str());

    Petition* petition = sGuildMgr.GetPetitionById(petitionGuid);
    if (!petition)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "[PetitionHandler] No petition exists for charter with guid %u for guild master %s",
            charter->GetObjectGuid().GetCounter(), _player->GetGuidStr().c_str());
        return;
    }

    if (HasTrialRestrictions())
    {
        SendNotification(LANG_RESTRICTED_ACCOUNT);
        return;
    }

    // Collect petition info data
    if (_player->GetGuildId())
    {
        TurnInPetitionResult turnInPacket;
        turnInPacket.Result = TurnInPetitionResult::AlreadyInGuild;
        SendClassic(turnInPacket);
        return;
    }

    if (_player->GetObjectGuid() != petition->GetOwnerGuid())
        return;

    if (!petition->IsComplete())
    {
        TurnInPetitionResult turnInPacket;
        turnInPacket.Result = TurnInPetitionResult::NeedMoreSignatures;
        SendClassic(turnInPacket);
        return;
    }

    if (sGuildMgr.GetGuildByName(petition->GetName()))
    {
        SendGuildCommandResult(GUILD_CREATE_S, petition->GetName(), ERR_GUILD_NAME_EXISTS_S);
        return;
    }

    // OK!

    Guild* guild = new Guild;
    if (!guild->Create(petition, _player))
    {
        delete guild;
        return;
    }

    // register guild
    sGuildMgr.AddGuild(guild);

    sGuildMgr.DeletePetition(petition);
    petition = nullptr; // deleted in GuildMgr::DeletePetition

    // Delete the charter item now. Deleting the charter deletes the petition
    // if it still exists, so we do not want to do it before the guild is
    // created.
    _player->DestroyItem(charter->GetBagSlot(), charter->GetSlot(), true);

    // created
    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "TURN IN PETITION %u", petitionGuid);

    TurnInPetitionResult turnInPacket;
    turnInPacket.Result = TurnInPetitionResult::Ok;
    SendClassic(turnInPacket);
}

void WorldSession::HandlePetitionShowListOpcode(PetitionShow const& packet)
{
    nonstd::optional<ObjectGuid> const npcGuid = ClassicGuidMapping::FromClassic(packet.PetitionerGuid);
    if (npcGuid)
        SendPetitionShowList(*npcGuid);
}

void WorldSession::SendPetitionShowList(ObjectGuid guid)
{
    Creature* pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WORLD: HandlePetitionShowListOpcode - %s not found or you can't interact with him.", guid.GetString().c_str());
        return;
    }

    // remove fake death
    if (GetPlayer()->HasUnitState(UNIT_STATE_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    PetitionShowList showList;
    showList.NpcGuid = ClassicGuidMapping::ToClassic(guid, realmID);
    showList.Price = GUILD_CHARTER_COST;
    SendClassic(showList);
    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Sent SMSG_PETITION_SHOW_LIST");
}
