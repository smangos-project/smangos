#include "StoreDump.h"
#include "DBCStores.h"
#include "SQLStorages.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "QuestDef.h"
#include "Log.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <type_traits>
#include <vector>

extern DBCStorage<ChatChannelsEntry> sChatChannelsStore;
extern DBCStorage<NamesProfanityEntry> sNamesProfanityStore;
extern DBCStorage<NamesReservedEntry> sNamesReservedStore;
extern DBCStorage<TaxiPathNodeEntry> sTaxiPathNodeStore;

struct DumpColumn
{
    char const* name;
    uint32 count = 1;
};

// One TSV file: a header line, then one line per record. Strings are escaped, NULL is \N,
// floats are written with enough digits to read back the same float32.
class DumpTable
{
    public:
        DumpTable(std::string const& dir, std::string const& name) : m_name(name)
        {
            m_file = fopen((dir + "/" + name + ".tsv").c_str(), "wb");
            if (!m_file)
                Fail("cannot open file");
        }

        ~DumpTable()
        {
            if (m_file)
                fclose(m_file);
        }

        void AddColumn(std::string const& name) { m_header.push_back(name); }

        void AddColumns(std::initializer_list<DumpColumn> columns)
        {
            for (DumpColumn const& column : columns)
            {
                if (column.count == 1)
                    AddColumn(column.name);
                else
                    for (uint32 i = 1; i <= column.count; ++i)
                        AddColumn(column.name + std::to_string(i));
            }
        }

        void EndHeader()
        {
            for (std::string const& name : m_header)
                Field(name);
            EndLine();
        }

        template<typename T> requires std::is_arithmetic_v<T>
        DumpTable& operator<<(T value)
        {
            char buf[32];
            if constexpr (std::is_same_v<T, bool>)
                snprintf(buf, sizeof(buf), "%d", value ? 1 : 0);
            else if constexpr (std::is_floating_point_v<T>)
                snprintf(buf, sizeof(buf), "%.9g", double(value));
            else if constexpr (std::is_signed_v<T>)
                snprintf(buf, sizeof(buf), "%lld", (long long)value);
            else
                snprintf(buf, sizeof(buf), "%llu", (unsigned long long)value);
            Field(buf);
            return *this;
        }

        DumpTable& operator<<(char const* value)
        {
            if (!value)
            {
                Field("\\N");
                return *this;
            }
            std::string escaped;
            for (char const* c = value; *c; ++c)
            {
                switch (*c)
                {
                    case '\\': escaped += "\\\\"; break;
                    case '\t': escaped += "\\t"; break;
                    case '\n': escaped += "\\n"; break;
                    case '\r': escaped += "\\r"; break;
                    default: escaped += *c; break;
                }
            }
            Field(escaped);
            return *this;
        }

        DumpTable& operator<<(std::string const& value) { return *this << value.c_str(); }

        template<typename T, size_t N>
        DumpTable& operator<<(T const (&values)[N])
        {
            for (T const& value : values)
                *this << value;
            return *this;
        }

        template<typename T, size_t N>
        DumpTable& operator<<(std::array<T, N> const& values)
        {
            for (T const& value : values)
                *this << value;
            return *this;
        }

        // A list of ids as one space-separated field.
        template<typename T>
        void List(std::vector<T> values)
        {
            std::sort(values.begin(), values.end());
            std::string joined;
            for (T value : values)
                joined += (joined.empty() ? "" : " ") + std::to_string(value);
            Field(joined);
        }

        // A zero-terminated id list (creature auras), \N when absent.
        void ZeroTerminatedList(uint32 const* values)
        {
            if (!values)
            {
                Field("\\N");
                return;
            }
            std::string joined;
            for (uint32 const* value = values; *value; ++value)
                joined += (joined.empty() ? "" : " ") + std::to_string(*value);
            Field(joined);
        }

        void EndRow()
        {
            if (m_fields != m_header.size())
                Fail("row has " + std::to_string(m_fields) + " fields, header " + std::to_string(m_header.size()));
            EndLine();
        }

        bool Close()
        {
            if (m_file && fclose(m_file) != 0)
                Fail("write failed");
            m_file = nullptr;
            return m_ok;
        }

    private:
        void Field(std::string const& text)
        {
            m_line += m_fields ? "\t" : "";
            m_line += text;
            ++m_fields;
        }

        void EndLine()
        {
            m_line += '\n';
            if (m_file && m_ok)
                fwrite(m_line.data(), 1, m_line.size(), m_file);
            m_line.clear();
            m_fields = 0;
        }

        void Fail(std::string const& reason)
        {
            if (m_ok)
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Store dump %s: %s", m_name.c_str(), reason.c_str());
            m_ok = false;
        }

        std::string m_name;
        FILE* m_file = nullptr;
        std::vector<std::string> m_header;
        std::string m_line;
        uint32 m_fields = 0;
        bool m_ok = true;
};

template<typename T>
static T ReadAt(char const* record, uint32& offset)
{
    T value;
    memcpy(&value, record + offset, sizeof(T));
    offset += sizeof(T);
    return value;
}

template<typename Map>
static std::vector<uint32> SortedKeys(Map const& map)
{
    std::vector<uint32> keys;
    keys.reserve(map.size());
    for (auto const& itr : map)
        keys.push_back(itr.first);
    std::sort(keys.begin(), keys.end());
    return keys;
}

// Columns are named f<position in the format string>; positions the loader skips are left out.
template<class T>
static bool DumpDBCStore(std::string const& dir, char const* name, DBCStorage<T> const& store)
{
    char const* fmt = store.GetFormat();
    DumpTable table(dir, name);
    for (uint32 x = 0; fmt[x]; ++x)
        if (fmt[x] == FT_IND || fmt[x] == FT_INT || fmt[x] == FT_FLOAT || fmt[x] == FT_BYTE || fmt[x] == FT_STRING)
            table.AddColumn("f" + std::to_string(x));
    table.EndHeader();

    for (uint32 id = 0; id < store.GetNumRows(); ++id)
    {
        char const* record = reinterpret_cast<char const*>(store.LookupEntry(id));
        if (!record)
            continue;

        uint32 offset = 0;
        for (uint32 x = 0; fmt[x]; ++x)
        {
            switch (fmt[x])
            {
                case FT_IND:
                case FT_INT: table << ReadAt<uint32>(record, offset); break;
                case FT_FLOAT: table << ReadAt<float>(record, offset); break;
                case FT_BYTE: table << ReadAt<uint8>(record, offset); break;
                case FT_STRING: table << ReadAt<char const*>(record, offset); break;
                default: break;
            }
        }
        table.EndRow();
    }
    return table.Close();
}

// Columns are named f<position in the destination format>. auraColumn holds a converted aura list.
static bool DumpSQLStore(std::string const& dir, SQLStorage const& store, int32 auraColumn = -1, char const* name = nullptr)
{
    char const* fmt = store.GetDstFormat();
    DumpTable table(dir, name ? name : store.GetTableName());
    for (uint32 x = 0; fmt[x]; ++x)
        table.AddColumn("f" + std::to_string(x));
    table.EndHeader();

    for (uint32 id = 0; id < store.GetMaxEntry(); ++id)
    {
        char const* record = store.LookupEntry<char>(id);
        if (!record)
            continue;

        uint32 offset = 0;
        for (uint32 x = 0; fmt[x]; ++x)
        {
            if (int32(x) == auraColumn)
            {
                table.ZeroTerminatedList(ReadAt<uint32 const*>(record, offset));
                continue;
            }
            switch (fmt[x])
            {
                case FT_LOGIC: table << ReadAt<bool>(record, offset); break;
                case FT_BYTE:
                case FT_NA_BYTE: table << ReadAt<uint8>(record, offset); break;
                case FT_INT:
                case FT_NA: table << ReadAt<uint32>(record, offset); break;
                case FT_FLOAT:
                case FT_NA_FLOAT: table << ReadAt<float>(record, offset); break;
                case FT_STRING:
                case FT_NA_POINTER: table << ReadAt<char const*>(record, offset); break;
                case FT_64BITINT: table << ReadAt<uint64>(record, offset); break;
                default: break;
            }
        }
        table.EndRow();
    }
    return table.Close();
}

static bool DumpSpells(std::string const& dir)
{
    DumpTable table(dir, "spell");
    table.AddColumns({
        {"Id"}, {"School"}, {"Category"}, {"Dispel"}, {"Mechanic"}, {"Attributes"}, {"AttributesEx"},
        {"AttributesEx2"}, {"AttributesEx3"}, {"AttributesEx4"}, {"Stances"}, {"StancesNot"}, {"Targets"},
        {"TargetCreatureType"}, {"RequiresSpellFocus"}, {"CasterAuraState"}, {"TargetAuraState"},
        {"CastingTimeIndex"}, {"RecoveryTime"}, {"CategoryRecoveryTime"}, {"InterruptFlags"},
        {"AuraInterruptFlags"}, {"ChannelInterruptFlags"}, {"procFlags"}, {"procChance"}, {"procCharges"},
        {"maxLevel"}, {"baseLevel"}, {"spellLevel"}, {"DurationIndex"}, {"powerType"}, {"manaCost"},
        {"manaCostPerlevel"}, {"manaPerSecond"}, {"manaPerSecondPerLevel"}, {"rangeIndex"}, {"speed"},
        {"StackAmount"}, {"Totem", MAX_SPELL_TOTEMS}, {"Reagent", MAX_SPELL_REAGENTS},
        {"ReagentCount", MAX_SPELL_REAGENTS}, {"EquippedItemClass"}, {"EquippedItemSubClassMask"},
        {"EquippedItemInventoryTypeMask"}, {"Effect", MAX_EFFECT_INDEX}, {"EffectDieSides", MAX_EFFECT_INDEX},
        {"EffectBaseDice", MAX_EFFECT_INDEX}, {"EffectDicePerLevel", MAX_EFFECT_INDEX},
        {"EffectRealPointsPerLevel", MAX_EFFECT_INDEX}, {"EffectBasePoints", MAX_EFFECT_INDEX},
        {"EffectBonusCoefficient", MAX_EFFECT_INDEX}, {"EffectMechanic", MAX_EFFECT_INDEX},
        {"EffectImplicitTargetA", MAX_EFFECT_INDEX}, {"EffectImplicitTargetB", MAX_EFFECT_INDEX},
        {"EffectRadiusIndex", MAX_EFFECT_INDEX}, {"EffectApplyAuraName", MAX_EFFECT_INDEX},
        {"EffectAmplitude", MAX_EFFECT_INDEX}, {"EffectMultipleValue", MAX_EFFECT_INDEX},
        {"EffectChainTarget", MAX_EFFECT_INDEX}, {"EffectItemType", MAX_EFFECT_INDEX},
        {"EffectMiscValue", MAX_EFFECT_INDEX}, {"EffectTriggerSpell", MAX_EFFECT_INDEX},
        {"EffectPointsPerComboPoint", MAX_EFFECT_INDEX},
        {"spellPriority"}, {"SpellName", MAX_DBC_LOCALE}, {"Rank", MAX_DBC_LOCALE}, {"ManaCostPercentage"},
        {"StartRecoveryCategory"}, {"StartRecoveryTime"}, {"MaxTargetLevel"}, {"SpellFamilyName"},
        {"SpellFamilyFlags"}, {"MaxAffectedTargets"}, {"DmgClass"}, {"PreventionType"},
        {"DmgMultiplier", MAX_EFFECT_INDEX}, {"MinTargetLevel"}, {"Custom"}, {"Internal"}, {"AllowedTargetMask"},
        {"ScriptId"}, {"Tags"}, {"StackKey"}, {"RankKey"}, {"SingleTargetKey"}, {"Cone"}, {"EnchantCharges"},
    });
    table.EndHeader();

    for (uint32 id = 0; id < sSpellMgr.GetMaxSpellId(); ++id)
    {
        SpellEntry const* s = sSpellMgr.GetSpellEntry(id);
        if (!s)
            continue;

        table << s->Id << s->School << s->Category << s->Dispel << s->Mechanic << s->Attributes << s->AttributesEx
            << s->AttributesEx2 << s->AttributesEx3 << s->AttributesEx4 << s->Stances << s->StancesNot << s->Targets
            << s->TargetCreatureType << s->RequiresSpellFocus << s->CasterAuraState << s->TargetAuraState
            << s->CastingTimeIndex << s->RecoveryTime << s->CategoryRecoveryTime << s->InterruptFlags
            << s->AuraInterruptFlags << s->ChannelInterruptFlags << s->procFlags << s->procChance << s->procCharges
            << s->maxLevel << s->baseLevel << s->spellLevel << s->DurationIndex << s->powerType << s->manaCost
            << s->manaCostPerlevel << s->manaPerSecond << s->manaPerSecondPerLevel << s->rangeIndex << s->speed
            << s->StackAmount << s->Totem << s->Reagent
            << s->ReagentCount << s->EquippedItemClass << s->EquippedItemSubClassMask
            << s->EquippedItemInventoryTypeMask << s->Effect << s->EffectDieSides
            << s->EffectBaseDice << s->EffectDicePerLevel
            << s->EffectRealPointsPerLevel << s->EffectBasePoints
            << s->EffectBonusCoefficient << s->EffectMechanic
            << s->EffectImplicitTargetA << s->EffectImplicitTargetB
            << s->EffectRadiusIndex << s->EffectApplyAuraName
            << s->EffectAmplitude << s->EffectMultipleValue
            << s->EffectChainTarget << s->EffectItemType
            << s->EffectMiscValue << s->EffectTriggerSpell
            << s->EffectPointsPerComboPoint
            << s->spellPriority << s->SpellName << s->Rank << s->ManaCostPercentage
            << s->StartRecoveryCategory << s->StartRecoveryTime << s->MaxTargetLevel << s->SpellFamilyName
            << s->SpellFamilyFlags << s->MaxAffectedTargets << s->DmgClass << s->PreventionType
            << s->DmgMultiplier << s->MinTargetLevel << s->Custom << s->Internal << s->AllowedTargetMask
            << s->ScriptId << uint64(s->Tags.to_ullong()) << s->StackKey << s->RankKey << s->SingleTargetKey
            << sSpellMgr.GetSpellCone(id) << sSpellMgr.GetSpellEnchantCharges(id);
        table.EndRow();
    }
    return table.Close();
}

static bool DumpSpellPairs(std::string const& dir)
{
    std::vector<SpellEntry const*> auras;
    for (uint32 id = 0; id < sSpellMgr.GetMaxSpellId(); ++id)
    {
        SpellEntry const* s = sSpellMgr.GetSpellEntry(id);
        if (s && (s->EffectApplyAuraName[0] || s->EffectApplyAuraName[1] || s->EffectApplyAuraName[2]))
            auras.push_back(s);
    }

    DumpTable table(dir, "spell_pairs");
    table.AddColumns({{"spell1"}, {"spell2"}, {"no_stack"}, {"rank"}, {"single_target"}});
    table.EndHeader();

    for (SpellEntry const* a : auras)
    {
        for (SpellEntry const* b : auras)
        {
            if (a == b)
                continue;
            uint32 noStack = sSpellMgr.IsNoStackSpellDueToSpell(a->Id, b->Id);
            uint32 rank = sSpellMgr.IsRankSpellDueToSpell(a, b->Id);
            uint32 single = Spells::IsSingleTargetSpells(a, b);
            if (noStack || rank || single)
            {
                table << a->Id << b->Id << noStack << rank << single;
                table.EndRow();
            }
        }
    }
    return table.Close();
}

static bool DumpCreatureTemplates(std::string const& dir)
{
    DumpTable table(dir, "creature_template");
    table.AddColumns({
        {"entry"}, {"name"}, {"subname"}, {"level_min"}, {"level_max"}, {"faction"}, {"npc_flags"},
        {"gossip_menu_id"}, {"display_id", MAX_DISPLAY_IDS_PER_CREATURE}, {"display_scale", MAX_DISPLAY_IDS_PER_CREATURE},
        {"display_probability", MAX_DISPLAY_IDS_PER_CREATURE}, {"display_total_probability"}, {"mount_display_id"},
        {"speed_walk"}, {"speed_run"}, {"detection_range"}, {"call_for_help_range"}, {"leash_range"}, {"type"},
        {"pet_family"}, {"rank"}, {"unit_class"}, {"xp_multiplier"}, {"health_multiplier"}, {"mana_multiplier"},
        {"armor_multiplier"}, {"damage_multiplier"}, {"damage_variance"}, {"damage_school"}, {"base_attack_time"},
        {"ranged_attack_time"}, {"holy_res"}, {"fire_res"}, {"nature_res"}, {"frost_res"}, {"shadow_res"},
        {"arcane_res"}, {"trainer_type"}, {"trainer_spell"}, {"trainer_class"}, {"trainer_race"}, {"loot_id"},
        {"pickpocket_loot_id"}, {"skinning_loot_id"}, {"gold_min"}, {"gold_max"}, {"totem_spell_id"},
        {"spell_list_id"}, {"pet_spell_list_id"}, {"spawn_spell_id"}, {"auras"}, {"ai_name"}, {"movement_type"},
        {"inhabit_type"}, {"civilian"}, {"racial_leader"}, {"equipment_id"}, {"trainer_id"}, {"vendor_id"},
        {"mechanic_immune_mask"}, {"school_immune_mask"}, {"immunity_flags"}, {"static_flags1"},
        {"static_flags2"}, {"flags_extra"}, {"script_id"},
    });
    table.EndHeader();

    CreatureInfoMap const& map = sObjectMgr.GetCreatureInfoMap();
    for (uint32 entry : SortedKeys(map))
    {
        CreatureInfo const& c = *map.at(entry);
        table << c.entry << c.name << c.subname << c.level_min << c.level_max << c.faction << c.npc_flags
            << c.gossip_menu_id << c.display_id << c.display_scale
            << c.display_probability << c.display_total_probability << c.mount_display_id
            << c.speed_walk << c.speed_run << c.detection_range << c.call_for_help_range << c.leash_range << c.type
            << c.pet_family << c.rank << c.unit_class << c.xp_multiplier << c.health_multiplier << c.mana_multiplier
            << c.armor_multiplier << c.damage_multiplier << c.damage_variance << c.damage_school << c.base_attack_time
            << c.ranged_attack_time << c.holy_res << c.fire_res << c.nature_res << c.frost_res << c.shadow_res
            << c.arcane_res << c.trainer_type << c.trainer_spell << c.trainer_class << c.trainer_race << c.loot_id
            << c.pickpocket_loot_id << c.skinning_loot_id << c.gold_min << c.gold_max << c.totem_spell_id
            << c.spell_list_id << c.pet_spell_list_id << c.spawn_spell_id;
        table.ZeroTerminatedList(c.auras);
        table << c.ai_name << c.movement_type
            << c.inhabit_type << c.civilian << c.racial_leader << c.equipment_id << c.trainer_id << c.vendor_id
            << c.mechanic_immune_mask << c.school_immune_mask << c.immunity_flags << c.static_flags1
            << c.static_flags2 << c.flags_extra << c.script_id;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpItemPrototypes(std::string const& dir)
{
    DumpTable table(dir, "item_template");
    table.AddColumns({
        {"ItemId"}, {"Class"}, {"SubClass"}, {"Name1"}, {"Description"}, {"DisplayInfoID"}, {"Quality"}, {"Flags"},
        {"BuyCount"}, {"BuyPrice"}, {"SellPrice"}, {"InventoryType"}, {"AllowableClass"}, {"AllowableRace"},
        {"ItemLevel"}, {"RequiredLevel"}, {"RequiredSkill"}, {"RequiredSkillRank"}, {"RequiredSpell"},
        {"RequiredHonorRank"}, {"RequiredCityRank"}, {"RequiredReputationFaction"}, {"RequiredReputationRank"},
        {"MaxCount"}, {"Stackable"}, {"ContainerSlots"},
    });
    for (uint32 i = 1; i <= MAX_ITEM_PROTO_STATS; ++i)
        for (char const* field : {"ItemStatType", "ItemStatValue"})
            table.AddColumn(field + std::to_string(i));
    table.AddColumns({{"Delay"}, {"RangedModRange"}, {"AmmoType"}});
    for (uint32 i = 1; i <= MAX_ITEM_PROTO_DAMAGES; ++i)
        for (char const* field : {"DamageMin", "DamageMax", "DamageType"})
            table.AddColumn(field + std::to_string(i));
    table.AddColumns({{"Block"}, {"Armor"}, {"HolyRes"}, {"FireRes"}, {"NatureRes"}, {"FrostRes"}, {"ShadowRes"}, {"ArcaneRes"}});
    for (uint32 i = 1; i <= MAX_ITEM_PROTO_SPELLS; ++i)
        for (char const* field : {"SpellId", "SpellTrigger", "SpellCharges", "SpellPPMRate", "SpellCooldown", "SpellCategory", "SpellCategoryCooldown"})
            table.AddColumn(field + std::to_string(i));
    table.AddColumns({
        {"Bonding"}, {"PageText"}, {"LanguageID"}, {"PageMaterial"}, {"StartQuest"}, {"LockID"}, {"Material"},
        {"Sheath"}, {"RandomProperty"}, {"ItemSet"}, {"MaxDurability"}, {"Area"}, {"Map"}, {"Duration"},
        {"BagFamily"}, {"DisenchantID"}, {"FoodType"}, {"MinMoneyLoot"}, {"MaxMoneyLoot"}, {"WrappedGift"},
        {"ExtraFlags"}, {"OtherTeamEntry"}, {"SourceQuestLevel"}, {"SourceQuestRaces"}, {"SourceQuestClasses"},
    });
    table.EndHeader();

    ItemPrototypeMap const& map = sObjectMgr.GetItemPrototypeMap();
    for (uint32 entry : SortedKeys(map))
    {
        ItemPrototype const& p = map.at(entry);
        table << p.ItemId << p.Class << p.SubClass << p.Name1 << p.Description << p.DisplayInfoID << p.Quality
            << p.Flags << p.BuyCount << p.BuyPrice << p.SellPrice << p.InventoryType << p.AllowableClass
            << p.AllowableRace << p.ItemLevel << p.RequiredLevel << p.RequiredSkill << p.RequiredSkillRank
            << p.RequiredSpell << p.RequiredHonorRank << p.RequiredCityRank << p.RequiredReputationFaction
            << p.RequiredReputationRank << p.MaxCount << p.Stackable << p.ContainerSlots;
        for (_ItemStat const& stat : p.ItemStat)
            table << stat.ItemStatType << stat.ItemStatValue;
        table << p.Delay << p.RangedModRange << p.AmmoType;
        for (_ItemDamage const& damage : p.Damage)
            table << damage.DamageMin << damage.DamageMax << damage.DamageType;
        table << p.Block << p.Armor << p.HolyRes << p.FireRes << p.NatureRes << p.FrostRes << p.ShadowRes << p.ArcaneRes;
        for (_ItemSpell const& spell : p.Spells)
            table << spell.SpellId << spell.SpellTrigger << spell.SpellCharges << spell.SpellPPMRate
                << spell.SpellCooldown << spell.SpellCategory << spell.SpellCategoryCooldown;
        table << p.Bonding << p.PageText << p.LanguageID << p.PageMaterial << p.StartQuest << p.LockID
            << p.Material << p.Sheath << p.RandomProperty << p.ItemSet << p.MaxDurability << p.Area << p.Map
            << p.Duration << p.BagFamily << p.DisenchantID << p.FoodType << p.MinMoneyLoot << p.MaxMoneyLoot
            << p.WrappedGift << p.ExtraFlags << p.OtherTeamEntry << p.SourceQuestLevel << p.SourceQuestRaces
            << p.SourceQuestClasses;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpGameObjectInfos(std::string const& dir)
{
    DumpTable table(dir, "gameobject_template");
    table.AddColumns({{"id"}, {"type"}, {"displayId"}, {"name"}, {"icon"}, {"faction"}, {"flags"}, {"size"}, {"data", 24},
        {"MinMoneyLoot"}, {"MaxMoneyLoot"}, {"ScriptId"}});
    table.EndHeader();

    GameObjectInfoMap const& map = sObjectMgr.GetGameObjectInfoMap();
    for (uint32 entry : SortedKeys(map))
    {
        GameObjectInfo const& g = *map.at(entry);
        table << g.id << g.type << g.displayId << g.name << g.icon << g.faction << g.flags << g.size << g.raw.data
            << g.MinMoneyLoot << g.MaxMoneyLoot << g.ScriptId;
        table.EndRow();
    }
    return table.Close();
}

class StoreDumper
{
    public:
        static bool DumpQuests(std::string const& dir)
        {
            DumpTable table(dir, "quest_template");
            table.AddColumns({
                {"QuestId"}, {"QuestMethod"}, {"ZoneOrSort"}, {"MinLevel"}, {"MaxLevel"}, {"QuestLevel"}, {"Type"},
                {"RequiredClasses"}, {"RequiredRaces"}, {"RequiredSkill"}, {"RequiredSkillValue"},
                {"RequiredCondition"}, {"RepObjectiveFaction"}, {"RepObjectiveValue"}, {"RequiredMinRepFaction"},
                {"RequiredMinRepValue"}, {"RequiredMaxRepFaction"}, {"RequiredMaxRepValue"}, {"SuggestedPlayers"},
                {"LimitTime"}, {"QuestFlags"}, {"SpecialFlags"}, {"PrevQuestId"}, {"NextQuestId"},
                {"ExclusiveGroup"}, {"BreadcrumbForQuestId"}, {"NextQuestInChain"}, {"SrcItemId"},
                {"SrcItemCount"}, {"SrcSpell"}, {"Title"}, {"Details"}, {"Objectives"}, {"OfferRewardText"},
                {"RequestItemsText"}, {"EndText"}, {"RewRepSpilloverMask"}, {"RewXP"}, {"RewOrReqMoney"},
                {"RewMoneyMaxLevel"}, {"RewSpell"}, {"RewSpellCast"}, {"RewMailTemplateId"}, {"RewMailDelaySecs"},
                {"RewMailMoney"}, {"PointMapId"}, {"PointX"}, {"PointY"}, {"PointOpt"}, {"IncompleteEmote"},
                {"CompleteEmote"}, {"QuestStartScript"}, {"QuestCompleteScript"},
                {"ObjectiveText", QUEST_OBJECTIVES_COUNT}, {"ReqItemId", QUEST_ITEM_OBJECTIVES_COUNT},
                {"ReqItemCount", QUEST_ITEM_OBJECTIVES_COUNT}, {"ReqSourceId", QUEST_SOURCE_ITEM_IDS_COUNT},
                {"ReqSourceCount", QUEST_SOURCE_ITEM_IDS_COUNT}, {"ReqCreatureOrGOId", QUEST_OBJECTIVES_COUNT},
                {"ReqCreatureOrGOCount", QUEST_OBJECTIVES_COUNT}, {"ReqSpell", QUEST_OBJECTIVES_COUNT},
                {"RewChoiceItemId", QUEST_REWARD_CHOICES_COUNT}, {"RewChoiceItemCount", QUEST_REWARD_CHOICES_COUNT},
                {"RewItemId", QUEST_REWARDS_COUNT}, {"RewItemCount", QUEST_REWARDS_COUNT},
                {"RewRepFaction", QUEST_REPUTATIONS_COUNT}, {"RewRepValue", QUEST_REPUTATIONS_COUNT},
                {"DetailsEmote", QUEST_EMOTE_COUNT}, {"DetailsEmoteDelay", QUEST_EMOTE_COUNT},
                {"OfferRewardEmote", QUEST_EMOTE_COUNT}, {"OfferRewardEmoteDelay", QUEST_EMOTE_COUNT},
                {"prevQuests"}, {"prevChainQuests"}, {"DependentBreadcrumbQuests"},
            });
            table.EndHeader();

            ObjectMgr::QuestMap const& map = sObjectMgr.GetQuestTemplates();
            for (uint32 entry : SortedKeys(map))
            {
                Quest const& q = *map.at(entry);
                table << q.QuestId << q.QuestMethod << q.ZoneOrSort << q.MinLevel << q.MaxLevel << q.QuestLevel << q.Type
                    << q.RequiredClasses << q.RequiredRaces << q.RequiredSkill << q.RequiredSkillValue
                    << q.RequiredCondition << q.RepObjectiveFaction << q.RepObjectiveValue << q.RequiredMinRepFaction
                    << q.RequiredMinRepValue << q.RequiredMaxRepFaction << q.RequiredMaxRepValue << q.SuggestedPlayers
                    << q.LimitTime << q.m_QuestFlags << q.m_SpecialFlags << q.PrevQuestId << q.NextQuestId
                    << q.ExclusiveGroup << q.BreadcrumbForQuestId << q.NextQuestInChain << q.SrcItemId
                    << q.SrcItemCount << q.SrcSpell << q.Title << q.Details << q.Objectives << q.OfferRewardText
                    << q.RequestItemsText << q.EndText << q.RewRepSpilloverMask << q.RewXP << q.RewOrReqMoney
                    << q.RewMoneyMaxLevel << q.RewSpell << q.RewSpellCast << q.RewMailTemplateId << q.RewMailDelaySecs
                    << q.RewMailMoney << q.PointMapId << q.PointX << q.PointY << q.PointOpt << q.IncompleteEmote
                    << q.CompleteEmote << q.QuestStartScript << q.QuestCompleteScript
                    << q.ObjectiveText << q.ReqItemId << q.ReqItemCount << q.ReqSourceId << q.ReqSourceCount
                    << q.ReqCreatureOrGOId << q.ReqCreatureOrGOCount << q.ReqSpell << q.RewChoiceItemId
                    << q.RewChoiceItemCount << q.RewItemId << q.RewItemCount << q.RewRepFaction << q.RewRepValue
                    << q.DetailsEmote << q.DetailsEmoteDelay << q.OfferRewardEmote << q.OfferRewardEmoteDelay;
                table.List(q.prevQuests);
                table.List(q.prevChainQuests);
                table.List(q.DependentBreadcrumbQuests);
                table.EndRow();
            }
            return table.Close();
        }
};

static bool DumpFactions(std::string const& dir)
{
    DumpTable table(dir, "faction");
    table.AddColumns({{"ID"}, {"reputationListID"}, {"BaseRepRaceMask", 4}, {"BaseRepClassMask", 4},
        {"BaseRepValue", 4}, {"ReputationFlags", 4}, {"team"}, {"name", 8}});
    table.EndHeader();

    FactionsMap const& map = sObjectMgr.GetFactionMap();
    for (uint32 id : SortedKeys(map))
    {
        FactionEntry const& f = map.at(id);
        table << f.ID << f.reputationListID << f.BaseRepRaceMask << f.BaseRepClassMask << f.BaseRepValue
            << f.ReputationFlags << f.team << f.name;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpFactionTemplates(std::string const& dir)
{
    DumpTable table(dir, "faction_template");
    table.AddColumns({{"ID"}, {"faction"}, {"factionFlags"}, {"ourMask"}, {"friendlyMask"}, {"hostileMask"},
        {"enemyFaction", 4}, {"friendFaction", 4}, {"isEnemyOfAnother"}});
    table.EndHeader();

    FactionTemplatesMap const& map = sObjectMgr.GetFactionTemplateMap();
    for (uint32 id : SortedKeys(map))
    {
        FactionTemplateEntry const& f = map.at(id);
        table << f.ID << f.faction << f.factionFlags << f.ourMask << f.friendlyMask << f.hostileMask
            << f.enemyFaction << f.friendFaction << f.isEnemyOfAnother;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpTaxiNodes(std::string const& dir)
{
    DumpTable table(dir, "taxi_nodes");
    table.AddColumns({{"ID"}, {"map_id"}, {"x"}, {"y"}, {"z"}, {"name", MAX_DBC_LOCALE}, {"MountCreatureID", 2}});
    table.EndHeader();

    for (uint32 id = 0; id < sObjectMgr.GetMaxTaxiNodeId(); ++id)
    {
        TaxiNodesEntry const* t = sObjectMgr.GetTaxiNodeEntry(id);
        if (!t)
            continue;
        table << t->ID << t->map_id << t->x << t->y << t->z << t->name << t->MountCreatureID;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpSkillLineAbilities(std::string const& dir)
{
    DumpTable table(dir, "skill_line_ability");
    table.AddColumns({{"id"}, {"skillId"}, {"spellId"}, {"racemask"}, {"classmask"}, {"req_skill_value"},
        {"forward_spellid"}, {"learnOnGetSkill"}, {"max_value"}, {"min_value"}, {"reqtrainpoints"}});
    table.EndHeader();

    for (uint32 id = 0; id < sObjectMgr.GetMaxSkillLineAbilityId(); ++id)
    {
        SkillLineAbilityEntry const* s = sObjectMgr.GetSkillLineAbility(id);
        if (!s)
            continue;
        table << s->id << s->skillId << s->spellId << s->racemask << s->classmask << s->req_skill_value
            << s->forward_spellid << s->learnOnGetSkill << s->max_value << s->min_value << s->reqtrainpoints;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpSoundEntries(std::string const& dir)
{
    DumpTable table(dir, "sound_entries");
    table.AddColumns({{"Id"}});
    table.EndHeader();

    std::set<uint32> ids(sObjectMgr.GetSoundEntryIds().begin(), sObjectMgr.GetSoundEntryIds().end());
    for (uint32 id : ids)
    {
        table << id;
        table.EndRow();
    }
    return table.Close();
}

static bool DumpWorldSafeLocs(std::string const& dir)
{
    DumpTable table(dir, "WorldSafeLocs");
    table.AddColumns({{"ID"}, {"map_id"}, {"x"}, {"y"}, {"z"}, {"orientation"}});
    table.EndHeader();

    auto const& map = sObjectMgr.GetWorldSafeLocs();
    for (uint32 id : SortedKeys(map))
    {
        WorldSafeLocsEntry const& e = map.at(id);
        table << e.ID << e.map_id << e.x << e.y << e.z << e.orientation;
        table.EndRow();
    }
    return table.Close();
}

bool DumpStores(std::string const& dir)
{
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Store dump: cannot create %s: %s", dir.c_str(), ec.message().c_str());
        return false;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Dumping stores to %s ...", dir.c_str());

    bool ok = true;
    ok &= DumpDBCStore(dir, "AuctionHouse", sAuctionHouseStore);
    ok &= DumpDBCStore(dir, "BankBagSlotPrices", sBankBagSlotPricesStore);
    ok &= DumpDBCStore(dir, "ChatChannels", sChatChannelsStore);
    ok &= DumpDBCStore(dir, "ChrClasses", sChrClassesStore);
    ok &= DumpDBCStore(dir, "ChrRaces", sChrRacesStore);
    ok &= DumpDBCStore(dir, "CinematicSequences", sCinematicSequencesStore);
    ok &= DumpDBCStore(dir, "CreatureDisplayInfo", sCreatureDisplayInfoStore);
    ok &= DumpDBCStore(dir, "CreatureDisplayInfoExtra", sCreatureDisplayInfoExtraStore);
    ok &= DumpDBCStore(dir, "CreatureModelData", sCreatureModelDataStore);
    ok &= DumpDBCStore(dir, "CreatureFamily", sCreatureFamilyStore);
    ok &= DumpDBCStore(dir, "CreatureType", sCreatureTypeStore);
    ok &= DumpDBCStore(dir, "DurabilityQuality", sDurabilityQualityStore);
    ok &= DumpDBCStore(dir, "DurabilityCosts", sDurabilityCostsStore);
    ok &= DumpDBCStore(dir, "Emotes", sEmotesStore);
    ok &= DumpDBCStore(dir, "EmotesText", sEmotesTextStore);
    ok &= DumpDBCStore(dir, "GameObjectDisplayInfo", sGameObjectDisplayInfoStore);
    ok &= DumpDBCStore(dir, "ItemBagFamily", sItemBagFamilyStore);
    ok &= DumpDBCStore(dir, "ItemRandomProperties", sItemRandomPropertiesStore);
    ok &= DumpDBCStore(dir, "ItemSet", sItemSetStore);
    ok &= DumpDBCStore(dir, "Lock", sLockStore);
    ok &= DumpDBCStore(dir, "NamesProfanity", sNamesProfanityStore);
    ok &= DumpDBCStore(dir, "NamesReserved", sNamesReservedStore);
    ok &= DumpDBCStore(dir, "QuestSort", sQuestSortStore);
    ok &= DumpDBCStore(dir, "SkillLine", sSkillLineStore);
    ok &= DumpDBCStore(dir, "SkillRaceClassInfo", sSkillRaceClassInfoStore);
    ok &= DumpDBCStore(dir, "SpellItemEnchantment", sSpellItemEnchantmentStore);
    ok &= DumpDBCStore(dir, "SpellCategory", sSpellCategoryStore);
    ok &= DumpDBCStore(dir, "SpellCastTimes", sSpellCastTimesStore);
    ok &= DumpDBCStore(dir, "SpellDuration", sSpellDurationStore);
    ok &= DumpDBCStore(dir, "SpellFocusObject", sSpellFocusObjectStore);
    ok &= DumpDBCStore(dir, "SpellRadius", sSpellRadiusStore);
    ok &= DumpDBCStore(dir, "SpellRange", sSpellRangeStore);
    ok &= DumpDBCStore(dir, "SpellShapeshiftForm", sSpellShapeshiftFormStore);
    ok &= DumpDBCStore(dir, "StableSlotPrices", sStableSlotPricesStore);
    ok &= DumpDBCStore(dir, "Talent", sTalentStore);
    ok &= DumpDBCStore(dir, "TalentTab", sTalentTabStore);
    ok &= DumpDBCStore(dir, "TaxiPath", sTaxiPathStore);
    ok &= DumpDBCStore(dir, "TaxiPathNode", sTaxiPathNodeStore);
    ok &= DumpDBCStore(dir, "TransportAnimation", sTransportAnimationStore);
    ok &= DumpDBCStore(dir, "WMOAreaTable", sWMOAreaTableStore);
    ok &= DumpWorldSafeLocs(dir);

    ok &= DumpSQLStore(dir, sCreatureDataAddonStorage, 7);
    ok &= DumpSQLStore(dir, sCreatureDisplayInfoAddonStorage);
    ok &= DumpSQLStore(dir, sGameObjectDisplayInfoAddonStorage, -1, "gameobject_display_info_addon");
    ok &= DumpSQLStore(dir, sPageTextStore);
    ok &= DumpSQLStore(dir, sMapStorage);
    ok &= DumpSQLStore(dir, sConditionStorage);
    ok &= DumpSQLStore(dir, sAreaStorage);
    ok &= DumpSQLStore(dir, sMailTemplateStorage, -1, "mail_text_template");
    ok &= DumpSQLStore(dir, sCreatureSpellDataStorage, -1, "pet_spell_data");

    ok &= DumpSpells(dir);
    ok &= DumpSpellPairs(dir);
    ok &= DumpCreatureTemplates(dir);
    ok &= DumpItemPrototypes(dir);
    ok &= DumpGameObjectInfos(dir);
    ok &= StoreDumper::DumpQuests(dir);
    ok &= DumpFactions(dir);
    ok &= DumpFactionTemplates(dir);
    ok &= DumpTaxiNodes(dir);
    ok &= DumpSkillLineAbilities(dir);
    ok &= DumpSoundEntries(dir);

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, ok ? "Store dump done." : "Store dump failed.");
    return ok;
}
