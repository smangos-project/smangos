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
DBCStorage <SummonPropertiesEntry> sSummonPropertiesStore(SummonPropertiesfmt);
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

template<class T, class Fill>
static void LoadDBCFromWorld(BarGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, char const* table, char const* columns, Fill fill, char const* orderBy = "")
{
    std::vector<std::pair<uint32, T>> rows;
    std::unique_ptr<QueryResult> result(WorldDatabase.PQuery("SELECT %s FROM `%s` %s", columns, table, orderBy));
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

void LoadDBCStores()
{
    uint32 const DBCFilesCount = 41;

    BarGoLink bar(DBCFilesCount);

    StoreProblemList badDbcFiles;

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

    LoadDBCFromWorld(bar, badDbcFiles, sChatChannelsStore, "dbc_chatchannels", "`ID`, `Flags`, `Name_lang`", [&](Field* f, ChatChannelsEntry& e)
    {
        e.ChannelID = f[0].GetUInt32();
        e.flags = f[1].GetUInt32();
        FillLocalizedString(sChatChannelsStore, const_cast<char const**>(e.pattern), f[2].GetCppString());
        return e.ChannelID;
    });

    LoadDBCFromWorld(bar, badDbcFiles, sChrClassesStore, "dbc_chrclasses", "`ID`, `DisplayPower`, `Name_lang`, `SpellClassSet`", [&](Field* f, ChrClassesEntry& e)
    {
        e.ClassID = f[0].GetUInt32();
        e.powerType = f[1].GetUInt32();
        FillLocalizedString(sChrClassesStore, const_cast<char const**>(e.name), f[2].GetCppString());
        e.spellfamily = f[3].GetUInt32();
        return e.ClassID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sChrRacesStore, "dbc_chrraces", "`ID`, `Flags`, `FactionID`, `MaleDisplayID`, `FemaleDisplayID`, `BaseLanguage`, `CreatureType`, `ResSicknessSpellID`, `CinematicSequenceID`, `Name_lang`", [&](Field* f, ChrRacesEntry& e)
    {
        e.RaceID = f[0].GetUInt32();
        e.Flags = f[1].GetUInt32();
        e.FactionID = f[2].GetUInt32();
        e.model_m = f[3].GetUInt32();
        e.model_f = f[4].GetUInt32();
        e.TeamID = f[5].GetUInt32();
        e.creatureType = f[6].GetUInt32();
        e.resSicknessSpellId = f[7].GetUInt32();
        e.CinematicSequence = f[8].GetUInt32();
        FillLocalizedString(sChrRacesStore, const_cast<char const**>(e.name), f[9].GetCppString());
        return e.RaceID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCinematicSequencesStore, "dbc_cinematicsequences", "`ID`", [](Field* f, CinematicSequencesEntry& e)
    {
        e.Id = f[0].GetUInt32();
        return e.Id;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureDisplayInfoStore, "dbc_creaturedisplayinfo", "`ID`, `ModelID`, `ExtendedDisplayInfoID`, `CreatureModelScale`", [](Field* f, CreatureDisplayInfoEntry& e)
    {
        e.Displayid = f[0].GetUInt32();
        e.ModelId = f[1].GetUInt32();
        e.ExtendedDisplayInfoID = f[2].GetUInt32();
        e.scale = f[3].GetFloat();
        return e.Displayid;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureDisplayInfoExtraStore, "dbc_creaturedisplayinfoextra", "`ID`, `DisplayRaceID`", [](Field* f, CreatureDisplayInfoExtraEntry& e)
    {
        e.DisplayExtraId = f[0].GetUInt32();
        e.Race = f[1].GetUInt32();
        return e.DisplayExtraId;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureModelDataStore, "dbc_creaturemodeldata", "`ID`, `Flags`, `ModelScale`, `CollisionHeight`", [&](Field* f, CreatureModelDataEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.flags = f[1].GetUInt32();
        e.modelName = const_cast<char*>(sCreatureModelDataStore.InternString(""));
        e.modelScale = f[2].GetFloat();
        e.collisionHeight = f[3].GetFloat();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureFamilyStore, "dbc_creaturefamily", "`ID`, `MinScale`, `MinScaleLevel`, `MaxScale`, `MaxScaleLevel`, `SkillLine1`, `SkillLine2`, `PetFoodMask`, `Name_lang`", [&](Field* f, CreatureFamilyEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.minScale = f[1].GetFloat();
        e.minScaleLevel = f[2].GetUInt32();
        e.maxScale = f[3].GetFloat();
        e.maxScaleLevel = f[4].GetUInt32();
        e.skillLine[0] = f[5].GetUInt32();
        e.skillLine[1] = f[6].GetUInt32();
        e.petFoodMask = f[7].GetUInt32();
        FillLocalizedString(sCreatureFamilyStore, const_cast<char const**>(e.Name), f[8].GetCppString());
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sCreatureTypeStore, "dbc_creaturetype", "`ID`", [](Field* f, CreatureTypeEntry& e)
    {
        e.ID = f[0].GetUInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sDurabilityCostsStore, "dbc_durabilitycosts", "`ID`, `WeaponSubClassCost1`, `WeaponSubClassCost2`, `WeaponSubClassCost3`, `WeaponSubClassCost4`, `WeaponSubClassCost5`, `WeaponSubClassCost6`, `WeaponSubClassCost7`, `WeaponSubClassCost8`, `WeaponSubClassCost9`, `WeaponSubClassCost10`, `WeaponSubClassCost11`, `WeaponSubClassCost12`, `WeaponSubClassCost13`, `WeaponSubClassCost14`, `WeaponSubClassCost15`, `WeaponSubClassCost16`, `WeaponSubClassCost17`, `WeaponSubClassCost18`, `WeaponSubClassCost19`, `WeaponSubClassCost20`, `WeaponSubClassCost21`, `ArmorSubClassCost1`, `ArmorSubClassCost2`, `ArmorSubClassCost3`, `ArmorSubClassCost4`, `ArmorSubClassCost5`, `ArmorSubClassCost6`, `ArmorSubClassCost7`, `ArmorSubClassCost8`", [](Field* f, DurabilityCostsEntry& e)
    {
        e.Itemlvl = f[0].GetUInt32();
        for (uint32 i = 0; i < 29; ++i)
            e.multiplier[i] = f[1 + i].GetUInt32();
        return e.Itemlvl;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sDurabilityQualityStore, "dbc_durabilityquality", "`ID`, `Data`", [](Field* f, DurabilityQualityEntry& e)
    {
        e.Id = f[0].GetUInt32();
        e.quality_mod = f[1].GetFloat();
        return e.Id;
    });
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
    LoadDBCFromWorld(bar, badDbcFiles, sGameObjectDisplayInfoStore, "dbc_gameobjectdisplayinfo", "`ID`, `ModelName`", [&](Field* f, GameObjectDisplayInfoEntry& e)
    {
        e.Displayid = f[0].GetUInt32();
        e.filename = const_cast<char*>(sGameObjectDisplayInfoStore.InternString(f[1].GetCppString()));
        return e.Displayid;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sItemBagFamilyStore, "dbc_itembagfamily", "`ID`", [](Field* f, ItemBagFamilyEntry& e)
    {
        e.ID = f[0].GetUInt32();
        return e.ID;
    });
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
    LoadDBCFromWorld(bar, badDbcFiles, sItemSetStore, "dbc_itemset", "`ID`, `Name_lang`, `RequiredSkill`, `RequiredSkillRank`", [&](Field* f, ItemSetEntry& e)
    {
        FillLocalizedString(sItemSetStore, const_cast<char const**>(e.name), f[1].GetCppString());
        e.required_skill_id = f[2].GetUInt32();
        e.required_skill_value = f[3].GetUInt32();
        return f[0].GetUInt32();
    });

    // The bonuses live in dbc_itemsetspell, one row each; eight fit a set.
    if (std::unique_ptr<QueryResult> result{ WorldDatabase.Query("SELECT `ItemSetID`, `SpellID`, `Threshold` FROM `dbc_itemsetspell` ORDER BY `ID`") })
    {
        do
        {
            Field* f = result->Fetch();
            if (ItemSetEntry* set = const_cast<ItemSetEntry*>(sItemSetStore.LookupEntry(f[0].GetUInt32())))
            {
                for (uint32 i = 0; i < 8; ++i)
                {
                    if (!set->spells[i])
                    {
                        set->spells[i] = f[1].GetUInt32();
                        set->items_to_triggerspell[i] = f[2].GetUInt32();
                        break;
                    }
                }
            }
        }
        while (result->NextRow());
    }
    LoadDBCFromWorld(bar, badDbcFiles, sLockStore, "dbc_lock", "`ID`, `Type1`, `Type2`, `Type3`, `Type4`, `Type5`, `Type6`, `Type7`, `Type8`, `Index1`, `Index2`, `Index3`, `Index4`, `Index5`, `Index6`, `Index7`, `Index8`, `Skill1`, `Skill2`, `Skill3`, `Skill4`, `Skill5`, `Skill6`, `Skill7`, `Skill8`", [](Field* f, LockEntry& e)
    {
        e.ID = f[0].GetUInt32();
        for (uint32 i = 0; i < MAX_LOCK_CASE; ++i)
        {
            e.Type[i] = f[1 + i].GetUInt32();
            e.Index[i] = f[9 + i].GetUInt32();
            e.Skill[i] = f[17 + i].GetUInt32();
        }
        return e.ID;
    });

    {
        uint32 index = 0;
        LoadDBCFromWorld(bar, badDbcFiles, sNamesProfanityStore, "dbc_namesprofanity", "`Name`", [&](Field* f, NamesProfanityEntry& e)
        {
            e.Name = sNamesProfanityStore.InternString(f[0].GetCppString());
            return index++;
        }, "ORDER BY `ID`");
    }
    {
        uint32 index = 0;
        LoadDBCFromWorld(bar, badDbcFiles, sNamesReservedStore, "dbc_namesreserved", "`Name`", [&](Field* f, NamesReservedEntry& e)
        {
            e.Name = sNamesReservedStore.InternString(f[0].GetCppString());
            return index++;
        }, "ORDER BY `ID`");
    }
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
    LoadDBCFromWorld(bar, badDbcFiles, sSkillLineStore, "dbc_skillline", "`ID`, `CategoryID`, `DisplayName_lang`", [&](Field* f, SkillLineEntry& e)
    {
        e.id = f[0].GetUInt32();
        e.categoryId = f[1].GetInt32();
        FillLocalizedString(sSkillLineStore, const_cast<char const**>(e.name), f[2].GetCppString());
        return e.id;
    });
    {
        uint32 skillRaceClassIndex = 0;
        LoadDBCFromWorld(bar, badDbcFiles, sSkillRaceClassInfoStore, "dbc_skillraceclassinfo", "`SkillID`, `RaceMask`, `ClassMask`, `Flags`, `MinLevel`, `SkillTierID`", [&](Field* f, SkillRaceClassInfoEntry& e)
        {
            e.skillId = f[0].GetUInt32();
            e.raceMask = f[1].GetUInt32();
            e.classMask = f[2].GetUInt32();
            e.flags = f[3].GetUInt32();
            e.reqLevel = f[4].GetUInt32();
            e.skillTierId = f[5].GetUInt32();
            return skillRaceClassIndex++;
        }, "ORDER BY `ID`");
    }
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

    LoadDBCFromWorld(bar, badDbcFiles, sSpellCategoryStore, "dbc_spellcategory", "`ID`, `Flags`", [](Field* f, SpellCategoryEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.Flags = f[1].GetUInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellCastTimesStore, "dbc_spellcasttimes", "`ID`, `Base`, `PerLevel`, `Minimum`", [](Field* f, SpellCastTimesEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.CastTime = f[1].GetInt32();
        e.CastTimePerLevel = f[2].GetInt32();
        e.MinCastTime = f[3].GetInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellDurationStore, "dbc_spellduration", "`ID`, `Duration`, `DurationPerLevel`, `MaxDuration`", [](Field* f, SpellDurationEntry& e)
    {
        e.ID = f[0].GetUInt32();
        for (uint32 i = 0; i < 3; ++i)
            e.Duration[i] = f[1 + i].GetInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellFocusObjectStore, "dbc_spellfocusobject", "`ID`", [](Field* f, SpellFocusObjectEntry& e)
    {
        e.ID = f[0].GetUInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellItemEnchantmentStore, "dbc_spellitemenchantment", "`ID`, `Effect1`, `Effect2`, `Effect3`, `EffectPointsMin1`, `EffectPointsMin2`, `EffectPointsMin3`, `EffectArg1`, `EffectArg2`, `EffectArg3`, `Name_lang`, `ItemVisual`, `Flags`", [&](Field* f, SpellItemEnchantmentEntry& e)
    {
        e.ID = f[0].GetUInt32();
        for (uint32 i = 0; i < 3; ++i)
        {
            e.type[i] = f[1 + i].GetUInt32();
            e.amount[i] = f[4 + i].GetUInt32();
            e.spellid[i] = f[7 + i].GetUInt32();
        }
        FillLocalizedString(sSpellItemEnchantmentStore, const_cast<char const**>(e.description), f[10].GetCppString());
        e.aura_id = static_cast<uint32>(f[11].GetInt32());
        e.slot = f[12].GetUInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellRadiusStore, "dbc_spellradius", "`ID`, `Radius`", [](Field* f, SpellRadiusEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.Radius = f[1].GetFloat();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellRangeStore, "dbc_spellrange", "`ID`, `RangeMin1`, `RangeMax1`", [](Field* f, SpellRangeEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.minRange = f[1].GetFloat();
        e.maxRange = f[2].GetFloat();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSummonPropertiesStore, "dbc_summonproperties", "`ID`, `Control`, `Faction`, `Title`, `Slot`, `Flags1`", [](Field* f, SummonPropertiesEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.Control = f[1].GetUInt32();
        e.Faction = f[2].GetUInt32();
        e.Title = f[3].GetUInt32();
        e.Slot = f[4].GetUInt32();
        e.Flags = f[5].GetUInt32();
        return e.ID;
    });
    LoadDBCFromWorld(bar, badDbcFiles, sSpellShapeshiftFormStore, "dbc_spellshapeshiftform", "`ID`, `Name_lang`, `Flags`, `CreatureType`", [&](Field* f, SpellShapeshiftFormEntry& e)
    {
        e.ID = f[0].GetUInt32();
        FillLocalizedString(sSpellShapeshiftFormStore, const_cast<char const**>(e.Name), f[1].GetCppString());
        e.flags1 = f[2].GetUInt32();
        e.creatureType = f[3].GetInt32();
        return e.ID;
    });
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

    LoadDBCFromWorld(bar, badDbcFiles, sTalentTabStore, "dbc_talenttab", "`ID`, `ClassMask`, `OrderIndex`", [&](Field* f, TalentTabEntry& e)
    {
        e.TalentTabID = f[0].GetUInt32();
        e.ClassMask = f[1].GetUInt32();
        e.tabpage = f[2].GetUInt32();
        return e.TalentTabID;
    });

    LoadDBCFromWorld(bar, badDbcFiles, sTaxiPathStore, "dbc_taxipath", "`ID`, `FromTaxiNode`, `ToTaxiNode`, `Cost`", [](Field* f, TaxiPathEntry& e)
    {
        e.ID = f[0].GetUInt32();
        e.from = f[1].GetUInt32();
        e.to = f[2].GetUInt32();
        e.price = f[3].GetUInt32();
        return e.ID;
    });
    for (uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID, entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    {
        uint32 index = 0;
        LoadDBCFromWorld(bar, badDbcFiles, sTaxiPathNodeStore, "dbc_taxipathnode", "`PathID`, `NodeIndex`, `ContinentID`, `Loc1`, `Loc2`, `Loc3`, `Flags`, `Delay`", [&](Field* f, TaxiPathNodeEntry& e)
        {
            e.path = f[0].GetUInt32();
            e.index = f[1].GetUInt32();
            e.mapid = f[2].GetUInt32();
            e.x = f[3].GetFloat();
            e.y = f[4].GetFloat();
            e.z = f[5].GetFloat();
            e.actionFlag = f[6].GetUInt32();
            e.delay = f[7].GetUInt32();
            return index++;
        }, "ORDER BY `ID`");
    }
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

    LoadDBCFromWorld(bar, badDbcFiles, sTransportAnimationStore, "dbc_transportanimation", "`ID`, `TransportID`, `TimeIndex`, `Pos1`, `Pos2`, `Pos3`", [&](Field* f, TransportAnimationEntry& e)
    {
        e.TransportEntry = f[1].GetUInt32();
        e.TimeSeg = f[2].GetUInt32();
        e.X = f[3].GetFloat();
        e.Y = f[4].GetFloat();
        e.Z = f[5].GetFloat();
        return f[0].GetUInt32();
    });

    LoadDBCFromWorld(bar, badDbcFiles, sWMOAreaTableStore, "dbc_wmoareatable", "`ID`, `WMOID`, `NameSetID`, `WMOGroupID`, `Flags`, `AreaTableID`", [](Field* f, WMOAreaTableEntry& e)
    {
        e.Id = f[0].GetUInt32();
        e.rootId = static_cast<int32>(f[1].GetUInt32());
        e.adtId = static_cast<int32>(f[2].GetUInt32());
        e.groupId = static_cast<int32>(f[3].GetUInt32());
        e.Flags = f[4].GetUInt32();
        e.areaId = f[5].GetUInt32();
        return e.Id;
    });
    for (uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if (WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
            sWMOAreaInfoByTripple.insert({ WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry });
    }

    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
        if (SkillRaceClassInfoEntry const* entry = sSkillRaceClassInfoStore.LookupEntry(i))
            if (sSkillLineStore.LookupEntry(entry->skillId))
                SkillRaceClassInfoBySkill.insert(SkillRaceClassInfoMap::value_type(entry->skillId, entry));

    if (!badDbcFiles.empty())
    {
        std::string str;
        for (const auto& bad_dbc_file : badDbcFiles)
            str += bad_dbc_file + '\n';

        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "\nSome required data stores (%u from %d) could not be loaded from the world database:\n%s", (uint32)badDbcFiles.size(), DBCFilesCount, str.c_str());
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
