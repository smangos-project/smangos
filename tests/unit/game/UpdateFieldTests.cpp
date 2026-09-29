#include "Objects/Updates/UpdateFieldStructs.h"
#include "Objects/Updates/ViewerDependentValues.h"
#include "ByteBuffer.h"

#include <catch_amalgamated.hpp>

namespace
{
template<typename T, typename V, int32 BlockBit, uint32 Bit>
void Fill(T& holder, UpdateField<V, BlockBit, Bit>(T::* field), V value)
{
    auto setter = MutableFieldReference<T, false>(holder).ModifyValue(field);
    SetUpdateFieldValue(setter, std::move(value));
}

template<typename T, typename V, std::size_t Size, uint32 Bit, int32 FirstElementBit>
void Fill(T& holder, UpdateFieldArray<V, Size, Bit, FirstElementBit>(T::* field), uint32 index, V value)
{
    auto setter = MutableFieldReference<T, false>(holder).ModifyValue(field, index);
    SetUpdateFieldValue(setter, std::move(value));
}
}

TEST_CASE("An item enchantment's create block is its fields in order", "[updatefields]")
{
    UFItemEnchantment enchant;
    Fill(enchant, &UFItemEnchantment::ID, 111);
    Fill(enchant, &UFItemEnchantment::Duration, 222u);
    Fill(enchant, &UFItemEnchantment::Charges, int16(7));
    Fill(enchant, &UFItemEnchantment::Inactive, uint16(9));

    ByteBuffer data;
    enchant.WriteCreate(data, nullptr, nullptr);

    int32 id; uint32 duration; int16 charges; uint16 inactive;
    data >> id >> duration >> charges >> inactive;
    CHECK(id == 111);
    CHECK(duration == 222u);
    CHECK(charges == 7);
    CHECK(inactive == 9);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("An item enchantment's update block sends only the changed fields", "[updatefields]")
{
    UFItemEnchantment enchant;
    Fill(enchant, &UFItemEnchantment::ID, 111);
    Fill(enchant, &UFItemEnchantment::Duration, 222u);
    Fill(enchant, &UFItemEnchantment::Charges, int16(7));
    Fill(enchant, &UFItemEnchantment::Inactive, uint16(9));
    enchant.ClearChangesMask();

    Fill(enchant, &UFItemEnchantment::Charges, int16(42));

    ByteBuffer data;
    enchant.WriteUpdate(data, false, nullptr, nullptr);

    uint32 block = data.ReadBits(5);
    data.ResetBitPos();
    CHECK(block == ((1u << 0) | (1u << 3))); // block-active bit + Charges' own bit

    int16 charges;
    data >> charges;
    CHECK(charges == 42);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("Ignoring the change mask sends every field regardless of what changed", "[updatefields]")
{
    UFItemEnchantment enchant;
    Fill(enchant, &UFItemEnchantment::ID, 111);
    Fill(enchant, &UFItemEnchantment::Duration, 222u);
    Fill(enchant, &UFItemEnchantment::Charges, int16(7));
    Fill(enchant, &UFItemEnchantment::Inactive, uint16(9));
    enchant.ClearChangesMask();
    Fill(enchant, &UFItemEnchantment::Charges, int16(42));

    ByteBuffer data;
    enchant.WriteUpdate(data, true, nullptr, nullptr);

    uint32 block = data.ReadBits(5);
    data.ResetBitPos();
    CHECK(block == 0b11111u);

    int32 id; uint32 duration; int16 charges; uint16 inactive;
    data >> id >> duration >> charges >> inactive;
    CHECK(id == 111);
    CHECK(duration == 222u);
    CHECK(charges == 42);
    CHECK(inactive == 9);
}

TEST_CASE("An object's viewer-dependent fields fall through to the plain value", "[updatefields]")
{
    UFObjectData objectData;
    Fill(objectData, &UFObjectData::EntryID, 555);
    Fill(objectData, &UFObjectData::DynamicFlags, 2u);
    Fill(objectData, &UFObjectData::Scale, 2.5f);

    ByteBuffer data;
    objectData.WriteCreate(data, EnumFlag<UpdateFieldFlag>(UpdateFieldFlag::None), nullptr, nullptr);

    int32 entry; uint32 dynamicFlags; float scale;
    data >> entry >> dynamicFlags >> scale;
    CHECK(entry == 555);
    CHECK(dynamicFlags == 2u);
    CHECK(scale == 2.5f);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("An object's update block only carries what changed", "[updatefields]")
{
    UFObjectData objectData;
    Fill(objectData, &UFObjectData::EntryID, 555);
    Fill(objectData, &UFObjectData::DynamicFlags, 2u);
    Fill(objectData, &UFObjectData::Scale, 2.5f);
    objectData.ClearChangesMask();

    Fill(objectData, &UFObjectData::Scale, 9.0f);

    ByteBuffer data;
    objectData.WriteUpdate(data, EnumFlag<UpdateFieldFlag>(UpdateFieldFlag::None), nullptr, nullptr);

    uint32 block = data.ReadBits(4);
    data.ResetBitPos();
    CHECK(block == ((1u << 0) | (1u << 3))); // block-active bit + Scale's own bit

    float scale;
    data >> scale;
    CHECK(scale == 9.0f);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("An item's create block covers arrays, a dynamic field and a nested struct", "[updatefields]")
{
    UFItemData itemData;
    Fill(itemData, &UFItemData::StackCount, 100u);
    Fill(itemData, &UFItemData::SpellCharges, 2u, 777);

    auto powersSetter = MutableFieldReference<UFItemData, false>(itemData).ModifyValue(&UFItemData::ArtifactPowers);
    UFArtifactPower& power = AddDynamicUpdateFieldValue(powersSetter);
    power.ArtifactPowerID = 42;
    power.PurchasedRank = 3;
    power.CurrentRankWithBonus = 5;

    auto enchantRef = MutableFieldReference<UFItemData, false>(itemData).ModifyValue(&UFItemData::Enchantment, 0);
    auto enchantIdSetter = enchantRef.ModifyValue(&UFItemEnchantment::ID);
    SetUpdateFieldValue(enchantIdSetter, 999);

    ByteBuffer data;
    itemData.WriteCreate(data, EnumFlag<UpdateFieldFlag>(UpdateFieldFlag::Owner), nullptr, nullptr);

    ClassicGuid guid;
    for (int i = 0; i < 4; ++i) // Owner, ContainedIn, Creator, GiftCreator
    {
        REQUIRE(guid.Read(data));
        CHECK(guid.IsEmpty());
    }

    uint32 stackCount, expiration;
    data >> stackCount >> expiration;
    CHECK(stackCount == 100u);
    CHECK(expiration == 0u);

    for (uint32 i = 0; i < 5; ++i)
    {
        int32 charge;
        data >> charge;
        CHECK(charge == (i == 2 ? 777 : 0));
    }

    uint32 dynamicFlags;
    data >> dynamicFlags;
    CHECK(dynamicFlags == 0u);

    for (uint32 i = 0; i < 13; ++i)
    {
        int32 id; uint32 duration; int16 charges; uint16 inactive;
        data >> id >> duration >> charges >> inactive;
        CHECK(id == (i == 0 ? 999 : 0));
        CHECK(duration == 0u);
        CHECK(charges == 0);
        CHECK(inactive == 0u);
    }

    int32 propertySeed, randomPropertiesId;
    data >> propertySeed >> randomPropertiesId;
    CHECK(propertySeed == 0);
    CHECK(randomPropertiesId == 0);

    uint32 durability, maxDurability;
    data >> durability >> maxDurability;
    CHECK(durability == 0u);
    CHECK(maxDurability == 0u);

    uint32 createPlayedTime;
    uint8 context;
    int64 createTime;
    data >> createPlayedTime >> context >> createTime;
    CHECK(createPlayedTime == 0u);
    CHECK(context == 0);
    CHECK(createTime == 0);

    uint64 artifactXp;
    uint8 itemAppearanceModId;
    data >> artifactXp >> itemAppearanceModId;
    CHECK(artifactXp == 0u);
    CHECK(itemAppearanceModId == 0);

    uint32 artifactPowerCount, gemCount;
    data >> artifactPowerCount >> gemCount;
    CHECK(artifactPowerCount == 1u);
    CHECK(gemCount == 0u);

    uint32 zoneFlags;
    data >> zoneFlags;
    CHECK(zoneFlags == 0u);

    ItemBonusKey bonusKey;
    data >> bonusKey;
    CHECK(bonusKey.ItemID == 0);
    CHECK(bonusKey.BonusListIDs.empty());

    uint16 debugItemLevel;
    data >> debugItemLevel;
    CHECK(debugItemLevel == 0u);

    int16 powerId; uint8 purchasedRank, currentRankWithBonus;
    data >> powerId >> purchasedRank >> currentRankWithBonus;
    CHECK(powerId == 42);
    CHECK(purchasedRank == 3);
    CHECK(currentRankWithBonus == 5);

    uint32 modifierCount = data.ReadBits(6);
    data.ResetBitPos();
    CHECK(modifierCount == 0u);

    CHECK(data.rpos() == data.size());
}

TEST_CASE("An item's owner-only fields drop out of another viewer's create block", "[updatefields]")
{
    UFItemData itemData;
    Fill(itemData, &UFItemData::StackCount, 100u);

    ByteBuffer ownerView;
    itemData.WriteCreate(ownerView, EnumFlag<UpdateFieldFlag>(UpdateFieldFlag::Owner), nullptr, nullptr);

    ByteBuffer otherView;
    itemData.WriteCreate(otherView, EnumFlag<UpdateFieldFlag>(UpdateFieldFlag::None), nullptr, nullptr);

    // StackCount(4) + Expiration(4) + SpellCharges[5](20) + Durability(4) + MaxDurability(4) +
    // ArtifactXP(8) + ItemAppearanceModID(1) + ZoneFlags(4) + DEBUGItemLevel(2) = 51 owner-only bytes.
    CHECK(ownerView.size() - otherView.size() == 51);
}

TEST_CASE("A customization list can be rewritten in a corpse's update field", "[updatefields]")
{
    UFCorpseData corpseData;
    auto setter = MutableFieldReference<UFCorpseData, false>(corpseData).ModifyValue(&UFCorpseData::Customizations);
    for (uint32 i = 1; i <= 2; ++i)
    {
        UFChrCustomizationChoice& choice = AddDynamicUpdateFieldValue(setter);
        choice.ChrCustomizationOptionID = i;
        choice.ChrCustomizationChoiceID = i * 100;
    }
    REQUIRE(corpseData.Customizations.size() == 2);
    CHECK(corpseData.Customizations[1].ChrCustomizationChoiceID == 200);

    ByteBuffer data;
    corpseData.Customizations[1].WriteCreate(data, nullptr, nullptr);
    uint32 option, choice;
    data >> option >> choice;
    CHECK(option == 2);
    CHECK(choice == 200);

    ClearDynamicUpdateFieldValues(setter);
    CHECK(corpseData.Customizations.size() == 0);
}
