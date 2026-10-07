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

#include <array>
#include "Common.h"
#include "Log.h"
#include "Creature.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellModifier.h"
#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "World.h"
#include "Map.h"
#include "GridMap.h"
#include "Utilities/Random.h"
#include "ScriptMgr.h"

static std::array<pAuraProcHandler, TOTAL_AURAS> BuildAuraProcHandlers()
{
    std::array<pAuraProcHandler, TOTAL_AURAS> table;
    table.fill(&Unit::HandleNULLProc);
    table[0] = &Unit::HandleNULLProc;  // SPELL_AURA_NONE
    table[4] = &Unit::HandleDummyAuraProc;  // SPELL_AURA_DUMMY
    table[7] = &Unit::HandleRemoveFearByDamageChanceProc;  // SPELL_AURA_MOD_FEAR
    table[13] = &Unit::HandleModDamageAuraProc;  // SPELL_AURA_MOD_DAMAGE_DONE
    table[18] = &Unit::HandleInvisibilityAuraProc;  // SPELL_AURA_MOD_INVISIBILITY
    table[22] = &Unit::HandleModResistanceAuraProc;  // SPELL_AURA_MOD_RESISTANCE
    table[26] = &Unit::HandleRemoveByDamageChanceProc;  // SPELL_AURA_MOD_ROOT
    table[42] = &Unit::HandleProcTriggerSpellAuraProc;  // SPELL_AURA_PROC_TRIGGER_SPELL
    table[43] = &Unit::HandleProcTriggerDamageAuraProc;  // SPELL_AURA_PROC_TRIGGER_DAMAGE
    table[60] = &Unit::HandleRemoveByDamageChanceProc;  // SPELL_AURA_MOD_PACIFY_SILENCE
    table[65] = &Unit::HandleModCastingSpeedNotStackAuraProc;  // SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK
    table[72] = &Unit::HandleModPowerCostSchoolAuraProc;  // SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT
    table[553] = &Unit::HandleModPowerCostSchoolAuraProc;  // SPELL_AURA_MOD_POWER_COST_SCHOOL
    table[74] = &Unit::HandleReflectSpellsSchoolAuraProc;  // SPELL_AURA_REFLECT_SPELLS_SCHOOL
    table[77] = &Unit::HandleMechanicImmuneResistanceAuraProc;  // SPELL_AURA_MECHANIC_IMMUNITY
    table[85] = &Unit::HandleCantTrigger;  // SPELL_AURA_MOD_POWER_REGEN
    table[109] = &Unit::HandleAddTargetTriggerAuraProc;  // SPELL_AURA_ADD_TARGET_TRIGGER
    table[112] = &Unit::HandleOverrideClassScriptAuraProc;  // SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    table[117] = &Unit::HandleMechanicImmuneResistanceAuraProc;  // SPELL_AURA_MOD_MECHANIC_RESISTANCE
    table[138] = &Unit::HandleHasteAuraProc;  // SPELL_AURA_MOD_MELEE_HASTE
    table[149] = &Unit::HandleCantTrigger;  // SPELL_AURA_RESIST_PUSHBACK
    return table;
}

std::array<pAuraProcHandler, TOTAL_AURAS> AuraProcHandler = BuildAuraProcHandlers();

// Fonctions Nostalrius
inline bool SpellCanTrigger(SpellEntry const* spellProto, SpellEntry const* procSpell, uint8 eff_idx = 0)
{
    return (procSpell && procSpell->SpellFamilyName == spellProto->SpellFamilyName && procSpell->SpellFamilyFlags & spellProto->EffectItemType[eff_idx]);
}

SpellProcEventTriggerCheck Unit::IsTriggeredAtSpellProcEvent(Unit* pVictim, SpellAuraHolder* holder, SpellEntry const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, SpellProcEventEntry const*& spellProcEvent, bool isSpellTriggeredByAuraOrItem) const
{
    SpellEntry const* spellProto = holder->GetSpellProto();
    /*
    if (procSpell)
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Flag : 0x%x, Extr : 0x%x. ProcSpell %u Aura %u",
            procFlag, procExtra, procSpell->Id, spellProto->Id);
    else
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Flag : 0x%x, Extr : 0x%x. Aura %u (ICON %u)",
            procFlag, procExtra, spellProto->Id, spellProto->SpellIconID);*/

    // Flurry can't proc on additional windfury attacks
    if (spellProto->HasTag(SPELL_TAG_FLURRY) && m_extraAttacks)
        return SPELL_PROC_TRIGGER_FAILED;

    // Don't proc weapons on Sap
    if (spellProto->Id == 14076 || spellProto->Id == 14094 || spellProto->Id == 14095)
        return SPELL_PROC_TRIGGER_FAILED;

    // [TODO]
    // Delete all these spells, and manage it via the DB (spell_proc_event)
    if (procSpell && !(procExtra & PROC_EX_CAST_END))
    {
        // Eye for an Eye
        if (spellProto->Id == 9799 || spellProto->Id == 25988)
        {
            if (!((procFlag & PROC_FLAG_TAKE_HARMFUL_SPELL) && (procExtra & PROC_EX_CRITICAL_HIT)))
                return SPELL_PROC_TRIGGER_FAILED;
        }
        // Improved Lay on Hands
        if (sSpellMgr.GetFirstSpellInChain(spellProto->Id) == 20234)
        {
            if (procSpell->SpellFamilyName == SPELLFAMILY_PALADIN && procSpell->Category == 56 && !isVictim)
                return SPELL_PROC_TRIGGER_OK;
            else
                return SPELL_PROC_TRIGGER_FAILED;
        }
        // Wrath of Cenarius - Spell Blasting
        if (spellProto->Id == 25906)
        {
            // Should be able to proc when negative magical effect lands on a target.
            if (!isVictim && (procSpell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC) && !procSpell->IsPositiveSpell() && (procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT)) && !(procSpell->IsSpellAppliesAura() && (procFlag & PROC_FLAG_DEAL_HARMFUL_PERIODIC)))
            {
                if (roll_chance_f((float)spellProto->procChance))
                    return SPELL_PROC_TRIGGER_OK;
                else
                    return SPELL_PROC_TRIGGER_ROLL_FAILED;
            }
        }
        // DRUID
        // Omen of Clarity
        if (spellProto->Id == 16864)
        {
            if (!procSpell && (procFlag & (PROC_FLAG_DEAL_MELEE_SWING | PROC_FLAG_DEAL_MELEE_ABILITY)))
            {
                if (roll_chance_u(10))
                    return SPELL_PROC_TRIGGER_OK;
                else
                    return SPELL_PROC_TRIGGER_ROLL_FAILED;
            }
            return SPELL_PROC_TRIGGER_FAILED;
        }
        // PRIEST
        // Inspiration
        if (spellProto->HasTag(SPELL_TAG_INSPIRATION) && spellProto->SpellFamilyName == SPELLFAMILY_PRIEST)
        {
            if (procSpell->IsFitToFamily<SPELLFAMILY_PRIEST, CF_PRIEST_PRAYER_OF_HEALING, CF_PRIEST_HEAL,
                CF_PRIEST_FLASH_HEAL, CF_PRIEST_FLASH_HEAL2, CF_PRIEST_GREATER_HEAL>() &&
                procExtra & PROC_EX_CRITICAL_HIT && procFlag & PROC_FLAG_DEAL_HELPFUL_SPELL)
                return SPELL_PROC_TRIGGER_OK;
            else
                return SPELL_PROC_TRIGGER_FAILED;
        }
        // SPELL_AURA_ADD_TARGET_PROC
        // Chance of proc calculated after.
        if (spellProto->EffectApplyAuraName[0] == SPELL_AURA_ADD_TARGET_TRIGGER)
        {
            // All spells that proc on the victim
            // Then the others
            if (isVictim)
                return SPELL_PROC_TRIGGER_FAILED;

            switch (spellProto->Id)
            {
                // Frosty Zap
                case 24392:
                    if (SpellCanTrigger(spellProto, procSpell))
                        return SPELL_PROC_TRIGGER_OK;
                    break;
            }

            return SPELL_PROC_TRIGGER_FAILED;
        }

        // World of Warcraft Client Patch 1.10.0 (2006-03-28)
        // - Execute - This ability will now work with Sweeping Strikes again. If
        //   the second victim is below 20 % health, they will be hit with the full
        //   Execute amount.If the second victim is not below 20 % health, they
        //   will be hit with normal melee swing damage.
        // - Whirlwind - When this ability is used with Sweeping Strikes, it will
        //   burn only one charge of Sweeping Strikes and will generate only one
        //   additional attack.

        // SHAMAN
        // Elemental Mastery
        // Do not consume aura if spell did not benefit from crit chance bonus.
        // Can happen if aura was cast after damaging spell was already launched.
        if (spellProto->Id == 16166 && (procExtra & PROC_EX_NORMAL_HIT))
            return SPELL_PROC_TRIGGER_FAILED;
    }

    if (holder->GetAuraScript())
        if (auto result = holder->GetAuraScript()->OnCheckProc(this, pVictim, holder, procSpell, procFlag, procExtra, attType, isVictim))
            return result.value();

    // Get proc Event Entry
    spellProcEvent = sSpellMgr.GetSpellProcEvent(spellProto->Id);

    // Custom hard-coded cases which depend on the proc event for firing...
    if (procSpell)
    {
        // Fear Ward always procs on any Fear (except ones cast by ourselves...)
        if (spellProto->Id == 6346 && isVictim)
        {
            if (procSpell->Mechanic == MECHANIC_FEAR)
                return SPELL_PROC_TRIGGER_OK;
            else
                return SPELL_PROC_TRIGGER_FAILED;
        }
    }

    // Get EventProcFlag
    uint32 EventProcFlag;
    EventProcFlag = spellProto->procFlags;
    // Continue if no trigger exist
    if (!EventProcFlag)
        return SPELL_PROC_TRIGGER_FAILED;

    // Check spellProcEvent data requirements
    if (!SpellMgr::IsSpellProcEventCanTriggeredBy(spellProcEvent, EventProcFlag, procSpell, procFlag, procExtra))
        return SPELL_PROC_TRIGGER_FAILED;

    // In most cases req get honor or XP from kill
    if ((EventProcFlag & PROC_FLAG_KILL) && IsPlayer())
    {
        bool allow = ((Player*)this)->IsHonorOrXPTarget(pVictim);
        if (!allow)
            return SPELL_PROC_TRIGGER_FAILED;
    }
    // Aura added by spell can`t trigger from self (prevent drop charges/do triggers)
    // But except periodic triggers (can triggered from self)
    if (procSpell && procSpell->Id == spellProto->Id && !(EventProcFlag & PROC_FLAG_TAKE_HARMFUL_PERIODIC))
        return SPELL_PROC_TRIGGER_FAILED;

    // Check if current equipment allows aura to proc
    if (!isVictim && IsPlayer() && !spellProto->HasAttribute(SPELL_ATTR_EX3_NO_PROC_EQUIP_REQUIREMENT))
    {
        if (spellProto->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            Item *item = nullptr;
            if (attType == BASE_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            else if (attType == OFF_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            else
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

            if (!CanUseEquippedWeapon(attType))
                return SPELL_PROC_TRIGGER_FAILED;

            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_WEAPON || !((1 << item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                return SPELL_PROC_TRIGGER_FAILED;
        }
        else if (spellProto->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_ARMOR || !((1 << item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                return SPELL_PROC_TRIGGER_FAILED;
        }
    }

    if (isSpellTriggeredByAuraOrItem && procSpell &&
        !procSpell->HasAttribute(SPELL_ATTR_EX3_NOT_A_PROC) &&
        !spellProto->HasAttribute(SPELL_ATTR_EX3_CAN_PROC_FROM_PROCS))
        return SPELL_PROC_TRIGGER_FAILED;

    if (spellProto->HasAttribute(SPELL_ATTR_EX3_ONLY_PROC_OUTDOORS) &&
       !GetTerrain()->IsOutdoors(GetPositionX(), GetPositionY(), GetPositionZ()))
        return SPELL_PROC_TRIGGER_FAILED;

    if (spellProto->HasAttribute(SPELL_ATTR_EX3_ONLY_PROC_ON_CASTER) &&
        holder->GetTarget()->GetObjectGuid() != holder->GetCasterGuid())
        return SPELL_PROC_TRIGGER_FAILED;

    // Get chance from spell
    float chance = (float)spellProto->procChance;

    // If PPM exist calculate chance from PPM
    if (!isVictim && spellProcEvent && spellProto->ProcsPerMinute != 0)
    {
        uint32 WeaponSpeed = GetAttackTime(attType);
        chance = GetPPMProcChance(WeaponSpeed, spellProto->ProcsPerMinute);
    }

    // Apply chance modifier aura
    if (Player* modOwner = GetSpellModOwner())
    {
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CHANCE_OF_SUCCESS, chance);
        if (modOwner->HasCheatOption(PLAYER_CHEAT_ALWAYS_PROC))
            return SPELL_PROC_TRIGGER_OK;
    }

    if (roll_chance_f(chance))
        return SPELL_PROC_TRIGGER_OK;

    return SPELL_PROC_TRIGGER_ROLL_FAILED;
}

SpellAuraProcResult Unit::TriggerProccedSpell(Unit* target, int32* basepoints, uint32 triggeredSpellId, Item* castItem, Aura* triggeredByAura, uint32 cooldown, ObjectGuid originalCaster, SpellEntry const* triggeredByParent)
{
    SpellEntry const* triggerEntry = sSpellMgr.GetSpellEntry(triggeredSpellId);

    if (!triggerEntry)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::TriggerProccedSpell: Script has nonexistent triggered spell %u", triggeredSpellId);
        return SPELL_AURA_PROC_FAILED;
    }

    return TriggerProccedSpell(target, basepoints, triggerEntry, castItem, triggeredByAura, cooldown);
}

SpellAuraProcResult Unit::TriggerProccedSpell(Unit* target, int32* basepoints, SpellEntry const* spellInfo, Item* castItem, Aura* triggeredByAura, uint32 cooldown, ObjectGuid originalCaster, SpellEntry const* triggeredByParent)
{
    // default case
    if (!target || (target != this && !target->IsAlive()))
        return SPELL_AURA_PROC_FAILED;

    if (!IsSpellReady(spellInfo))
        return SPELL_AURA_PROC_FAILED;

    if (basepoints && (basepoints[EFFECT_INDEX_0] || basepoints[EFFECT_INDEX_1] || basepoints[EFFECT_INDEX_2]))
        CastCustomSpell(target, spellInfo,
            basepoints[EFFECT_INDEX_0] ? basepoints[EFFECT_INDEX_0] : 0,
            basepoints[EFFECT_INDEX_1] ? basepoints[EFFECT_INDEX_1] : 0,
            basepoints[EFFECT_INDEX_2] ? basepoints[EFFECT_INDEX_2] : 0,
            true, castItem, triggeredByAura);
    else
        CastSpell(target, spellInfo, true, castItem, triggeredByAura, originalCaster, nullptr, triggeredByParent);

    if (cooldown)
        AddCooldown(spellInfo, nullptr, false, cooldown);

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleHasteAuraProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* /*procSpell*/, uint32 /*procFlag*/, uint32 procEx, uint32 cooldown)
{
    // Flurry: last charge crit will reapply the buff, don't remove any charges
    if (triggeredByAura->GetSpellProto()->HasTag(SPELL_TAG_FLURRY) &&
        triggeredByAura->GetHolder()->GetAuraCharges() <= 1 &&
        (procEx & PROC_EX_CRITICAL_HIT))
        return SPELL_AURA_PROC_FAILED;

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleDummyAuraProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    SpellEntry const* dummySpell = triggeredByAura->GetSpellProto();
    float  triggerAmount = triggeredByAura->GetModifier()->m_amount;

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    uint32 triggered_spell_id = 0;
    Unit* target = pVictim;
    int32  basepoints[MAX_EFFECT_INDEX] = {0, 0, 0};

    switch (dummySpell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        case SPELLFAMILY_WARRIOR:                          // Sweeping Strikes and Retaliation are warrior spells in db2
        {
            switch (dummySpell->Id)
            {
                // Illusion Passive
                case 7131:
                {
                    if (Creature* pCreature = ToCreature())
                        pCreature->DespawnOrUnsummon(1);
                    return SPELL_AURA_PROC_OK;
                }
                // Eye for an Eye
                case 9799:
                case 25988:
                {
                    // prevent damage back from weapon special attacks
                    if (!procSpell || procSpell->DmgClass != SPELL_DAMAGE_CLASS_MAGIC)
                        return SPELL_AURA_PROC_FAILED;

                    // return damage % to attacker but < 50% own total health
                    basepoints[0] = rand_dither(triggerAmount * int32(originalAmount) / 100);
                    if (basepoints[0] > (int32)GetMaxHealth() / 2)
                        basepoints[0] = (int32)GetMaxHealth() / 2;

                    triggered_spell_id = 25997;
                    break;
                }
                // Sweeping Strikes
                case 12292:
                case 18765:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // dont trigger from non damaging spells, amount is 1 for non damaging spells if they hit
                    // tested on classic that rend does not trigger sweeping strikes
                    if (amount <= 1)
                        return SPELL_AURA_PROC_FAILED;

                    // Prevent chain of triggered spell from same triggered spell
                    if (procSpell && (procSpell->Id == 26654 || procSpell->Id == 12723))
                        return SPELL_AURA_PROC_FAILED;

                    // Fix range for target selection when proccing SS with whirlwind. Whirlwind doesn't
                    // have a radius set on its prototype, but it is 8 yards.
                    float radius = ATTACK_DISTANCE;
                    if (procSpell && procSpell->Id == 1680)
                        radius = 8.0f;

                    // World of Warcraft Client Patch 1.7.0 (2005-09-13)
                    // - Sweeping Strikes will now ignore dead targets, and will ignore PvP
                    //   enabled targets if you are not PvP enabled.
                    target = SelectRandomUnfriendlyTarget(pVictim, radius, false, true, true);
                    if (!target)
                        return SPELL_AURA_PROC_FAILED;

                    // World of Warcraft Client Patch 1.10.0 (2006-03-28)
                    // - Execute - This ability will now work with Sweeping Strikes again. If
                    //   the second victim is below 20 % health, they will be hit with the full
                    //   Execute amount.If the second victim is not below 20 % health, they
                    //   will be hit with normal melee swing damage.
                    // Case for Execute. This will only run when procced by Execute
                    if (procSpell && procSpell->Id == 20647)
                    {
                        if (pVictim->GetHealthPercent() <= 20.0f && target->GetHealthPercent() <= 20.0f)  // If Both Target A and target B is less or equal than 20% do full damage
                        {
                            basepoints[0] = rand_ditheru(amount * 100 / CalcArmorReducedDamage(pVictim, 100));
                            triggered_spell_id = 12723; // Note this SS id deals 1 damage by itself (Cannot crit)
                        }
                        else if (pVictim->GetHealthPercent() <= 20.0f)    // If only Target A is less or equal than 20% and target B is over 20% do Basic attack damage
                        {
                            triggered_spell_id = 26654; // This SS deals damage equal to AA also this spell ID can crit ?? Maybe this explains the rumor of SS criting since it only scales with spell crit ? = 5% crit.
                        }
                        else // Full damage on anything else (Shouldn't really ever be used) since execute can only be used less or equal than 20% anyway.
                        {
                            basepoints[0] = rand_ditheru(amount * 100 / CalcArmorReducedDamage(pVictim, 100));
                            triggered_spell_id = 12723; // Note this SS id deals 1 damage by itself (Cannot crit)
                        }
                    }
                    else // Full damage on anything else
                    {
                        basepoints[0] = rand_ditheru(amount * 100 / CalcArmorReducedDamage(pVictim, 100));
                        triggered_spell_id = 12723; // Note this SS id deals 1 damage by itself (Cannot crit)
                    }

                    break;
                }
                // Retaliation
                case 20230:
                {
                    // check attack comes not from behind
                    if (!HasInArc(pVictim))
                        return SPELL_AURA_PROC_FAILED;

                    // World of Warcraft Client Patch 1.7.0 (2005-09-13)
                    // - Retaliation - Will now cause a maximum of 30 retaliatory strikes in
                    //  15 seconds.In addition, retaliatory strikes will not be possible
                    //  while stunned.
                    if (HasUnitState(UNIT_STATE_CAN_NOT_REACT))
                        return SPELL_AURA_PROC_FAILED;

                    triggered_spell_id = 22858;
                    break;
                }
                // Twisted Reflection (boss spell)
                case 21063:
                    triggered_spell_id = 21064;
                    break;
                // Unstable Power
                case 24658:
                {
                    // Need to remove one 24659 aura
                    // It does consume 2 charges on using Holy Nova, confirmed on classic era ptr.
                    RemoveAuraHolderFromStack(24659);
                    return SPELL_AURA_PROC_OK;
                }
                // Restless Strength
                case 24661:
                {
                    // Need remove one 24662 aura
                    RemoveAuraHolderFromStack(24662);
                    return SPELL_AURA_PROC_OK;
                }
                // Viscidus Frost Weakness
                // Disabled for now, handled on Viscidus script
                /*
                case 25926:
                {
                    if (!procSpell)
                        return SPELL_AURA_PROC_FAILED;

                    if (GetSchoolMask(procSpell->School) == SPELL_SCHOOL_MASK_FROST)
                    {
                        ++triggeredByAura->GetModifier()->m_amount;
                        triggerAmount = triggeredByAura->GetModifier()->m_amount;

                        if (triggerAmount == 100)
                            triggered_spell_id = 26034; // Slowed
                        else if (triggerAmount == 150)
                            triggered_spell_id = 26036; // Slowed more
                        else if (triggerAmount == 200)
                            triggered_spell_id = 25937; // Freeze
                    }

                    break;
                }
                */
                // Viscidus Freeze
                case 25937:
                {
                    if (procSpell)
                    {
                        // Wand spell entry marked as physical, we need to add an exception here
                        if (GetSchoolMask(procSpell->School) != SPELL_SCHOOL_MASK_NORMAL || procSpell->Id == 5019)
                            return SPELL_AURA_PROC_FAILED;
                    }

                    ++triggeredByAura->GetModifier()->m_amount;
                    triggerAmount = triggeredByAura->GetModifier()->m_amount;

                    if (triggerAmount == 50)
                        MonsterTextEmote(11346, this, true); // begins to crack!
                    else if (triggerAmount == 100)
                        MonsterTextEmote(11347, nullptr, true); // looks ready to shatter!
                    else if (triggerAmount == 150)
                    {
                        RemoveAurasDueToSpell(25937);
                        triggered_spell_id = 25938; // Explode
                    }

                    break;
                }
                // Adaptive Warding (Frostfire Regalia set)
                case 28764:
                {
                    if (!procSpell)
                        return SPELL_AURA_PROC_FAILED;

                    // find Mage Armor
                    bool found = false;
                    AuraList const& mRegenInterupt = GetAurasByType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
                    for (const auto iter : mRegenInterupt)
                    {
                        if (SpellEntry const* iterSpellProto = iter->GetSpellProto())
                        {
                            if (iterSpellProto->IsFitToFamily<SPELLFAMILY_MAGE, CF_MAGE_MAGE_ARMOR>())
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found)
                        return SPELL_AURA_PROC_FAILED;

                    switch (GetFirstSchoolInMask(procSpell->GetSpellSchoolMask()))
                    {
                        case SPELL_SCHOOL_NORMAL:
                        case SPELL_SCHOOL_HOLY:
                            return SPELL_AURA_PROC_FAILED;                   // ignored
                        case SPELL_SCHOOL_FIRE:
                            triggered_spell_id = 28765;
                            break;
                        case SPELL_SCHOOL_NATURE:
                            triggered_spell_id = 28768;
                            break;
                        case SPELL_SCHOOL_FROST:
                            triggered_spell_id = 28766;
                            break;
                        case SPELL_SCHOOL_SHADOW:
                            triggered_spell_id = 28769;
                            break;
                        case SPELL_SCHOOL_ARCANE:
                            triggered_spell_id = 28770;
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }

                    target = this;
                    break;
                }
                // Obsidian Armor (Justice Bearer`s Pauldrons shoulder)
                case 27539:
                {
                    if (!procSpell)
                        return SPELL_AURA_PROC_FAILED;

                    switch (GetFirstSchoolInMask(procSpell->GetSpellSchoolMask()))
                    {
                        case SPELL_SCHOOL_NORMAL:
                            return SPELL_AURA_PROC_FAILED;                   // ignore
                        case SPELL_SCHOOL_HOLY:
                            triggered_spell_id = 27536;
                            break;
                        case SPELL_SCHOOL_FIRE:
                            triggered_spell_id = 27533;
                            break;
                        case SPELL_SCHOOL_NATURE:
                            triggered_spell_id = 27538;
                            break;
                        case SPELL_SCHOOL_FROST:
                            triggered_spell_id = 27534;
                            break;
                        case SPELL_SCHOOL_SHADOW:
                            triggered_spell_id = 27535;
                            break;
                        case SPELL_SCHOOL_ARCANE:
                            triggered_spell_id = 27540;
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }

                    target = this;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Magic Absorption
            if (sSpellMgr.GetFirstSpellInChain(dummySpell->Id) == 29441)
            {
                if (GetPowerType() != POWER_MANA)
                    return SPELL_AURA_PROC_FAILED;

                // mana reward
                basepoints[0] = rand_dither(triggerAmount * GetMaxPower(POWER_MANA) / 100);
                target = this;
                triggered_spell_id = 29442;
                break;
            }
            // Master of Elements
            if (sSpellMgr.GetFirstSpellInChain(dummySpell->Id) == 29074)
            {
                if (!procSpell)
                    return SPELL_AURA_PROC_FAILED;

                // mana cost save
                int32 cost = procSpell->manaCost + procSpell->ManaCostPercentage * GetCreateMana() / 100;
                basepoints[0] = rand_dither(cost * triggerAmount / 100);
                if (basepoints[0] <= 0)
                    return SPELL_AURA_PROC_FAILED;

                target = this;
                triggered_spell_id = 29077;
                break;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
            break;
        case SPELLFAMILY_PRIEST:
        {
            switch (dummySpell->Id)
            {
                // Vampiric Embrace
                case 15286:
                {
                    if (!pVictim || !pVictim->IsAlive())
                        return SPELL_AURA_PROC_FAILED;

                    // pVictim is caster of aura
                    if (triggeredByAura->GetCasterGuid() != pVictim->GetObjectGuid())
                        return SPELL_AURA_PROC_FAILED;

                    // heal amount
                    basepoints[0] = rand_dither(triggerAmount * amount / 100);

                    // don't heal for 0
                    if (basepoints[0] < 1)
                        basepoints[0] = 1;

                    pVictim->CastCustomSpell(pVictim, 15290, basepoints[0], {}, {}, true, castItem, triggeredByAura);
                    return SPELL_AURA_PROC_OK;                                // no hidden cooldown
                }
                // Oracle Healing Bonus ("Garments of the Oracle" set)
                case 26169:
                {
                    // heal amount
                    basepoints[0] = int32(amount * 0.1f);
                    if (!basepoints[0])
                        return SPELL_AURA_PROC_FAILED;

                    target = this;
                    triggered_spell_id = 26170;
                    break;
                }
                // Greater Heal (Vestments of Faith (Priest Tier 3) - 4 pieces bonus)
                case 28809:
                {
                    triggered_spell_id = 28810;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (dummySpell->Id)
            {
                // Healing Touch (Dreamwalker Raiment set)
                case 28719:
                {
                    // mana back
                    basepoints[0] = int32(procSpell->manaCost * 30 / 100);
                    target = this;
                    triggered_spell_id = 28742;
                    break;
                }
                // Healing Touch Refund (Idol of Longevity trinket)
                case 28847:
                {
                    target = this;
                    triggered_spell_id = 28848;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            switch (dummySpell->Id)
            {
                // Clean Escape
                case 23582:
                    // triggered spell have same masks and etc with main Vanish spell
                    if (!procSpell || procSpell->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_NONE)
                        return SPELL_AURA_PROC_FAILED;
                    triggered_spell_id = 23583;
                    break;

                // Blade Flurry
                case 13877:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // prevent chain of triggered spell from same triggered spell
                    if (procSpell && procSpell->Id == 22482)
                        return SPELL_AURA_PROC_FAILED;

                    target = SelectRandomUnfriendlyTarget(pVictim, 5.0f, false, true);

                    if (!target)
                        return SPELL_AURA_PROC_FAILED;

                     // Reconstitute damage before armor reduction
                    basepoints[0] = rand_ditheru(amount * 100 / CalcArmorReducedDamage(pVictim, 100));

                    triggered_spell_id = 22482;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
            break;
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Righteousness - melee proc dummy
            if ((dummySpell->IsFitToFamilyMask<CF_PALADIN_SEALS>()) && triggeredByAura->GetEffIndex() == EFFECT_INDEX_0)
            {
                if (!pVictim || !pVictim->IsAlive())
                    return SPELL_AURA_PROC_FAILED;

                if (GetTypeId() != TYPEID_PLAYER)
                    return SPELL_AURA_PROC_FAILED;

                uint32 spellId;
                switch (triggeredByAura->GetId())
                {
                    case 20154:
                    case 21084:
                        spellId = 25742;
                        break;     // Rank 1
                    case 20287:
                        spellId = 25740;
                        break;     // Rank 2
                    case 20288:
                        spellId = 25739;
                        break;     // Rank 3
                    case 20289:
                        spellId = 25738;
                        break;     // Rank 4
                    case 20290:
                        spellId = 25737;
                        break;     // Rank 5
                    case 20291:
                        spellId = 25736;
                        break;     // Rank 6
                    case 20292:
                        spellId = 25735;
                        break;     // Rank 7
                    case 20293:
                        spellId = 25713;
                        break;     // Rank 8
                    default:
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleDummyAuraProc: non handled possibly SoR (Id = %u)", triggeredByAura->GetId());
                        return SPELL_AURA_PROC_FAILED;
                }
                float MAX_WSP = 4.0f;
                float MIN_WSP = 1.5f;

                Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                float speed = (item ? item->GetProto()->Delay : BASE_ATTACK_TIME) / 1000.0f;

                float minDmg = triggerAmount / 87.0f;
                float maxDmg = triggerAmount / 25.0f;

                float damageBasePoints = (maxDmg - minDmg) * ((speed - MIN_WSP) / (MAX_WSP - MIN_WSP)) + minDmg;

                // Apply Improved Seal of Rightousness talent
                // Modifier is applied on base damage only (changed patch 2.1.0)
                uint32 impSoRList[] = { 20224, 20225, 20330, 20331, 20332 };
                for (uint32 i : impSoRList) {
                    SpellModifier *mod = ((Player*)this)->GetSpellMod(SPELLMOD_ALL_EFFECTS, i);
                    if (mod && mod->type == SPELLMOD_PCT && mod->value > 0)
                        damageBasePoints += damageBasePoints*(float)mod->value / 100.0f;
                }

                float damagePoint = damageBasePoints;

                // apply damage bonuses manually
                if (damagePoint >= 0)
                {
                    damagePoint = SpellDamageBonusDone(pVictim, dummySpell, EFFECT_INDEX_0, damagePoint, SPELL_DIRECT_DAMAGE);
                    damagePoint = pVictim->SpellDamageBonusTaken(this, dummySpell, EFFECT_INDEX_0, damagePoint, SPELL_DIRECT_DAMAGE);
                }

                CastCustomSpell(pVictim, spellId, rand_dither(damagePoint), {}, {}, true, nullptr, triggeredByAura);
                // Seal of Righteousness can proc weapon enchants. mechanic removed in 2.1.0
                ((Player*)this)->CastItemCombatSpell(pVictim, BASE_ATTACK);
                return SPELL_AURA_PROC_OK;                                // no hidden cooldown
            }

            switch (dummySpell->Id)
            {
                // Holy Power (Redemption Armor set)
                case 28789:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28795;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28793;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28791;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28790;     // Increases the friendly target's armor
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            switch (dummySpell->Id)
            {
                // Totemic Power (The Earthshatterer set)
                case 28823:
                {
                    if (!pVictim)
                        return SPELL_AURA_PROC_FAILED;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28824;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28825;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28826;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28827;     // Increases the friendly target's armor
                            break;
                        default:
                            return SPELL_AURA_PROC_FAILED;
                    }
                    break;
                }
                // Lesser Healing Wave (Totem of Flowing Water Relic)
                case 28849:
                {
                    target = this;
                    triggered_spell_id = 28850;
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    // processed charge only counting case
    if (!triggered_spell_id)
        return SPELL_AURA_PROC_OK;

    return TriggerProccedSpell(target, basepoints, triggered_spell_id, castItem, triggeredByAura, cooldown);
}

SpellAuraProcResult Unit::HandleProcTriggerSpellAuraProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 procFlags, uint32 procEx, uint32 cooldown)
{
    // Get triggered aura spell info
    SpellEntry const* auraSpellInfo = triggeredByAura->GetSpellProto();

    // Basepoints of trigger aura
    float triggerAmount = triggeredByAura->GetModifier()->m_amount;

    // Set trigger spell id, target, custom basepoints
    uint32 trigger_spell_id = auraSpellInfo->EffectTriggerSpell[triggeredByAura->GetEffIndex()];
    Unit*  target = nullptr;
    int32  basepoints[MAX_EFFECT_INDEX] = {0, 0, 0};

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    // Try handle unknown trigger spells
    // Custom requirements (not listed in procEx) Warning! damage dealing after this
    // Custom triggered spells
    switch (auraSpellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (auraSpellInfo->Id)
            {
                //case 5301:  break;                        // Defensive State (DND)
                //case 7137:  break;                        // Shadow Charge (Rank 1)
                //case 7377:  break;                        // Take Immune Periodic Damage <Not Working>
                //case 13358: break;                        // Defensive State (DND)
                //case 16092: break;                        // Defensive State (DND)
                //case 18943: break;                        // Double Attack
                //case 19194: break;                        // Double Attack
                //case 19817: break;                        // Double Attack
                //case 19818: break;                        // Double Attack
                //case 22835: break;                        // Drunken Rage
                //    trigger_spell_id = 14822; break;
                case 23780:                                 // Aegis of Preservation (Aegis of Preservation trinket)
                    trigger_spell_id = 23781;
                    break;
                //case 24949: break;                        // Defensive State 2 (DND)
                case 27522:                                 // Mana Drain Trigger
                {
                    // On successful melee or ranged attack gain $29471s1 mana and if possible drain $27526s1 mana from the target.
                    if (IsAlive())
                        CastSpell(this, 29471, true, castItem, triggeredByAura);
                    if (pVictim && pVictim->IsAlive())
                        CastSpell(pVictim, 27526, true, castItem, triggeredByAura);
                    return SPELL_AURA_PROC_OK;
                }
                case 31255:                                 // Deadly Swiftness (Rank 1)
                {
                    // whenever you deal damage to a target who is below 20% health.
                    //if (pVictim->GetHealth() > pVictim->GetMaxHealth() / 5)
                        //return SPELL_AURA_PROC_FAILED;

                    target = this;
                    trigger_spell_id = 22588;
                    break;
                }
                case 28200:                                 // Talisman of Ascendance
                {
                    if (procSpell && (procSpell->IsAreaOfEffectSpell() || procSpell->Effect[0] == SPELL_EFFECT_SCRIPT_EFFECT))
                        return SPELL_AURA_PROC_FAILED;
                    break;
                }
            }
            break;
        case SPELLFAMILY_MAGE:
            if (auraSpellInfo->Id == 26467)                 // Persistent Shield (Scarab Brooch trinket)
            {
                // This spell originally trigger 13567 - Dummy Trigger (vs dummy effect)
                basepoints[0] = amount * 15 / 100;
                target = pVictim;
                trigger_spell_id = 26470;
            }
            break;
        case SPELLFAMILY_WARRIOR:
            break;
        case SPELLFAMILY_WARLOCK:
        {
            // Pyroclasm
            if (sSpellMgr.GetFirstSpellInChain(auraSpellInfo->Id) == 18096)
            {
                if (!pVictim || !pVictim->IsAlive() || pVictim == this || procSpell == nullptr)
                    return SPELL_AURA_PROC_FAILED;
                // Calculate spell tick count for spells
                uint32 tick = 1; // Default tick = 1

                if (procSpell->HasTag(SPELL_TAG_SOUL_FIRE))
                    tick = 1; // Feu de l'ame
                // Hellfire have 15 tick
                else if (procSpell->IsFitToFamilyMask<CF_WARLOCK_HELLFIRE>())
                    tick = 15;
                // Rain of Fire have 4 tick
                else if (procSpell->IsFitToFamilyMask<CF_WARLOCK_RAIN_OF_FIRE>())
                    tick = 4;
                else
                    return SPELL_AURA_PROC_FAILED;

                // Calculate chance = baseChance / tick
                float chance = 0;
                switch (auraSpellInfo->Id)
                {
                    case 18096: // Rank 1
                        chance = 13.0f / tick;
                        break;
                    case 18073: // Rank 2
                        chance = 26.0f / tick;
                        break;
                }

                // Roll chance
                if (!roll_chance_f(chance))
                    return SPELL_AURA_PROC_FAILED;

                trigger_spell_id = 18093;
            }
            // Cheat Death
            else if (auraSpellInfo->Id == 28845)
            {
                // When your health drops below 20% ....
                int32 health20 = int32(GetMaxHealth()) / 5;
                if (int32(GetHealth()) - int32(amount) >= health20 || int32(GetHealth()) < health20)
                    return SPELL_AURA_PROC_FAILED;
            }
            // Consequences ne proc pas sur soi-meme (Flammes infernales)
            else if (sSpellMgr.GetFirstSpellInChain(auraSpellInfo->Id) == 18119)
            {
                if (pVictim == this)
                    return SPELL_AURA_PROC_FAILED;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Shadowguard
            if (sSpellMgr.GetFirstSpellInChain(auraSpellInfo->Id) == 18137)
            {
                switch (auraSpellInfo->Id)
                {
                    case 18137:
                        trigger_spell_id = 28377;
                        break;   // Rank 1
                    case 19308:
                        trigger_spell_id = 28378;
                        break;   // Rank 2
                    case 19309:
                        trigger_spell_id = 28379;
                        break;   // Rank 3
                    case 19310:
                        trigger_spell_id = 28380;
                        break;   // Rank 4
                    case 19311:
                        trigger_spell_id = 28381;
                        break;   // Rank 5
                    case 19312:
                        trigger_spell_id = 28382;
                        break;   // Rank 6
                    default:
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u not handled in SG", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }
            }
            // Blessed Recovery
            else if (sSpellMgr.GetFirstSpellInChain(auraSpellInfo->Id) == 27811)
            {
                switch (auraSpellInfo->Id)
                {
                    case 27811:
                        trigger_spell_id = 27813;
                        break;
                    case 27815:
                        trigger_spell_id = 27817;
                        break;
                    case 27816:
                        trigger_spell_id = 27818;
                        break;
                    default:
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u not handled in BR", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }
                basepoints[0] = rand_dither(amount * triggerAmount / 100 / 3);
                target = this;
            }
            break;
        }
        case SPELLFAMILY_DRUID:
            break;
        case SPELLFAMILY_HUNTER:
        {
            switch (auraSpellInfo->Id)
            {
                case 5118: // Aspect of the Cheetah
                case 13159: // Aspect of the Pack
                {
                    // dont trigger from non damaging spells, amount is 1 for non damaging spells if they hit
                    if (amount <= 1)
                        return SPELL_AURA_PROC_FAILED;

                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Judgement of Light and Judgement of Wisdom
            if (auraSpellInfo->IsFitToFamilyMask<CF_PALADIN_JUDGEMENT_OF_WISDOM_LIGHT>())
            {
                switch (auraSpellInfo->Id)
                {
                    // Judgement of Light
                    case 20185:
                        trigger_spell_id = 20267;
                        break; // Rank 1
                    case 20344:
                        trigger_spell_id = 20341;
                        break; // Rank 2
                    case 20345:
                        trigger_spell_id = 20342;
                        break; // Rank 3
                    case 20346:
                        trigger_spell_id = 20343;
                        break; // Rank 4
                    // Judgement of Wisdom
                    case 20186:
                        trigger_spell_id = 20268;
                        break; // Rank 1
                    case 20354:
                        trigger_spell_id = 20352;
                        break; // Rank 2
                    case 20355:
                        trigger_spell_id = 20353;
                        break; // Rank 3
                    default:
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u miss posibly Judgement of Light/Wisdom", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }

                // Need to pass victim guid so its not overwritten with aura caster.
                // Intentionally do not pass triggeredByAura here.
                // Seal of Light healing is done by the person who attacks,
                // and does not increase threat of the original caster.
                pVictim->CastSpell(pVictim, trigger_spell_id, true, castItem, triggeredByAura, pVictim->GetObjectGuid());
                return SPELL_AURA_PROC_OK;                        // no hidden cooldown
            }
            // Illumination
            else if (sSpellMgr.GetFirstSpellInChain(auraSpellInfo->Id) == 20210)
            {
                if (!procSpell)
                    return SPELL_AURA_PROC_FAILED;

                Player* pPlayer = ToPlayer();
                if (!pPlayer)
                    return SPELL_AURA_PROC_FAILED;

                // procspell is triggered spell but we need mana cost of original casted spell
                SpellEntry const* originalSpell = procSpell;

                // Holy Shock
                if (procSpell->IsFitToFamilyMask<CF_PALADIN_HOLY_SHOCK>())
                {
                    uint32 originalSpellId;
                    switch (procSpell->Id)
                    {
                        case 25914: originalSpellId = 20473; break;
                        case 25913: originalSpellId = 20929; break;
                        case 25903: originalSpellId = 20930; break;
                        default:
                            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u not handled in HShock", procSpell->Id);
                            return SPELL_AURA_PROC_FAILED;
                    }
                    originalSpell = sSpellMgr.GetSpellEntry(originalSpellId);
                }

                basepoints[0] = originalSpell->manaCost;
                trigger_spell_id = 20272;
                target = this;
                break;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Lightning Shield (overwrite non existing triggered spell call in spell.dbc
            if (auraSpellInfo->IsFitToFamilyMask<CF_SHAMAN_LIGHTNING_SHIELD>())
            {
                switch (auraSpellInfo->Id)
                {
                    case 324:                               // Rank 1
                        trigger_spell_id = 26364;
                        break;
                    case 325:                               // Rank 2
                        trigger_spell_id = 26365;
                        break;
                    case 905:                               // Rank 3
                        trigger_spell_id = 26366;
                        break;
                    case 945:                               // Rank 4
                        trigger_spell_id = 26367;
                        break;
                    case 8134:                              // Rank 5
                        trigger_spell_id = 26369;
                        break;
                    case 10431:                             // Rank 6
                        trigger_spell_id = 26370;
                        break;
                    case 10432:                             // Rank 7
                        trigger_spell_id = 26363;
                        break;
                    default:
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u not handled in LShield", auraSpellInfo->Id);
                        return SPELL_AURA_PROC_FAILED;
                }
            }
            // Lightning Shield (The Ten Storms set)
            else if (auraSpellInfo->Id == 23551)
            {
                trigger_spell_id = 23552;
                target = pVictim;
            }
            // Damage from Lightning Shield (The Ten Storms set)
            else if (auraSpellInfo->Id == 23552)
                trigger_spell_id = 27635;
            // Mana Surge (The Earthfury set)
            else if (auraSpellInfo->Id == 23572)
            {
                if (!procSpell)
                    return SPELL_AURA_PROC_FAILED;
                basepoints[0] = procSpell->manaCost * 35 / 100;
                trigger_spell_id = 23571;
                target = this;
            }
            break;
        }
        default:
            break;
    }

    // All ok. Check current trigger spell
    SpellEntry const* triggerEntry = sSpellMgr.GetSpellEntry(trigger_spell_id);
    if (!triggerEntry)
    {
        // Not cast unknown spell
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Unit::HandleProcTriggerSpell: Spell %u have %u in EffectTriggered[%d], not handled custom case?", auraSpellInfo->Id, trigger_spell_id, triggeredByAura->GetEffIndex());
        return SPELL_AURA_PROC_FAILED;
    }

    // not allow proc extra attack spell at extra attack
    if (m_extraAttacks && triggerEntry->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS) && triggerEntry->Id != 20178)
        return SPELL_AURA_PROC_FAILED;

    // Custom basepoints/target for exist spell
    // dummy basepoints or other customs
    switch (trigger_spell_id)
    {
        // Cast positive spell on enemy target
        case 7099:  // Curse of Mending
        case 20233: // Improved Lay on Hands (cast on target)
        {
            target = pVictim;
            break;
        }
        // Combo points add triggers (need add combopoint only for main target, and after possible combopoints reset)
        case 15250: // Rogue Setup
        {
            if (!pVictim || pVictim != GetVictim())  // applied only for main target
                return SPELL_AURA_PROC_FAILED;
            break;                                   // continue normal case
        }
        // Finishing moves that add combo points
        case 14189: // Seal Fate (Netherblade set)
        case 14157: // Ruthlessness
        {
            // Need add combopoint AFTER finishing move (or they get dropped in finish phase)
            if (!sWorld.getConfig(CONFIG_UINT32_SPELL_PROC_DELAY))
            {
                if (Spell* spell = GetCurrentSpell(CURRENT_GENERIC_SPELL))
                {
                    if (Unit* pTarget = spell->m_targets.getUnitTarget())
                    {
                        m_Events.AddLambdaEventAtOffset([me = this, targetGuid = pTarget->GetObjectGuid(), trigger_spell_id]()
                        {
                            if (!me->IsInWorld() || !me->IsAlive())
                                return;
                            if (Unit* pTarget = me->GetMap()->GetUnit(targetGuid))
                                me->CastSpell(pTarget, trigger_spell_id, true);
                        }, BATCHING_INTERVAL);
                    }
                    return SPELL_AURA_PROC_OK;
                }
                return SPELL_AURA_PROC_FAILED;
            }
            break;
        }
    }

    // try detect target manually if not set
    if (target == nullptr)
        target = !(procFlags & PROC_FLAG_DEAL_HELPFUL_SPELL) && Spells::IsPositiveSpell(trigger_spell_id) ? this : pVictim;

    return TriggerProccedSpell(target, basepoints, triggerEntry, castItem, triggeredByAura, cooldown, GetObjectGuid(), procSpell);
}

SpellAuraProcResult Unit::HandleProcTriggerDamageAuraProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 procFlags, uint32 procEx, uint32 cooldown)
{
    SpellEntry const* spellInfo = triggeredByAura->GetSpellProto();
    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "ProcDamageAndSpell: doing %.2f damage from spell id %u (triggered by auratype %u of spell %u)",
                     triggeredByAura->GetModifier()->m_amount, spellInfo->Id, triggeredByAura->GetModifier()->m_auraname, triggeredByAura->GetId());

    if (!pVictim || !pVictim->IsAlive())
        return SPELL_AURA_PROC_FAILED;

    // Trigger damage can be resisted...
    if (SpellMissInfo missInfo = SpellHitResult(pVictim, spellInfo, triggeredByAura->GetEffIndex(), false))
    {
        SendSpellDamageResist(pVictim, spellInfo->Id);
        return SPELL_AURA_PROC_OK;
    }

    // World of Warcraft Client Patch 1.9.0 (2006-01-03)
    // - Seal of Righteousness - Now does holy damage on every swing.

    SpellNonMeleeDamage damageInfo(this, pVictim, spellInfo->Id, static_cast<SpellSchools>(spellInfo->School));
    float fdamage = CalculateSpellEffectValue(pVictim, spellInfo, triggeredByAura->GetEffIndex());
    fdamage = SpellDamageBonusDone(pVictim, spellInfo, triggeredByAura->GetEffIndex(), fdamage, SPELL_DIRECT_DAMAGE);
    fdamage = pVictim->SpellDamageBonusTaken(this, spellInfo, triggeredByAura->GetEffIndex(), fdamage, SPELL_DIRECT_DAMAGE);
    damageInfo.damage = rand_ditheru(fdamage);
    damageInfo.target->CalculateAbsorbResistBlock(this, &damageInfo, spellInfo);
    DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb);
    SendSpellNonMeleeDamageLog(&damageInfo);
    DealSpellDamage(&damageInfo, true);
    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleOverrideClassScriptAuraProc(Unit* pVictim, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/ , uint32 cooldown)
{
    int32 scriptId = triggeredByAura->GetModifier()->m_miscvalue;

    if (!pVictim || !pVictim->IsAlive())
        return SPELL_AURA_PROC_FAILED;

    Item* castItem = triggeredByAura->GetCastItemGuid() && GetTypeId() == TYPEID_PLAYER
                     ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGuid()) : nullptr;

    // Basepoints of trigger aura
    float triggerAmount = triggeredByAura->GetModifier()->m_amount;

    uint32 triggered_spell_id = 0;

    switch (scriptId)
    {
        case 4309: // Crepuscule
        {
            triggered_spell_id = 17941;
            break;
        }
        case 836:                                           // Improved Blizzard (Rank 1)
        {
            if (!procSpell || !procSpell->HasTag(SPELL_TAG_BLIZZARD))
                return SPELL_AURA_PROC_FAILED;
            triggered_spell_id = 12484;
            break;
        }
        case 988:                                           // Improved Blizzard (Rank 2)
        {
            if (!procSpell || !procSpell->HasTag(SPELL_TAG_BLIZZARD))
                return SPELL_AURA_PROC_FAILED;
            triggered_spell_id = 12485;
            break;
        }
        case 989:                                           // Improved Blizzard (Rank 3)
        {
            if (!procSpell || !procSpell->HasTag(SPELL_TAG_BLIZZARD))
                return SPELL_AURA_PROC_FAILED;
            triggered_spell_id = 12486;
            break;
        }
        case 4086:                                          // Improved Mend Pet (Rank 1)
        case 4087:                                          // Improved Mend Pet (Rank 2)
        {
            if (!roll_chance_i(triggerAmount))
                return SPELL_AURA_PROC_FAILED;

            triggered_spell_id = 24406;
            break;
        }
        case 3656: // Corrupted Healing
        {
            // only proc on direct healing
            if (procSpell->HasEffect(SPELL_EFFECT_HEAL))
                triggered_spell_id = 23402;
            break;
        }
        case 4533: // Druid T3 Bonus: 28716 (50% chance)
        {
            if (procSpell->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_REJUVENATION>())
            {
                switch (pVictim->GetPowerType())
                {
                case POWER_MANA:
                    triggered_spell_id = 28722;
                    break;
                case POWER_RAGE:
                    triggered_spell_id = 28723;
                    break;
                case POWER_ENERGY:
                    triggered_spell_id = 28724;
                    break;
                }
            }
            break;
        }
        case 4537: // Druid T3 Bonus: 28744
        {
            if (procSpell->IsFitToFamily<SPELLFAMILY_DRUID, CF_DRUID_REGROWTH>())
                triggered_spell_id = 28750;

            break;
        }
    }

    // not processed
    if (!triggered_spell_id)
        return SPELL_AURA_PROC_OK;

    return TriggerProccedSpell(pVictim, nullptr, triggered_spell_id, castItem, triggeredByAura, cooldown);
}

SpellAuraProcResult Unit::HandleModCastingSpeedNotStackAuraProc(Unit* /*pVictim*/, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* /*triggeredByAura*/, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    // Skip melee hits or instant cast spells
    return !(procSpell == nullptr || procSpell->GetCastTime(this) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleReflectSpellsSchoolAuraProc(Unit* /*pVictim*/, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    // Skip Melee hits and spells ws wrong school
    return !(procSpell == nullptr || (triggeredByAura->GetModifier()->m_miscvalue & GetSchoolMask(procSpell->School)) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleModPowerCostSchoolAuraProc(Unit* /*pVictim*/, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    // Skip melee hits and spells ws wrong school or zero cost
    return !(procSpell == nullptr ||
             (procSpell->manaCost == 0 && procSpell->ManaCostPercentage == 0) ||           // Cost check
             (triggeredByAura->GetModifier()->m_miscvalue & GetSchoolMask(procSpell->School)) == 0) ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;  // School check
}

SpellAuraProcResult Unit::HandleMechanicImmuneResistanceAuraProc(Unit* /*pVictim*/, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    // Compare mechanic
    return !(procSpell == nullptr || procSpell->Mechanic != triggeredByAura->GetModifier()->m_miscvalue)
           ? SPELL_AURA_PROC_OK : SPELL_AURA_PROC_FAILED;
}

// Nostalrius
SpellAuraProcResult Unit::HandleAddTargetTriggerAuraProc(Unit* pVictim, uint32 /*amount*/, uint32 /*originalAmount*/, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/ , uint32 cooldown)
{
    SpellEntry const* aurEntry = triggeredByAura->GetSpellProto();
    uint32 trigger_spell_id = aurEntry->EffectTriggerSpell[0];
    float chance = 0.0f;

    if (aurEntry->EffectBasePoints[0] != -1)
        chance = aurEntry->EffectBasePoints[0];
    else
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u has chance = -1 but not handled in core ...", aurEntry->Id);

    // Si il y a plusieurs ticks ...
    // Blizzard - 8 ticks (fix procs abuses de morsure de givre)
    if (aurEntry->IsFitToFamily<SPELLFAMILY_MAGE, CF_MAGE_BLIZZARD>())
        chance /= 8;

    // Chance = -1 -> A calculer manuellement
    if (!roll_chance_f(chance))
        return SPELL_AURA_PROC_FAILED;

    if (trigger_spell_id)
    {
        bool bTarget = true;
        switch (aurEntry->Id)
        {
            case 17768: // Casque tete de loup
            case 24392: // Gelee soudaine
            case 14179: // Frappes implacables
                bTarget = false;
                break;
        }

        if (bTarget)
        {
            // Cone de glace
            if (pVictim->GetGUID() == GetGUID())
                return SPELL_AURA_PROC_FAILED;
            pVictim->CastSpell(pVictim, trigger_spell_id, true, nullptr, triggeredByAura);
        }
        else
            CastSpell(this, trigger_spell_id, true, nullptr, triggeredByAura);
    }
    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleModResistanceAuraProc(Unit* /*pVictim*/, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* /*procSpell*/, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    SpellEntry const* spellInfo = triggeredByAura->GetSpellProto();

    // Inner Fire
    if (spellInfo->IsFitToFamily<SPELLFAMILY_PRIEST, CF_PRIEST_INNER_FIRE>())
    {
        // only at real damage
        if (!amount)
            return SPELL_AURA_PROC_FAILED;
    }

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleModDamageAuraProc(Unit* /*pVictim*/, uint32 /*amount*/, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const* procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 /*cooldown*/)
{
    // the aura school mask must match the spell school
    uint32 const schoolMask = procSpell ? GetSchoolMask(procSpell->School) : SPELL_SCHOOL_MASK_NORMAL;
    if (!(schoolMask & triggeredByAura->GetModifier()->m_miscvalue))
        return SPELL_AURA_PROC_FAILED;

    switch (triggeredByAura->GetId())
    {
        case 24659: // Zandalarian Hero Charm - Unstable Power
        {
            if (!procSpell)
                return SPELL_AURA_PROC_FAILED;

            /*
            World of Warcraft Client Patch 1.11.0 (2006-06-20)
            - Judgement of Command: Now consumes a charge of the Zandalarian Hero
              Charm. In addition, when this spell is resisted it will no longer
              erroneously still do damage.
            - Judgement of Righteousness: Now consumes a charge of the Zandalarian
              Hero Charm.
            - Shadowguard: This Troll Priest racial spell now works with Vampiric
              Embrace, Blackout, and Shadow Weaving. In addition, the damage from
              Shadowguard will now consume charges of the Zandalarian Hero Charm's
              Unstable Power aura.
            - Zandalarian Hero Charm: The damage and healing on this item have been
              reduced by 30%. Instead of granting 35 damage and 70 healing per
              charge, it now grants 25 damage and 50 healing per charge. Several
              Paladin spells, Starshards, and Lightning Shield were not consuming
              charges of this trinket. All those spells have been fixed. In
              addition, totems which now benefit from increased damage and healing
              will also consume charges (Healing Stream Totem, Searing Totem, Magma
              Totem, and Fire Nova Totem)
            World of Warcraft Client Patch 1.10.0 (2006-03-28)
            - The charges from the Zandalarian Hero Charm will now be consumed by
              melee and ranged abilities and spells which do non-physical damage.
              This includes: Hammer of Wrath, Judgement of Righteousness, Seal of
              Command, Judgement of Command, Volley, and Arcane Shot. The trinket
              will also now burn charges from each casting of a damage over time
              spell, heal over time spell, and area aura spells such as Blizzard
              and Consecration. Only one charge will be burned per area spell cast,
              rather than multiple charges per target hit as was previously the
              case.
            */
            if (!procSpell->IsDirectDamageSpell() &&
                !procSpell->IsHealSpell() &&
                !procSpell->HasAura(SPELL_AURA_PERIODIC_DAMAGE) &&
                !procSpell->HasAura(SPELL_AURA_PERIODIC_LEECH) &&
                !procSpell->HasAura(SPELL_AURA_PERIODIC_HEALTH_FUNNEL) &&
                !procSpell->HasTag(SPELL_TAG_DAMAGE_TOTEM))
                return SPELL_AURA_PROC_FAILED;

            RemoveAuraHolderFromStack(24659);
            return SPELL_AURA_PROC_OK;
        }
    }

    return SPELL_AURA_PROC_OK;
}

SpellAuraProcResult Unit::HandleRemoveByDamageChanceProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    // The chance to dispel an aura depends on the damage taken with respect to the casters level.
    uint32 max_dmg = GetLevel() > 8 ? 25 * GetLevel() - 150 : 50;
    float chance = float(amount) / max_dmg * 100.0f;
    if (roll_chance_f(chance))
    {
        triggeredByAura->SetInUse(true);
        RemoveAurasByCasterSpell(triggeredByAura->GetId(), triggeredByAura->GetCasterGuid());
        triggeredByAura->SetInUse(false);
        return SPELL_AURA_PROC_OK;
    }

    return SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleRemoveFearByDamageChanceProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    if (!originalAmount)
        return SPELL_AURA_PROC_FAILED;

    switch (triggeredByAura->GetSpellProto()->Mechanic)
    {
        case MECHANIC_FEAR:
        case MECHANIC_TURN:
            break;
        default:
            return SPELL_AURA_PROC_FAILED;
    }

    // Formula derived from Youfie's post here:
    // https://forum.nostalrius.org/viewtopic.php?f=5&t=17424#p119432

    // The chance to dispel an aura depends on the damage taken with respect to the caster's level.
    uint32 max_dmg = GetLevel() > 8 ? 25 * GetLevel() - 150 : 50;

    // Players are 3x more likely to break fears
    if (IsPlayer())
        max_dmg *= 0.333f;

    // World of Warcraft Client Patch 1.11.0 (2006-06-20)
    // - Fear: The calculations to determine if Fear effects should break due
    //   to receiving damage have been changed. In addition, the chance for a
    //   damage over time spell to break Fear is now significantly lower.
    // DOT spells are 3x less likely to break fears after 1.11
    if (procFlag & PROC_FLAG_TAKE_HARMFUL_PERIODIC)
        max_dmg *= 3;

    // for players, this means max_dmg = 450 at level 60, or 1350 if the damage source is a dot
    // for mobs, this means max_dmg = 1350 at level 60, or 4050 if the damage source is a dot

    // World of Warcraft Client Patch 1.11.0 (2006-06-20)
    // - Fear: The calculations to determine if Fear effects should break due
    //   to receiving damage have been changed.The old calculation used the
    //   base damage of the ability.The new calculation uses the final amount
    //   of damage dealt, after all modifiers.

    float chance = float(originalAmount) / float(max_dmg) * 100.0f;
    if (roll_chance_f(chance))
    {
        triggeredByAura->SetInUse(true);
        RemoveAurasByCasterSpell(triggeredByAura->GetId(), triggeredByAura->GetCasterGuid());
        triggeredByAura->SetInUse(false);
        return SPELL_AURA_PROC_OK;
    }

    return SPELL_AURA_PROC_FAILED;
}

SpellAuraProcResult Unit::HandleInvisibilityAuraProc(Unit* pVictim, uint32 amount, uint32 originalAmount, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    if (triggeredByAura->GetSpellProto()->HasAttribute(SPELL_ATTR_PASSIVE) || !triggeredByAura->GetSpellProto()->IsPositiveSpell())
        return SPELL_AURA_PROC_FAILED;

    RemoveAurasDueToSpell(triggeredByAura->GetId());
    return SPELL_AURA_PROC_OK;
}
