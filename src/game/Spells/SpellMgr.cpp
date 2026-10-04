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

#include "SpellMgr.h"
#include "SummonKind.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "ProgressBar.h"
#include "DBCStores.h"
#include "World.h"
#include "Spell.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "BattleGround.h"

#include <cmath>
#include <map>

using namespace Spells;

SpellMgr::SpellMgr()
{
}

SpellMgr& SpellMgr::Instance()
{
    static SpellMgr spellMgr;
    return spellMgr;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                                0     1             2                    3                    4                    5
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `id`, `target_map`, `target_position_x`, `target_position_y`, `target_position_z`, `target_orientation` FROM `spell_target_position`"));
    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell target destination coordinates", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.mapId       = fields[1].GetUInt32();
        st.x           = fields[2].GetFloat();
        st.y           = fields[3].GetFloat();
        st.z           = fields[4].GetFloat();
        st.o           = fields[5].GetFloat();

        MapEntry const* mapEntry = sMapStorage.LookupEntry<MapEntry>(st.mapId);
        if (!mapEntry)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Spell_ID, st.mapId);
            continue;
        }

        if (st.x == 0 && st.y == 0 && st.z == 0)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Spell (ID:%u) target coordinates not provided.", Spell_ID);
            continue;
        }

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(Spell_ID);
        if (!spellInfo)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Spell (ID:%u) listed in `spell_target_position` does not exist.", Spell_ID);
            continue;
        }

        bool found = false;
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo->EffectImplicitTargetA[i] == TARGET_LOCATION_DATABASE || spellInfo->EffectImplicitTargetB[i] == TARGET_LOCATION_DATABASE)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_LOCATION_DATABASE (17).", Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
        ++count;

    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell target destination coordinates", count);
}

template <typename EntryType, typename WorkerType, typename StorageType>
struct SpellRankHelper
{
    SpellRankHelper(SpellMgr &_mgr, StorageType &_storage): mgr(_mgr), worker(_storage), customRank(0) {}
    void RecordRank(EntryType &entry, uint32 spell_id)
    {
        SpellEntry const* spell = sSpellMgr.GetSpellEntry(spell_id);
        if (!spell)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `%s` does not exist", spell_id, worker.TableName());
            return;
        }

        uint32 first_id = mgr.GetFirstSpellInChain(spell_id);

        // most spell ranks expected same data
        if (first_id)
        {
            firstRankSpells.insert(first_id);

            if (first_id != spell_id)
            {
                if (!worker.IsValidCustomRank(entry, spell_id, first_id))
                    return;
                // for later check that first rank also added
                else
                {
                    firstRankSpellsWithCustomRanks.insert(first_id);
                    ++customRank;
                }
            }
        }

        worker.AddEntry(entry, spell);
    }
    void FillHigherRanks()
    {
        // check that first rank added for custom ranks
        for (std::set<uint32>::const_iterator itr = firstRankSpellsWithCustomRanks.begin(); itr != firstRankSpellsWithCustomRanks.end(); ++itr)
            if (!worker.HasEntry(*itr))
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u must be listed in `%s` as first rank for listed custom ranks of spell but not found!", *itr, worker.TableName());

        // fill absent non first ranks data base at first rank data
        for (std::set<uint32>::const_iterator itr = firstRankSpells.begin(); itr != firstRankSpells.end(); ++itr)
        {
            if (worker.SetStateToEntry(*itr))
                mgr.doForHighRanks(*itr, worker);
        }
    }
    std::set<uint32> firstRankSpells;
    std::set<uint32> firstRankSpellsWithCustomRanks;

    SpellMgr &mgr;
    WorkerType worker;
    uint32 customRank;
};

struct DoSpellProcEvent
{
    DoSpellProcEvent(SpellProcEventMap& _spe_map) : spe_map(_spe_map), customProc(0), count(0) {}
    void operator()(uint32 spell_id)
    {
        SpellProcEventEntry const& spe = state->second;
        // add ranks only for not filled data (some ranks have ppm data different for ranks for example)
        SpellProcEventMap::const_iterator spellItr = spe_map.find(spell_id);
        if (spellItr == spe_map.end())
            spe_map[spell_id] = spe;
        // if custom rank data added then it must be same except ppm
        else
        {
            SpellProcEventEntry const& r_spe = spellItr->second;
            if (spe.schoolMask != r_spe.schoolMask)
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` as custom rank have different schoolMask from first rank in chain", spell_id);

            if (spe.spellFamilyName != r_spe.spellFamilyName)
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyName from first rank in chain", spell_id);

            for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                if (spe.spellFamilyMask[i] != r_spe.spellFamilyMask[i])
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyMask from first rank in chain", spell_id);
                    break;
                }
            }

            if (spe.procEx != r_spe.procEx)
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` as custom rank have different procEx from first rank in chain", spell_id);

        }
    }

    char const* TableName()
    {
        return "spell_proc_event";
    }
    bool IsValidCustomRank(SpellProcEventEntry const& spe, uint32 entry, uint32 first_id)
    {
        // let have independent data in table for spells with ppm rates (exist rank dependent ppm rate spells)
        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);
        if (!spellInfo || !spellInfo->ProcsPerMinute)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            return false;
        }
        return true;
    }
    void AddEntry(SpellProcEventEntry const& spe, SpellEntry const* spell)
    {
        spe_map[spell->Id] = spe;

        bool isCustom = false;

        if (spell->procFlags == 0)
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` probally not triggered spell (no proc flags)", spell->Id);

        // totally redundant record
        if (!spe.schoolMask && !spe.procEx && !spe.spellFamilyName)
        {
            bool empty = true;
            for (uint64 i : spe.spellFamilyMask)
            {
                if (i)
                {
                    empty = false;
                    break;
                }
            }
            if (empty)
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_event` not have any useful data", spell->Id);
        }

        if (isCustom)
            ++customProc;
        else
            ++count;
    }

    bool HasEntry(uint32 spellId)
    {
        return spe_map.count(spellId) > 0;
    }
    bool SetStateToEntry(uint32 spellId)
    {
        return (state = spe_map.find(spellId)) != spe_map.end();
    }
    SpellProcEventMap& spe_map;
    SpellProcEventMap::const_iterator state;

    uint32 customProc;
    uint32 count;
};

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    //                                                                0        1             2                  3                   4                   5                   6            7         8          9               10
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `procEx` FROM `spell_proc_event`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> No spell proc event conditions loaded");
        return;
    }

    SpellRankHelper<SpellProcEventEntry, DoSpellProcEvent, SpellProcEventMap> rankHelper(*this, mSpellProcEventMap);

    BarGoLink bar(result->GetRowCount());
    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.spellFamilyName = fields[2].GetUInt32();

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
            spe.spellFamilyMask[i] = fields[3 + i].GetUInt64();

        spe.procEx          = fields[6].GetUInt32();

        rankHelper.RecordRank(spe, entry);

    }
    while (result->NextRow());

    rankHelper.FillHigherRanks();

    // Where the aura keeps its own class mask in the effect (auras 107, 108, 109, 112), the proc mask is
    // the table value; elsewhere it is the effect's class mask.
    std::map<std::pair<uint32, uint32>, uint64> effectMasks;
    result = WorldDatabase.Query("SELECT `SpellID`, `EffectIndex`, `EffectSpellClassMask1`, `EffectSpellClassMask2` FROM `dbc_spelleffect` WHERE `DifficultyID` = 0 AND `EffectIndex` < 3 AND `Effect` <> 0");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            effectMasks[{fields[0].GetUInt32(), fields[1].GetUInt32()}] = uint64(fields[2].GetUInt32()) | (uint64(fields[3].GetUInt32()) << 32);
        } while (result->NextRow());
    }
    for (auto& [spellId, spe] : mSpellProcEventMap)
    {
        SpellEntry const* spell = GetSpellEntry(spellId);
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            uint32 const aura = spell->EffectApplyAuraName[i];
            if (aura == 107 || aura == 108 || aura == 109 || aura == 112)
                continue;
            auto itr = effectMasks.find({spellId, i});
            spe.spellFamilyMask[i] = itr != effectMasks.end() ? itr->second : 0;
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u extra spell proc event conditions +%u custom proc (inc. +%u custom ranks)",  rankHelper.worker.count, rankHelper.worker.customProc, rankHelper.customRank);
}

struct DoSpellProcItemEnchant
{
    DoSpellProcItemEnchant(SpellProcItemEnchantMap& _procMap, float _ppm) : procMap(_procMap), ppm(_ppm) {}
    void operator()(uint32 spell_id)
    {
        procMap[spell_id] = ppm;
    }

    SpellProcItemEnchantMap& procMap;
    float ppm;
};

void SpellMgr::LoadSpellProcItemEnchant()
{
    mSpellProcItemEnchantMap.clear();                       // need for reload case

    uint32 count = 0;

    //                                                               0        1
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `entry`, `ppmRate` FROM `spell_proc_item_enchant`"));
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u proc item enchant definitions", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();
        float ppmRate = fields[1].GetFloat();

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);

        if (!spellInfo)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_item_enchant` does not exist", entry);
            continue;
        }

        uint32 first_id = GetFirstSpellInChain(entry);

        if (first_id != entry)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_proc_item_enchant` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            continue;
        }

        mSpellProcItemEnchantMap[entry] = ppmRate;

        // also add to high ranks
        DoSpellProcItemEnchant worker(mSpellProcItemEnchantMap, ppmRate);
        doForHighRanks(entry, worker);

        ++count;
    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u proc item enchant definitions", count);
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellEntry const* procSpell, uint32 procFlags, uint32 procExtra)
{
    // Store extra req
    uint32 procEvent_procEx = spellProcEvent ? spellProcEvent->procEx : PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    // Either procs only on cast end, or only on hit.
    if ((procExtra & PROC_EX_CAST_END) != (procEvent_procEx & PROC_EX_CAST_END))
        return false;

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_HEARTBEAT | PROC_FLAG_KILL | PROC_FLAG_ON_TRAP_ACTIVATION))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // For melee triggers
        if (procSpell == nullptr)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & GetSchoolMask(procSpell->School)) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;
        }
    }

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // Don't allow proc from periodic heal if no extra requirement is defined
        if (EventProcFlag & (PROC_FLAG_DEAL_HARMFUL_PERIODIC | PROC_FLAG_TAKE_HARMFUL_PERIODIC) && (procExtra & PROC_EX_PERIODIC_POSITIVE))
            return false;

        // No extra req, so can trigger for (damage/healing present) and hit/crit
        if (procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT))
            return true;
    }
    else // all spells hits here only if resist/reflect/immune/evade/periodic
    {
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Exist req for PROC_EX_NO_PERIODIC
        if ((procEvent_procEx & PROC_EX_NO_PERIODIC) &&
            ((procFlags & (PROC_FLAG_DEAL_HARMFUL_PERIODIC | PROC_FLAG_TAKE_HARMFUL_PERIODIC))
            ||
            (procSpell && procSpell->IsSpellAppliesPeriodicAura())))
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

void SpellMgr::LoadSpellGroups()
{
    mSpellSpellGroup.clear();                                  // need for reload case
    mSpellGroupSpell.clear();

    uint32 count = 0;

    //                                                                0           1
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `group_id`, `spell_id` FROM `spell_group` ORDER BY `group_id`, `group_spell_id`, `spell_id`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell group definitions", count);
        return;
    }

    std::set<uint32> groups;
    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();
        Field* fields = result->Fetch();
        uint32 group_id = fields[0].GetUInt32();
        int32 spell_id = fields[1].GetInt32();

        groups.insert(std::set<uint32>::value_type(group_id));
        mSpellGroupSpell.insert(SpellGroupSpellMap::value_type((SpellGroup)group_id, spell_id));

    }
    while (result->NextRow());

    for (SpellGroupSpellMap::iterator itr = mSpellGroupSpell.begin(); itr != mSpellGroupSpell.end();)
    {
        if (itr->second < 0)
        {
            if (groups.find(abs(itr->second)) == groups.end())
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SpellGroup id %u listed in `spell_groups` does not exist", abs(itr->second));
                mSpellGroupSpell.erase(itr++);
            }
            else
                ++itr;
        }
        else
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(itr->second);

            if (!spellInfo)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_group` does not exist", itr->second);
                mSpellGroupSpell.erase(itr++);
            }
            // Needed for the fix "A more powerful spell is already active".
            /*else if (GetSpellRank(itr->second) > 1)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_group` is not first rank of spell", itr->second);
                mSpellGroupSpell.erase(itr++);
            }*/
            else
                ++itr;
        }
    }

    for (const auto group : groups)
    {
        std::set<uint32> spells;
        GetSetOfSpellsInSpellGroup(SpellGroup(group), spells);

        for (const auto spell : spells)
        {
            ++count;
            mSpellSpellGroup.insert(SpellSpellGroupMap::value_type(spell, SpellGroup(group)));
        }
    }
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell group definitions", count);
}

void SpellMgr::LoadSpellGroupStackRules()
{
    mSpellGroupStack.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                                0           1
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `group_id`, `stack_rule` FROM `spell_group_stack_rules`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell group stack rules", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();
        Field* fields = result->Fetch();

        uint32 group_id = fields[0].GetUInt32();
        uint8 stack_rule = fields[1].GetUInt32();
        if (stack_rule >= SPELL_GROUP_STACK_RULE_MAX)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SpellGroupStackRule %u listed in `spell_group_stack_rules` does not exist", stack_rule);
            continue;
        }

        SpellGroupSpellMapBounds spellGroup = GetSpellGroupSpellMapBounds((SpellGroup)group_id);

        if (spellGroup.first == spellGroup.second)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "SpellGroup id %u listed in `spell_group_stack_rules` does not exist", group_id);
            continue;
        }

        mSpellGroupStack[(SpellGroup)group_id] = (SpellGroupStackRule)stack_rule;

        ++count;
    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell group stack rules", count);
}

bool SpellMgr::ListMorePowerfulSpells(uint32 spellId, std::vector<uint32>& list) const
{
    std::vector<uint32> spellGroupIds;
    std::vector<uint32>::iterator spellGroupIdsIt;
    // first = groupid, second = spellId
    for (const auto& itr : mSpellGroupSpell)
    {
        if (itr.second == spellId)
        {
            // A spell can be in multiple groups. We care about the 'SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN' group
            SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(itr.first);
            // This group has no rule... No entry in 'spell_group_stack_rule'?
            if (found == mSpellGroupStack.end())
                continue;
            SpellGroupStackRule stackRule = found->second;
            if (stackRule == SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN)
            {
                spellGroupIds.push_back(itr.first);
            }
        }
    }
    if (spellGroupIds.empty())
        return false;
    for (spellGroupIdsIt = spellGroupIds.begin(); spellGroupIdsIt != spellGroupIds.end(); ++spellGroupIdsIt)
    {
        bool spellPassed = false;
        for (const auto& itr : mSpellGroupSpell)
        {
            if (itr.first != *(spellGroupIdsIt))
                continue;
            if (!spellPassed)
            {
                if (itr.second == spellId)
                    spellPassed = true;
                continue;
            }
            list.push_back(itr.second);
        }
        MANGOS_ASSERT(spellPassed == true);
    }
    return !list.empty();
}

bool SpellMgr::ListLessPowerfulSpells(uint32 spellId, std::vector<uint32>& list) const
{
    std::vector<uint32> spellGroupIds;
    std::vector<uint32>::iterator spellGroupIdsIt;
    // first = groupid, second = spellId
    for (const auto& itr : mSpellGroupSpell)
    {
        if (itr.second == spellId)
        {
            // A spell can be in multiple groups. We care about the 'SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN' group
            SpellGroupStackMap::const_iterator found = mSpellGroupStack.find(itr.first);
            // This group has no rule... No entry in 'spell_group_stack_rule'?
            if (found == mSpellGroupStack.end())
                continue;
            SpellGroupStackRule stackRule = found->second;
            if (stackRule == SPELL_GROUP_STACK_RULE_POWERFULL_CHAIN)
            {
                spellGroupIds.push_back(itr.first);
            }
        }
    }
    if (spellGroupIds.empty())
        return false;
    for (spellGroupIdsIt = spellGroupIds.begin(); spellGroupIdsIt != spellGroupIds.end(); ++spellGroupIdsIt)
    {
        for (const auto& itr : mSpellGroupSpell)
        {
            if (itr.first != *(spellGroupIdsIt))
                continue;
            if (itr.second == spellId)
                break;
            list.push_back(itr.second);
        }
    }
    return !list.empty();
}

static char const* const SpellTagNames[] =
{
    "SPELL_TAG_AGONY_DOOM",
    "SPELL_TAG_ARGENT_DAWN_COMMISSION",
    "SPELL_TAG_ASPECT",
    "SPELL_TAG_BATTLE_SHOUT",
    "SPELL_TAG_BERSERKING",
    "SPELL_TAG_BLIZZARD",
    "SPELL_TAG_CORRUPTION",
    "SPELL_TAG_DAMAGE_TOTEM",
    "SPELL_TAG_DAZE",
    "SPELL_TAG_DEMORALIZING_SHOUT",
    "SPELL_TAG_EXECUTE",
    "SPELL_TAG_FLASH_OF_LIGHT",
    "SPELL_TAG_FLURRY",
    "SPELL_TAG_HEALTH_FUNNEL_HEAL",
    "SPELL_TAG_HELLFIRE",
    "SPELL_TAG_HELLFIRE_CHANNEL",
    "SPELL_TAG_ICE_BLOCK",
    "SPELL_TAG_INSPIRATION",
    "SPELL_TAG_OLD_JUDGEMENT_OF_COMMAND",
    "SPELL_TAG_JUDGEMENT_OF_COMMAND",
    "SPELL_TAG_KEEPS_STEALTH",
    "SPELL_TAG_PREDATORY_STRIKES",
    "SPELL_TAG_PYROBLAST",
    "SPELL_TAG_SAP",
    "SPELL_TAG_SEAL_OF_RIGHTEOUSNESS",
    "SPELL_TAG_SEAL_OF_RIGHTEOUSNESS_DAMAGE",
    "SPELL_TAG_SOULSTONE_RESURRECTION",
    "SPELL_TAG_SOUL_FIRE",
    "SPELL_TAG_STONECLAW_TOTEM",
    "SPELL_TAG_TOTEM_HAS_THREAT",
    "SPELL_TAG_WARLOCK_ARMOR",
    "SPELL_TAG_WHIRLWIND",
};
static_assert(std::size(SpellTagNames) == MAX_SPELL_TAG, "SpellTagNames must list every SpellTag");

void SpellMgr::LoadSpellTags()
{
    for (auto const& spell : mSpellEntryMap)
        if (spell)
            spell->Tags.reset();

    uint32 count = 0;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `spell_id`, `tag` FROM `spell_tag`"));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const spellId = fields[0].GetUInt32();
            std::string const name = fields[1].GetCppString();
            auto const tag = std::find(std::begin(SpellTagNames), std::end(SpellTagNames), name) - std::begin(SpellTagNames);

            if (spellId >= mSpellEntryMap.size() || !mSpellEntryMap[spellId])
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_tag` does not exist", spellId);
                continue;
            }
            if (tag >= MAX_SPELL_TAG)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u has unknown tag %s in `spell_tag`", spellId, name.c_str());
                continue;
            }

            mSpellEntryMap[spellId]->Tags.set(tag);
            ++count;
        }
        while (result->NextRow());
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell tags", count);
}

void SpellMgr::LoadSpellStackKeys()
{
    for (auto const& spell : mSpellEntryMap)
    {
        if (spell)
        {
            spell->StackKey = 0;
            spell->RankKey = 0;
            spell->SingleTargetKey = 0;
        }
    }

    uint32 count = 0;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `spell_id`, `stack_key`, `rank_key`, `single_target_key` FROM `spell_stack_key`"));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const spellId = fields[0].GetUInt32();

            if (spellId >= mSpellEntryMap.size() || !mSpellEntryMap[spellId])
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_stack_key` does not exist", spellId);
                continue;
            }

            SpellEntry& spell = *mSpellEntryMap[spellId];
            spell.StackKey = fields[1].GetUInt32();
            spell.RankKey = fields[2].GetUInt32();
            spell.SingleTargetKey = fields[3].GetUInt32();
            ++count;
        }
        while (result->NextRow());
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell stack keys", count);
}

void SpellMgr::LoadSpellClientVisuals()
{
    for (auto const& spell : mSpellEntryMap)
        if (spell)
            spell->ClientVisualId = 0, spell->HasClientActiveIcon = false;

    uint32 count = 0;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query(
        "SELECT `SpellID`, `ID` FROM `dbc_spellxspellvisual` WHERE `DifficultyID` = 0 AND `SpellVisualID` <> 0 "
        "AND `ViewerUnitConditionID` = 0 AND `ViewerPlayerConditionID` = 0 AND `CasterUnitConditionID` = 0 AND `CasterPlayerConditionID` = 0 "
        "ORDER BY `SpellID`, `Priority` DESC, `ID`"));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const spellId = fields[0].GetUInt32();
            if (spellId >= mSpellEntryMap.size() || !mSpellEntryMap[spellId])
                continue;

            SpellEntry& spell = *mSpellEntryMap[spellId];
            if (!spell.ClientVisualId)
            {
                spell.ClientVisualId = fields[1].GetUInt32();
                ++count;
            }
        }
        while (result->NextRow());
    }

    result = std::unique_ptr<QueryResult>(WorldDatabase.Query(
        "SELECT DISTINCT `SpellID` FROM `dbc_spellxspellvisual` WHERE `DifficultyID` = 0 AND `ActiveIconFileID` <> 0"));
    if (result)
    {
        do
        {
            uint32 const spellId = result->Fetch()[0].GetUInt32();
            if (spellId < mSpellEntryMap.size() && mSpellEntryMap[spellId])
                mSpellEntryMap[spellId]->HasClientActiveIcon = true;
        }
        while (result->NextRow());
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell client visuals", count);
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                                0        1
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `entry`, `mask` FROM `spell_elixir`"));
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell elixir definitions", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(entry);

        if (!spellInfo)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell elixir definitions", count);
}

struct DoSpellThreat
{
    DoSpellThreat(SpellThreatMap& _threatMap) : threatMap(_threatMap), count(0) {}
    void operator()(uint32 spell_id)
    {
        SpellThreatEntry const& ste = state->second;
        // add ranks only for not filled data (spells adding flat threat are usually different for ranks)
        SpellThreatMap::const_iterator spellItr = threatMap.find(spell_id);
        if (spellItr == threatMap.end())
            threatMap[spell_id] = ste;

        // just assert that entry is not redundant
        else
        {
            SpellThreatEntry const& r_ste = spellItr->second;
            if (ste.threat == r_ste.threat && ste.multiplier == r_ste.multiplier && ste.inverseEffectMask == r_ste.inverseEffectMask)
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_threat` as custom rank has same data as Rank 1, so redundant", spell_id);
        }
    }
    char const* TableName()
    {
        return "spell_threat";
    }
    bool IsValidCustomRank(SpellThreatEntry const& ste, uint32 entry, uint32 first_id)
    {
        if (!ste.threat)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_threat` is not first rank (%u) in chain and has no threat", entry, first_id);
            // prevent loading unexpected data
            return false;
        }
        return true;
    }
    void AddEntry(SpellThreatEntry const& ste, SpellEntry const* spell)
    {
        threatMap[spell->Id] = ste;

        // flat threat bonus and attack power bonus currently only work properly when all
        // effects have same targets, otherwise, we'd need to seperate it by effect index
        if (ste.inverseEffectMask == 0 && ste.threat != 0)
        {
            uint32 const* effect = spell->Effect;
            uint32 const* targetA = spell->EffectImplicitTargetA;
            if ((effect[EFFECT_INDEX_0] && effect[EFFECT_INDEX_1] && targetA[EFFECT_INDEX_0] != targetA[EFFECT_INDEX_1]) ||
                (effect[EFFECT_INDEX_0] && effect[EFFECT_INDEX_2] && targetA[EFFECT_INDEX_0] != targetA[EFFECT_INDEX_2]) ||
                (effect[EFFECT_INDEX_1] && effect[EFFECT_INDEX_2] && targetA[EFFECT_INDEX_1] != targetA[EFFECT_INDEX_2]))
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_threat` has effects with different targets, threat may be assigned incorrectly. Consider using inverse effect mask.", spell->Id);
        }
        ++count;
    }
    bool HasEntry(uint32 spellId)
    {
        return threatMap.count(spellId) > 0;
    }
    bool SetStateToEntry(uint32 spellId)
    {
        return (state = threatMap.find(spellId)) != threatMap.end();
    }

    SpellThreatMap& threatMap;
    SpellThreatMap::const_iterator state;
    uint32 count;
};

void SpellMgr::LoadSpellThreats()
{
    mSpellThreatMap.clear();                                // need for reload case

    //                                                                0        1         2             3
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `entry`, `threat`, `multiplier`, `inverse_effect_mask` FROM `spell_threat`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> No spell threat entries loaded.");
        return;
    }

    SpellRankHelper<SpellThreatEntry, DoSpellThreat, SpellThreatMap> rankHelper(*this, mSpellThreatMap);

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();

        SpellThreatEntry ste;
        ste.threat = fields[1].GetUInt16();
        ste.multiplier = fields[2].GetFloat();
        ste.inverseEffectMask = fields[3].GetUInt8();

        rankHelper.RecordRank(ste, entry);

    }
    while (result->NextRow());

    rankHelper.FillHigherRanks();

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell threat entries", rankHelper.worker.count);
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const* spellInfo_1, uint32 spellId_2) const
{
    SpellEntry const* spellInfo_2 = sSpellMgr.GetSpellEntry(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return false;
    if (spellInfo_1->Id == spellId_2) return false;
    // Nostalrius : Check generique.
    if (spellInfo_1->SpellFamilyName == spellInfo_2->SpellFamilyName &&
        spellInfo_1->SpellFamilyFlags == spellInfo_2->SpellFamilyFlags &&
        spellInfo_1->RankKey && spellInfo_1->RankKey == spellInfo_2->RankKey &&
        spellInfo_1->SpellFamilyName != SPELLFAMILY_GENERIC &&
        spellInfo_1->Effect[0] == spellInfo_2->Effect[0] &&
        spellInfo_1->EffectApplyAuraName[0] == spellInfo_2->EffectApplyAuraName[0] &&
        (spellInfo_1->EffectApplyAuraName[0] != SPELL_AURA_ADD_FLAT_MODIFIER ||
         spellInfo_1->EffectMiscValue[0] == spellInfo_2->EffectMiscValue[0]))
    {
        // Same modifier but it affects different spells
        if (spellInfo_1->EffectApplyAuraName[0] == SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo_1->EffectItemType[0] != 0 && spellInfo_2->EffectItemType[0] != 0 &&
            !(spellInfo_1->EffectItemType[0] & spellInfo_2->EffectItemType[0]))
        {
            return GetFirstSpellInChain(spellInfo_1->Id) == GetFirstSpellInChain(spellId_2);
        }

        return true;
    }

    return GetFirstSpellInChain(spellInfo_1->Id) == GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const
{
    SpellGroup unused;
    if (CheckSpellGroupStackRules(spellId_1, spellId_2, unused) != SPELL_GROUP_STACK_RULE_DEFAULT)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "[STACK][%u/%u] Pas de stack (DB/spell_group)", spellId_1, spellId_2);
        return false;
    }

    SpellEntry const* spellInfo_1 = sSpellMgr.GetSpellEntry(spellId_1);
    SpellEntry const* spellInfo_2 = sSpellMgr.GetSpellEntry(spellId_2);

    if (!spellInfo_1 || !spellInfo_2)
        return false;

    if (spellId_1 == spellId_2)
        return spellInfo_1->HasAttribute(SPELL_ATTR_EX_AURA_UNIQUE);

    auto const isOneOf = [](uint32 id, std::initializer_list<uint32> ids) { return std::find(ids.begin(), ids.end(), id) != ids.end(); };

    // Resurrection sickness
    if ((spellInfo_1->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS) != (spellInfo_2->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
        return false;

    // Allow stack passive and not passive spells
    if ((spellInfo_1->Attributes & SPELL_ATTR_PASSIVE) != (spellInfo_2->Attributes & SPELL_ATTR_PASSIVE))
        return false;

    // Specific spell family spells
    switch (spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (spellInfo_2->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:                   // same family case
                {
                    // Thunderfury
                    if ((spellInfo_1->Id == 21992 && spellInfo_2->Id == 27648) ||
                            (spellInfo_2->Id == 21992 && spellInfo_1->Id == 27648))
                        return false;

                    // Brood Affliction: Bronze
                    if ((spellInfo_1->Id == 23170 && spellInfo_2->Id == 23171) ||
                            (spellInfo_2->Id == 23170 && spellInfo_1->Id == 23171))
                        return false;

                    // Regular and Night Elf Ghost
                    if ((spellInfo_1->Id == 8326 && spellInfo_2->Id == 20584) ||
                            (spellInfo_2->Id == 8326 && spellInfo_1->Id == 20584))
                        return false;

                    break;
                }
                case SPELLFAMILY_WARRIOR:
                {
                    // Improved Hamstring -> Hamstring (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & uint64(0x2)) && spellInfo_1->Id == 23694)
                        return false;
                    break;
                }
                case SPELLFAMILY_HUNTER:
                {
                    // Concussive Shot and Imp. Concussive Shot (multi-family check)
                    if (spellInfo_1->Id == 19410 && spellInfo_2->Id == 5116)
                        return false;

                    // Improved Wing Clip -> Wing Clip (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & uint64(0x40)) && spellInfo_1->Id == 19229)
                        return false;
                    break;
                }
            }
            break;
        case SPELLFAMILY_MAGE:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_MAGE)
            {
                // Pyroblast and Fireball
                if ((spellInfo_1->HasTag(SPELL_TAG_PYROBLAST) && GetFirstSpellInChain(spellId_2) == 133) ||
                        (spellInfo_2->HasTag(SPELL_TAG_PYROBLAST) && GetFirstSpellInChain(spellId_1) == 133))
                    return false;

                // Blizzard & Chilled (and some other stacked with blizzard spells
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x80)) && (spellInfo_2->SpellFamilyFlags & uint64(0x100000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x80)) && (spellInfo_1->SpellFamilyFlags & uint64(0x100000))))
                    return false;

                // Fireball & Pyroblast (Dots)
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x1)) && (spellInfo_2->SpellFamilyFlags & uint64(0x400000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x1)) && (spellInfo_1->SpellFamilyFlags & uint64(0x400000))))
                    return false;

                // Arcane Missiles
                if (spellInfo_1->IsFitToFamilyMask(1 << CF_MAGE_ARCANE_MISSILES_CHANNEL) && spellInfo_2->IsFitToFamilyMask(1 << CF_MAGE_ARCANE_MISSILES_CHANNEL))
                    return false;
            }
            break;
        case SPELLFAMILY_WARLOCK:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARLOCK)
            {
                // Siphon Life and Drain Life
                if ((GetFirstSpellInChain(spellId_1) == 18265 && GetFirstSpellInChain(spellId_2) == 689) ||
                        (GetFirstSpellInChain(spellId_2) == 18265 && GetFirstSpellInChain(spellId_1) == 689))
                    return false;

                // Corruption and (Curse of Agony or Curse of Doom)
                if ((spellInfo_1->HasTag(SPELL_TAG_CORRUPTION) && spellInfo_2->HasTag(SPELL_TAG_AGONY_DOOM)) ||
                        (spellInfo_2->HasTag(SPELL_TAG_CORRUPTION) && spellInfo_1->HasTag(SPELL_TAG_AGONY_DOOM)))
                    return false;
            }
            break;
        case SPELLFAMILY_WARRIOR:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARRIOR)
            {
                // Rend and Deep Wound
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x20)) && (spellInfo_2->SpellFamilyFlags & uint64(0x1000000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x20)) && (spellInfo_1->SpellFamilyFlags & uint64(0x1000000000))))
                    return false;

                // Demoralizing Shout and Battle Shout
                if ((spellInfo_1->HasTag(SPELL_TAG_DEMORALIZING_SHOUT) && spellInfo_2->HasTag(SPELL_TAG_BATTLE_SHOUT)) ||
                        (spellInfo_2->HasTag(SPELL_TAG_DEMORALIZING_SHOUT) && spellInfo_1->HasTag(SPELL_TAG_BATTLE_SHOUT)))
                    return false;
            }

            // Hamstring -> Improved Hamstring (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & uint64(0x2)) && spellInfo_2->Id == 23694)
                return false;

            break;
        case SPELLFAMILY_PRIEST:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST)
            {
                //Greater Heal Renew (8/8 transcendence) (and Renew.)
                if (spellId_1==22009 || spellId_2==22009)
                    return false;

                //Devouring Plague and Shadow Vulnerability
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x2000000)) && (spellInfo_2->SpellFamilyFlags & uint64(0x800000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x2000000)) && (spellInfo_1->SpellFamilyFlags & uint64(0x800000000))))
                    return false;

                //StarShards and Shadow Word: Pain
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x200000)) && (spellInfo_2->SpellFamilyFlags & uint64(0x8000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x200000)) && (spellInfo_1->SpellFamilyFlags & uint64(0x8000))))
                    return false;
            }
            break;
        case SPELLFAMILY_DRUID:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_DRUID)
            {
                // Cat Form and Feline Swiftness Passive* (1.x specific conflict)
                if (isOneOf(spellId_1, {24864, 24867, 24868}) && isOneOf(spellId_2, {24864, 24867, 24868}))
                    return false;
            }

            break;
        case SPELLFAMILY_ROGUE:
            // Garrote -> Garrote-Silence (multi-family check)
            if (isOneOf(spellId_1, {14077, 14078}) && isOneOf(spellId_2, {14077, 14078}))
                return false;
            break;
        case SPELLFAMILY_HUNTER:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_HUNTER)
            {
                // Rapid Fire & Quick Shots
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x20)) && (spellInfo_2->SpellFamilyFlags & uint64(0x20000000000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x20)) && (spellInfo_1->SpellFamilyFlags & uint64(0x20000000000))))
                    return false;

                // Serpent Sting & (Immolation/Explosive Trap Effect)
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x4)) && (spellInfo_2->SpellFamilyFlags & uint64(0x00000004000))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x4)) && (spellInfo_1->SpellFamilyFlags & uint64(0x00000004000))))
                    return false;

                // Wyvern Sting DoT & Immolation Trap Effect - using family flags
                if (((spellInfo_1->SpellFamilyFlags & uint64(0x10000)) && (spellInfo_2->SpellFamilyFlags & uint64(0x4))) ||
                        ((spellInfo_2->SpellFamilyFlags & uint64(0x10000)) && (spellInfo_1->SpellFamilyFlags & uint64(0x4))))
                    return false;

                // Bestial Wrath
                if (isOneOf(spellId_1, {24395, 24396, 24397, 26592, 28758, 29218}) && isOneOf(spellId_2, {24395, 24396, 24397, 26592, 28758, 29218}))
                    return false;
            }

            // Wing Clip -> Improved Wing Clip (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & uint64(0x40)) && spellInfo_2->Id == 19229)
                return false;

            // Concussive Shot and Imp. Concussive Shot (multi-family check)
            if (spellInfo_2->Id == 19410 && spellInfo_1->Id == 5116)
                return false;
            break;
        case SPELLFAMILY_PALADIN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN)
            {
                // Paladin Seals
                if (spellInfo_1->IsSealSpell() && spellInfo_2->IsSealSpell())
                    return true;
                // Concentration Aura and Improved Concentration Aura and Aura Mastery
                if (isOneOf(spellId_1, {20254, 20255, 20256, 27736}) && isOneOf(spellId_2, {20254, 20255, 20256, 27736}))
                    return false;
            }
            break;
        case SPELLFAMILY_SHAMAN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_SHAMAN)
            {
                // Windfury weapon
                if (GetFirstSpellInChain(spellId_1) == 8232 && GetFirstSpellInChain(spellId_2) == 8232 &&
                        !spellInfo_1->IsFitToFamilyMask(spellInfo_2->SpellFamilyFlags))
                    return false;
            }
            break;
        default:
            break;
    }

    // more generic checks
    if (spellInfo_1->StackKey && spellInfo_1->StackKey == spellInfo_2->StackKey &&
            spellInfo_1->IsPositiveSpell() == spellInfo_2->IsPositiveSpell() // judgement does not remove the enemy seal in a pala-pala duel
       )
    {
        bool isModifier = false;
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                    spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                    spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                    spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER)
                isModifier = true;
        }

        if (!isModifier)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "[STACK][%u/%u] Meme icone et non modifier.", spellId_1, spellId_2);
            return true;
        }
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "[STACK][%u/%u] IsRankSpellDueToSpell", spellId_1, spellId_2);
        return true;
    }

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    // Why would a buff and a debuff ever be exclusive with each other?
    // Fixes Corrupted Mind getting removed by Lightning Shield.
    if (spellInfo_1->IsPositiveSpell() != spellInfo_2->IsPositiveSpell())
        return false;

    // potions work differently
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_POTION)
        return false;

    bool dummy_only = true;
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
                spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
                spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
                spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

        // ignore dummy only spells
        if (spellInfo_1->Effect[i] && spellInfo_1->Effect[i] != SPELL_EFFECT_DUMMY && spellInfo_1->EffectApplyAuraName[i] != SPELL_AURA_DUMMY)
            dummy_only = false;
    }
    if (dummy_only)
        return false;

    sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "[STACK][%u/%u] not dummy only.", spellId_1, spellId_2);
    return true;
}

bool SpellMgr::IsProfessionOrRidingSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionOrRidingSkill(skill);
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsTradeskillSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    return spellInfo->Attributes & SPELL_ATTR_IS_TRADESKILL;
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId) == 1;
}

bool SpellMgr::IsSkillBonusSpell(uint32 spellId) const
{
    SkillLineAbilityMapBounds bounds = GetSkillLineAbilityMapBoundsBySpellId(spellId);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* pAbility = _spell_idx->second;
        if (!pAbility || pAbility->learnOnGetSkill != ABILITY_LEARNED_ON_GET_PROFESSION_SKILL)
            continue;

        if (pAbility->req_skill_value > 0)
            return true;
    }

    return false;
}

SpellEntry const* SpellMgr::SelectAuraRankForLevel(SpellEntry const* spellInfo, uint32 level) const
{
    // fast case
    if (level + 10 >= spellInfo->spellLevel)
        return spellInfo;

    // ignore selection for passive spells
    if (spellInfo->IsPassiveSpell())
        return spellInfo;

    bool needRankSelection = false;
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // for simple aura in check apply to any non caster based targets, in rank search mode to any explicit targets
        if (((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                (IsExplicitPositiveTarget(spellInfo->EffectImplicitTargetA[i]) ||
                 IsAreaEffectPossitiveTarget(SpellTarget(spellInfo->EffectImplicitTargetA[i])))) ||
                spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY) &&
                spellInfo->IsPositiveEffect(SpellEffectIndex(i)))
        {
            needRankSelection = true;
            break;
        }
    }

    // not required (rank check more slow so check it here)
    if (!needRankSelection || GetSpellRank(spellInfo->Id) == 0)
        return spellInfo;

    for (uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const* nextSpellInfo = sSpellMgr.GetSpellEntry(nextSpellId);
        if (!nextSpellInfo)
            break;

        // if found appropriate level
        if (level + 10 >= nextSpellInfo->spellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return nullptr;
}

typedef std::unordered_map<uint32, uint32> AbilitySpellPrevMap;

static void LoadSpellChains_AbilityHelper(SpellChainMap& chainMap, AbilitySpellPrevMap const& prevRanks, uint32 spell_id, uint32 prev_id, uint32 deep = 30)
{
    // spell already listed in chains store
    SpellChainMap::const_iterator chain_itr = chainMap.find(spell_id);
    if (chain_itr != chainMap.end())
    {
        MANGOS_ASSERT(chain_itr->second.prev == prev_id && "LoadSpellChains_AbilityHelper: Conflicting data in talents or spell abilities dbc");
        return;
    }

    // prev rank listed in main chain table (can fill correct data directly)
    SpellChainMap::const_iterator prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr != chainMap.end())
    {
        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_chain_itr->second.first;
        node.rank  = prev_chain_itr->second.rank + 1;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    // prev spell not listed in prev ranks store, so it first rank
    AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(prev_id);
    if (prev_itr == prevRanks.end())
    {
        SpellChainNode prev_node;
        prev_node.prev  = 0;
        prev_node.first = prev_id;
        prev_node.rank  = 1;
        prev_node.req   = 0;
        chainMap[prev_id] = prev_node;

        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_id;
        node.rank  = 2;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    if (deep == 0)
    {
        MANGOS_ASSERT(false && "LoadSpellChains_AbilityHelper: Infinity cycle in spell ability data");
        return;
    }

    // prev rank listed, so process it first
    LoadSpellChains_AbilityHelper(chainMap, prevRanks, prev_id, prev_itr->second, deep - 1);

    // prev rank must be listed now
    prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr == chainMap.end())
        return;

    SpellChainNode node;
    node.prev  = prev_id;
    node.first = prev_chain_itr->second.first;
    node.rank  = prev_chain_itr->second.rank + 1;
    node.req   = 0;
    chainMap[spell_id] = node;
}

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case
    mSpellChainsNext.clear();                               // need for reload case

    // load known data for talents
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        // not add ranks for 1 ranks talents (if exist non ranks spells then it will included in table data)
        if (!talentInfo->RankID[1])
            continue;

        for (int j = 0; j < MAX_TALENT_RANK; j++)
        {
            uint32 spell_id = talentInfo->RankID[j];
            if (!spell_id)
                continue;

            if (!sSpellMgr.GetSpellEntry(spell_id))
            {
                //sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Talent %u not exist as spell",spell_id);
                continue;
            }

            SpellChainNode node;
            node.prev  = (j > 0) ? talentInfo->RankID[j - 1] : 0;
            node.first = talentInfo->RankID[0];
            node.rank  = j + 1;
            node.req   = 0;

            mSpellChains[spell_id] = node;
        }
    }

    // load known data from spell abilities
    {
        // we can calculate ranks only after full data generation
        AbilitySpellPrevMap prevRanks;
        for (const auto& ab_itr : mSkillLineAbilityMapBySpellId)
        {
            uint32 spell_id = ab_itr.first;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* spell_entry = sSpellMgr.GetSpellEntry(spell_id);
            if (!spell_entry)                                   // no cases
                continue;

            // ignore spell without forwards (non ranked or missing info in skill abilities)
            uint32 forward_id = ab_itr.second->forward_spellid;

            // by some strange reason < 3.x clients not have forward spell for 2366
            if (spell_id == 2366)                           // Herb Gathering, Apprentice
                forward_id = 2368;

            // Seal of Righteousness (20154) make double in spellbook
            if (spell_id == 20154)
                continue;

            if (!forward_id)
                continue;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* forward_entry = sSpellMgr.GetSpellEntry(forward_id);
            if (!forward_entry)
                continue;

            // some forward spells still exist but excluded from real use as ranks and not listed in skill abilities now
            SkillLineAbilityMapBounds bounds = mSkillLineAbilityMapBySpellId.equal_range(forward_id);
            if (bounds.first == bounds.second)
                continue;

            // spell already listed in chains store
            SpellChainMap::const_iterator chain_itr = mSpellChains.find(forward_id);
            if (chain_itr != mSpellChains.end())
            {
                MANGOS_ASSERT(chain_itr->second.prev == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // spell already listed in prev ranks store
            AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(forward_id);
            if (prev_itr != prevRanks.end())
            {
                MANGOS_ASSERT(prev_itr->second == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // prev rank listed in main chain table (can fill correct data directly)
            SpellChainMap::const_iterator prev_chain_itr = mSpellChains.find(spell_id);
            if (prev_chain_itr != mSpellChains.end())
            {
                SpellChainNode node;
                node.prev  = spell_id;
                node.first = prev_chain_itr->second.first;
                node.rank  = prev_chain_itr->second.rank + 1;
                node.req   = 0;

                mSpellChains[forward_id] = node;
                continue;
            }

            // need temporary store for later rank calculation
            prevRanks[forward_id] = spell_id;
        }

        while (!prevRanks.empty())
        {
            uint32 spell_id = prevRanks.begin()->first;
            uint32 prev_id  = prevRanks.begin()->second;
            prevRanks.erase(prevRanks.begin());

            LoadSpellChains_AbilityHelper(mSpellChains, prevRanks, spell_id, prev_id);
        }
    }

    // load custom case
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `spell_id`, `prev_spell`, `first_spell`, `rank`, `req_spell` FROM `spell_chain`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded 0 spell chain records");
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "`spell_chains` table is empty!");
        return;
    }

    uint32 dbc_count = mSpellChains.size();
    uint32 new_count = 0;
    uint32 req_count = 0;

    BarGoLink bar(result->GetRowCount());
    do
    {
        bar.step();
        Field* fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();
        node.req   = fields[4].GetUInt32();

        if (!sSpellMgr.GetSpellEntry(spell_id))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_chain` does not exist", spell_id);
            continue;
        }

        SpellChainMap::iterator chain_itr = mSpellChains.find(spell_id);
        if (chain_itr != mSpellChains.end())
        {
            if (chain_itr->second.rank != node.rank)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected rank %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.rank);
                continue;
            }

            if (chain_itr->second.prev != node.prev)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected prev %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.prev);
                continue;
            }

            if (chain_itr->second.first != node.first)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected first %u by DBC data.",
                                spell_id, node.prev, node.first, node.rank, node.req, chain_itr->second.first);
                continue;
            }

            // update req field by table data
            if (node.req)
            {
                chain_itr->second.req = node.req;
                ++req_count;
                continue;
            }

            // in other case redundant
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) already added (talent or spell ability with forward) and non need in `spell_chain`",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (node.prev != 0 && !sSpellMgr.GetSpellEntry(node.prev))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has nonexistent previous rank spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (!sSpellMgr.GetSpellEntry(node.first))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing first rank spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if ((spell_id == node.first) != (node.rank <= 1) ||
                (spell_id == node.first) != (node.prev == 0) ||
                (node.rank <= 1) != (node.prev == 0))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not compatible chain data.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        if (node.req != 0 && !sSpellMgr.GetSpellEntry(node.req))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing required spell.",
                            spell_id, node.prev, node.first, node.rank, node.req);
            continue;
        }

        // talents not required data in spell chain for work, but must be checked if present for integrity
        if (TalentSpellPos const* pos = GetTalentSpellPos(spell_id))
        {
            if (node.rank != pos->rank + 1)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong rank.",
                                spell_id, node.prev, node.first, node.rank, node.req);
                continue;
            }

            if (TalentEntry const* talentEntry = sTalentStore.LookupEntry(pos->talent_id))
            {
                if (node.first != talentEntry->RankID[0])
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong first rank spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }

                if (node.rank > 1 && node.prev != talentEntry->RankID[node.rank - 1 - 1])
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong prev rank spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }

                if (node.req != talentEntry->DependsOnSpell)
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong required spell.",
                                    spell_id, node.prev, node.first, node.rank, node.req);
                    continue;
                }
            }
        }

        mSpellChains[spell_id] = node;

        ++new_count;
    }
    while (result->NextRow());

    // additional integrity checks
    for (const auto& itr : mSpellChains)
    {
        if (itr.second.prev)
        {
            SpellChainMap::const_iterator i_prev = mSpellChains.find(itr.second.prev);
            if (i_prev == mSpellChains.end())
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found previous rank spell in table.",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
            }
            else if (i_prev->second.first != itr.second.first)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_prev->second.prev, i_prev->second.first, i_prev->second.rank, i_prev->second.req);
            }
            else if (i_prev->second.rank + 1 != itr.second.rank)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_prev->second.prev, i_prev->second.first, i_prev->second.rank, i_prev->second.req);
            }
        }

        if (itr.second.req)
        {
            SpellChainMap::const_iterator i_req = mSpellChains.find(itr.second.req);
            if (i_req == mSpellChains.end())
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found required rank spell in table.",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
            }
            else if (i_req->second.first == itr.second.first)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell from same spell chain (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_req->second.prev, i_req->second.first, i_req->second.rank, i_req->second.req);
            }
            else if (i_req->second.req)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell with required spell (prev: %u, first: %u, rank: %d, req: %u).",
                                itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req,
                                i_req->second.prev, i_req->second.first, i_req->second.rank, i_req->second.req);
            }
        }
    }

    // fill next rank cache
    for (const auto& itr : mSpellChains)
    {
        uint32 spell_id = itr.first;
        SpellChainNode const& node = itr.second;

        if (node.prev)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.prev, spell_id));

        if (node.req)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.req, spell_id));
    }

    // check single rank redundant cases (single rank talents not added by default so this can be only custom cases)
    for (const auto& itr : mSpellChains)
    {
        // skip non-first ranks, and spells with additional reqs
        if (itr.second.rank > 1 || itr.second.req)
            continue;

        if (mSpellChainsNext.find(itr.first) == mSpellChainsNext.end())
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has single rank data, so redundant.",
                            itr.first, itr.second.prev, itr.second.first, itr.second.rank, itr.second.req);
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell chain records (%u from DBC data with %u req field updates, and %u loaded from table)", dbc_count + new_count, dbc_count, req_count, new_count);
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    BarGoLink bar(sSpellMgr.GetMaxSpellId());
    for (uint32 spell = 0; spell < sSpellMgr.GetMaxSpellId(); ++spell)
    {
        bar.step();
        SpellEntry const* entry = sSpellMgr.GetSpellEntry(spell);

        if (!entry)
            continue;

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.step     = entry->CalculateSimpleValue(SpellEffectIndex(i));
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = dbc_node.step * 75;
                dbc_node.maxvalue = dbc_node.step * 75;

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u Spell Learn Skills from DBC", dbc_count);
}

void SpellMgr::LoadSpellEnchantCharges()
{
    mSpellEnchantChargesMap.clear();                              // need for reload case

    std::map<uint32, uint32> enchantCharges;
    std::unique_ptr<QueryResult> result = WorldDatabase.Query("SELECT `ID`, `Charges` FROM `dbc_spellitemenchantment` WHERE `Charges` <> 0");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            enchantCharges[fields[0].GetUInt32()] = fields[1].GetUInt32();
        } while (result->NextRow());
    }

    uint32 count = 0;
    for (std::unique_ptr<SpellEntry> const& spell : mSpellEntryMap)
    {
        if (!spell)
            continue;

        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spell->Effect[i] != SPELL_EFFECT_ENCHANT_ITEM && spell->Effect[i] != SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY &&
                spell->Effect[i] != SPELL_EFFECT_ENCHANT_HELD_ITEM)
                continue;

            auto itr = enchantCharges.find(spell->EffectMiscValue[i]);
            if (itr != enchantCharges.end())
            {
                mSpellEnchantChargesMap[spell->Id] = itr->second;
                ++count;
            }
            break;
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell enchant charges", count);
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    //                                                                0        1          2
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `entry`, `SpellID`, `Active` FROM `spell_learn_spell`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded 0 spell learn spells");
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    BarGoLink bar(result->GetRowCount());
    do
    {
        bar.step();
        Field* fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.active     = fields[2].GetBool();
        node.autoLearned = false;

        if (!sSpellMgr.GetSpellEntry(spell_id))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_learn_spell` does not exist", spell_id);
            continue;
        }

        if (!sSpellMgr.GetSpellEntry(node.spell))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_learn_spell` learning nonexistent spell %u", spell_id, node.spell);
            continue;
        }

        if (GetTalentSpellCost(node.spell))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_learn_spell` attempt learning talent spell %u, skipped", spell_id, node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id, node));

        ++count;
    }
    while (result->NextRow());

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < sSpellMgr.GetMaxSpellId(); ++spell)
    {
        SpellEntry const* entry = sSpellMgr.GetSpellEntry(spell);

        if (!entry)
            continue;

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.active      = true;                // all dbc based learned spells is active (show in spell book or hide by client itself)

                // ignore learning nonexistent spells (broken/outdated/or generic learning spell 483
                if (!sSpellMgr.GetSpellEntry(dbc_node.spell))
                    continue;

                // talent or passive spells or skill-step spells auto-casted and not need dependent learning,
                // pet teaching spells don't must be dependent learning (casted)
                // other required explicit dependent learning
                dbc_node.autoLearned = entry->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET || GetTalentSpellCost(spell) > 0 || entry->IsPassiveSpell() || entry->HasEffect(SPELL_EFFECT_SKILL_STEP);

                SpellLearnSpellMapBounds db_node_bounds = GetSpellLearnSpellMapBounds(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                                        spell, dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                 // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell, dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count);
}

void SpellMgr::LoadSpellScriptTarget()
{
    std::set<uint32> conditions;

    // Load existing condition Ids so we can check for wrong condition Id later.
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `condition_entry` FROM `conditions`"));

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 conditionId = fields[0].GetUInt32();
            conditions.insert(conditionId);
        }
        while (result->NextRow());
    }

    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    result = WorldDatabase.PQuery("SELECT `entry`, `type`, `targetEntry`, `conditionId`, `inverseEffectMask` FROM `spell_script_target`");

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, ">> Loaded 0 SpellScriptTarget. DB table `spell_script_target` is empty.");
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();
        uint32 conditionId = fields[3].GetUInt32();
        uint32 effectMask  = fields[4].GetUInt32();

        SpellEntry const* spellProto = sSpellMgr.GetSpellEntry(spellId);

        if (!spellProto)
        {
            if (!sSpellMgr.IsExistingSpellId(spellId))
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.", spellId, targetEntry);
            continue;
        }

        if (conditionId)
        {
            if (conditions.find(conditionId) == conditions.end())
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: conditionId %u listed for spell %u does not exist.", conditionId, spellId);
                continue;
            }
        }

        bool targetfound = false;
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_LOCATION_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_LOCATION_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetB[i] == TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_SRC_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_GAMEOBJECTS_SCRIPT_AOE_AT_DEST_LOC ||
                spellProto->EffectImplicitTargetA[i] == TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60 ||
                spellProto->EffectImplicitTargetB[i] == TARGET_ENUM_UNITS_SCRIPT_IN_CONE_60)
            {
                targetfound = true;
                break;
            }
        }

        if (!targetfound)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implicit target TARGET_UNIT_SCRIPT_NEAR_CASTER(38) or TARGET_LOCATION_SCRIPT_NEAR_CASTER (46) or TARGET_GAMEOBJECT_SCRIPT_NEAR_CASTER (40).", spellId, targetEntry);
            continue;
        }

        if (type >= MAX_SPELL_TARGET_TYPE)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.", type, targetEntry);
            continue;
        }

        // Checks by target type
        switch (type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if (!targetEntry)
                    break;

                if (!sObjectMgr.GetGameObjectTemplate(targetEntry))
                {
                    if (!sObjectMgr.IsExistingGameObjectId(targetEntry))
                        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: gameobject template entry %u does not exist.", targetEntry);
                    continue;
                }
                break;
            }
            case SPELL_TARGET_TYPE_PLAYER:
            {
                // nothing to check
                break;
            }
            default:
            {
                if (!targetEntry)
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: target entry == 0 for not GO target type (%u).", type);
                    continue;
                }
                if (!sObjectMgr.GetCreatureTemplate(targetEntry))
                {
                    if (!sObjectMgr.IsExistingCreatureId(targetEntry))
                        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Table `spell_script_target`: creature template entry %u does not exist.", targetEntry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId, SpellTargetEntry(SpellTargetType(type), targetEntry, conditionId, effectMask)));

        ++count;
    }
    while (result->NextRow());

    // Check all spells
    /* Disabled (lot errors at this moment)
    for(uint32 i = 1; i < sSpellMgr.GetMaxSpellId; ++i)
    {
        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(i);
        if (!spellInfo)
            continue;

        bool found = false;
        for (uint8 j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if (spellInfo->EffectImplicitTargetA[j] == TARGET_UNIT_SCRIPT_NEAR_CASTER || spellInfo->EffectImplicitTargetA[j] != TARGET_UNIT_CASTER && spellInfo->EffectImplicitTargetB[j] == TARGET_UNIT_SCRIPT_NEAR_CASTER)
            {
                SpellScriptTarget::const_iterator lower = GetBeginSpellScriptTarget(spellInfo->Id);
                SpellScriptTarget::const_iterator upper = GetEndSpellScriptTarget(spellInfo->Id);
                if (lower==upper)
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell (ID: %u) has effect EffectImplicitTargetA/EffectImplicitTargetB = %u (TARGET_UNIT_SCRIPT_NEAR_CASTER), but does not have record in `spell_script_target`",spellInfo->Id,TARGET_UNIT_SCRIPT_NEAR_CASTER);
                    break;                                  // effects of spell
                }
            }
        }
    }
    */

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                               0        1      2
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `spell`, `pet`, `aura` FROM `spell_pet_auras`"));
    if (!result)
    {

        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell pet auras", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 spell = fields[0].GetUInt32();
        uint32 pet = fields[1].GetUInt32();
        uint32 aura = fields[2].GetUInt32();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if (itr != mSpellPetAuraMap.end())
            itr->second.AddAura(pet, aura);
        else
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(spell);
            if (!spellInfo)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            uint8 i = 0;
            for (; i < MAX_EFFECT_INDEX; ++i)
                if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                        spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if (i == MAX_EFFECT_INDEX)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellMgr.GetSpellEntry(aura);
            if (!spellInfo2)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER_PET, spellInfo->CalculateSimpleValue(SpellEffectIndex(i)));
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell pet auras", count);
}

// Some checks for spells, to prevent adding deprecated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch (spellInfo->Effect[i])
        {
            case 0:
                continue;

            // craft spell for crafting nonexistent item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (!sObjectMgr.GetItemPrototype(spellInfo->EffectItemType[i]))
                {
                    if (msg)
                    {
                        if (pl)
                            pl->PSendSysMessage("Craft spell %u create item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, spellInfo->EffectItemType[i]);
                        else
                            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Craft spell %u create item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellMgr.GetSpellEntry(spellInfo->EffectTriggerSpell[i]);
                if (!IsSpellValid(spellInfo2, pl, msg))
                {
                    if (msg)
                    {
                        if (pl)
                            pl->PSendSysMessage("Spell %u learn to broken spell %u, and then...", spellInfo->Id, spellInfo->EffectTriggerSpell[i]);
                        else
                            sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Spell %u learn to invalid spell %u, and then...", spellInfo->Id, spellInfo->EffectTriggerSpell[i]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if (need_check_reagents)
    {
        for (int j : spellInfo->Reagent)
        {
            if (j > 0 && !sObjectMgr.GetItemPrototype(j))
            {
                if (msg)
                {
                    if (pl)
                        pl->PSendSysMessage("Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, j);
                    else
                        sLog.Out(LOG_DBERROR, LOG_LVL_ERROR, "Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.", spellInfo->Id, j);
                }
                return false;
            }
        }
    }

    return true;
}

void SpellMgr::LoadSpellCones()
{
    mSpellCones.clear();                              // need for reload case

    uint32 count = 0;

    std::unique_ptr<QueryResult> result = WorldDatabase.Query("SELECT `SpellID`, `ConeDegrees` FROM `dbc_spelltargetrestrictions` WHERE `DifficultyID` = 0 AND `ConeDegrees` <> 60");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 const entry = fields[0].GetUInt32();
            float const degrees = fields[1].GetFloat();

            if (!GetSpellEntry(entry))
                continue;

            if (degrees < -360.0f || degrees > 360.0f)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u has a cone angle %g outside of the valid range", entry, degrees);
                continue;
            }

            mSpellCones[entry] = degrees * M_PI_F / 180.0f;
            ++count;
        } while (result->NextRow());
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell cones", count);
}

void SpellMgr::LoadSpellAreas()
{
    mSpellAreaMap.clear();                                  // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    uint32 count = 0;

    //                                                               0        1       2              3                     4            5             6           7         8
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `spell`, `area`, `quest_start`, `quest_start_active`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast` FROM `spell_area`"));

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell area requirements", count);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId             = spell;
        spellArea.areaId              = fields[1].GetUInt32();
        spellArea.questStart          = fields[2].GetUInt32();
        spellArea.questStartCanActive = fields[3].GetBool();
        spellArea.questEnd            = fields[4].GetUInt32();
        spellArea.auraSpell           = fields[5].GetInt32();
        spellArea.raceMask            = fields[6].GetUInt32();
        spellArea.gender              = Gender(fields[7].GetUInt8());
        spellArea.autocast            = fields[8].GetBool();

        if (!sSpellMgr.GetSpellEntry(spell))
        {
            if (!sSpellMgr.IsExistingSpellId(spell))
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for (SpellAreaMap::const_iterator itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok = false;
                break;
            }

            if (!ok)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` already listed with similar requirements.", spell);
                continue;
            }

        }

        if (spellArea.areaId && !AreaEntry::GetById(spellArea.areaId))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong area (%u) requirement", spell, spellArea.areaId);
            continue;
        }

        if (spellArea.questStart && !sObjectMgr.GetQuestTemplate(spellArea.questStart))
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong start quest (%u) requirement", spell, spellArea.questStart);
            continue;
        }

        if (spellArea.questEnd)
        {
            if (!sObjectMgr.GetQuestTemplate(spellArea.questEnd))
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong end quest (%u) requirement", spell, spellArea.questEnd);
                continue;
            }

            if (spellArea.questEnd == spellArea.questStart && !spellArea.questStartCanActive)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have quest (%u) requirement for start and end in same time", spell, spellArea.questEnd);
                continue;
            }
        }

        if (spellArea.auraSpell)
        {
            SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(abs(spellArea.auraSpell));
            if (!spellInfo)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong aura spell (%u) requirement", spell, abs(spellArea.auraSpell));
                continue;
            }

            bool validSpellEffect = false;
            for (uint8 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
            {
                switch (spellInfo->EffectApplyAuraName[i])
                {
                    case SPELL_AURA_DUMMY:
                    case SPELL_AURA_GHOST:
                        validSpellEffect = true;
                        break;
                }
            }

            if (!validSpellEffect)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have aura spell requirement (%u) without dummy/ghost aura in spell effects", spell, abs(spellArea.auraSpell));
                continue;
            }

            if (uint32(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for (SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell, spellArea.auraSpell);
                    continue;
                }
            }
        }

        if (spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE) == 0)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong race mask (%u) requirement", spell, spellArea.raceMask);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_FEMALE && spellArea.gender != GENDER_MALE)
        {
            sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Spell %u listed in `spell_area` have wrong gender (%u) requirement", spell, spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell, spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if (spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, sa));

        // for search at quest start/reward
        if (spellArea.questStart)
        {
            if (spellArea.questStartCanActive)
                mSpellAreaForActiveQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
            else
                mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, sa));
        }

        // for search at quest start/reward
        if (spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, sa));

        // for search at aura apply
        if (spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell), sa));

        ++count;
    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spell area requirements", count);
}

SpellCastResult SpellMgr::GetSpellAllowedInLocationError(SpellEntry const* spellInfo, Unit const* caster, Player const* player)
{
    // Spell can be casted only in battleground
    if (spellInfo->HasAttribute(SPELL_ATTR_EX3_ONLY_BATTLEGROUNDS) && (!player || !player->InBattleGround()))
        return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    uint32 mapId = caster ? caster->GetMapId() : (player ? player->GetMapId() : 0);

    switch (spellInfo->Id)
    {
        // Alterac Valley
        case 22564:                                         // Recall (Alliance)
        case 22563:                                         // Recall (Horde)
        case 23538:                                         // Battle Standard (Horde)
        case 23539:                                         // Battle Standard (Alliance)
        {
            if (!player)
                return SPELL_FAILED_REQUIRES_AREA;

            BattleGround* bg = player->GetBattleGround();

            return player->GetMapId() == MAP_ALTERAC_VALLEY && bg
                   && bg->GetStatus() != STATUS_WAIT_JOIN ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        }
        // Warsong Gulch
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
            return player && player->GetMapId() == MAP_WARSONG_GULCH && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        case 2584:                                          // Waiting to Resurrect
        {
            return player && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        {
            MapEntry const* mapEntry = sMapStorage.LookupEntry<MapEntry>(mapId);
            if (!mapEntry)
                return SPELL_FAILED_REQUIRES_AREA;
            return mapEntry->IsBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
    }

    if (caster)
    {
        SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellInfo->Id);
        if (saBounds.first != saBounds.second)
        {
            // Performance optimization:
            // Compute zone/area only if necessay. Not at every spell cast.
            uint32 zone, area;
            caster->GetZoneAndAreaId(zone, area);
            for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
                if (itr->second.IsFitToRequirements(player, zone, area))
                    return SPELL_CAST_OK;
            return SPELL_FAILED_REQUIRES_AREA;
        }
    }
    return SPELL_CAST_OK;
}

uint32 SpellMgr::GetRequiredAreaForSpell(uint32 spellId)
{
    SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellId);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second.areaId)
                return itr->second.areaId;
        }
    }

    // Not defined in database.
    switch (spellId)
    {
        // Warsong Gulch
        case 23333: // Warsong Flag
        case 23335: // Silverwing Flag
            return 3277;
    }

    return 0;
}

SpellCastResult SpellMgr::GetSpellAllowedInLocationError(SpellEntry const* spellInfo, uint32 zone_id, uint32 area_id, Player const* player)
{
    // DB base check (if non empty then must fit at least single for allow)
    SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellInfo->Id);
    if (saBounds.first != saBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if (itr->second.IsFitToRequirements(player, zone_id, area_id))
                return SPELL_CAST_OK;
        }
        return SPELL_FAILED_REQUIRES_AREA;
    }
    return GetSpellAllowedInLocationError(spellInfo, nullptr /* zone/area already checked */, player);
}

void SpellMgr::LoadSkillLineAbilityMaps()
{
    mSkillLineAbilityMapBySpellId.clear();
    mSkillLineAbilityMapBySkillId.clear();

    uint32 const rows = sObjectMgr.GetMaxSkillLineAbilityId();
    uint32 count = 0;

    BarGoLink bar(rows);
    for (uint32 row = 0; row < rows; ++row)
    {
        bar.step();
        if (SkillLineAbilityEntry const* entry = sObjectMgr.GetSkillLineAbility(row))
        {
            mSkillLineAbilityMapBySpellId.insert(SkillLineAbilityMap::value_type(entry->spellId, entry));
            mSkillLineAbilityMapBySkillId.insert(SkillLineAbilityMap::value_type(entry->skillId, entry));
            ++count;
        }
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u SkillLineAbility MultiMaps Data", count);
}

void SpellMgr::LoadSkillRaceClassInfoMap()
{
    mSkillRaceClassInfoMap.clear();

    BarGoLink bar(sSkillRaceClassInfoStore.GetNumRows());
    uint32 count = 0;

    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
    {
        bar.step();
        SkillRaceClassInfoEntry const* skillRCInfo = sSkillRaceClassInfoStore.LookupEntry(i);
        if (!skillRCInfo)
            continue;

        // not all skills really listed in ability skills list
        if (!sSkillLineStore.LookupEntry(skillRCInfo->skillId))
            continue;

        mSkillRaceClassInfoMap.insert(SkillRaceClassInfoMap::value_type(skillRCInfo->skillId, skillRCInfo));

        ++count;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u SkillRaceClassInfo MultiMap Data", count);
}

void SpellMgr::CheckUsedSpells(char const* table)
{
    uint32 countSpells = 0;
    uint32 countMasks = 0;

    //                                                                0         1                  2                  3                4             5            6       7
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT `spellid`, `SpellFamilyName`, `SpellFamilyMask`, `SpellCategory`, `EffectType`, `EffectAura`, `EffectIdx`, `Name`, `Code` FROM %s", table));

    if (!result)
    {
        BarGoLink bar(1);

        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "`%s` table is empty!", table);
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        Field* fields = result->Fetch();

        bar.step();

        uint32 spell       = fields[0].GetUInt32();
        int32  family      = fields[1].GetInt32();
        uint64 familyMask  = fields[2].GetUInt64();
        int32  category    = fields[3].GetInt32();
        int32  effectType  = fields[4].GetInt32();
        int32  auraType    = fields[5].GetInt32();
        int32  effectIdx   = fields[6].GetInt32();
        std::string name   = fields[7].GetCppString();
        std::string code   = fields[8].GetCppString();

        // checks of correctness requirements itself

        if (family < -1 || family > SPELLFAMILY_UNK3)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Table '%s' for spell %u have wrong SpellFamily value(%u), skipped.", table, spell, family);
            continue;
        }

        // TODO: for spellCategory better check need dbc loading
        if (category < -1 || (category >= 0 && sSpellCategoriesStore.find(category) == sSpellCategoriesStore.end()))
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Table '%s' for spell %u have wrong SpellCategory value(%u), skipped.", table, spell, category);
            continue;
        }

        if (effectType < -1 || effectType >= TOTAL_SPELL_EFFECTS)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Table '%s' for spell %u have wrong SpellEffect type value(%u), skipped.", table, spell, effectType);
            continue;
        }

        if (auraType < -1 || auraType >= TOTAL_AURAS)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Table '%s' for spell %u have wrong SpellAura type value(%u), skipped.", table, spell, auraType);
            continue;
        }

        if (effectIdx < -1 || effectIdx >= 3)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Table '%s' for spell %u have wrong EffectIdx value(%u), skipped.", table, spell, effectIdx);
            continue;
        }

        // now checks of requirements

        if (spell)
        {
            ++countSpells;

            SpellEntry const* spellEntry = sSpellMgr.GetSpellEntry(spell);
            if (!spellEntry)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' not exist but used in %s.", spell, name.c_str(), code.c_str());
                continue;
            }

            if (family >= 0 && spellEntry->SpellFamilyName != uint32(family))
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' family(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->SpellFamilyName, family, code.c_str());
                continue;
            }

            if (familyMask != uint64(0xFFFFFFFFFFFFFFFF))
            {
                if (familyMask == uint64(0x0000000000000000))
                {
                    if (spellEntry->SpellFamilyFlags)
                    {
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' not fit to (" UI64FMTD ") but used in %s.",
                                      spell, name.c_str(), familyMask, code.c_str());
                        continue;
                    }

                }
                else
                {
                    if (!spellEntry->IsFitToFamilyMask(familyMask))
                    {
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' not fit to (" I64FMT ") but used in %s.", spell, name.c_str(), familyMask, code.c_str());
                        continue;
                    }

                }
            }

            if (category >= 0 && spellEntry->Category != uint32(category))
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' category(%u) <> %u but used in %s.", spell, name.c_str(), spellEntry->Category, category, code.c_str());
                continue;
            }

            if (effectIdx >= EFFECT_INDEX_0)
            {
                if (effectType >= 0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' effect%d <> %u but used in %s.", spell, name.c_str(), effectIdx + 1, effectType, code.c_str());
                    continue;
                }

                if (auraType >= 0 && spellEntry->EffectApplyAuraName[effectIdx] != uint32(auraType))
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' aura%d <> %u but used in %s.", spell, name.c_str(), effectIdx + 1, auraType, code.c_str());
                    continue;
                }

            }
            else
            {
                if (effectType >= 0 && !spellEntry->HasEffect(SpellEffects(effectType)))
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' not have effect %u but used in %s.", spell, name.c_str(), effectType, code.c_str());
                    continue;
                }

                if (auraType >= 0 && !spellEntry->HasAura(AuraType(auraType)))
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spell %u '%s' not have aura %u but used in %s.", spell, name.c_str(), auraType, code.c_str());
                    continue;
                }
            }
        }
        else
        {
            ++countMasks;

            bool found = false;
            for (uint32 spellId = 1; spellId < sSpellMgr.GetMaxSpellId(); ++spellId)
            {
                SpellEntry const* spellEntry = sSpellMgr.GetSpellEntry(spellId);
                if (!spellEntry)
                    continue;

                if (family >= 0 && spellEntry->SpellFamilyName != uint32(family))
                    continue;

                if (familyMask != uint64(0xFFFFFFFFFFFFFFFF))
                {
                    if (familyMask == uint64(0x0000000000000000))
                    {
                        if (spellEntry->SpellFamilyFlags)
                            continue;
                    }
                    else
                    {
                        if (!spellEntry->IsFitToFamilyMask(familyMask))
                            continue;
                    }
                }

                if (category >= 0 && spellEntry->Category != uint32(category))
                    continue;

                if (effectIdx >= 0)
                {
                    if (effectType >= 0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                        continue;

                    if (auraType >= 0 && spellEntry->EffectApplyAuraName[effectIdx] != uint32(auraType))
                        continue;
                }
                else
                {
                    if (effectType >= 0 && !spellEntry->HasEffect(SpellEffects(effectType)))
                        continue;

                    if (auraType >= 0 && !spellEntry->HasAura(AuraType(auraType)))
                        continue;
                }

                found = true;
                break;
            }

            if (!found)
            {
                if (effectIdx >= 0)
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spells '%s' not found for family %i (" I64FMT ") category(%i) effect%d(%i) aura%d(%i) but used in %s",
                                  name.c_str(), family, familyMask, category, effectIdx + 1, effectType, effectIdx + 1, auraType, code.c_str());
                else
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Spells '%s' not found for family %i (" I64FMT ") category(%i) effect(%i) aura(%i) but used in %s",
                                  name.c_str(), family, familyMask, category, effectType, auraType, code.c_str());
                continue;
            }
        }

    }
    while (result->NextRow());

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Checked %u spells and %u spell masks", countSpells, countMasks);
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if (gender != GENDER_NONE)
    {
        // not in expected gender
        if (!player || gender != player->GetGender())
            return false;
    }

    if (raceMask)
    {
        // not in expected race
        if (!player || !(raceMask & player->GetRaceMask()))
            return false;
    }

    if (areaId)
    {
        // not in expected zone
        if (newZone != areaId && newArea != areaId)
            return false;
    }

    if (questStart)
    {
        // not in expected required quest state
        if (!player || ((!questStartCanActive || !player->IsActiveQuest(questStart)) && !player->GetQuestRewardStatus(questStart)))
            return false;
    }

    if (questEnd)
    {
        // not in expected forbidden quest state
        if (!player || player->GetQuestRewardStatus(questEnd))
            return false;
    }

    if (auraSpell)
    {
        // not have expected aura
        if (!player)
            return false;
        if (auraSpell > 0)
            // have expected aura
            return player->HasAura(auraSpell);
        else
            // not have expected aura
            return !player->HasAura(-auraSpell);
    }

    return true;
}

void SpellMgr::LoadExistingSpellIds()
{
    mExistingSpellsSet.clear();

    Field* fields;
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT `ID` FROM `dbc_spellname`"));

    if (result)
    {
        do
        {
            fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            mExistingSpellsSet.insert(id);
        } while (result->NextRow());
    }
}

namespace SpellInternal
{
    bool IsSpellAppliesAura(SpellEntry const* spellInfo)
    {
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (IsEffectAppliesAura(spellInfo->Effect[i]) && spellInfo->EffectApplyAuraName[i])
                return true;
        }
        return false;
    }

    // Spells that apply damage or heal over time
    // Returns false for periodic and direct mixed spells (immolate, etc)
    bool IsSpellAppliesPeriodicAura(SpellEntry const* spellInfo)
    {
        bool periodic = false;
        bool direct = false;
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            switch (spellInfo->Effect[i])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_POWER_DRAIN:
                case SPELL_EFFECT_HEALTH_LEECH:
                case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
                case SPELL_EFFECT_POWER_BURN:
                case SPELL_EFFECT_HEAL:
                    direct = true;
                    break;
                case SPELL_EFFECT_APPLY_AURA:
                    switch (spellInfo->EffectApplyAuraName[i])
                    {
                        case SPELL_AURA_PERIODIC_DAMAGE:
                        case SPELL_AURA_PERIODIC_HEAL:
                        case SPELL_AURA_PERIODIC_ENERGIZE:
                        case SPELL_AURA_OBS_MOD_HEALTH:
                        case SPELL_AURA_PERIODIC_LEECH:
                        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                        case SPELL_AURA_PERIODIC_MANA_LEECH:
                        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                        case SPELL_AURA_POWER_BURN_MANA:
                        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                            periodic = true;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }
        return periodic && !direct;
    }

    bool IsPassiveSpellStackableWithRanks(SpellEntry const* spellInfo)
    {
        if (!spellInfo->IsPassiveSpell())
            return false;

        for (uint32 i : spellInfo->Effect)
        {
            if (SpellEffects(i) == SPELL_EFFECT_APPLY_AURA || SpellEffects(i) == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                return false;
        }
        return true;
    }

    bool IsHealSpell(SpellEntry const* spellInfo)
    {
        // Holy Light/Flash of Light
        if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN)
        {
            if (spellInfo->IsFitToFamilyMask<CF_PALADIN_FLASH_OF_LIGHT2>() ||
                spellInfo->IsFitToFamilyMask<CF_PALADIN_HOLY_LIGHT2>())
                return true;
        }

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            switch (spellInfo->Effect[i])
            {
                case SPELL_EFFECT_HEAL:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                    return true;
                case SPELL_EFFECT_APPLY_AURA:
                case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
                case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
                case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                {
                    switch (spellInfo->EffectApplyAuraName[i])
                    {
                        case SPELL_AURA_PERIODIC_HEAL:
                            return true;
                    }
                    break;
                }
            }
        }

        return false;
    }

    bool IsDirectDamageSpell(SpellEntry const* spellInfo)
    {
        for (uint32 i : spellInfo->Effect)
        {
            if (IsDirectDamageEffect(i))
                return true;
        }

        return false;
    }

    bool IsSpellWithCasterSourceTargetsOnly(SpellEntry const* spellInfo)
    {
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            uint32 targetA = spellInfo->EffectImplicitTargetA[i];
            if (targetA && !IsCasterSourceTarget(targetA))
                return false;

            uint32 targetB = spellInfo->EffectImplicitTargetB[i];
            if (targetB && !IsCasterSourceTarget(targetB))
                return false;

            if (!targetA && !targetB)
                return false;
        }
        return true;
    }

    bool IsAreaOfEffectSpell(SpellEntry const* spellInfo)
    {
        return
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_0])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_1])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetA[EFFECT_INDEX_2])) ||
            IsAreaEffectTarget(SpellTarget(spellInfo->EffectImplicitTargetB[EFFECT_INDEX_2]));
    }

    bool HasAreaAuraEffect(SpellEntry const* spellInfo)
    {
        for (uint32 i : spellInfo->Effect)
            if (IsAreaAuraEffect(i))
                return true;
        return false;
    }

    bool IsDismountSpell(SpellEntry const* spellInfo)
    {
        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA) && (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY) && (spellInfo->EffectMiscValue[i] == MECHANIC_MOUNT))
                return true;
        }
        return false;
    }

    bool IsCharmSpell(SpellEntry const* spellInfo)
    {
        return spellInfo->HasAura(SPELL_AURA_MOD_CHARM) || spellInfo->HasAura(SPELL_AURA_MOD_POSSESS);
    }

    bool IsReflectableSpell(SpellEntry const* spellInfo)
    {
        return spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && !spellInfo->HasAttribute(SPELL_ATTR_IS_ABILITY)
            && !spellInfo->HasAttribute(SPELL_ATTR_EX_NO_REFLECTION) && !spellInfo->HasAttribute(SPELL_ATTR_NO_IMMUNITIES)
            && !spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) && !spellInfo->IsPositiveSpell();
    }

    bool IsSpellWithDelayableEffects(SpellEntry const* spellInfo)
    {
        if (spellInfo->IsCCSpell())
            return true;

        // Flash of Light triggers another spell to do the heal.
        if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->HasTag(SPELL_TAG_FLASH_OF_LIGHT))
            return true;

        // Demonic Sacrifice should be batched, so it's possible to use it together with Voidwalker's Sacrifice.
        // https://www.youtube.com/watch?v=mrEsmmJomsg
        if (spellInfo->Id == 18788)
            return true;

        // Execute should be batched.
        if (spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->IsFitToFamilyMask<CF_WARRIOR_EXECUTE>())
            return true;

        if (spellInfo->IsChanneledSpell() || spellInfo->IsNextMeleeSwingSpell() || spellInfo->IsRangedSpell())
            return false;

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo->Effect[i] && !spellInfo->IsDelayableEffect(i))
                return false;
        }

        return true;
    }

    // These spells should be delayed even if delay is turned off to function properly.
    bool MustDelayEffects(SpellEntry const* spellInfo)
    {
        if (spellInfo->HasEffect(SPELL_EFFECT_SPIRIT_HEAL))
            return true;

        return false;
    }

    bool IsBinary(SpellEntry const* spellInfo)
    {
        bool isBinary = false;

        // Non-magic spells are not affected
        if (spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MAGIC)
            return false;

        // Same for physical spells (charges)
        if (spellInfo->School == SPELL_SCHOOL_NORMAL)
            return false;

        bool foundNoDamageAura = false;
        for (int eff = 0; eff < 3; ++eff)
        {
            // Micro opt - don't iterate anymore if we already have an aura
            if (foundNoDamageAura)
                break;

            switch (spellInfo->Effect[eff])
            {
                case SPELL_EFFECT_INTERRUPT_CAST:
                    foundNoDamageAura = true;
                    break;
                case SPELL_EFFECT_APPLY_AURA:
                    switch (spellInfo->EffectApplyAuraName[eff])
                    {
                        case SPELL_AURA_MOD_DECREASE_SPEED:
                        case SPELL_AURA_MOD_FEAR:
                        case SPELL_AURA_MOD_STUN:
                        case SPELL_AURA_MOD_PACIFY:
                        case SPELL_AURA_MOD_ROOT:
                        case SPELL_AURA_MOD_SILENCE:
                        case SPELL_AURA_MOD_DISARM:
                        case SPELL_AURA_MOD_RESISTANCE:
                        case SPELL_AURA_MOD_DAMAGE_TAKEN:
                            foundNoDamageAura = true;
                            break;
                    }
                    break;
                case SPELL_EFFECT_KNOCK_BACK:
                    foundNoDamageAura = true;
                    break;
            }
        }
        isBinary = foundNoDamageAura;

        if (spellInfo->Id == 26143)    // SPELL_MIND_FLAY (C'Thuns Eye Tentacles)
            isBinary = true;
        else if (spellInfo->Id == 26478)
            isBinary = true;           // SPELL_GROUND_RUPTURE_NATURE (C'thuns Giant tentacles ground rupture)

        return isBinary;
    }

    bool IsNonPeriodicDispel(SpellEntry const* spellInfo)
    {
        if (spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
        {
            for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                if (spellInfo->Effect[i] != 0 && (spellInfo->Effect[i] != SPELL_EFFECT_DISPEL || spellInfo->EffectRadiusIndex[i] != 0))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool IsPvEHeartBeat(SpellEntry const* spellInfo)
    {
        if (!spellInfo->HasAttribute(SPELL_ATTR_HEARTBEAT_RESIST))
            return false;

        for (uint32 i : spellInfo->EffectApplyAuraName)
        {
            switch (i)
            {
                case SPELL_AURA_MOD_FEAR:
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                case SPELL_AURA_MOD_CONFUSE:
                    return false;
            }
        }

        return true;
    }

    bool IsCCSpell(SpellEntry const* spellInfo)
    {
        if (spellInfo->IsChanneledSpell())
            return false;
        if (spellInfo->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
            return false;

        switch (spellInfo->GetDiminishingReturnsGroup(false))
        {
            case DIMINISHING_NONE:
            case DIMINISHING_LIMITONLY:
                return false;
        }
        return true;
    }

    uint32 GetAllowedTargetMask(SpellEntry const* spellInfo)
    {
        uint32 targetMask = spellInfo->Targets;

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo->Effect[i])
            {
                targetMask |= GetAllowedTargetMaskForTargetType((SpellTarget)spellInfo->EffectImplicitTargetA[i]);
                targetMask |= GetAllowedTargetMaskForTargetType((SpellTarget)spellInfo->EffectImplicitTargetB[i]);
            }
        }

        if (targetMask & (TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY | TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT_ALLY | TARGET_FLAG_UNIT_DEAD | TARGET_FLAG_UNIT_MINIPET))
            targetMask |= TARGET_FLAG_UNIT;

        if (targetMask & (TARGET_FLAG_LOCKED))
            targetMask |= TARGET_FLAG_ITEM | TARGET_FLAG_GAMEOBJECT;

        if (targetMask & (TARGET_FLAG_ITEM))
            targetMask |= TARGET_FLAG_TRADE_ITEM;

        // unreleased player corpse is still a unit
        if (targetMask & (TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_CORPSE_ALLY))
            targetMask |= TARGET_FLAG_UNIT;

        return targetMask;
    }
}

void SpellMgr::AssignInternalSpellFlags()
{
    for (auto& pSpellEntry : mSpellEntryMap)
    {
        if (pSpellEntry)
        {
            if (SpellInternal::IsSpellAppliesAura(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_APPLIES_AURA;

            if (SpellInternal::IsSpellAppliesPeriodicAura(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_APPLIES_PERIODIC_AURA;

            if (SpellInternal::IsPassiveSpellStackableWithRanks(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_PASSIVE_STACK_WITH_RANKS;

            if (pSpellEntry->IsPositiveSpell(nullptr, nullptr))
                pSpellEntry->Internal |= SPELL_INTERNAL_POSITIVE;

            if (SpellInternal::IsHealSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_HEAL;

            if (SpellInternal::IsDirectDamageSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_DIRECT_DAMAGE;

            if (SpellInternal::IsSpellWithCasterSourceTargetsOnly(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_CASTER_SOURCE_TARGETS;

            if (SpellInternal::IsAreaOfEffectSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_AOE;

            if (SpellInternal::HasAreaAuraEffect(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_AOE_AURA;

            if (SpellInternal::IsDismountSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_DISMOUNT;

            if (SpellInternal::IsCharmSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_CHARM;

            if (SpellInternal::IsReflectableSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_REFLECTABLE;

            if (SpellInternal::IsSpellWithDelayableEffects(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_DELAYABLE_EFFECTS;

            if (SpellInternal::IsBinary(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_BINARY;

            if (SpellInternal::IsNonPeriodicDispel(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_NON_PERIODIC_DISPEL;

            if (SpellInternal::IsPvEHeartBeat(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_PVE_HEARTBEAT;

            if (SpellInternal::IsCCSpell(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_CROWD_CONTROL;

            if (SpellInternal::MustDelayEffects(pSpellEntry.get()))
                pSpellEntry->Internal |= SPELL_INTERNAL_MUST_DELAY_EFFECTS;

            pSpellEntry->AllowedTargetMask = SpellInternal::GetAllowedTargetMask(pSpellEntry.get());
        }
    }
}


namespace
{
    void ClampTo(uint32& value, uint32 max, uint32& clamped)
    {
        if (value > max)
        {
            value = 0;
            ++clamped;
        }
    }
}

void SpellMgr::LoadSpells()
{
    uint32 oldMSTime = WorldTimer::getMSTime();

    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT MAX(`ID`) FROM `dbc_spellname`"));
    if (!result)
    {
        BarGoLink bar(1);
        bar.step();

        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded 0 spells. DB table `dbc_spellname` is empty.");
        return;
    }

    uint32 const maxEntry = (*result)[0].GetUInt32() + 1;
    if (maxEntry > mSpellEntryMap.size())
        mSpellEntryMap.resize(maxEntry);

    BarGoLink bar(1);
    bar.step();

    // Rows of the other spell tables that belong to no spell.
    std::map<std::string, uint32> ignored;
    auto forEachRow = [&](char const* table, char const* query, auto&& fill)
    {
        std::unique_ptr<QueryResult> rows = WorldDatabase.Query(query);
        if (!rows)
            return;
        do
        {
            Field* f = rows->Fetch();
            uint32 const id = f[0].GetUInt32();
            SpellEntry* spell = id < mSpellEntryMap.size() ? mSpellEntryMap[id].get() : nullptr;
            if (!spell)
            {
                ++ignored[table];
                continue;
            }
            fill(*spell, f);
        } while (rows->NextRow());
    };

    result = WorldDatabase.Query("SELECT `ID`, `Name_lang` FROM `dbc_spellname`");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            uint32 const id = f[0].GetUInt32();
            std::unique_ptr<SpellEntry>& spell = mSpellEntryMap[id];
            if (!spell)
                spell = std::make_unique<SpellEntry>();
            else
            {
                // Reload: same address, values from scratch, values other loaders own kept.
                SpellEntry fresh;
                fresh.Internal = spell->Internal;
                fresh.AllowedTargetMask = spell->AllowedTargetMask;
                fresh.Tags = spell->Tags;
                fresh.StackKey = spell->StackKey;
                fresh.RankKey = spell->RankKey;
                fresh.SingleTargetKey = spell->SingleTargetKey;
                fresh.ClientVisualId = spell->ClientVisualId;
                fresh.HasClientActiveIcon = spell->HasClientActiveIcon;
                *spell = fresh;
            }
            spell->Id = id;
            spell->procChance = 101;
            spell->SpellName = f[1].GetCppString();
        } while (result->NextRow());
    }

    forEachRow("dbc_spellmisc", "SELECT `SpellID`, `Attributes1`, `Attributes2`, `Attributes3`, `Attributes4`, `Attributes5`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `SchoolMask`, `Speed`, `Attributes9` FROM `dbc_spellmisc` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.Attributes = f[1].GetUInt32();
        s.AttributesEx = f[2].GetUInt32();
        s.AttributesEx2 = f[3].GetUInt32();
        s.AttributesEx3 = f[4].GetUInt32();
        s.AttributesEx4 = f[5].GetUInt32();
        s.CastingTimeIndex = f[6].GetUInt32();
        s.DurationIndex = f[7].GetUInt32();
        s.rangeIndex = f[8].GetUInt32();
        uint32 const mask = f[9].GetUInt32();
        s.School = 0;
        for (uint32 school = 0; mask && school < 32; ++school)
        {
            if (mask & (1u << school))
            {
                s.School = school;
                break;
            }
        }
        s.speed = f[10].GetFloat();
        s.AuraPointsOnClient = f[11].GetUInt32() & 0x1000;
    });
    forEachRow("dbc_spell", "SELECT `ID`, `NameSubtext_lang` FROM `dbc_spell`", [](SpellEntry& s, Field* f)
    {
        s.Rank = f[1].GetCppString();
    });
    forEachRow("dbc_spellcategories", "SELECT `SpellID`, `Category`, `DispelType`, `Mechanic`, `DefenseType`, `PreventionType`, `StartRecoveryCategory` FROM `dbc_spellcategories` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.Category = f[1].GetUInt32();
        s.Dispel = f[2].GetUInt32();
        s.Mechanic = f[3].GetUInt32();
        s.DmgClass = f[4].GetUInt32();
        s.PreventionType = f[5].GetUInt32();
        s.StartRecoveryCategory = f[6].GetUInt32();
    });
    forEachRow("dbc_spellclassoptions", "SELECT `SpellID`, `SpellClassSet`, `SpellClassMask1`, `SpellClassMask2` FROM `dbc_spellclassoptions`",
        [](SpellEntry& s, Field* f)
    {
        s.SpellFamilyName = f[1].GetUInt32();
        s.SpellFamilyFlags = uint64(f[2].GetUInt32()) | (uint64(f[3].GetUInt32()) << 32);
    });
    forEachRow("dbc_spellcooldowns", "SELECT `SpellID`, `RecoveryTime`, `CategoryRecoveryTime`, `StartRecoveryTime` FROM `dbc_spellcooldowns` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.RecoveryTime = f[1].GetUInt32();
        s.CategoryRecoveryTime = f[2].GetUInt32();
        s.StartRecoveryTime = f[3].GetUInt32();
    });
    forEachRow("dbc_spellinterrupts", "SELECT `SpellID`, `InterruptFlags`, `AuraInterruptFlags1`, `ChannelInterruptFlags1` FROM `dbc_spellinterrupts` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.InterruptFlags = f[1].GetUInt32();
        s.AuraInterruptFlags = f[2].GetUInt32();
        s.ChannelInterruptFlags = f[3].GetUInt32();
    });
    forEachRow("dbc_spelllevels", "SELECT `SpellID`, `BaseLevel`, `MaxLevel`, `SpellLevel` FROM `dbc_spelllevels` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.baseLevel = f[1].GetUInt32();
        s.maxLevel = f[2].GetUInt32();
        s.spellLevel = f[3].GetUInt32();
    });
    std::map<uint32, float> procRates;
    result = WorldDatabase.Query("SELECT `ID`, `BaseProcRate` FROM `dbc_spellprocsperminute`");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            procRates[f[0].GetUInt32()] = f[1].GetFloat();
        } while (result->NextRow());
    }
    forEachRow("dbc_spellauraoptions", "SELECT `SpellID`, `CumulativeAura`, `ProcChance`, `ProcCharges`, `ProcTypeMask1`, `ProcCategoryRecovery`, `SpellProcsPerMinuteID` FROM `dbc_spellauraoptions` WHERE `DifficultyID` = 0",
        [&procRates](SpellEntry& s, Field* f)
    {
        s.StackAmount = f[1].GetUInt32();
        s.procChance = f[2].GetUInt32();
        s.procCharges = f[3].GetUInt32();
        s.procFlags = f[4].GetUInt32();
        s.ProcCooldown = f[5].GetUInt32();
        auto itr = procRates.find(f[6].GetUInt32());
        s.ProcsPerMinute = itr != procRates.end() ? itr->second : 0.0f;
    });
    forEachRow("dbc_spellaurarestrictions", "SELECT `SpellID`, `CasterAuraState`, `TargetAuraState` FROM `dbc_spellaurarestrictions` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.CasterAuraState = f[1].GetUInt32();
        s.TargetAuraState = f[2].GetUInt32();
    });
    forEachRow("dbc_spellcastingrequirements", "SELECT `SpellID`, `RequiresSpellFocus` FROM `dbc_spellcastingrequirements`",
        [](SpellEntry& s, Field* f)
    {
        s.RequiresSpellFocus = f[1].GetUInt32();
    });
    forEachRow("dbc_spellequippeditems", "SELECT `SpellID`, `EquippedItemClass`, `EquippedItemSubclass`, `EquippedItemInvTypes` FROM `dbc_spellequippeditems`",
        [](SpellEntry& s, Field* f)
    {
        s.EquippedItemClass = f[1].GetInt32();
        s.EquippedItemSubClassMask = f[2].GetInt32();
        s.EquippedItemInventoryTypeMask = f[3].GetInt32();
    });
    forEachRow("dbc_spellpower", "SELECT `SpellID`, `PowerType`, `ManaCost`, `ManaCostPerLevel`, `ManaPerSecond`, `PowerCostPct` FROM `dbc_spellpower` WHERE `OrderIndex` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.powerType = f[1].GetUInt32();
        s.manaCost = f[2].GetUInt32();
        s.manaCostPerlevel = f[3].GetUInt32();
        s.manaPerSecond = f[4].GetUInt32();
        s.ManaCostPercentage = uint32(std::lround(f[5].GetFloat()));
    });
    forEachRow("dbc_spellreagents", "SELECT `SpellID`, `Reagent1`, `Reagent2`, `Reagent3`, `Reagent4`, `Reagent5`, `Reagent6`, `Reagent7`, `Reagent8`, `ReagentCount1`, `ReagentCount2`, `ReagentCount3`, `ReagentCount4`, `ReagentCount5`, `ReagentCount6`, `ReagentCount7`, `ReagentCount8` FROM `dbc_spellreagents`",
        [](SpellEntry& s, Field* f)
    {
        for (uint32 i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            s.Reagent[i] = f[1 + i].GetInt32();
            s.ReagentCount[i] = f[9 + i].GetUInt32();
        }
    });
    forEachRow("dbc_spellshapeshift", "SELECT `SpellID`, `ShapeshiftMask1`, `ShapeshiftExclude1` FROM `dbc_spellshapeshift`",
        [](SpellEntry& s, Field* f)
    {
        s.Stances = f[1].GetUInt32();
        s.StancesNot = f[2].GetUInt32();
    });
    forEachRow("dbc_spelltargetrestrictions", "SELECT `SpellID`, `Targets`, `TargetCreatureType`, `MaxTargets`, `MaxTargetLevel` FROM `dbc_spelltargetrestrictions` WHERE `DifficultyID` = 0",
        [](SpellEntry& s, Field* f)
    {
        s.Targets = f[1].GetUInt32();
        s.TargetCreatureType = f[2].GetUInt32();
        s.MaxAffectedTargets = f[3].GetUInt32();
        s.MaxTargetLevel = f[4].GetUInt32();
    });
    forEachRow("dbc_spelltotems", "SELECT `SpellID`, `Totem1`, `Totem2` FROM `dbc_spelltotems`", [](SpellEntry& s, Field* f)
    {
        s.Totem[0] = f[1].GetUInt32();
        s.Totem[1] = f[2].GetUInt32();
    });
    forEachRow("dbc_spelleffect", "SELECT `SpellID`, `EffectIndex`, `Effect`, `EffectAura`, `EffectBasePoints`, `EffectDieSides`, `EffectRealPointsPerLevel`, `EffectPointsPerResource`, `EffectAuraPeriod`, `EffectAmplitude`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectMechanic`, `EffectMiscValue1`, `EffectMiscValue2`, `ImplicitTarget1`, `ImplicitTarget2`, `EffectTriggerSpell`, `EffectBonusCoefficient`, `EffectRadiusIndex1`, `EffectRadiusIndex2`, `EffectItemType`, `EffectSpellClassMask1`, `EffectSpellClassMask2` FROM `dbc_spelleffect` WHERE `DifficultyID` = 0 AND `EffectIndex` < 3 AND `Effect` <> 0",
        [](SpellEntry& s, Field* f)
    {
        uint32 const i = f[1].GetUInt32();
        s.Effect[i] = f[2].GetUInt32();
        s.EffectApplyAuraName[i] = f[3].GetUInt32();
        s.EffectBasePoints[i] = f[4].GetInt32();
        s.EffectDieSides[i] = f[5].GetInt32();
        s.EffectRealPointsPerLevel[i] = f[6].GetFloat();
        s.EffectPointsPerComboPoint[i] = f[7].GetFloat();
        s.EffectAmplitude[i] = f[8].GetUInt32();
        s.EffectMultipleValue[i] = f[9].GetFloat();
        s.DmgMultiplier[i] = f[10].GetFloat();
        s.EffectChainTarget[i] = f[11].GetUInt32();
        s.EffectMechanic[i] = f[12].GetUInt32();
        s.EffectMiscValue[i] = f[13].GetInt32();
        s.EffectMiscValueB[i] = f[14].GetInt32();
        s.EffectImplicitTargetA[i] = f[15].GetUInt32();
        s.EffectImplicitTargetB[i] = f[16].GetUInt32();
        s.EffectTriggerSpell[i] = f[17].GetUInt32();
        s.EffectBonusCoefficient[i] = f[18].GetFloat();
        uint32 const radius2 = f[20].GetUInt32();
        s.EffectRadiusIndex[i] = radius2 ? radius2 : f[19].GetUInt32();
        switch (s.EffectApplyAuraName[i])
        {
            case 107:
            case 108:
            case 109:
            case 112:
                s.EffectItemType[i] = uint64(f[22].GetUInt32()) | (uint64(f[23].GetUInt32()) << 32);
                break;
            default:
                s.EffectItemType[i] = f[21].GetUInt32();
                break;
        }
    });

    for (std::unique_ptr<SpellEntry> const& spell : mSpellEntryMap)
    {
        if (!spell)
            continue;
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            spell->EffectBaseDice[i] = spell->EffectDieSides[i] ? 1 : 0;
    }

    forEachRow("spell_extra", "SELECT `spell_id`, `custom_flags`, `script_name`, `min_target_level`, `effect_base_dice1`, `effect_base_dice2`, `effect_base_dice3`, `effect_dice_per_level1`, `effect_dice_per_level2`, `effect_dice_per_level3`, `default_coefficient1`, `default_coefficient2`, `default_coefficient3` FROM `spell_extra`",
        [](SpellEntry& s, Field* f)
    {
        s.Custom = f[1].GetUInt32();
        s.ScriptId = sScriptMgr.GetScriptId(f[2].GetString());
        s.MinTargetLevel = f[3].GetUInt32();
        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (!f[4 + i].IsNULL())
                s.EffectBaseDice[i] = f[4 + i].GetUInt32();
            s.EffectDicePerLevel[i] = f[7 + i].GetFloat();
            if (f[10 + i].GetUInt32() == 1)
                s.EffectBonusCoefficient[i] = -1.0f;
        }
    });

    std::map<std::string, uint32> clamped;
    for (std::unique_ptr<SpellEntry> const& spell : mSpellEntryMap)
    {
        if (!spell)
            continue;

        for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            ClampTo(spell->Effect[i], TOTAL_SPELL_EFFECTS - 1, clamped["Effect"]);
            ClampTo(spell->EffectApplyAuraName[i], TOTAL_AURAS - 1, clamped["EffectApplyAuraName"]);
            if (spell->Effect[i] == SPELL_EFFECT_SUMMON)
                spell->EffectSummonKind[i] = GetSummonKind(uint32(spell->EffectMiscValueB[i]), sSummonPropertiesStore.LookupEntry(uint32(spell->EffectMiscValueB[i])));
            ClampTo(spell->EffectImplicitTargetA[i], TARGET_LOCATION_UNIT_POSITION, clamped["EffectImplicitTargetA"]);
            ClampTo(spell->EffectImplicitTargetB[i], TARGET_LOCATION_UNIT_POSITION, clamped["EffectImplicitTargetB"]);
            ClampTo(spell->EffectMechanic[i], MECHANIC_SLOW_CAST_SPEED, clamped["EffectMechanic"]);
        }
        ClampTo(spell->Mechanic, MECHANIC_SLOW_CAST_SPEED, clamped["Mechanic"]);
        ClampTo(spell->Dispel, DISPEL_ZG_TICKET, clamped["Dispel"]);
        ClampTo(spell->School, MAX_SPELL_SCHOOL - 1, clamped["School"]);
        if (spell->powerType >= MAX_POWERS && spell->powerType != POWER_HEALTH)
        {
            spell->powerType = 0;
            ++clamped["powerType"];
        }

        // It seems that in vanilla when the Amplitude of a
        // periodic aura was 0, it defaulted to a 5 seconds timer.
        // Check out Violet Tragan (Item 8526, Spell 6727).
        // There are comments from vanilla saying it does
        // damage every 5 seconds, while TBC comments say
        // it does no damage, so behavior changed then.
        // In classic the spell was edited in 1.13.6 and
        // the timer was set to 5000, as it didn't work before.
        for (int i = EFFECT_INDEX_0; i <= EFFECT_INDEX_2; ++i)
        {
            switch (spell->EffectApplyAuraName[i])
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                case SPELL_AURA_PERIODIC_MANA_FUNNEL:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_POWER_BURN_MANA:
                    if (spell->EffectAmplitude[i] == 0)
                        spell->EffectAmplitude[i] = 5000;
                    break;
            }
        }

        // Attribute replaced with aura state in patch 1.8.
        if (spell->HasAttribute(SPELL_ATTR_EX2_ENABLE_AFTER_PARRY))
            spell->CasterAuraState = spell->SpellFamilyName == SPELLFAMILY_HUNTER ? AURA_STATE_HUNTER_PARRY : AURA_STATE_DEFENSE;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Loaded %u spells in %ums.", uint32(mSpellEntryMap.size()), WorldTimer::getMSTimeDiffToNow(oldMSTime));
    for (auto const& [table, count] : ignored)
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Ignored %u rows of `%s` that belong to no spell.", count, table.c_str());
    for (auto const& [field, count] : clamped)
    {
        if (count)
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Set %u out-of-range `%s` values to 0.", count, field.c_str());
    }
}
