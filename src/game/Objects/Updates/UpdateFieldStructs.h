#ifndef UPDATEFIELDSTRUCTS_H
#define UPDATEFIELDSTRUCTS_H

#include "UpdateField.h"
#include "UpdateFieldMask.h"
#include "EnumFlag.h"
#include "Server/Packets/Classic/Guid.h"
#include "Objects/GameObjectDefines.h"
#include <string>
#include <vector>

// The 1.15 client's update-field layout: one struct per object-data level (ObjectData, UnitData,
// PlayerData, ...), each a plain field list with a generated WriteCreate/WriteUpdate. Field types
// and bit positions are the wire format; nothing here computes a value (see ViewerDependentValues.h
// for the few fields that vary by viewer).

// A position with only some of x/y/z/o present on the wire, selected by Tag.
enum class PositionStreamTag { XY, XYZ, XYZO };
struct PositionXY   { static constexpr auto Tag = PositionStreamTag::XY; };
struct PositionXYZ  { static constexpr auto Tag = PositionStreamTag::XYZ; };
struct PositionXYZO { static constexpr auto Tag = PositionStreamTag::XYZO; };

template<typename Tag>
struct TaggedPosition
{
    constexpr TaggedPosition() { }
    constexpr TaggedPosition(float x, float y) : Pos_x(x), Pos_y(y) { }
    constexpr TaggedPosition(float x, float y, float z) : Pos_x(x), Pos_y(y), Pos_z(z) { }
    constexpr TaggedPosition(float x, float y, float z, float o) : Pos_x(x), Pos_y(y), Pos_z(z), Pos_o(o) { }

    bool operator==(TaggedPosition const&) const = default;

    float Pos_x = 0.0f;
    float Pos_y = 0.0f;
    float Pos_z = 0.0f;
    float Pos_o = 0.0f;
};

template<typename Tag>
ByteBuffer& operator<<(ByteBuffer& buf, TaggedPosition<Tag> const& pos)
{
    buf << pos.Pos_x;
    if constexpr (Tag::Tag != PositionStreamTag::XY)
        buf << pos.Pos_y;
    if constexpr (Tag::Tag == PositionStreamTag::XYZ || Tag::Tag == PositionStreamTag::XYZO)
        buf << pos.Pos_z;
    if constexpr (Tag::Tag == PositionStreamTag::XYZO)
        buf << pos.Pos_o;
    return buf;
}

template<typename Tag>
ByteBuffer& operator>>(ByteBuffer& buf, TaggedPosition<Tag>& pos)
{
    buf >> pos.Pos_x;
    if constexpr (Tag::Tag != PositionStreamTag::XY)
        buf >> pos.Pos_y;
    if constexpr (Tag::Tag == PositionStreamTag::XYZ || Tag::Tag == PositionStreamTag::XYZO)
        buf >> pos.Pos_z;
    if constexpr (Tag::Tag == PositionStreamTag::XYZO)
        buf >> pos.Pos_o;
    return buf;
}

ByteBuffer& operator<<(ByteBuffer& data, ClassicGuid const& guid);
ByteBuffer& operator>>(ByteBuffer& data, ClassicGuid& guid);
ByteBuffer& operator<<(ByteBuffer& data, QuaternionData const& quat);
ByteBuffer& operator>>(ByteBuffer& data, QuaternionData& quat);

// Item's bonus-list selector (item bonuses are not yet a vmangos concept; the field rides along
// empty until they are).
struct ItemBonusKey
{
    int32 ItemID = 0;
    std::vector<int32> BonusListIDs;

    bool operator==(ItemBonusKey const& right) const { return ItemID == right.ItemID && BonusListIDs == right.BonusListIDs; }
    bool operator!=(ItemBonusKey const& right) const { return !(*this == right); }
};

ByteBuffer& operator<<(ByteBuffer& data, ItemBonusKey const& itemBonusKey);
ByteBuffer& operator>>(ByteBuffer& data, ItemBonusKey& itemBonusKey);

// Retail Mythic+ score, unused by any vmangos content; kept so ActivePlayerData's layout matches
// the client's.
struct DungeonScoreMapSummary
{
    int32 ChallengeModeID = 0;
    float MapScore = 0.0f;
    int32 BestRunLevel = 0;
    int32 BestRunDurationMS = 0;
    bool FinishedSuccess = false;
};

struct DungeonScoreSummary
{
    float OverallScoreCurrentSeason = 0.0f;
    float LadderScoreCurrentSeason = 0.0f;
    std::vector<DungeonScoreMapSummary> Runs;

    bool operator==(DungeonScoreSummary const& right) const;
    bool operator!=(DungeonScoreSummary const& right) const { return !(*this == right); }
};

ByteBuffer& operator<<(ByteBuffer& data, DungeonScoreMapSummary const& run);
ByteBuffer& operator<<(ByteBuffer& data, DungeonScoreSummary const& summary);
ByteBuffer& operator>>(ByteBuffer& data, DungeonScoreSummary& summary);

// Retail Perks Program vendor entry, unused by any vmangos content; same reasoning as
// DungeonScoreSummary above.
struct PerksVendorItem
{
    int32 VendorItemID = 0;
    int32 MountID = 0;
    int32 BattlePetSpeciesID = 0;
    int32 TransmogSetID = 0;
    int32 ItemModifiedAppearanceID = 0;
    int32 Field_14 = 0;
    int32 Field_18 = 0;
    int32 Price = 0;
    int64 AvailableUntil = 0;
    bool Disabled = false;
    bool Field_41 = false;

    bool operator==(PerksVendorItem const& right) const;
    bool operator!=(PerksVendorItem const& right) const { return !(*this == right); }
};

ByteBuffer& operator<<(ByteBuffer& data, PerksVendorItem const& item);
ByteBuffer& operator>>(ByteBuffer& data, PerksVendorItem& item);

class AreaTrigger;
class Bag;
class ByteBuffer;
class Corpse;
class Creature;
class DynamicObject;
class GameObject;
class Item;
class Object;
class Player;
class Unit;

struct UFObjectData : public IsUpdateFieldStructureTag, public HasChangesMask<4>
{
    UpdateField<int32, 0, 1> EntryID;
    struct EntryIDTag : ViewerDependentValueTag<int32> {};
    UpdateField<uint32, 0, 2> DynamicFlags;
    struct DynamicFlagsTag : ViewerDependentValueTag<uint32> {};
    UpdateField<float, 0, 3> Scale;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Object const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Object const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Object const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFItemEnchantment : public IsUpdateFieldStructureTag, public HasChangesMask<5>
{
    UpdateField<int32, 0, 1> ID;
    UpdateField<uint32, 0, 2> Duration;
    UpdateField<int16, 0, 3> Charges;
    UpdateField<uint16, 0, 4> Inactive;

    void WriteCreate(ByteBuffer& data, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Item const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFItemMod : public IsUpdateFieldStructureTag
{
    uint8 Type;
    int32 Value;

    void WriteCreate(ByteBuffer& data, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Item const* owner, Player const* receiver) const;
    bool operator==(UFItemMod const& right) const;
    bool operator!=(UFItemMod const& right) const { return !(*this == right); }
};

struct UFItemModList : public IsUpdateFieldStructureTag, public HasChangesMask<1>
{
    DynamicUpdateField<UFItemMod, -1, 0> Values;

    void WriteCreate(ByteBuffer& data, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Item const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFArtifactPower : public IsUpdateFieldStructureTag
{
    int16 ArtifactPowerID;
    uint8 PurchasedRank;
    uint8 CurrentRankWithBonus;

    void WriteCreate(ByteBuffer& data, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Item const* owner, Player const* receiver) const;
    bool operator==(UFArtifactPower const& right) const;
    bool operator!=(UFArtifactPower const& right) const { return !(*this == right); }
};

struct UFSocketedGem : public IsUpdateFieldStructureTag, public HasChangesMask<20>
{
    UpdateField<int32, 0, 1> ItemID;
    UpdateField<uint8, 0, 2> Context;
    UpdateFieldArray<uint16, 16, 3, 4> BonusListIDs;

    void WriteCreate(ByteBuffer& data, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Item const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFItemData : public IsUpdateFieldStructureTag, public HasChangesMask<43>
{
    DynamicUpdateField<UFArtifactPower, 0, 1> ArtifactPowers;
    DynamicUpdateField<UFSocketedGem, 0, 2> Gems;
    UpdateField<ClassicGuid, 0, 3> Owner;
    UpdateField<ClassicGuid, 0, 4> ContainedIn;
    UpdateField<ClassicGuid, 0, 5> Creator;
    UpdateField<ClassicGuid, 0, 6> GiftCreator;
    UpdateField<uint32, 0, 7> StackCount;
    UpdateField<uint32, 0, 8> Expiration;
    UpdateField<uint32, 0, 9> DynamicFlags;
    UpdateField<int32, 0, 10> PropertySeed;
    UpdateField<int32, 0, 11> RandomPropertiesID;
    UpdateField<uint32, 0, 12> Durability;
    UpdateField<uint32, 0, 13> MaxDurability;
    UpdateField<uint32, 0, 14> CreatePlayedTime;
    UpdateField<uint8, 0, 15> Context;
    UpdateField<int64, 0, 16> CreateTime;
    UpdateField<uint64, 0, 17> ArtifactXP;
    UpdateField<uint8, 0, 18> ItemAppearanceModID;
    UpdateField<UFItemModList, 0, 19> Modifiers;
    UpdateField<uint32, 0, 20> ZoneFlags;
    UpdateField<ItemBonusKey, 0, 21> ItemBonusKey;
    UpdateField<uint16, 0, 22> DEBUGItemLevel;
    UpdateFieldArray<int32, 5, 23, 24> SpellCharges;
    UpdateFieldArray<UFItemEnchantment, 13, 29, 30> Enchantment;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Item const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Item const* owner, Player const* receiver) const;
    static void AppendAllowedFieldsMaskForFlag(Mask& allowedMaskForTarget, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    static void FilterDisallowedFieldsMaskForFlag(Mask& changesMask, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    void ClearChangesMask();
};

struct UFContainerData : public IsUpdateFieldStructureTag, public HasChangesMask<39>
{
    UpdateField<uint32, 0, 1> NumSlots;
    UpdateFieldArray<ClassicGuid, 36, 2, 3> Slots;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Bag const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Bag const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Bag const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFUnitChannel : public IsUpdateFieldStructureTag
{
    int32 SpellID;
    int32 SpellXSpellVisualID;

    void WriteCreate(ByteBuffer& data, Unit const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Unit const* owner, Player const* receiver) const;
    bool operator==(UFUnitChannel const& right) const;
    bool operator!=(UFUnitChannel const& right) const { return !(*this == right); }
};

struct UFVisibleItem : public IsUpdateFieldStructureTag, public HasChangesMask<4>
{
    UpdateField<int32, 0, 1> ItemID;
    UpdateField<uint16, 0, 2> ItemAppearanceModID;
    UpdateField<uint16, 0, 3> ItemVisual;

    void WriteCreate(ByteBuffer& data, Unit const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Unit const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFPassiveSpellHistory : public IsUpdateFieldStructureTag
{
    int32 SpellID;
    int32 AuraSpellID;

    void WriteCreate(ByteBuffer& data, Unit const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Unit const* owner, Player const* receiver) const;
    bool operator==(UFPassiveSpellHistory const& right) const;
    bool operator!=(UFPassiveSpellHistory const& right) const { return !(*this == right); }
};

struct UFUnitData : public IsUpdateFieldStructureTag, public HasChangesMask<228>
{
    UpdateField<std::vector<uint32>, 0, 1> StateWorldEffectIDs;
    struct StateWorldEffectIDsTag : ViewerDependentValueTag<std::vector<uint32>> {};
    DynamicUpdateField<UFPassiveSpellHistory, 0, 2> PassiveSpells;
    DynamicUpdateField<int32, 0, 3> WorldEffects;
    DynamicUpdateField<ClassicGuid, 0, 4> ChannelObjects;
    UpdateField<int64, 0, 5> Health;
    struct HealthTag : ViewerDependentValueTag<int64> {};
    UpdateField<int64, 0, 6> MaxHealth;
    struct MaxHealthTag : ViewerDependentValueTag<int64> {};
    UpdateField<int32, 0, 7> DisplayID;
    struct DisplayIDTag : ViewerDependentValueTag<int32> {};
    UpdateField<uint32, 0, 8> NpcFlags;
    struct NpcFlagsTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 9> NpcFlags2;
    struct NpcFlags2Tag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 10> StateSpellVisualID;
    struct StateSpellVisualIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 11> StateAnimID;
    struct StateAnimIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 12> StateAnimKitID;
    struct StateAnimKitIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<ClassicGuid, 0, 13> Charm;
    UpdateField<ClassicGuid, 0, 14> Summon;
    UpdateField<ClassicGuid, 0, 15> Critter;
    UpdateField<ClassicGuid, 0, 16> CharmedBy;
    UpdateField<ClassicGuid, 0, 17> SummonedBy;
    UpdateField<ClassicGuid, 0, 18> CreatedBy;
    UpdateField<ClassicGuid, 0, 19> DemonCreator;
    UpdateField<ClassicGuid, 0, 20> LookAtControllerTarget;
    UpdateField<ClassicGuid, 0, 21> Target;
    struct TargetTag : ViewerDependentValueTag<ClassicGuid> {};
    UpdateField<ClassicGuid, 0, 22> BattlePetCompanionGUID;
    UpdateField<uint64, 0, 23> BattlePetDBID;
    UpdateField<UFUnitChannel, 0, 24> ChannelData;
    UpdateField<uint32, 0, 25> SummonedByHomeRealm;
    UpdateField<uint8, 0, 26> Race;
    UpdateField<uint8, 0, 27> ClassId;
    UpdateField<uint8, 0, 28> PlayerClassId;
    UpdateField<uint8, 0, 29> Sex;
    UpdateField<uint8, 0, 30> DisplayPower;
    UpdateField<uint32, 0, 31> OverrideDisplayPowerID;
    UpdateField<int32, 32, 33> Level;
    UpdateField<int32, 32, 34> EffectiveLevel;
    UpdateField<int32, 32, 35> ContentTuningID;
    UpdateField<int32, 32, 36> ScalingLevelMin;
    UpdateField<int32, 32, 37> ScalingLevelMax;
    UpdateField<int32, 32, 38> ScalingLevelDelta;
    UpdateField<int32, 32, 39> ScalingFactionGroup;
    UpdateField<int32, 32, 40> FactionTemplate;
    struct FactionTemplateTag : ViewerDependentValueTag<int32> {};
    UpdateField<uint32, 32, 41> Flags;
    struct FlagsTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 32, 42> Flags2;
    struct Flags2Tag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 32, 43> Flags3;
    struct Flags3Tag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 32, 44> Flags4;
    struct Flags4Tag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 32, 45> AuraState;
    struct AuraStateTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 32, 46> RangedAttackRoundBaseTime;
    UpdateField<float, 32, 47> BoundingRadius;
    UpdateField<float, 32, 48> CombatReach;
    UpdateField<float, 32, 49> DisplayScale;
    UpdateField<int32, 32, 50> NativeDisplayID;
    UpdateField<float, 32, 51> NativeXDisplayScale;
    UpdateField<int32, 32, 52> MountDisplayID;
    UpdateField<float, 32, 53> MinDamage;
    UpdateField<float, 32, 54> MaxDamage;
    UpdateField<float, 32, 55> MinOffHandDamage;
    UpdateField<float, 32, 56> MaxOffHandDamage;
    UpdateField<uint8, 32, 57> StandState;
    UpdateField<uint8, 32, 58> PetTalentPoints;
    UpdateField<uint8, 32, 59> VisFlags;
    UpdateField<uint8, 32, 60> AnimTier;
    UpdateField<uint32, 32, 61> PetNumber;
    UpdateField<uint32, 32, 62> PetNameTimestamp;
    UpdateField<uint32, 32, 63> PetExperience;
    UpdateField<uint32, 64, 65> PetNextLevelExperience;
    UpdateField<float, 64, 66> ModCastingSpeed;
    UpdateField<float, 64, 67> ModSpellHaste;
    UpdateField<float, 64, 68> ModHaste;
    UpdateField<float, 64, 69> ModRangedHaste;
    UpdateField<float, 64, 70> ModHasteRegen;
    UpdateField<float, 64, 71> ModTimeRate;
    UpdateField<int32, 64, 72> CreatedBySpell;
    UpdateField<int32, 64, 73> EmoteState;
    UpdateField<int16, 64, 74> TrainingPointsUsed;
    UpdateField<int16, 64, 75> TrainingPointsTotal;
    UpdateField<int32, 64, 76> BaseMana;
    UpdateField<int32, 64, 77> BaseHealth;
    UpdateField<uint8, 64, 78> SheatheState;
    UpdateField<uint8, 64, 79> PvpFlags;
    struct PvpFlagsTag : ViewerDependentValueTag<uint8> {};
    UpdateField<uint8, 64, 80> PetFlags;
    UpdateField<uint8, 64, 81> ShapeshiftForm;
    UpdateField<int32, 64, 82> AttackPower;
    UpdateField<int32, 64, 83> AttackPowerModPos;
    UpdateField<int32, 64, 84> AttackPowerModNeg;
    UpdateField<float, 64, 85> AttackPowerMultiplier;
    UpdateField<int32, 64, 86> RangedAttackPower;
    UpdateField<int32, 64, 87> RangedAttackPowerModPos;
    UpdateField<int32, 64, 88> RangedAttackPowerModNeg;
    UpdateField<float, 64, 89> RangedAttackPowerMultiplier;
    UpdateField<int32, 64, 90> SetAttackSpeedAura;
    UpdateField<float, 64, 91> Lifesteal;
    UpdateField<float, 64, 92> MinRangedDamage;
    UpdateField<float, 64, 93> MaxRangedDamage;
    UpdateField<float, 64, 94> MaxHealthModifier;
    UpdateField<float, 64, 95> HoverHeight;
    UpdateField<int32, 96, 97> MinItemLevelCutoff;
    UpdateField<int32, 96, 98> MinItemLevel;
    UpdateField<int32, 96, 99> MaxItemLevel;
    UpdateField<int32, 96, 100> WildBattlePetLevel;
    UpdateField<uint32, 96, 101> BattlePetCompanionNameTimestamp;
    UpdateField<int32, 96, 102> InteractSpellID;
    struct InteractSpellIDTag : ViewerDependentValueTag<int32> {};
    UpdateField<int32, 96, 103> ScaleDuration;
    UpdateField<int32, 96, 104> LooksLikeMountID;
    UpdateField<int32, 96, 105> LooksLikeCreatureID;
    UpdateField<int32, 96, 106> LookAtControllerID;
    UpdateField<int32, 96, 107> PerksVendorItemID;
    UpdateField<ClassicGuid, 96, 108> GuildGUID;
    UpdateField<ClassicGuid, 96, 109> SkinningOwnerGUID;
    UpdateField<int32, 96, 110> FlightCapabilityID;
    UpdateField<float, 96, 111> GlideEventSpeedDivisor;                         // Movement speed gets divided by this value when evaluating what GlideEvents to use
    UpdateField<uint32, 96, 112> SilencedSchoolMask;
    UpdateField<uint32, 96, 113> CurrentAreaID;
    UpdateField<ClassicGuid, 96, 114> ComboTarget;
    UpdateField<float, 96, 115> Field_2F0;
    UpdateField<float, 96, 116> Field_2F4;
    UpdateFieldArray<float, 10, 117, 118> PowerRegenFlatModifier;
    UpdateFieldArray<float, 10, 117, 128> PowerRegenInterruptedFlatModifier;
    UpdateFieldArray<int32, 10, 117, 138> Power;
    UpdateFieldArray<int32, 10, 117, 148> MaxPower;
    UpdateFieldArray<float, 10, 117, 158> ModPowerRegen;                        // Applies to power regen only if expansion < 2, hidden from lua
    UpdateFieldArray<UFVisibleItem, 3, 168, 169> VirtualItems;
    UpdateFieldArray<uint32, 3, 172, 173> AttackRoundBaseTime;
    UpdateFieldArray<int32, 5, 176, 177> Stats;
    UpdateFieldArray<int32, 5, 176, 182> StatPosBuff;
    UpdateFieldArray<int32, 5, 176, 187> StatNegBuff;
    UpdateFieldArray<int32, 7, 192, 193> Resistances;
    UpdateFieldArray<int32, 7, 192, 200> ResistanceBuffModsPositive;
    UpdateFieldArray<int32, 7, 192, 207> ResistanceBuffModsNegative;
    UpdateFieldArray<int32, 7, 192, 214> PowerCostModifier;
    UpdateFieldArray<float, 7, 192, 221> PowerCostMultiplier;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Unit const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Unit const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Unit const* owner, Player const* receiver) const;
    static void AppendAllowedFieldsMaskForFlag(Mask& allowedMaskForTarget, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    static void FilterDisallowedFieldsMaskForFlag(Mask& changesMask, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    void ClearChangesMask();
};

struct UFChrCustomizationChoice : public IsUpdateFieldStructureTag
{
    uint32 ChrCustomizationOptionID;
    uint32 ChrCustomizationChoiceID;

    void WriteCreate(ByteBuffer& data, Object const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Object const* owner, Player const* receiver) const;
    bool operator==(UFChrCustomizationChoice const& right) const;
    bool operator!=(UFChrCustomizationChoice const& right) const { return !(*this == right); }
};

struct UFQuestLog : public IsUpdateFieldStructureTag, public HasChangesMask<29>
{
    UpdateField<int64, 0, 1> EndTime;
    UpdateField<int32, 0, 2> QuestID;
    UpdateField<uint32, 0, 3> StateFlags;
    UpdateFieldArray<uint16, 24, 4, 5> ObjectiveProgress;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFArenaCooldown : public IsUpdateFieldStructureTag, public HasChangesMask<9>
{
    UpdateField<int32, 0, 1> SpellID;
    UpdateField<int32, 0, 2> ItemID;
    UpdateField<int32, 0, 3> Charges;
    UpdateField<uint32, 0, 4> Flags;
    UpdateField<uint32, 0, 5> StartTime;
    UpdateField<uint32, 0, 6> EndTime;
    UpdateField<uint32, 0, 7> NextChargeTime;
    UpdateField<uint8, 0, 8> MaxCharges;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFZonePlayerForcedReaction : public IsUpdateFieldStructureTag, public HasChangesMask<3>
{
    UpdateField<int32, 0, 1> FactionID;
    UpdateField<int32, 0, 2> Reaction;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFDeclinedNames : public IsUpdateFieldStructureTag, public HasChangesMask<6>
{
    UpdateFieldArray<std::string, 5, 0, 1> Name;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFCustomTabardInfo : public IsUpdateFieldStructureTag, public HasChangesMask<5>
{
    UpdateField<int32, -1, 0> EmblemStyle;
    UpdateField<int32, -1, 1> EmblemColor;
    UpdateField<int32, -1, 2> BorderStyle;
    UpdateField<int32, -1, 3> BorderColor;
    UpdateField<int32, -1, 4> BackgroundColor;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFPlayerData : public IsUpdateFieldStructureTag, public HasChangesMask<148>
{
    DynamicUpdateField<UFChrCustomizationChoice, 0, 1> Customizations;
    DynamicUpdateField<UFArenaCooldown, 0, 2> ArenaCooldowns;
    DynamicUpdateField<int32, 0, 3> VisualItemReplacements;
    UpdateField<ClassicGuid, 0, 4> DuelArbiter;
    UpdateField<ClassicGuid, 0, 5> WowAccount;
    UpdateField<ClassicGuid, 0, 6> BnetAccount;
    UpdateField<uint64, 0, 7> GuildClubMemberID;
    UpdateField<ClassicGuid, 0, 8> LootTargetGUID;
    UpdateField<uint32, 0, 9> PlayerFlags;
    UpdateField<uint32, 0, 10> PlayerFlagsEx;
    UpdateField<uint32, 0, 11> GuildRankID;
    UpdateField<uint32, 0, 12> GuildDeleteDate;
    UpdateField<int32, 0, 13> GuildLevel;
    UpdateField<uint8, 0, 14> NumBankSlots;
    UpdateField<uint8, 0, 15> NativeSex;
    UpdateField<uint8, 0, 16> Inebriation;
    UpdateField<uint8, 0, 17> PvpTitle;
    UpdateField<uint8, 0, 18> ArenaFaction;
    UpdateField<uint8, 0, 19> PvpRank;
    UpdateField<int32, 0, 20> Field_88;
    UpdateField<uint32, 0, 21> DuelTeam;
    UpdateField<int32, 0, 22> GuildTimeStamp;
    UpdateField<int32, 0, 23> PlayerTitle;
    UpdateField<int32, 0, 24> FakeInebriation;
    UpdateField<uint32, 0, 25> VirtualPlayerRealm;
    UpdateField<uint32, 0, 26> CurrentSpecID;
    UpdateField<int32, 0, 27> TaxiMountAnimKitID;
    UpdateField<uint8, 0, 28> CurrentBattlePetBreedQuality;
    UpdateField<int32, 0, 29> HonorLevel;
    UpdateField<int64, 0, 30> LogoutTime;
    UpdateField<std::string, 0, 31> Name;
    UpdateField<int32, 32, 33> Field_13C;
    UpdateField<int32, 32, 34> Field_140;
    UpdateField<int32, 32, 35> CurrentBattlePetSpeciesID;
    UpdateField<DungeonScoreSummary, 32, 36> DungeonScore;
    OptionalUpdateField<UFDeclinedNames, 32, 37> DeclinedNames;
    UpdateField<UFCustomTabardInfo, 32, 38> PersonalTabard;
    UpdateFieldArray<uint8, 2, 39, 40> PartyType;
    UpdateFieldArray<UFQuestLog, 25, 42, 43> QuestLog;
    UpdateFieldArray<UFVisibleItem, 19, 68, 69> VisibleItems;
    UpdateFieldArray<float, 6, 88, 89> AvgItemLevel;
    UpdateFieldArray<UFZonePlayerForcedReaction, 32, 95, 96> ForcedReactions;
    UpdateFieldArray<uint32, 19, 128, 129> Field_3120;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Player const* owner, Player const* receiver) const;
    static void AppendAllowedFieldsMaskForFlag(Mask& allowedMaskForTarget, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    static void FilterDisallowedFieldsMaskForFlag(Mask& changesMask, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags);
    void ClearChangesMask();
    bool IsQuestLogChangesMaskSkipped() const { return false; } // bandwidth savings aren't worth the cpu time
};

struct UFSkillInfo : public IsUpdateFieldStructureTag, public HasChangesMask<1793>
{
    UpdateFieldArray<uint16, 256, 0, 1> SkillLineID;
    UpdateFieldArray<uint16, 256, 0, 257> SkillStep;
    UpdateFieldArray<uint16, 256, 0, 513> SkillRank;
    UpdateFieldArray<uint16, 256, 0, 769> SkillStartingRank;
    UpdateFieldArray<uint16, 256, 0, 1025> SkillMaxRank;
    UpdateFieldArray<int16, 256, 0, 1281> SkillTempBonus;
    UpdateFieldArray<uint16, 256, 0, 1537> SkillPermBonus;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFBitVector : public IsUpdateFieldStructureTag, public HasChangesMask<2>
{
    DynamicUpdateField<uint64, 0, 1> Values;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFBitVectors : public IsUpdateFieldStructureTag, public HasChangesMask<14>
{
    UpdateFieldArray<UFBitVector, 13, 0, 1> Values;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFPlayerDataElement : public IsUpdateFieldStructureTag
{
    uint32 Type;
    float FloatValue;
    int64 Int64Value;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFPlayerDataElement const& right) const;
    bool operator!=(UFPlayerDataElement const& right) const { return !(*this == right); }
};

struct UFRestInfo : public IsUpdateFieldStructureTag, public HasChangesMask<3>
{
    UpdateField<uint32, 0, 1> Threshold;
    UpdateField<uint8, 0, 2> StateID;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFPVPInfo : public IsUpdateFieldStructureTag, public HasChangesMask<19>
{
    UpdateField<bool, 0, 1> Disqualified;
    UpdateField<int8, 0, 2> Bracket;
    UpdateField<int32, 0, 3> PvpRatingID;
    UpdateField<uint32, 0, 4> WeeklyPlayed;
    UpdateField<uint32, 0, 5> WeeklyWon;
    UpdateField<uint32, 0, 6> SeasonPlayed;
    UpdateField<uint32, 0, 7> SeasonWon;
    UpdateField<uint32, 0, 8> Rating;
    UpdateField<uint32, 0, 9> WeeklyBestRating;
    UpdateField<uint32, 0, 10> SeasonBestRating;
    UpdateField<uint32, 0, 11> PvpTierID;
    UpdateField<uint32, 0, 12> WeeklyBestWinPvpTierID;
    UpdateField<uint32, 0, 13> Field_28;
    UpdateField<uint32, 0, 14> Field_2C;
    UpdateField<uint32, 0, 15> WeeklyRoundsPlayed;
    UpdateField<uint32, 0, 16> WeeklyRoundsWon;
    UpdateField<uint32, 0, 17> SeasonRoundsPlayed;
    UpdateField<uint32, 0, 18> SeasonRoundsWon;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFResearch : public IsUpdateFieldStructureTag
{
    int16 ResearchProjectID;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFResearch const& right) const;
    bool operator!=(UFResearch const& right) const { return !(*this == right); }
};

struct UFCharacterRestriction : public IsUpdateFieldStructureTag
{
    int32 Field_0;
    int32 Field_4;
    int32 Field_8;
    uint32 Type;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFCharacterRestriction const& right) const;
    bool operator!=(UFCharacterRestriction const& right) const { return !(*this == right); }
};

struct UFSpellPctModByLabel : public IsUpdateFieldStructureTag
{
    int32 ModIndex;
    float ModifierValue;
    int32 LabelID;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFSpellPctModByLabel const& right) const;
    bool operator!=(UFSpellPctModByLabel const& right) const { return !(*this == right); }
};

struct UFSpellFlatModByLabel : public IsUpdateFieldStructureTag
{
    int32 ModIndex;
    int32 ModifierValue;
    int32 LabelID;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFSpellFlatModByLabel const& right) const;
    bool operator!=(UFSpellFlatModByLabel const& right) const { return !(*this == right); }
};

struct UFCompletedProject : public IsUpdateFieldStructureTag, public HasChangesMask<4>
{
    UpdateField<uint32, 0, 1> ProjectID;
    UpdateField<int64, 0, 2> FirstCompleted;
    UpdateField<uint32, 0, 3> CompletionCount;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFResearchHistory : public IsUpdateFieldStructureTag, public HasChangesMask<2>
{
    DynamicUpdateField<UFCompletedProject, 0, 1> CompletedProjects;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFTraitEntry : public IsUpdateFieldStructureTag
{
    int32 TraitNodeID;
    int32 TraitNodeEntryID;
    int32 Rank;
    int32 GrantedRanks;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFTraitEntry const& right) const;
    bool operator!=(UFTraitEntry const& right) const { return !(*this == right); }
};

struct UFTraitSubTreeCache : public IsUpdateFieldStructureTag
{
    std::vector<UFTraitEntry> Entries;
    int32 TraitSubTreeID;
    uint32 Active;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFTraitSubTreeCache const& right) const;
    bool operator!=(UFTraitSubTreeCache const& right) const { return !(*this == right); }
};

struct UFTraitConfig : public IsUpdateFieldStructureTag, public HasChangesMask<14>
{
    DynamicUpdateField<UFTraitEntry, 0, 1> Entries;
    DynamicUpdateField<UFTraitSubTreeCache, 0, 2> SubTrees;
    UpdateField<int32, 0, 3> ID;
    UpdateField<std::string, 4, 5> Name;
    UpdateField<int32, 4, 6> Type;
    UpdateField<int32, 4, 7> SkillLineID;
    UpdateField<int32, 8, 9> ChrSpecializationID;
    UpdateField<int32, 8, 10> CombatConfigFlags;
    UpdateField<int32, 8, 11> LocalIdentifier;
    UpdateField<int32, 12, 13> TraitSystemID;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFCategoryCooldownMod : public IsUpdateFieldStructureTag
{
    int32 SpellCategoryID;
    int32 ModCooldown;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFCategoryCooldownMod const& right) const;
    bool operator!=(UFCategoryCooldownMod const& right) const { return !(*this == right); }
};

struct UFWeeklySpellUse : public IsUpdateFieldStructureTag
{
    int32 SpellCategoryID;
    uint8 Uses;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    bool operator==(UFWeeklySpellUse const& right) const;
    bool operator!=(UFWeeklySpellUse const& right) const { return !(*this == right); }
};

struct UFStablePetInfo : public IsUpdateFieldStructureTag, public HasChangesMask<9>
{
    UpdateField<uint32, 0, 1> PetSlot;
    UpdateField<uint32, 0, 2> PetNumber;
    UpdateField<uint32, 0, 3> CreatureID;
    UpdateField<uint32, 0, 4> DisplayID;
    UpdateField<uint32, 0, 5> ExperienceLevel;
    UpdateField<std::string, 0, 6> Name;
    UpdateField<uint8, 0, 7> PetFlags;
    UpdateField<uint8, 0, 8> Field_96;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFStableInfo : public IsUpdateFieldStructureTag, public HasChangesMask<3>
{
    DynamicUpdateField<UFStablePetInfo, 0, 1> Pets;
    UpdateField<ClassicGuid, 0, 2> StableMaster;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFBankTabSettings : public IsUpdateFieldStructureTag, public HasChangesMask<4>
{
    UpdateField<std::string, -1, 0> Name;
    UpdateField<std::string, -1, 1> Icon;
    UpdateField<std::string, -1, 2> Description;
    UpdateField<int32, -1, 3> DepositFlags;

    void WriteCreate(ByteBuffer& data, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFActivePlayerData : public IsUpdateFieldStructureTag, public HasChangesMask<1443>
{
    UpdateField<bool, 0, 1> SortBagsRightToLeft;
    UpdateField<bool, 0, 2> InsertItemsLeftToRight;
    UpdateFieldArray<DynamicUpdateFieldBase<uint16>, 1, 23, 24> ResearchSites;
    UpdateFieldArray<DynamicUpdateFieldBase<uint32>, 1, 25, 26> ResearchSiteProgress;
    UpdateFieldArray<DynamicUpdateFieldBase<UFResearch>, 1, 27, 28> Research;
    DynamicUpdateField<uint64, 0, 3> KnownTitles;
    DynamicUpdateField<int32, 0, 6> DailyQuestsCompleted;
    DynamicUpdateField<int32, 0, 7> AvailableQuestLineXQuestIDs;
    DynamicUpdateField<int32, 0, 8> Field_1000;
    DynamicUpdateField<int32, 0, 9> Heirlooms;
    DynamicUpdateField<uint32, 0, 10> HeirloomFlags;
    DynamicUpdateField<int32, 0, 11> Toys;
    DynamicUpdateField<uint32, 0, 12> Transmog;
    DynamicUpdateField<int32, 0, 13> ConditionalTransmog;
    DynamicUpdateField<int32, 0, 14> SelfResSpells;
    DynamicUpdateField<UFSpellPctModByLabel, 0, 16> SpellPctModByLabel;
    DynamicUpdateField<UFSpellFlatModByLabel, 0, 17> SpellFlatModByLabel;
    DynamicUpdateField<UFQuestLog, 0, 18> TaskQuests;
    DynamicUpdateField<UFCategoryCooldownMod, 0, 20> CategoryCooldownMods;
    DynamicUpdateField<UFWeeklySpellUse, 0, 21> WeeklySpellUses;
    DynamicUpdateField<UFPlayerDataElement, 0, 4> CharacterDataElements;
    DynamicUpdateField<UFPlayerDataElement, 0, 5> AccountDataElements;
    DynamicUpdateField<UFCharacterRestriction, 0, 15> CharacterRestrictions;
    DynamicUpdateField<UFTraitConfig, 0, 19> TraitConfigs;
    DynamicUpdateField<UFBankTabSettings, 0, 22> AccountBankTabSettings;
    UpdateField<ClassicGuid, 0, 29> FarsightObject;
    UpdateField<ClassicGuid, 0, 30> SummonedBattlePetGUID;
    UpdateField<uint64, 0, 31> Coinage;
    UpdateField<uint64, 0, 32> AccountBankCoinage;
    UpdateField<int32, 0, 33> XP;
    UpdateField<int32, 0, 34> NextLevelXP;
    UpdateField<int32, 0, 35> TrialXP;
    UpdateField<UFSkillInfo, 0, 36> Skill;
    UpdateField<int32, 0, 37> CharacterPoints;
    UpdateField<int32, 38, 39> MaxTalentTiers;
    UpdateField<uint32, 38, 40> TrackCreatureMask;
    UpdateField<float, 38, 41> MainhandExpertise;
    UpdateField<float, 38, 42> OffhandExpertise;
    UpdateField<float, 38, 43> RangedExpertise;
    UpdateField<float, 38, 44> CombatRatingExpertise;
    UpdateField<float, 38, 45> BlockPercentage;
    UpdateField<float, 38, 46> DodgePercentage;
    UpdateField<float, 38, 47> DodgePercentageFromAttribute;
    UpdateField<float, 38, 48> ParryPercentage;
    UpdateField<float, 38, 49> ParryPercentageFromAttribute;
    UpdateField<float, 38, 50> CritPercentage;
    UpdateField<float, 38, 51> RangedCritPercentage;
    UpdateField<float, 38, 52> OffhandCritPercentage;
    UpdateField<int32, 38, 53> ShieldBlock;
    UpdateField<float, 38, 54> ShieldBlockCritPercentage;
    UpdateField<float, 38, 55> Mastery;
    UpdateField<float, 38, 56> Speed;
    UpdateField<float, 38, 57> Avoidance;
    UpdateField<float, 38, 58> Sturdiness;
    UpdateField<int32, 38, 59> Versatility;
    UpdateField<float, 38, 60> VersatilityBonus;
    UpdateField<float, 38, 61> PvpPowerDamage;
    UpdateField<float, 38, 62> PvpPowerHealing;
    UpdateField<UFBitVectors, 38, 63> BitVectors;
    UpdateField<int32, 38, 64> ModHealingDonePos;
    UpdateField<float, 38, 65> ModHealingPercent;
    UpdateField<float, 38, 66> ModHealingDonePercent;
    UpdateField<float, 38, 67> ModPeriodicHealingDonePercent;
    UpdateField<float, 38, 68> ModSpellPowerPercent;
    UpdateField<float, 38, 69> ModResiliencePercent;
    UpdateField<float, 70, 71> OverrideSpellPowerByAPPercent;
    UpdateField<float, 70, 72> OverrideAPBySpellPowerPercent;
    UpdateField<int32, 70, 73> ModTargetResistance;
    UpdateField<int32, 70, 74> ModTargetPhysicalResistance;
    UpdateField<uint32, 70, 75> LocalFlags;
    UpdateField<uint8, 70, 76> GrantableLevels;
    UpdateField<uint8, 70, 77> MultiActionBars;
    UpdateField<uint8, 70, 78> LifetimeMaxRank;
    UpdateField<uint8, 70, 79> NumRespecs;
    UpdateField<int32, 70, 80> AmmoID;
    UpdateField<uint32, 70, 81> PvpMedals;
    UpdateField<uint16, 70, 82> TodayHonorableKills;
    UpdateField<uint16, 70, 83> TodayDishonorableKills;
    UpdateField<uint16, 70, 84> YesterdayHonorableKills;
    UpdateField<uint16, 70, 85> YesterdayDishonorableKills;
    UpdateField<uint16, 70, 86> LastWeekHonorableKills;
    UpdateField<uint16, 70, 87> LastWeekDishonorableKills;
    UpdateField<uint16, 70, 88> ThisWeekHonorableKills;
    UpdateField<uint16, 70, 89> ThisWeekDishonorableKills;
    UpdateField<uint32, 70, 90> ThisWeekContribution;
    UpdateField<uint32, 70, 91> LifetimeHonorableKills;
    UpdateField<uint32, 70, 92> LifetimeDishonorableKills;
    UpdateField<uint32, 70, 93> Field_F24;
    UpdateField<uint32, 70, 94> YesterdayContribution;
    UpdateField<uint32, 70, 95> LastWeekContribution;
    UpdateField<uint32, 70, 96> LastWeekRank;
    UpdateField<int32, 70, 97> WatchedFactionIndex;
    UpdateField<int32, 70, 98> MaxLevel;
    UpdateField<int32, 70, 99> ScalingPlayerLevelDelta;
    UpdateField<int32, 70, 100> MaxCreatureScalingLevel;
    UpdateField<int32, 70, 101> PetSpellPower;
    UpdateField<float, 102, 103> UiHitModifier;
    UpdateField<float, 102, 104> UiSpellHitModifier;
    UpdateField<int32, 102, 105> HomeRealmTimeOffset;
    UpdateField<float, 102, 106> ModPetHaste;
    UpdateField<uint8, 102, 107> LocalRegenFlags;
    UpdateField<uint8, 102, 108> AuraVision;
    UpdateField<uint8, 102, 109> NumBackpackSlots;
    UpdateField<int32, 102, 110> OverrideSpellsID;
    UpdateField<int32, 102, 111> LfgBonusFactionID;
    UpdateField<uint16, 102, 112> LootSpecID;
    UpdateField<uint32, 102, 113> OverrideZonePVPType;
    UpdateField<int32, 102, 114> Honor;
    UpdateField<int32, 102, 115> HonorNextLevel;
    UpdateField<int32, 102, 116> Field_F74;
    UpdateField<uint8, 102, 117> Field_1261;
    UpdateField<int32, 102, 118> PvpTierMaxFromWins;
    UpdateField<int32, 102, 119> PvpLastWeeksTierMaxFromWins;
    UpdateField<uint8, 102, 120> PvpRankProgress;
    UpdateField<int32, 102, 121> PerksProgramCurrency;
    UpdateField<UFResearchHistory, 102, 122> ResearchHistory;
    UpdateField<PerksVendorItem, 102, 123> FrozenPerksVendorItem;
    UpdateField<int32, 102, 124> TimerunningSeasonID;
    UpdateField<int32, 102, 125> TransportServerTime;
    UpdateField<uint32, 102, 126> ActiveCombatTraitConfigID;
    UpdateField<uint16, 102, 127> GlyphsEnabled;
    UpdateField<uint8, 102, 128> LfgRoles;
    OptionalUpdateField<UFStableInfo, 102, 129> PetStable;
    UpdateField<uint8, 102, 130> NumStableSlots;
    UpdateFieldArray<ClassicGuid, 146, 131, 132> InvSlots;
    UpdateFieldArray<uint32, 2, 278, 279> TrackResourceMask;
    UpdateFieldArray<float, 7, 281, 282> SpellCritPercentage;
    UpdateFieldArray<int32, 7, 281, 289> ModDamageDonePos;
    UpdateFieldArray<int32, 7, 281, 296> ModDamageDoneNeg;
    UpdateFieldArray<float, 7, 281, 303> ModDamageDonePercent;
    UpdateFieldArray<UFRestInfo, 2, 310, 311> RestInfo;
    UpdateFieldArray<float, 3, 313, 314> WeaponDmgMultipliers;
    UpdateFieldArray<float, 3, 313, 317> WeaponAtkSpeedMultipliers;
    UpdateFieldArray<uint32, 12, 320, 321> BuybackPrice;
    UpdateFieldArray<int64, 12, 320, 333> BuybackTimestamp;
    UpdateFieldArray<int32, 32, 345, 346> CombatRatings;
    UpdateFieldArray<UFPVPInfo, 9, 378, 379> PvpInfo;
    UpdateFieldArray<uint32, 4, 388, 389> NoReagentCostMask;
    UpdateFieldArray<int32, 2, 393, 394> ProfessionSkillLine;
    UpdateFieldArray<uint32, 4, 396, 397> BagSlotFlags;
    UpdateFieldArray<uint32, 7, 401, 402> BankBagSlotFlags;
    UpdateFieldArray<uint64, 1000, 409, 410> QuestCompleted;
    UpdateFieldArray<uint32, 9, 1410, 1411> GlyphSlots;
    UpdateFieldArray<uint32, 9, 1410, 1420> Glyphs;
    UpdateFieldArray<uint64, 13, 1429, 1430> Field_4348;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Player const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFGameObjectData : public IsUpdateFieldStructureTag, public HasChangesMask<20>
{
    UpdateField<std::vector<uint32>, 0, 1> StateWorldEffectIDs;
    struct StateWorldEffectIDsTag : ViewerDependentValueTag<std::vector<uint32>> {};
    DynamicUpdateField<int32, 0, 2> EnableDoodadSets;
    DynamicUpdateField<int32, 0, 3> WorldEffects;
    UpdateField<int32, 0, 4> DisplayID;
    UpdateField<uint32, 0, 5> SpellVisualID;
    UpdateField<uint32, 0, 6> StateSpellVisualID;
    struct StateSpellVisualIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 7> SpawnTrackingStateAnimID;
    struct SpawnTrackingStateAnimIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<uint32, 0, 8> SpawnTrackingStateAnimKitID;
    struct SpawnTrackingStateAnimKitIDTag : ViewerDependentValueTag<uint32> {};
    UpdateField<ClassicGuid, 0, 9> CreatedBy;
    UpdateField<ClassicGuid, 0, 10> GuildGUID;
    UpdateField<uint32, 0, 11> Flags;
    struct FlagsTag : ViewerDependentValueTag<uint32> {};
    UpdateField<QuaternionData, 0, 12> ParentRotation;
    UpdateField<int32, 0, 13> FactionTemplate;
    UpdateField<int32, 0, 14> Level;
    UpdateField<int8, 0, 15> State;
    struct StateTag : ViewerDependentValueTag<int8> {};
    UpdateField<int8, 0, 16> TypeID;
    UpdateField<uint8, 0, 17> PercentHealth;
    UpdateField<uint32, 0, 18> ArtKit;
    UpdateField<uint32, 0, 19> CustomParam;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, GameObject const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, GameObject const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, GameObject const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFDynamicObjectData : public IsUpdateFieldStructureTag, public HasChangesMask<7>
{
    UpdateField<ClassicGuid, 0, 1> Caster;
    UpdateField<uint8, 0, 2> Type;
    UpdateField<int32, 0, 3> SpellXSpellVisualID;
    UpdateField<int32, 0, 4> SpellID;
    UpdateField<float, 0, 5> Radius;
    UpdateField<uint32, 0, 6> CastTime;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, DynamicObject const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, DynamicObject const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, DynamicObject const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFCorpseData : public IsUpdateFieldStructureTag, public HasChangesMask<32>
{
    DynamicUpdateField<UFChrCustomizationChoice, 0, 1> Customizations;
    UpdateField<uint32, 0, 2> DynamicFlags;
    UpdateField<ClassicGuid, 0, 3> Owner;
    UpdateField<ClassicGuid, 0, 4> PartyGUID;
    UpdateField<ClassicGuid, 0, 5> GuildGUID;
    UpdateField<uint32, 0, 6> DisplayID;
    UpdateField<uint8, 0, 7> RaceID;
    UpdateField<uint8, 0, 8> Sex;
    UpdateField<uint8, 0, 9> Class;
    UpdateField<uint32, 0, 10> Flags;
    UpdateField<int32, 0, 11> FactionTemplate;
    UpdateFieldArray<uint32, 19, 12, 13> Items;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Corpse const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Corpse const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Corpse const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFScaleCurve : public IsUpdateFieldStructureTag, public HasChangesMask<7>
{
    UpdateField<bool, 0, 1> OverrideActive;
    UpdateField<uint32, 0, 2> StartTimeOffset;
    UpdateField<uint32, 0, 3> ParameterCurve;
    UpdateFieldArray<TaggedPosition<PositionXY>, 2, 4, 5> Points;

    void WriteCreate(ByteBuffer& data, AreaTrigger const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, AreaTrigger const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFVisualAnim : public IsUpdateFieldStructureTag, public HasChangesMask<5>
{
    UpdateField<bool, 0, 1> IsDecay;
    UpdateField<uint32, 0, 2> AnimationDataID;
    UpdateField<uint32, 0, 3> AnimKitID;
    UpdateField<uint32, 0, 4> AnimProgress;

    void WriteCreate(ByteBuffer& data, AreaTrigger const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, bool ignoreChangesMask, AreaTrigger const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFAreaTriggerData : public IsUpdateFieldStructureTag, public HasChangesMask<20>
{
    UpdateField<UFScaleCurve, 0, 1> OverrideScaleCurve;
    UpdateField<UFScaleCurve, 0, 2> ExtraScaleCurve;
    UpdateField<UFScaleCurve, 0, 3> OverrideMoveCurveX;
    UpdateField<UFScaleCurve, 0, 4> OverrideMoveCurveY;
    UpdateField<UFScaleCurve, 0, 5> OverrideMoveCurveZ;
    UpdateField<ClassicGuid, 0, 6> Caster;
    UpdateField<uint32, 0, 7> Duration;
    UpdateField<uint32, 0, 8> TimeToTarget;
    UpdateField<uint32, 0, 9> TimeToTargetScale;
    UpdateField<uint32, 0, 10> TimeToTargetExtraScale;
    UpdateField<uint32, 0, 11> TimeToTargetPos;                                 // Linked to m_overrideMoveCurve
    UpdateField<int32, 0, 12> SpellID;
    UpdateField<int32, 0, 13> SpellForVisuals;
    UpdateField<int32, 0, 14> SpellXSpellVisualID;
    UpdateField<float, 0, 15> BoundsRadius2D;
    UpdateField<uint32, 0, 16> DecalPropertiesID;
    UpdateField<ClassicGuid, 0, 17> CreatingEffectGUID;
    UpdateField<ClassicGuid, 0, 18> OrbitPathTarget;
    UpdateField<UFVisualAnim, 0, 19> VisualAnim;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, AreaTrigger const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, AreaTrigger const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, AreaTrigger const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFSceneObjectData : public IsUpdateFieldStructureTag, public HasChangesMask<5>
{
    UpdateField<int32, 0, 1> ScriptPackageID;
    UpdateField<uint32, 0, 2> RndSeedVal;
    UpdateField<ClassicGuid, 0, 3> CreatedBy;
    UpdateField<uint32, 0, 4> SceneType;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Object const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Object const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Object const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

struct UFVendorData : public IsUpdateFieldStructureTag, public HasChangesMask<2>
{
    UpdateField<int32, 0, 1> Flags;

    void WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Creature const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Creature const* owner, Player const* receiver) const;
    void WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Creature const* owner, Player const* receiver) const;
    void ClearChangesMask();
};

#endif // UPDATEFIELDSTRUCTS_H
