/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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

#include "DBCStores.h"
#include "Log.h"
#include "ProgressBar.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "DBCfmt.h"
#include "SpellMgr.h"
#include "ObjectMgr.h"

#include <map>
#include <vector>
#include <regex>

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) : groupId(g), rootId(r), adtId(a)
    {
    }

    bool operator <(WMOAreaTableTripple const& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

DBCStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);
DBCStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CinematicSequencesEntry> sCinematicSequencesStore(CinematicSequencesEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(CreatureDisplayInfoExtrafmt);
DBCStorage <CreatureModelDataEntry> sCreatureModelDataStore(CreatureModelDatafmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DBCStorage <CreatureTypeEntry> sCreatureTypeStore(CreatureTypefmt);
DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);
DBCStorage <EmotesEntry> sEmotesStore(EmotesEntryfmt);
DBCStorage <EmotesTextEntry> sEmotesTextStore(EmotesTextEntryfmt);
DBCStorage <GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(GameObjectDisplayInfofmt);
DBCStorage <ItemBagFamilyEntry>           sItemBagFamilyStore(ItemBagFamilyfmt);
//DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
DBCStorage <ItemRandomPropertiesEntry> sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
DBCStorage <LockEntry> sLockStore(LockEntryfmt);
DBCStorage<NamesProfanityEntry> sNamesProfanityStore(NamesProfanityEntryfmt);
DBCStorage<NamesReservedEntry> sNamesReservedStore(NamesReservedEntryfmt);
static std::wstring const emacsStartOfLineToken = L"\\<"; // equivalent to ^
static std::wstring const emacsEndOfLineToken = L"\\>";   // equivalent to $
typedef std::vector<std::wregex> NameValidationRegexContainer;
NameValidationRegexContainer NamesProfaneValidators;
NameValidationRegexContainer NamesReservedValidators;
DBCStorage <QuestSortEntry> sQuestSortStore(QuestSortEntryfmt);
DBCStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DBCStorage <SkillRaceClassInfoEntry> sSkillRaceClassInfoStore(SkillRaceClassInfofmt);
SkillRaceClassInfoMap SkillRaceClassInfoBySkill;
DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DBCStorage <SpellCategoryEntry> sSpellCategoryStore(SpellCategoryfmt);
SpellCategoriesStore sSpellCategoriesStore;
PetFamilySpellsStore sPetFamilySpellsStore;
DBCStorage <SpellCastTimesEntry> sSpellCastTimesStore(SpellCastTimefmt);
DBCStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
DBCStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DBCStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DBCStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DBCStorage <SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(SpellShapeshiftfmt);
DBCStorage <StableSlotPricesEntry> sStableSlotPricesStore(StableSlotPricesfmt);
DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellPosMap sTalentSpellPosMap;
DBCStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);

TaxiMask sTaxiNodesMask;
// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
DBCStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC store data but sTaxiPathNodesByPath used for fast access to entries (it's not owner pointed data).
TaxiPathNodesByPath sTaxiPathNodesByPath;
DBCStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);

DBCStorage <TransportAnimationEntry> sTransportAnimationStore(TransportAnimationfmt);

DBCStorage <WMOAreaTableEntry>  sWMOAreaTableStore(WMOAreaTableEntryfmt);
std::map<WMOAreaTableTripple, WMOAreaTableEntry const*> sWMOAreaInfoByTripple;
//DBCStorage <WorldMapOverlayEntry> sWorldMapOverlayStore(WorldMapOverlayEntryfmt);

typedef std::vector<std::string> StoreProblemList;

bool IsAcceptableClientBuild(uint32 build)
{
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
        if (int(build) == accepted_versions[i])
            return true;

    return false;
}

std::string AcceptableClientBuildsListStr()
{
    std::ostringstream data;
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
        data << accepted_versions[i] << " ";
    return data.str();
}

static bool LoadDBC_assert_print(uint32 fsize, uint32 rsize, std::string const& filename)
{
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

template<class T>
inline void LoadDBC(uint32& availableDbcLocales, BarGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, std::string const& dbc_path, std::string const& filename)
{
    // compatibility format and C++ structure sizes
    MANGOS_ASSERT(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    std::string dbc_filename = dbc_path + filename;
    if (storage.Load(dbc_filename.c_str()))
    {
        bar.step();
        for (uint8 i = 0; fullLocaleNameList[i].name; ++i)
        {
            if (!(availableDbcLocales & (1 << i)))
                continue;

            std::string dbc_filename_loc = dbc_path + fullLocaleNameList[i].name + "/" + filename;
            if (!storage.LoadStringsFrom(dbc_filename_loc.c_str()))
                availableDbcLocales &= ~(1 << i);           // mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) nonexistent
        FILE* f = fopen(dbc_filename.c_str(), "rb");
        if (f)
        {
            char buf[100];
            snprintf(buf, 100, " (exist, but have %d fields instead " SIZEFMTD ") Wrong client version DBC file?", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errlist.push_back(dbc_filename + buf);
            fclose(f);
        }
        else
            errlist.push_back(dbc_filename);
    }
}

template<class T, class Fill>
static void LoadDBCFromWorld(BarGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, char const* table, char const* columns, Fill fill)
{
    std::vector<std::pair<uint32, T>> rows;
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT %s FROM `%s`", columns, table));
    if (result)
    {
        do
        {
            T entry{};
            uint32 const id = fill(result->Fetch(), entry);
            rows.emplace_back(id, entry);
        }
        while (result->NextRow());
    }

    if (storage.LoadRows(rows))
        bar.step();
    else
        errlist.push_back(table);
}

// The world database holds the enUS text only; other locales stay empty.
template<class T>
static void FillLocalizedString(DBCStorage<T>& storage, char const** out, std::string const& enUS)
{
    out[0] = storage.InternString(enUS);
    for (uint32 i = 1; i < 8; ++i)
        out[i] = "";
}

void LoadDBCStores(std::string const& dataPath)
{
    std::string dbcPath = dataPath + "dbc/";

    uint32 const DBCFilesCount = 34;

    BarGoLink bar(DBCFilesCount);

    StoreProblemList badDbcFiles;

    // bitmask for index of fullLocaleNameList
    uint32 availableDbcLocales = 0xFFFFFFFF;

    LoadDBCFromWorld(bar, badDbcFiles, sAuctionHouseStore, "dbc_auctionhouse", "`ID`, `FactionID`, `DepositRate`, `ConsignmentRate`", [](Field* f, AuctionHouseEntry& e)
    {
        e.houseId = f[0].GetUInt32();
        e.faction = f[1].GetUInt32();
        e.depositPercent = f[2].GetUInt32();
        e.cutPercent = f[3].GetUInt32();
        return e.houseId;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sBankBagSlotPricesStore, "dbc_bankbagslotprices", "`ID`, `Cost`", [](Field* f, BankBagSlotPricesEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.price = f[1].GetUInt32();
        return e.ID;
    });

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sChatChannelsStore,        dbcPath, "ChatChannels.dbc");

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sChrClassesStore,          dbcPath, "ChrClasses.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sChrRacesStore,            dbcPath, "ChrRaces.dbc");
    LoadDBCFromWorld(bar, badDbcFiles, sCinematicSequencesStore, "dbc_cinematicsequences", "`ID`", [](Field* f, CinematicSequencesEntry& e)
    {
        e.Id = f[0].GetUInt32();
        return e.Id;
    });
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sCreatureDisplayInfoStore, dbcPath, "CreatureDisplayInfo.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sCreatureDisplayInfoExtraStore, dbcPath, "CreatureDisplayInfoExtra.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sCreatureModelDataStore,   dbcPath, "CreatureModelData.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sCreatureFamilyStore,      dbcPath, "CreatureFamily.dbc");
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureTypeStore, "dbc_creaturetype", "`ID`", [](Field* f, CreatureTypeEntry& e)
    {
        e.ID = f[0].GetUInt32();
        return e.ID;
    });
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sDurabilityCostsStore,     dbcPath, "DurabilityCosts.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sDurabilityQualityStore,   dbcPath, "DurabilityQuality.dbc");
    LoadDBCFromWorld(bar, badDbcFiles, sEmotesStore, "dbc_emotes", "`ID`, `EmoteSlashCommand`, `EmoteFlags`, `EmoteSpecProc`, `EmoteSpecProcParam`", [&](Field* f, EmotesEntry& e)
    {
        e.Id = f[0].GetUInt32();
        e.Name = const_cast<char*>(sEmotesStore.InternString(f[1].GetCppString()));
        e.Flags = f[2].GetUInt32();
        e.EmoteType = f[3].GetUInt32();
        e.UnitStandState = f[4].GetUInt32();
        return e.Id;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sEmotesTextStore, "dbc_emotestext", "`ID`, `EmoteID`", [&](Field* f, EmotesTextEntry& e)
    {
        e.Id = f[0].GetUInt32();
        e.textid = f[1].GetUInt32();
        return e.Id;
    });
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sGameObjectDisplayInfoStore, dbcPath, "GameObjectDisplayInfo.dbc");
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_8_4
    LoadDBCFromWorld(bar, badDbcFiles, sItemBagFamilyStore, "dbc_itembagfamily", "`ID`", [](Field* f, ItemBagFamilyEntry& e)
    {
        e.ID = f[0].GetUInt32();
        return e.ID;
    });
#endif
    //LoadDBC(availableDbcLocales,bar,badDbcFiles,sItemDisplayInfoStore,     dbcPath,"ItemDisplayInfo.dbc");     -- not used currently
    LoadDBCFromWorld(bar, badDbcFiles, sItemRandomPropertiesStore, "dbc_itemrandomproperties", "`ID`, `Name_lang`, `Enchantment1`, `Enchantment2`, `Enchantment3`", [&](Field* f, ItemRandomPropertiesEntry& e)
    {
        e.ID = f[0].GetUInt32();
        std::string const name = f[1].GetCppString();
        e.internalName = const_cast<char*>(sItemRandomPropertiesStore.InternString(name));
        FillLocalizedString(sItemRandomPropertiesStore, const_cast<char const**>(e.nameSuffix), name);
        for (uint32 i = 0; i < 3; ++i)
            e.enchant_id[i] = f[2 + i].GetUInt32();
        return e.ID;
    });
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sItemSetStore,             dbcPath, "ItemSet.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sLockStore,                dbcPath, "Lock.dbc");

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sNamesProfanityStore,      dbcPath, "NamesProfanity.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sNamesReservedStore,       dbcPath, "NamesReserved.dbc");
    for (uint32 i = 0; i < sNamesProfanityStore.GetNumRows(); ++i)
    {
        NamesProfanityEntry const* namesProfanity = sNamesProfanityStore.LookupEntry(i);
        if (!namesProfanity)
            continue;

        std::wstring wname;
        ASSERT(Utf8toWStr(namesProfanity->Name, wname));

        // the dbc uses emacs regex syntax
        auto index = wname.find(emacsStartOfLineToken, 0);
        if (index != std::wstring::npos)
            wname.replace(index, emacsStartOfLineToken.length(), L"^");

        index = wname.find(emacsEndOfLineToken, 0);
        if (index != std::wstring::npos)
            wname.replace(index, emacsEndOfLineToken.length(), L"$");

        NamesProfaneValidators.emplace_back(wname, std::regex::icase | std::regex::optimize);
    }
    for (uint32 i = 0; i < sNamesReservedStore.GetNumRows(); ++i)
    {
        NamesReservedEntry const* namesReserved = sNamesReservedStore.LookupEntry(i);
        if (!namesReserved)
            continue;

        std::wstring wname;
        ASSERT(Utf8toWStr(namesReserved->Name, wname));

        // the dbc uses emacs regex syntax
        auto index = wname.find(emacsStartOfLineToken, 0);
        if (index != std::wstring::npos)
            wname.replace(index, emacsStartOfLineToken.length(), L"^");

        index = wname.find(emacsEndOfLineToken, 0);
        if (index != std::wstring::npos)
            wname.replace(index, emacsEndOfLineToken.length(), L"$");

        NamesReservedValidators.emplace_back(wname, std::regex::icase | std::regex::optimize);
    }

    LoadDBCFromWorld(bar, badDbcFiles, sQuestSortStore, "dbc_questsort", "`ID`", [](Field* f, QuestSortEntry& e)
    {
        e.id = f[0].GetUInt32();
        return e.id;
    });
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSkillLineStore,           dbcPath, "SkillLine.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSkillRaceClassInfoStore,  dbcPath, "SkillRaceClassInfo.dbc");
    for (uint32 i = 1; i < sSpellMgr.GetMaxSpellId(); ++i)
    {
        SpellEntry const* spell = sSpellMgr.GetSpellEntry(i);
        if (spell && spell->Category)
            sSpellCategoriesStore[spell->Category].insert(i);
    }

    for (uint32 j = 0; j < sObjectMgr.GetMaxSkillLineAbilityId(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sObjectMgr.GetSkillLineAbility(j);

        if (!skillLine)
            continue;
        if (skillLine->learnOnGetSkill != ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL)
            continue;

        SpellEntry const* spellInfo = sSpellMgr.GetSpellEntry(skillLine->spellId);
        if (!spellInfo || !(spellInfo->Attributes & SPELL_ATTR_PASSIVE))
            continue;

        for (uint32 i = 1; i < sCreatureFamilyStore.GetNumRows(); ++i)
        {
            CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(i);
            if (!cFamily)
                continue;

            if (skillLine->skillId != cFamily->skillLine[0] && skillLine->skillId != cFamily->skillLine[1])
                continue;

            sPetFamilySpellsStore[i].insert(spellInfo->Id);
        }
    }

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellCategoryStore,       dbcPath, "SpellCategory.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellCastTimesStore,      dbcPath, "SpellCastTimes.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellDurationStore,       dbcPath, "SpellDuration.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellFocusObjectStore,    dbcPath, "SpellFocusObject.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellItemEnchantmentStore, dbcPath, "SpellItemEnchantment.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellRadiusStore,         dbcPath, "SpellRadius.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellRangeStore,          dbcPath, "SpellRange.dbc");
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sSpellShapeshiftFormStore, dbcPath, "SpellShapeshiftForm.dbc");
    LoadDBCFromWorld(bar, badDbcFiles, sStableSlotPricesStore, "dbc_stableslotprices", "`ID`, `Cost`", [](Field* f, StableSlotPricesEntry& e)
    {
        e.Slot = f[0].GetUInt32();
        e.Price = f[1].GetUInt32();
        return e.Slot;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sTalentStore, "dbc_talent", "`ID`, `TabID`, `TierID`, `ColumnIndex`, `SpellRank1`, `SpellRank2`, `SpellRank3`, `SpellRank4`, `SpellRank5`, `PrereqTalent1`, `PrereqRank1`, `RequiredSpellID`", [&](Field* f, TalentEntry& e)
    {
        e.TalentID = f[0].GetUInt32();
        e.TalentTab = f[1].GetUInt32();
        e.Row = f[2].GetUInt32();
        e.Col = f[3].GetUInt32();
        for (uint32 i = 0; i < MAX_TALENT_RANK; ++i)
            e.RankID[i] = f[4 + i].GetUInt32();
        e.DependsOn = f[9].GetUInt32();
        e.DependsOnRank = f[10].GetUInt32();
        e.DependsOnSpell = f[11].GetUInt32();
        return e.TalentID;
    });

    // create talent spells set
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; j++)
            if (talentInfo->RankID[j])
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i, j);
    }

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sTalentTabStore,           dbcPath, "TalentTab.dbc");

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sTaxiPathStore,            dbcPath, "TaxiPath.dbc");
    for (uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID, entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadDBC(availableDbcLocales, bar, badDbcFiles, sTaxiPathNodeStore,        dbcPath, "TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            if (pathLength[entry->path] < entry->index + 1)
                pathLength[entry->path] = entry->index + 1;
        }
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    // fill data (pointers to sTaxiPathNodeStore elements
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            sTaxiPathNodesByPath[entry->path].set(entry->index, entry);

    // Initialize global taxinodes mask
    // include existing nodes that have at least single not spell base (scripted) path
    {
        std::set<uint32> spellPaths;
        for (uint32 i = 1; i < sSpellMgr.GetMaxSpellId(); ++i)
            if (SpellEntry const* sInfo = sSpellMgr.GetSpellEntry(i))
                for (uint8 j = 0; j < MAX_EFFECT_INDEX; ++j)
                    if (sInfo->Effect[j] == 123 /*SPELL_EFFECT_SEND_TAXI*/)
                        spellPaths.insert(sInfo->EffectMiscValue[j]);

        memset(sTaxiNodesMask, 0, sizeof(sTaxiNodesMask));
        for (uint32 i = 1; i < sObjectMgr.GetMaxTaxiNodeId(); ++i)
        {
            TaxiNodesEntry const* node = sObjectMgr.GetTaxiNodeEntry(i);
            if (!node)
                continue;

            TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(i);
            if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
            {
                bool ok = false;
                for (const auto& dest_i : src_i->second)
                {
                    // not spell path
                    if (spellPaths.find(dest_i.second.ID) == spellPaths.end())
                    {
                        ok = true;
                        break;
                    }
                }

                if (!ok)
                    continue;
            }

            // valid taxi network node
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1 << ((i - 1) % 32);
            sTaxiNodesMask[field] |= submask;
        }
    }

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sTransportAnimationStore, dbcPath, "TransportAnimation.dbc");

    LoadDBC(availableDbcLocales, bar, badDbcFiles, sWMOAreaTableStore,        dbcPath, "WMOAreaTable.dbc");
    for (uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if (WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
            sWMOAreaInfoByTripple.insert({ WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry });
    }
    //LoadDBC(availableDbcLocales,bar,badDbcFiles,sWorldMapOverlayStore,     dbcPath,"WorldMapOverlay.dbc");

    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
        if (SkillRaceClassInfoEntry const* entry = sSkillRaceClassInfoStore.LookupEntry(i))
            if (sSkillLineStore.LookupEntry(entry->skillId))
                SkillRaceClassInfoBySkill.insert(SkillRaceClassInfoMap::value_type(entry->skillId, entry));

    // error checks
    if (badDbcFiles.size() >= DBCFilesCount)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "\nIncorrect DataDir value in mangosd.conf or ALL required *.dbc files (%d) not found by path: %sdbc", DBCFilesCount, dataPath.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }
    else if (!badDbcFiles.empty())
    {
        std::string str;
        for (const auto& bad_dbc_file : badDbcFiles)
            str += bad_dbc_file + "\n";

        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "\nSome required *.dbc files (%u from %d) not found or not compatible:\n%s", (uint32)badDbcFiles.size(), DBCFilesCount, str.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ">> Initialized %d data stores", DBCFilesCount);
}

char const* GetPetName(uint32 petfamily, uint32 dbclang)
{
    if (!petfamily)
        return nullptr;
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if (!pet_family)
        return nullptr;
    return pet_family->Name[dbclang] ? pet_family->Name[dbclang] : nullptr;
}

TalentSpellPos const* GetTalentSpellPos(uint32 spellId)
{
    TalentSpellPosMap::const_iterator itr = sTalentSpellPosMap.find(spellId);
    if (itr == sTalentSpellPosMap.end())
        return nullptr;

    return &itr->second;
}

uint32 GetTalentSpellCost(TalentSpellPos const* pos)
{
    if (pos)
        return pos->rank + 1;

    return 0;
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    return GetTalentSpellCost(GetTalentSpellPos(spellId));
}

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid)
{
    auto i = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(rootid, adtid, groupid));
    if (i == sWMOAreaInfoByTripple.end())
        return nullptr;
    return i->second;

}

ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id)
{
    // not sorted, numbering index from 0
    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(i);
        if (ch && ch->ChannelID == channel_id)
            return ch;
    }
    return nullptr;
}

ChatChannelsEntry const* GetChannelEntryFor(std::string const& name)
{
    // not sorted, numbering index from 0
    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(i);
        if (ch)
        {
            // need to remove %s from entryName if it exists before we match
            for (const auto loc : ch->pattern)
            {
                std::string entryName(loc);
                std::size_t removeString = entryName.find("%s");
                // Not loaded locale
                if (entryName.empty())
                    continue;

                if (removeString != std::string::npos)
                    entryName.replace(removeString, 2, "");

                if (name.find(entryName) != std::string::npos)
                    return ch;
            }
        }
    }
    return nullptr;
}

bool Zone2MapCoordinates(float& x, float& y, uint32 zone)
{
    ZoneMapBounds const* maEntry = sObjectMgr.GetZoneMapBounds(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
        return false;

    std::swap(x, y);                                        // at client map coords swapped
    x = x * ((maEntry->x2 - maEntry->x1) / 100) + maEntry->x1;
    y = y * ((maEntry->y2 - maEntry->y1) / 100) + maEntry->y1; // client y coord from top to down

    return true;
}

bool Map2ZoneCoordinates(float& x, float& y, uint32 zone)
{
    ZoneMapBounds const* maEntry = sObjectMgr.GetZoneMapBounds(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
        return false;

    x = (x - maEntry->x1) / ((maEntry->x2 - maEntry->x1) / 100);
    y = (y - maEntry->y1) / ((maEntry->y2 - maEntry->y1) / 100); // client y coord from top to down
    std::swap(x, y);                                        // client have map coords swapped

    return true;
}

SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_)
{
    SkillRaceClassInfoBounds bounds = SkillRaceClassInfoBySkill.equal_range(skill);
    for (SkillRaceClassInfoMap::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second->raceMask && !(itr->second->raceMask & (1 << (race - 1))))
            continue;
        if (itr->second->classMask && !(itr->second->classMask & (1 << (class_ - 1))))
            continue;

        return itr->second;
    }

    return nullptr;
}

char const* GetUnitRaceName(uint8 race, uint8 locale)
{
    ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(race);
    return raceEntry ? raceEntry->name[locale] : nullptr;
}

char const* GetUnitClassName(uint8 class_, uint8 locale)
{
    ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(class_);
    return classEntry ? classEntry->name[locale] : nullptr;
}

uint8 ValidateName(std::wstring const& name)
{
    for (std::wregex const& regex : NamesProfaneValidators)
        if (std::regex_search(name, regex))
            return CHAR_NAME_PROFANE;

    for (std::wregex const& regex : NamesReservedValidators)
        if (std::regex_search(name, regex))
            return CHAR_NAME_RESERVED;       

    return CHAR_NAME_SUCCESS;
}
