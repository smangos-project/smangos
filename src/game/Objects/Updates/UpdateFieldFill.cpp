// Copies the flat 1.12 values array into the 1.15 typed update-field structs. Plain copies only
// (unpacking packed words, widening types); a field with no 1.12 source is left at its default.
// Written against the 1.12 field meanings (Objects/UpdateFields_1_12_1.h) and the game code that
// sets them, not against any other server's field-mapping tables.

#include "UpdateFieldFill.h"
#include "UpdateFieldStructs.h"
#include "Objects/Object.h"
#include "Objects/Unit.h"
#include "Objects/Player.h"
#include "Objects/UnitDefines.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "World.h"
#include <algorithm>

namespace
{
ClassicGuid ToClassic(ObjectGuid const& guid)
{
    return ClassicGuidMapping::ToClassic(guid, realmID);
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
    FillField(unit, &Unit::m_unitData, &UFUnitData::PvpFlags, unit.GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_MISC_FLAGS));
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

    // Combo points only exist for players in 1.12, stored as the target's guid, not the unit's own.
    if (player)
        FillField(unit, &Unit::m_unitData, &UFUnitData::ComboTarget, ToClassic(player->GetGuidValue(PLAYER_FIELD_COMBO_TARGET)));

    for (uint32 i = 0; i < MAX_POWERS && i < 5; ++i)
    {
        FillField(unit, &Unit::m_unitData, &UFUnitData::Power, i, int32(unit.GetUInt32Value(UNIT_FIELD_POWER1 + i)));
        FillField(unit, &Unit::m_unitData, &UFUnitData::MaxPower, i, int32(unit.GetUInt32Value(UNIT_FIELD_MAXPOWER1 + i)));
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

    for (uint32 i = 0; i < 3; ++i)
    {
        FillField(unit.m_unitData->VirtualItems[i], [&]() { return unit.m_values.ModifyValue(&Unit::m_unitData).ModifyValue(&UFUnitData::VirtualItems, i); },
            &UFVisibleItem::ItemID, int32(unit.GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i)));
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

    for (uint8 slot = 0; slot < 20; ++slot) // 1.12 has 20 quest log slots (MAX_QUEST_LOG_SIZE)
    {
        auto getQuestLog = [&]() { return player.m_values.ModifyValue(&Player::m_playerData).ModifyValue(&UFPlayerData::QuestLog, slot); };
        UFQuestLog const& current = player.m_playerData->QuestLog[slot];
        uint16 questBase = PLAYER_QUEST_LOG_1_1 + slot * 3;
        FillField(current, getQuestLog, &UFQuestLog::QuestID, int32(player.GetUInt32Value(questBase)));
        FillField(current, getQuestLog, &UFQuestLog::StateFlags, player.GetUInt32Value(questBase + 1) >> 24);
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
    else
    {
        // Game objects, corpses, items/containers and dynamic objects get their own fill in a
        // later step; their typed structs stay at their zero-initialized default until then.
        FillObjectData(object, 0);
    }
}
