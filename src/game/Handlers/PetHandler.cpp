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
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "CreatureAI.h"
#include "Pet.h"
#include "Group.h"
#include "Utilities/Random.h"
#include "Packets/Classic/GuidMapping.h"
#include "Packets/PetMessages.h"

namespace
{
ObjectGuid ResolveGuid(ClassicGuid const& guid)
{
    auto const resolved = ClassicGuidMapping::FromClassic(guid);
    return resolved ? *resolved : ObjectGuid();
}
}

void WorldSession::HandlePetAction(PetAction const& packet)
{
    uint32 spellid = UNIT_ACTION_BUTTON_ACTION(packet.Action);
    uint8 flag = UNIT_ACTION_BUTTON_TYPE(packet.Action);           // delete = 0x07 CastSpell = C1

    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    ObjectGuid const targetGuid = ResolveGuid(packet.Target);

    // used also for charmed creature/player
    Unit* pCharmedUnit = _player->GetMap()->GetUnit(petGuid);
    if (!pCharmedUnit)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetAction: %s not exist.", petGuid.GetString().c_str());
        return;
    }

    if (GetPlayer()->GetObjectGuid() != pCharmedUnit->GetCharmerOrOwnerGuid())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetAction: %s isn't controlled by %s.", petGuid.GetString().c_str(), GetPlayer()->GetGuidStr().c_str());
        return;
    }

    if (!pCharmedUnit->IsAlive())
        return;

    Creature* pCharmedCreature = pCharmedUnit->ToCreature();

    if (!pCharmedCreature)
    {
        // controller player can only do melee attack
        if (!(flag == ACT_REACTION || flag == ACT_COMMAND))
            return;
    }
    else if (pCharmedCreature->IsPet())
    {
        // pet can have action bar disabled
        if (!((Pet*)pCharmedUnit)->IsEnabled())
            return;
    }

    CharmInfo* charmInfo = pCharmedUnit->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WorldSession::HandlePetAction: object (GUID: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", pCharmedUnit->GetGUIDLow(), pCharmedUnit->GetTypeId());
        return;
    }

    switch (flag)
    {
        case ACT_COMMAND:                                   // 0x07
        {
            Unit* pTarget = targetGuid.IsEmpty() ? nullptr : _player->GetMap()->GetUnit(targetGuid);
            pCharmedUnit->HandlePetCommand((CommandStates)spellid, pTarget);
            break;
        }
        case ACT_REACTION:                                  // 0x6
            switch (spellid)
            {
                case REACT_PASSIVE:                         //passive
                    pCharmedUnit->InterruptNonMeleeSpells(false);
                    pCharmedUnit->AttackStop();
                // no break
                case REACT_DEFENSIVE:                       //recovery
                case REACT_AGGRESSIVE:                      //activete
                    charmInfo->SetReactState(ReactStates(spellid));
                    break;
            }
            break;
        case ACT_DISABLED:                                  // 0x81    spell (disabled), ignore
        case ACT_PASSIVE:                                   // 0x01
        case ACT_ENABLED:                                   // 0xC1    spell
        {
            Unit* pUnitTarget = nullptr;
            if (targetGuid)
                pUnitTarget = _player->GetMap()->GetUnit(targetGuid);

            // do not cast unknown spells
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spellid);
            if (!spellInfo)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WORLD: unknown PET spell id %i", spellid);
                return;
            }

            if (!pCharmedUnit->IsSpellReady(spellInfo))
            {
                pCharmedUnit->SendPetCastFail(spellid, SPELL_FAILED_NOT_READY);
                return;
            }

            // do not cast not learned spells
            if (!pCharmedUnit->HasSpell(spellid) || spellInfo->IsPassiveSpell())
            {
                pCharmedUnit->SendPetCastFail(spellid, SPELL_FAILED_NOT_KNOWN);
                return;
            }

            bool const explicitlySelectedTarget = Spells::IsExplicitlySelectedUnitTarget(spellInfo->EffectImplicitTargetA[0]);

            if (!pUnitTarget && explicitlySelectedTarget)
            {
                pCharmedUnit->SendPetCastFail(spellid, SPELL_FAILED_BAD_IMPLICIT_TARGETS);
                return;
            }

            if (pUnitTarget == pCharmedUnit &&
                // Cannot cast negative spells on yourself. Handle it here since casting negative
                // spells on yourself is frequently used within the core itself for certain mechanics.
                ((explicitlySelectedTarget && !spellInfo->IsPositiveSpell(pCharmedUnit, pUnitTarget)) ||
                // spells not castable on self like Fire Shield
                spellInfo->HasAttribute(SPELL_ATTR_EX_EXCLUDE_CASTER)))
            {
                pCharmedUnit->SendPetCastFail(spellid, SPELL_FAILED_BAD_TARGETS);
                return;
            }

            // remove not needed target
            if (pUnitTarget && pUnitTarget != pCharmedUnit && !explicitlySelectedTarget)
                pUnitTarget = nullptr;

            // make sure pet is facing target
            if (pUnitTarget && pUnitTarget != pCharmedUnit && !pCharmedUnit->HasUnitState(UNIT_STATE_CAN_NOT_REACT) &&
                spellInfo->IsNeedFaceTarget() && !pCharmedUnit->IsFacingTarget(pUnitTarget))
            {
                float orientation = pCharmedUnit->GetAngle(pUnitTarget);
                pCharmedUnit->SetFacingTo(orientation);
                pCharmedUnit->SetOrientation(orientation);
            }

            pCharmedUnit->ClearUnitState(UNIT_STATE_MOVING);
            SpellCastResult result = pCharmedUnit->CastSpell(pUnitTarget, spellInfo, false);

            if (result == SPELL_CAST_OK)
            {
                if (pCharmedUnit->IsPet())
                    ((Pet*)pCharmedUnit)->CheckLearning(spellid);

                if (pCharmedUnit->IsMoving() && pCharmedUnit->IsNoMovementSpellCasted())
                    pCharmedUnit->StopMoving();
            }
            else
                pCharmedUnit->SendPetCastFail(spellid, result);


            break;
        }
        default:
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WORLD: unknown PET flag Action %i and spellid %i.", uint32(flag), spellid);
    }
}

void WorldSession::HandlePetNameQueryOpcode(QueryPetName const& packet)
{
    SendPetNameQuery(ResolveGuid(packet.Pet));
}

void WorldSession::SendPetNameQuery(ObjectGuid petGuid)
{
    QueryPetNameResponse response;
    response.Pet = ClassicGuidMapping::ToClassic(petGuid, realmID);

    Creature* pet = _player->GetMap()->GetAnyTypeCreature(petGuid);
    if (pet && pet->GetCharmInfo())
    {
        response.Allow = true;
        response.Name = pet->GetName();
        response.Timestamp = pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP);
    }

    SendClassic(response);
}

void WorldSession::HandlePetSetAction(PetSetAction const& packet)
{
    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    Creature* pet = _player->GetMap()->GetAnyTypeCreature(petGuid);

    if (!pet || (pet != _player->GetPet() && pet != _player->GetCharm()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetSetAction: Unknown pet or pet owner.");
        return;
    }

    // pet can have action bar disabled
    if (pet->IsPet() && !((Pet*)pet)->IsEnabled())
        return;

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WorldSession::HandlePetSetAction: object (GUID: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", pet->GetGUIDLow(), pet->GetTypeId());
        return;
    }

    // ignore invalid position
    if (packet.Index >= MAX_UNIT_ACTION_BAR_INDEX)
        return;

    uint8 const position = uint8(packet.Index);
    uint32 const spellId = UNIT_ACTION_BUTTON_ACTION(packet.Action);
    uint8 const act_state = UNIT_ACTION_BUTTON_TYPE(packet.Action);

    // Command and reaction buttons can be moved but not removed. A move is one message per button, so
    // a command is only placed while the bar still holds it elsewhere, and a command's slot is only
    // cleared by putting something in it.
    if (act_state == ACT_COMMAND || act_state == ACT_REACTION)
    {
        bool placedElsewhere = false;
        for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        {
            UnitActionBarEntry const* entry = charmInfo->GetActionBarEntry(i);
            if (i != position && entry->GetAction() == spellId && entry->GetType() == act_state)
                placedElsewhere = true;
        }
        if (!placedElsewhere)
            return;
    }
    else if (!spellId)
    {
        UnitActionBarEntry const* current = charmInfo->GetActionBarEntry(position);
        if (current->GetType() == ACT_COMMAND || current->GetType() == ACT_REACTION)
            return;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X", _player->GetName(), uint32(position), spellId, uint32(act_state));

    // if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
    if (!((act_state == ACT_ENABLED || act_state == ACT_DISABLED || act_state == ACT_PASSIVE) && spellId && !pet->HasSpell(spellId)))
    {
        // sign for autocast
        if (act_state == ACT_ENABLED && spellId)
        {
            if (pet->IsCharmed())
                charmInfo->ToggleCreatureAutocast(spellId, true);
            else
                ((Pet*)pet)->ToggleAutocast(spellId, true);
        }
        // sign for no/turn off autocast
        else if (act_state == ACT_DISABLED && spellId)
        {
            if (pet->IsCharmed())
                charmInfo->ToggleCreatureAutocast(spellId, false);
            else
                ((Pet*)pet)->ToggleAutocast(spellId, false);
        }

        charmInfo->SetActionBar(position, spellId, ActiveStates(act_state));
    }
}

void WorldSession::HandlePetRename(PetRename const& packet)
{
    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    Pet* pet = _player->GetMap()->GetPet(petGuid);
    // check it!
    if (!pet || pet->GetPetType() != HUNTER_PET ||
            !pet->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_RENAME) ||
            pet->GetOwnerGuid() != _player->GetObjectGuid() || !pet->GetCharmInfo())
        return;

    PetNameInvalidReason res = ObjectMgr::CheckPetName(packet.Name);
    if (res != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(res, packet.Pet, packet.PetNumber, packet.Name);
        return;
    }

    if (sObjectMgr.IsReservedName(packet.Name))
    {
        SendPetNameInvalid(PET_NAME_RESERVED, packet.Pet, packet.PetNumber, packet.Name);
        return;
    }

    pet->SetName(packet.Name);

    if (_player->GetGroup())
        _player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

    pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_RENAME);

    CharacterDatabase.BeginTransaction();
    std::string safeName = packet.Name;
    CharacterDatabase.escape_string(safeName);
    CharacterDatabase.PExecute("UPDATE `character_pet` SET `name` = '%s', `renamed` = '1' WHERE `owner_guid` = '%u' AND `id` = '%u'", safeName.c_str(), _player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber());
    CharacterDatabase.CommitTransaction();

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr)));
}

void WorldSession::HandlePetAbandon(PetAbandon const& packet)
{
    if (!_player->IsInWorld())
        return;

    // pet/charmed
    if (Unit* petUnit = _player->GetMap()->GetUnit(ResolveGuid(packet.Pet)))
    {
        if (petUnit->GetOwnerGuid() != _player->GetObjectGuid() || !petUnit->GetCharmInfo())
            return;

        Creature* petCreature = petUnit->GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(petUnit) : nullptr;
        Pet* pet = petCreature && petCreature->IsPet() ? static_cast<Pet*>(petUnit) : nullptr;

        if (pet)
        {
            // Permanently abandon pet
            if (pet->GetPetType() == HUNTER_PET)
                pet->Unsummon(PET_SAVE_AS_DELETED, _player);
            // Simply dismiss
            else
                pet->Unsummon(PET_SAVE_NOT_IN_SLOT);
        }
        else if (petUnit->GetObjectGuid() == _player->GetCharmGuid())
        {
            _player->Uncharm();
        }
    }
}

void WorldSession::HandlePetStopAttack(PetStopAttack const& packet)
{
    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    Unit* pet = GetPlayer()->GetMap()->GetUnit(petGuid); // pet or controlled creature/player
    if (!pet)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetStopAttack: %s doesn't exist.", petGuid.GetString().c_str());
        return;
    }

    if (GetPlayer()->GetObjectGuid() != pet->GetCharmerOrOwnerGuid())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetStopAttack: %s isn't charm/pet of %s.", petGuid.GetString().c_str(), GetPlayer()->GetGuidStr().c_str());
        return;
    }

    if (!pet->IsAlive())
        return;

    pet->AttackStop();
}

void WorldSession::HandlePetSpellAutocastOpcode(PetSpellAutocast const& packet)
{
    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    Creature* pet = _player->GetMap()->GetAnyTypeCreature(petGuid);
    if (!pet || (petGuid != _player->GetPetGuid() && petGuid != _player->GetCharmGuid()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetSpellAutocastOpcode. %s isn't pet of %s .", petGuid.GetString().c_str(), GetPlayer()->GetGuidStr().c_str());
        return;
    }

    // do not add not learned spells / passive spells
    if (!pet->HasSpell(packet.SpellId) || !Spells::IsAutocastable(packet.SpellId))
        return;

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WorldSession::HandlePetSpellAutocastOpcod: %s is considered pet-like but doesn't have a charminfo!", petGuid.GetString().c_str());
        return;
    }

    if (pet->IsCharmed())
        pet->GetCharmInfo()->ToggleCreatureAutocast(packet.SpellId, packet.Enabled);
    else
        ((Pet*)pet)->ToggleAutocast(packet.SpellId, packet.Enabled);

    charmInfo->SetSpellAutocast(packet.SpellId, packet.Enabled);
}

void WorldSession::HandlePetCastSpellOpcode(PetCastSpell const& packet)
{
    SpellCastRequest const& request = packet.Cast;
    ObjectGuid const petGuid = ResolveGuid(packet.Pet);
    Creature* pet = _player->GetMap()->GetAnyTypeCreature(petGuid);

    if (!pet || (petGuid != _player->GetPetGuid() && petGuid != _player->GetCharmGuid()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "HandlePetCastSpellOpcode: %s isn't pet of %s .", petGuid.GetString().c_str(), GetPlayer()->GetGuidStr().c_str());
        return;
    }

    SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(request.SpellId);
    if (!spellInfo)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WORLD: unknown PET spell id %i", request.SpellId);
        return;
    }

    if (!pet->IsSpellReady(spellInfo))
        return;

    // do not cast not learned spells
    if (!pet->HasSpell(request.SpellId) || spellInfo->IsPassiveSpell())
        return;

    SpellCastTargets targets;
    targets.ReadTargetData(request.Target);
    targets.PrepareForSpellSystem(pet);

    pet->ClearUnitState(UNIT_STATE_MOVING);

    Spell* spell = new Spell(pet, spellInfo, false);
    spell->m_targets = std::move(targets);
    spell->m_clientCastId = request.CastId;
    spell->m_clientVisual = request.Visual.SpellXSpellVisualId;

    SpellCastResult result = spell->CheckPetCast(nullptr);
    if (result == SPELL_CAST_OK)
    {
        if (pet->IsPet())
        {
            ((Pet*)pet)->CheckLearning(request.SpellId);

            //10% chance to play special pet attack talk, else growl
            //actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
            if (((Pet*)pet)->GetPetType() == SUMMON_PET && (urand(0, 100) < 10))
                pet->SendPetTalk((uint32)PET_TALK_SPECIAL_SPELL);
            else
                pet->SendPetAIReaction();
        }

        SpellPrepare prepare;
        prepare.ClientCastId = request.CastId;
        prepare.ServerCastId = spell->GetCastId();
        SendClassic(prepare);

        spell->prepare();
    }
    else
    {
        pet->SendPetCastFail(request.SpellId, result, &request.CastId);
        if (pet->IsSpellReady(spellInfo))
            GetPlayer()->SendClearCooldown(request.SpellId, pet);

        spell->finish(false);
        spell->Delete();
    }
}

void WorldSession::SendPetNameInvalid(uint32 error, ClassicGuid const& pet, int32 petNumber, std::string const& name)
{
    PetNameInvalid packet;
    packet.Result = uint8(error);
    packet.Pet = pet;
    packet.PetNumber = petNumber;
    packet.Name = name;
    SendClassic(packet);
}
