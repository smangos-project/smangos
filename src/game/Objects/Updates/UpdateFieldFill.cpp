// Copies the flat 1.12 values array into the 1.15 typed update-field structs. Plain copies only
// (unpacking packed words, widening types); a field with no 1.12 source is left at its default.
// Written against the 1.12 field meanings (Objects/UpdateFields_1_12_1.h) and the game code that
// sets them, not against any other server's field-mapping tables.

#include "UpdateFieldFill.h"
#include "UpdateFieldStructs.h"
#include "Objects/Object.h"
#include "Objects/Unit.h"
#include "Objects/Player.h"
#include "Objects/Pet.h"
#include "Objects/Creature.h"
#include "Objects/Item.h"
#include "Objects/Bag.h"
#include "Objects/GameObject.h"
#include "Objects/Corpse.h"
#include "Objects/DynamicObject.h"
#include "Objects/UnitDefines.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "Server/Packets/Classic/InventorySlots.h"
#include "World.h"
#include <algorithm>

namespace
{
ClassicGuid ToClassic(ObjectGuid const& guid)
{
    return ClassicGuidMapping::ToClassic(guid, realmID);
}

// TrinityCore UnitDefines.h; 1.12 has no equivalent flag, so this stays a local literal rather
// than pulling in a game-side enum this file doesn't otherwise need.
constexpr uint32 MODERN_UNIT_FLAG2_REGENERATE_POWER = 0x00000800;

// UNIT_FIELD_POWER/UNIT_FIELD_MAXPOWER are indexed by a per-class wire slot in the modern client
// (ChrClassesXPowerTypes.db2), not by this engine's own vanilla Powers ordinal -- e.g. a Rogue's
// Energy is modern slot 0, not Powers::POWER_ENERGY's ordinal (3). A real client has no DB2 entry
// for a resource in the wrong slot and never renders that class's power bar at all. Table cross-
// checked against a live reference capture (see vibemangos's own GetModernPowerWireSlot, which
// found the same mapping the same way).
int8 GetModernPowerWireSlot(Unit const& unit, Powers power)
{
    if (Player const* player = unit.ToPlayer())
    {
        switch (player->GetClass())
        {
            case CLASS_WARRIOR:
                return power == POWER_RAGE ? 0 : -1;
            case CLASS_PALADIN:
            case CLASS_HUNTER:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                return power == POWER_MANA ? 0 : -1;
            case CLASS_ROGUE:
                return power == POWER_ENERGY ? 0 : -1;
            case CLASS_DRUID:
                switch (power)
                {
                    case POWER_MANA:   return 0;
                    case POWER_RAGE:   return 1;
                    case POWER_ENERGY: return 2;
                    default:           return -1;
                }
            default:
                return -1;
        }
    }
    if (Pet const* pet = unit.ToPet())
    {
        if (pet->GetPetType() == HUNTER_PET)
        {
            switch (power)
            {
                case POWER_FOCUS:     return 0;
                case POWER_HAPPINESS: return 3;
                default:              return -1;
            }
        }
        return -1;
    }
    // Generic creature: vanilla only ever assigns POWER_MANA in practice, and slot 0 is the
    // modern default for every class's primary resource.
    return power == POWER_MANA ? 0 : -1;
}

// Combo points have no wire field of their own; the modern client reads them from the same
// per-class Power[] wire slot table as every other resource (only Rogues and Druids have combo
// points in Classic Era). Neither slot collides with that class's own GetModernPowerWireSlot
// target above (a Rogue's own resource is slot 0, a Druid's is 0-2), so this can't double-write a
// slot meant for a real resource.
int8 GetModernComboPointsWireSlot(Player const& player)
{
    switch (player.GetClass())
    {
        case CLASS_ROGUE: return 1;
        case CLASS_DRUID: return 3;
        default:          return -1;
    }
}
}

void FillObjectData(Object& object, uint32 dynamicFlags)
{
    FillField(object, &Object::m_objectData, &UFObjectData::EntryID, int32(object.GetEntry()));
    FillField(object, &Object::m_objectData, &UFObjectData::Scale, object.GetObjectScale());
    FillField(object, &Object::m_objectData, &UFObjectData::DynamicFlags, dynamicFlags);
}

void FillUnitData(Unit& unit)
{
    FillObjectData(unit, unit.GetUInt32Value(UNIT_DYNAMIC_FLAGS));

    Player* player = unit.ToPlayer();

    FillField(unit, &Unit::m_unitData, &UFUnitData::Health, int64(unit.GetUInt32Value(UNIT_FIELD_HEALTH)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MaxHealth, int64(unit.GetUInt32Value(UNIT_FIELD_MAXHEALTH)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::DisplayID, int32(unit.GetUInt32Value(UNIT_FIELD_DISPLAYID)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::NpcFlags, unit.GetUInt32Value(UNIT_NPC_FLAGS));

    FillField(unit, &Unit::m_unitData, &UFUnitData::Charm, ToClassic(unit.GetGuidValue(UNIT_FIELD_CHARM)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::Summon, ToClassic(unit.GetGuidValue(UNIT_FIELD_SUMMON)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::CharmedBy, ToClassic(unit.GetGuidValue(UNIT_FIELD_CHARMEDBY)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::SummonedBy, ToClassic(unit.GetGuidValue(UNIT_FIELD_SUMMONEDBY)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::CreatedBy, ToClassic(unit.GetGuidValue(UNIT_FIELD_CREATEDBY)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::Target, ToClassic(unit.GetGuidValue(UNIT_FIELD_TARGET)));
    // No flat 1.12 field on the unit itself; a player's guild membership is PLAYER_GUILDID.
    FillField(unit, &Unit::m_unitData, &UFUnitData::GuildGUID,
        player && player->GetGuildId() ? ClassicGuidMapping::MakeGuild(player->GetGuildId()) : ClassicGuid());

    FillField(*unit.m_unitData->ChannelData, [&]() { return unit.m_values.ModifyValue(&Unit::m_unitData).ModifyValue(&UFUnitData::ChannelData); },
        &UFUnitChannel::SpellID, int32(unit.GetUInt32Value(UNIT_CHANNEL_SPELL)));

    FillField(unit, &Unit::m_unitData, &UFUnitData::Race, unit.GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::ClassId, unit.GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS));
    FillField(unit, &Unit::m_unitData, &UFUnitData::PlayerClassId, player ? player->GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS) : uint8(0));
    FillField(unit, &Unit::m_unitData, &UFUnitData::Sex, unit.GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER));
    FillField(unit, &Unit::m_unitData, &UFUnitData::DisplayPower, unit.GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_POWER_TYPE));

    FillField(unit, &Unit::m_unitData, &UFUnitData::Level, int32(unit.GetUInt32Value(UNIT_FIELD_LEVEL)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::EffectiveLevel, int32(unit.GetUInt32Value(UNIT_FIELD_LEVEL)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::FactionTemplate, int32(unit.GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::Flags, unit.GetUInt32Value(UNIT_FIELD_FLAGS));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AuraState, unit.GetUInt32Value(UNIT_FIELD_AURASTATE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::RangedAttackRoundBaseTime, unit.GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME));
    FillField(unit, &Unit::m_unitData, &UFUnitData::BoundingRadius, unit.GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS));
    FillField(unit, &Unit::m_unitData, &UFUnitData::CombatReach, unit.GetFloatValue(UNIT_FIELD_COMBATREACH));
    FillField(unit, &Unit::m_unitData, &UFUnitData::NativeDisplayID, int32(unit.GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MountDisplayID, int32(unit.GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MinDamage, unit.GetFloatValue(UNIT_FIELD_MINDAMAGE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MaxDamage, unit.GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MinOffHandDamage, unit.GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MaxOffHandDamage, unit.GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MinRangedDamage, unit.GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::MaxRangedDamage, unit.GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));

    FillField(unit, &Unit::m_unitData, &UFUnitData::StandState, unit.GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::VisFlags, unit.GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG));
    FillField(unit, &Unit::m_unitData, &UFUnitData::ShapeshiftForm, unit.GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_SHAPESHIFT_FORM));
    FillField(unit, &Unit::m_unitData, &UFUnitData::SheatheState, unit.GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE));
    // The 1.12 misc-flags byte's PVP/FFA_PVP bits are frozen at their creation-time default
    // (Player::Create always sets PVP, never FFA_PVP -- see the "Nostalrius: Hackfix" comment at
    // Object.cpp's FFA_PVP checks), not live state; the modern PvpFlags field needs live state, so
    // this reads the same accessors the game logic itself uses (UNIT_FLAG_PVP, PLAYER_FLAGS_FFA_PVP).
    {
        uint8 pvpFlags = 0;
        if (unit.IsPvP())
            pvpFlags |= UNIT_BYTE2_FLAG_PVP;
        if (player && player->IsFFAPvP())
            pvpFlags |= UNIT_BYTE2_FLAG_FFA_PVP;
        FillField(unit, &Unit::m_unitData, &UFUnitData::PvpFlags, pvpFlags);
    }
    FillField(unit, &Unit::m_unitData, &UFUnitData::PetFlags, unit.GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS));

    FillField(unit, &Unit::m_unitData, &UFUnitData::PetNumber, unit.GetUInt32Value(UNIT_FIELD_PETNUMBER));
    FillField(unit, &Unit::m_unitData, &UFUnitData::PetNameTimestamp, unit.GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));
    FillField(unit, &Unit::m_unitData, &UFUnitData::PetExperience, unit.GetUInt32Value(UNIT_FIELD_PETEXPERIENCE));
    FillField(unit, &Unit::m_unitData, &UFUnitData::PetNextLevelExperience, unit.GetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP));
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModCastingSpeed, unit.GetFloatValue(UNIT_MOD_CAST_SPEED));
    FillField(unit, &Unit::m_unitData, &UFUnitData::CreatedBySpell, int32(unit.GetUInt32Value(UNIT_CREATED_BY_SPELL)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::EmoteState, int32(unit.GetUInt32Value(UNIT_NPC_EMOTESTATE)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::TrainingPointsUsed, unit.GetInt16Value(UNIT_TRAINING_POINTS, 0));
    FillField(unit, &Unit::m_unitData, &UFUnitData::TrainingPointsTotal, unit.GetInt16Value(UNIT_TRAINING_POINTS, 1));
    FillField(unit, &Unit::m_unitData, &UFUnitData::BaseMana, int32(unit.GetUInt32Value(UNIT_FIELD_BASE_MANA)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::BaseHealth, int32(unit.GetUInt32Value(UNIT_FIELD_BASE_HEALTH)));

    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackPower, int32(unit.GetInt32Value(UNIT_FIELD_ATTACK_POWER)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackPowerModPos, int32(unit.GetUInt16Value(UNIT_FIELD_ATTACK_POWER_MODS, 0)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackPowerModNeg, int32(unit.GetUInt16Value(UNIT_FIELD_ATTACK_POWER_MODS, 1)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackPowerMultiplier, unit.GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER));
    FillField(unit, &Unit::m_unitData, &UFUnitData::RangedAttackPower, int32(unit.GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::RangedAttackPowerModPos, int32(unit.GetUInt16Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS, 0)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::RangedAttackPowerModNeg, int32(unit.GetUInt16Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS, 1)));
    FillField(unit, &Unit::m_unitData, &UFUnitData::RangedAttackPowerMultiplier, unit.GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER));

    // 1.15 has no flat home for a fixed hover height; 0 would sink every model into the ground.
    FillField(unit, &Unit::m_unitData, &UFUnitData::HoverHeight, 1.0f);

    // Client-required non-zero placeholders with no 1.12 flat source (decision 5 rule 1): a 0
    // haste/time-rate multiplier renders as "does nothing", not "no bonus applied".
    FillField(unit, &Unit::m_unitData, &UFUnitData::Flags2, MODERN_UNIT_FLAG2_REGENERATE_POWER);
    FillField(unit, &Unit::m_unitData, &UFUnitData::DisplayScale, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::NativeXDisplayScale, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModHaste, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModRangedHaste, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModSpellHaste, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModHasteRegen, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ModTimeRate, 1.0f);
    FillField(unit, &Unit::m_unitData, &UFUnitData::ScaleDuration, 100);
    FillField(unit, &Unit::m_unitData, &UFUnitData::LookAtControllerID, -1);
    // Slot 6 (RunicPower) has no classic-era data source and is left unset, matching a real
    // reference capture, which never populates it either.
    for (uint32 i = 0; i < 6; ++i)
        FillField(unit, &Unit::m_unitData, &UFUnitData::ModPowerRegen, i, 1.0f);

    // Combo points only exist for players in 1.12, stored as the target's guid, not the unit's own.
    if (player)
        FillField(unit, &Unit::m_unitData, &UFUnitData::ComboTarget, ToClassic(player->GetGuidValue(PLAYER_FIELD_COMBO_TARGET)));

    // See GetModernPowerWireSlot: the wire slot a power lives in depends on the unit's class, not
    // on its own vanilla Powers ordinal.
    for (uint32 i = 0; i < MAX_POWERS && i < 5; ++i)
    {
        int8 const wireSlot = GetModernPowerWireSlot(unit, Powers(i));
        if (wireSlot < 0)
            continue;
        FillField(unit, &Unit::m_unitData, &UFUnitData::Power, uint32(wireSlot), int32(unit.GetUInt32Value(UNIT_FIELD_POWER1 + i)));
        FillField(unit, &Unit::m_unitData, &UFUnitData::MaxPower, uint32(wireSlot), int32(unit.GetUInt32Value(UNIT_FIELD_MAXPOWER1 + i)));
    }

    // Combo points have no field of their own in 1.12 (Player::m_comboPoints, plain int); the
    // modern client reads them out of this same per-class Power[] wire slot.
    if (player)
    {
        int8 const comboSlot = GetModernComboPointsWireSlot(*player);
        if (comboSlot >= 0)
            FillField(unit, &Unit::m_unitData, &UFUnitData::Power, uint32(comboSlot), int32(player->GetComboPoints()));
    }

    for (uint32 i = 0; i < 5; ++i)
    {
        FillField(unit, &Unit::m_unitData, &UFUnitData::Stats, i, int32(unit.GetUInt32Value(UNIT_FIELD_STAT0 + i)));
        FillField(unit, &Unit::m_unitData, &UFUnitData::StatPosBuff, i, player ? int32(player->GetUInt32Value(PLAYER_FIELD_POSSTAT0 + i)) : 0);
        FillField(unit, &Unit::m_unitData, &UFUnitData::StatNegBuff, i, player ? int32(player->GetUInt32Value(PLAYER_FIELD_NEGSTAT0 + i)) : 0);
    }

    for (uint32 i = 0; i < 7; ++i)
    {
        FillField(unit, &Unit::m_unitData, &UFUnitData::Resistances, i, int32(unit.GetInt32Value(UNIT_FIELD_RESISTANCES + i)));
        FillField(unit, &Unit::m_unitData, &UFUnitData::ResistanceBuffModsPositive, i, player ? int32(player->GetUInt32Value(PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE + i)) : 0);
        FillField(unit, &Unit::m_unitData, &UFUnitData::ResistanceBuffModsNegative, i, player ? int32(player->GetUInt32Value(PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + i)) : 0);
        FillField(unit, &Unit::m_unitData, &UFUnitData::PowerCostModifier, i, int32(unit.GetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + i)));
        FillField(unit, &Unit::m_unitData, &UFUnitData::PowerCostMultiplier, i, unit.GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i));
    }

    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackRoundBaseTime, 0, unit.GetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 0));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackRoundBaseTime, 1, unit.GetUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1));
    FillField(unit, &Unit::m_unitData, &UFUnitData::AttackRoundBaseTime, 2, unit.GetUInt32Value(UNIT_FIELD_RANGEDATTACKTIME));

    // UNIT_VIRTUAL_ITEM_SLOT_DISPLAY holds a display id (Creature::SetVirtualItem); the modern
    // field wants the item id itself, cached separately since nothing else on the flat side keeps
    // it (Creature::GetVirtualItemId).
    Creature const* creature = unit.ToCreature();
    for (uint32 i = 0; i < 3; ++i)
    {
        FillField(unit.m_unitData->VirtualItems[i], [&]() { return unit.m_values.ModifyValue(&Unit::m_unitData).ModifyValue(&UFUnitData::VirtualItems, i); },
            &UFVisibleItem::ItemID, int32(creature ? creature->GetVirtualItemId(WeaponAttackType(i)) : 0));
    }
}

void FillPlayerData(Player& player)
{
    FillField(player, &Player::m_playerData, &UFPlayerData::DuelArbiter, ToClassic(player.GetGuidValue(PLAYER_DUEL_ARBITER)));
    FillField(player, &Player::m_playerData, &UFPlayerData::PlayerFlags, player.GetUInt32Value(PLAYER_FLAGS));
    FillField(player, &Player::m_playerData, &UFPlayerData::GuildRankID, player.GetUInt32Value(PLAYER_GUILDRANK));
    FillField(player, &Player::m_playerData, &UFPlayerData::NumBankSlots, player.GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_BANK_BAG_SLOTS));
    FillField(player, &Player::m_playerData, &UFPlayerData::NativeSex, player.GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER));
    FillField(player, &Player::m_playerData, &UFPlayerData::Inebriation, uint8(std::min<uint16>(player.GetDrunkValue(), 100)));
    FillField(player, &Player::m_playerData, &UFPlayerData::DuelTeam, player.GetUInt32Value(PLAYER_DUEL_TEAM));
    FillField(player, &Player::m_playerData, &UFPlayerData::GuildTimeStamp, int32(player.GetUInt32Value(PLAYER_GUILD_TIMESTAMP)));

    // No 1.12 flat source. Without a real per-account WowAccount guid, a modern client can't
    // resolve another player's identity and shows "Unknown" on their nameplate.
    FillField(player, &Player::m_playerData, &UFPlayerData::WowAccount, ClassicGuidMapping::MakeAccount(player.GetSession()->GetAccountId()));
    FillField(player, &Player::m_playerData, &UFPlayerData::HonorLevel, 1);
    FillField(player, &Player::m_playerData, &UFPlayerData::AvgItemLevel, 3, 1.0f);

    for (uint8 slot = 0; slot < 20; ++slot) // 1.12 has 20 quest log slots (MAX_QUEST_LOG_SIZE)
    {
        auto getQuestLog = [&]() { return player.m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UFPlayerData::QuestLog, slot); };
        UFQuestLog const& current = player.m_playerData->QuestLog[slot];
        uint16 questBase = PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET;
        uint32 const countState = player.GetUInt32Value(questBase + QUEST_COUNT_STATE_OFFSET);
        FillField(current, getQuestLog, &UFQuestLog::QuestID, int32(player.GetUInt32Value(questBase + QUEST_ID_OFFSET)));
        FillField(current, getQuestLog, &UFQuestLog::StateFlags, countState >> 24);
        FillField(current, getQuestLog, &UFQuestLog::EndTime, int64(player.GetUInt32Value(questBase + QUEST_TIME_OFFSET)));
        for (uint32 objective = 0; objective < 4; ++objective)
            FillField(current, getQuestLog, &UFQuestLog::ObjectiveProgress, objective, uint16((countState >> (objective * 6)) & 0x3F));
    }

    for (uint8 slot = 0; slot < 19; ++slot)
    {
        FillField(player.m_playerData->VisibleItems[slot], [&]() { return player.m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UFPlayerData::VisibleItems, slot); },
            &UFVisibleItem::ItemID, int32(player.GetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + slot * 12)));
    }
}

void FillActivePlayerData(Player& player)
{
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::FarsightObject, ToClassic(player.GetGuidValue(PLAYER_FARSIGHT)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::Coinage, uint64(player.GetUInt32Value(PLAYER_FIELD_COINAGE)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::XP, int32(player.GetUInt32Value(PLAYER_XP)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::NextLevelXP, int32(player.GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::CharacterPoints, int32(player.GetUInt32Value(PLAYER_CHARACTER_POINTS1)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::TrackCreatureMask, player.GetUInt32Value(PLAYER_TRACK_CREATURES));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::BlockPercentage, player.GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::DodgePercentage, player.GetFloatValue(PLAYER_DODGE_PERCENTAGE));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ParryPercentage, player.GetFloatValue(PLAYER_PARRY_PERCENTAGE));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::CritPercentage, player.GetFloatValue(PLAYER_CRIT_PERCENTAGE));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::RangedCritPercentage, player.GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::MultiActionBars, player.GetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_ACTION_BARS));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LifetimeMaxRank, player.GetByteValue(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTES_OFFSET_HIGHEST_HONOR_RANK));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::AmmoID, int32(player.GetUInt32Value(PLAYER_AMMO_ID)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::PvpMedals, player.GetUInt32Value(PLAYER_FIELD_PVP_MEDALS));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::TodayHonorableKills, player.GetUInt16Value(PLAYER_FIELD_SESSION_KILLS, 0));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::TodayDishonorableKills, player.GetUInt16Value(PLAYER_FIELD_SESSION_KILLS, 1));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::YesterdayHonorableKills, player.GetUInt16Value(PLAYER_FIELD_YESTERDAY_KILLS, 0));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::YesterdayDishonorableKills, player.GetUInt16Value(PLAYER_FIELD_YESTERDAY_KILLS, 1));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LastWeekHonorableKills, player.GetUInt16Value(PLAYER_FIELD_LAST_WEEK_KILLS, 0));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LastWeekDishonorableKills, player.GetUInt16Value(PLAYER_FIELD_LAST_WEEK_KILLS, 1));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ThisWeekHonorableKills, player.GetUInt16Value(PLAYER_FIELD_THIS_WEEK_KILLS, 0));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ThisWeekDishonorableKills, player.GetUInt16Value(PLAYER_FIELD_THIS_WEEK_KILLS, 1));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ThisWeekContribution, player.GetUInt32Value(PLAYER_FIELD_THIS_WEEK_CONTRIBUTION));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LifetimeHonorableKills, player.GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LifetimeDishonorableKills, player.GetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORBALE_KILLS));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::YesterdayContribution, player.GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LastWeekContribution, player.GetUInt32Value(PLAYER_FIELD_LAST_WEEK_CONTRIBUTION));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::LastWeekRank, player.GetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::WatchedFactionIndex, int32(player.GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX)));

    {
        auto getSkill = [&]() { return player.m_values.ModifyValue(&Player::m_activePlayerData).ModifyValue(&UFActivePlayerData::Skill); };
        UFSkillInfo const& current = *player.m_activePlayerData->Skill;
        for (uint32 pos = 0; pos < 128; ++pos)
        {
            uint16 infoIndex = PLAYER_SKILL_INFO_1_1 + pos * 3;
            uint16 valueIndex = infoIndex + 1;
            uint16 bonusIndex = infoIndex + 2;
            FillField(current, getSkill, &UFSkillInfo::SkillLineID, pos, player.GetUInt16Value(infoIndex, 0));
            FillField(current, getSkill, &UFSkillInfo::SkillStep, pos, player.GetUInt16Value(infoIndex, 1));
            FillField(current, getSkill, &UFSkillInfo::SkillRank, pos, player.GetUInt16Value(valueIndex, 0));
            FillField(current, getSkill, &UFSkillInfo::SkillMaxRank, pos, player.GetUInt16Value(valueIndex, 1));
            FillField(current, getSkill, &UFSkillInfo::SkillTempBonus, pos, player.GetInt16Value(bonusIndex, 0));
            FillField(current, getSkill, &UFSkillInfo::SkillPermBonus, pos, player.GetUInt16Value(bonusIndex, 1));
        }
    }

    // The flat guid fields run in server slot order, two words each, from the head slot to the keyring.
    for (ClassicInventorySlotRun const& run : CLASSIC_INVENTORY_SLOT_RUNS)
    {
        for (uint8 i = 0; i < run.Count; ++i)
        {
            uint8 const serverSlot = run.ServerStart + i;
            FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::InvSlots, uint32(run.ModernStart + i),
                ToClassic(player.GetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + serverSlot * 2)));
        }
    }

    for (uint32 i = 0; i < 20 && i < 32; ++i)
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::CombatRatings, i, int32(player.GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + i)));

    for (uint32 i = 0; i < 12; ++i)
    {
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::BuybackPrice, i, player.GetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + i));
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::BuybackTimestamp, i, int64(player.GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + i)));
    }

    {
        auto getRestInfo = [&]() { return player.m_values.ModifyValue(&Player::m_activePlayerData).ModifyValue(&UFActivePlayerData::RestInfo, 0); };
        UFRestInfo const& current = player.m_activePlayerData->RestInfo[0];
        FillField(current, getRestInfo, &UFRestInfo::Threshold, player.GetUInt32Value(PLAYER_REST_STATE_EXPERIENCE));
        FillField(current, getRestInfo, &UFRestInfo::StateID, player.GetByteValue(PLAYER_BYTES_2, PLAYER_BYTES_2_OFFSET_REST_STATE));
    }

    // RestInfo[1] is the honor-rest slot (TrinityCore RestMgr.h REST_TYPE_HONOR); 1.12 has no
    // honor-rest concept at all, so this has no flat source. A StateID of 0 isn't a valid
    // PlayerRestState client-side and crashes MainMenuBar.lua's exhaustion tick on every
    // PLAYER_ENTERING_WORLD; REST_STATE_NORMAL (2) is TrinityCore's own always-on default for it.
    {
        auto getRestInfo = [&]() { return player.m_values.ModifyValue(&Player::m_activePlayerData).ModifyValue(&UFActivePlayerData::RestInfo, 1); };
        UFRestInfo const& current = player.m_activePlayerData->RestInfo[1];
        FillField(current, getRestInfo, &UFRestInfo::Threshold, uint32(1));
        FillField(current, getRestInfo, &UFRestInfo::StateID, uint8(2));
    }

    // Client-required non-zero placeholders with no 1.12 flat source (decision 5 rule 1).
    for (uint32 i = 0; i < 7; ++i)
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ModDamageDonePercent, i, 1.0f);
    for (uint32 i = 0; i < 3; ++i)
    {
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::WeaponDmgMultipliers, i, 1.0f);
        FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::WeaponAtkSpeedMultipliers, i, 1.0f);
    }
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ModHealingPercent, 1.0f);
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ModHealingDonePercent, 1.0f);
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::ModPetHaste, 1.0f);
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::MaxLevel, int32(sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::HonorNextLevel, int32(5500));
    // 1.12's INVENTORY_SLOT_ITEM_START/_END is the 16-slot main backpack; without this the client
    // has no bag-slot count to render for itself.
    FillField(player, &Player::m_activePlayerData, &UFActivePlayerData::NumBackpackSlots,
        uint8(INVENTORY_SLOT_ITEM_END - INVENTORY_SLOT_ITEM_START));
}

void FillItemData(Item& item)
{
    FillObjectData(item, 0);

    FillField(item, &Item::m_itemData, &UFItemData::Owner, ToClassic(item.GetOwnerGuid()));
    FillField(item, &Item::m_itemData, &UFItemData::ContainedIn, ToClassic(item.GetGuidValue(ITEM_FIELD_CONTAINED)));
    FillField(item, &Item::m_itemData, &UFItemData::Creator, ToClassic(item.GetGuidValue(ITEM_FIELD_CREATOR)));
    FillField(item, &Item::m_itemData, &UFItemData::GiftCreator, ToClassic(item.GetGuidValue(ITEM_FIELD_GIFTCREATOR)));
    FillField(item, &Item::m_itemData, &UFItemData::StackCount, item.GetUInt32Value(ITEM_FIELD_STACK_COUNT));
    FillField(item, &Item::m_itemData, &UFItemData::Expiration, item.GetUInt32Value(ITEM_FIELD_DURATION));
    FillField(item, &Item::m_itemData, &UFItemData::DynamicFlags, item.GetUInt32Value(ITEM_FIELD_FLAGS));
    FillField(item, &Item::m_itemData, &UFItemData::PropertySeed, int32(item.GetItemSuffixFactor()));
    FillField(item, &Item::m_itemData, &UFItemData::RandomPropertiesID, item.GetItemRandomPropertyId());
    FillField(item, &Item::m_itemData, &UFItemData::Durability, item.GetUInt32Value(ITEM_FIELD_DURABILITY));
    FillField(item, &Item::m_itemData, &UFItemData::MaxDurability, item.GetUInt32Value(ITEM_FIELD_MAXDURABILITY));

    for (uint8 i = 0; i < 5; ++i)
        FillField(item, &Item::m_itemData, &UFItemData::SpellCharges, i, item.GetSpellCharges(i));

    for (uint32 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
    {
        auto getEnchant = [&]() { return item.m_values.ModifyValue(&Item::m_itemData).ModifyValue(&UFItemData::Enchantment, slot); };
        UFItemEnchantment const& current = item.m_itemData->Enchantment[slot];
        EnchantmentSlot enchantSlot = EnchantmentSlot(slot);
        FillField(current, getEnchant, &UFItemEnchantment::ID, int32(item.GetEnchantmentId(enchantSlot)));
        FillField(current, getEnchant, &UFItemEnchantment::Duration, item.GetEnchantmentDuration(enchantSlot));
        FillField(current, getEnchant, &UFItemEnchantment::Charges, int16(item.GetEnchantmentCharges(enchantSlot)));
    }
}

void FillContainerData(Bag& bag)
{
    FillField(bag, &Bag::m_containerData, &UFContainerData::NumSlots, bag.GetBagSize());

    for (uint32 i = 0; i < MAX_BAG_SIZE; ++i)
    {
        Item const* slotItem = bag.GetItemByPos(uint8(i));
        FillField(bag, &Bag::m_containerData, &UFContainerData::Slots, i, slotItem ? ToClassic(slotItem->GetObjectGuid()) : ClassicGuid());
    }
}

void FillGameObjectData(GameObject& gameObject)
{
    FillObjectData(gameObject, gameObject.GetUInt32Value(GAMEOBJECT_DYN_FLAGS));

    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::DisplayID, int32(gameObject.GetDisplayId()));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::Flags, gameObject.GetUInt32Value(GAMEOBJECT_FLAGS));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::ParentRotation, gameObject.GetLocalRotation());
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::FactionTemplate, int32(gameObject.GetUInt32Value(GAMEOBJECT_FACTION)));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::Level, int32(gameObject.GetUInt32Value(GAMEOBJECT_LEVEL)));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::State, int8(gameObject.GetGoState()));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::TypeID, int8(gameObject.GetGoType()));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::PercentHealth, uint8(gameObject.GetGoAnimProgress()));
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::ArtKit, gameObject.GetGoArtKit());
    FillField(gameObject, &GameObject::m_gameObjectData, &UFGameObjectData::CreatedBy, ToClassic(gameObject.GetOwnerGuid()));
}

void FillCorpseData(Corpse& corpse)
{
    FillObjectData(corpse, 0);

    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::Owner, ToClassic(corpse.GetOwnerGuid()));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::DisplayID, corpse.GetUInt32Value(CORPSE_FIELD_DISPLAY_ID));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::RaceID, corpse.GetByteValue(CORPSE_FIELD_BYTES_1, 1));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::Sex, corpse.GetByteValue(CORPSE_FIELD_BYTES_1, 2));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::Flags, corpse.GetUInt32Value(CORPSE_FIELD_FLAGS));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::DynamicFlags, corpse.GetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::FactionTemplate, int32(corpse.GetFactionTemplateId()));
    FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::GuildGUID,
        corpse.GetUInt32Value(CORPSE_FIELD_GUILD) ? ClassicGuidMapping::MakeGuild(corpse.GetUInt32Value(CORPSE_FIELD_GUILD)) : ClassicGuid());

    for (uint32 i = 0; i < 19; ++i)
        FillField(corpse, &Corpse::m_corpseData, &UFCorpseData::Items, i, corpse.GetUInt32Value(CORPSE_FIELD_ITEM + i));
}

void FillDynamicObjectData(DynamicObject& dynamicObject)
{
    FillObjectData(dynamicObject, 0);

    FillField(dynamicObject, &DynamicObject::m_dynamicObjectData, &UFDynamicObjectData::Caster, ToClassic(dynamicObject.GetCasterGuid()));
    FillField(dynamicObject, &DynamicObject::m_dynamicObjectData, &UFDynamicObjectData::Type, uint8(dynamicObject.GetType()));
    FillField(dynamicObject, &DynamicObject::m_dynamicObjectData, &UFDynamicObjectData::SpellID, int32(dynamicObject.GetSpellId()));
    FillField(dynamicObject, &DynamicObject::m_dynamicObjectData, &UFDynamicObjectData::Radius, dynamicObject.GetRadius());
}

void FillUpdateFieldData(Object& object)
{
    if (Unit* unit = object.ToUnit())
    {
        FillUnitData(*unit);
        if (Player* player = unit->ToPlayer())
        {
            FillPlayerData(*player);
            FillActivePlayerData(*player);
        }
    }
    else if (Item* item = object.ToItem())
    {
        FillItemData(*item);
        if (Bag* bag = item->ToBag())
            FillContainerData(*bag);
    }
    else if (GameObject* gameObject = object.ToGameObject())
        FillGameObjectData(*gameObject);
    else if (Corpse* corpse = object.ToCorpse())
        FillCorpseData(*corpse);
    else if (DynamicObject* dynamicObject = object.ToDynamicObject())
        FillDynamicObjectData(*dynamicObject);
    else
        FillObjectData(object, 0);
}
