#include "UpdateFieldStructs.h"
#include "ByteBuffer.h"
#include "ViewerDependentValues.h"

// PlayerData and CorpseData both nest structs (VisibleItem, ChrCustomizationChoice) whose
// WriteCreate/WriteUpdate take a less-derived owner type (Unit, Object); the implicit upcast at
// each call site needs Player and Corpse's real inheritance chain, so this file (unlike
// UpdateFieldStructs.cpp) is not header-light.
#include "Objects/Player.h"
#include "Objects/Corpse.h"

void UFPlayerData::WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const
{
    data << DuelArbiter;
    data << WowAccount;
    data << BnetAccount;
    data << uint64(GuildClubMemberID);
    data << LootTargetGUID;
    data << uint32(PlayerFlags);
    data << uint32(PlayerFlagsEx);
    data << uint32(GuildRankID);
    data << uint32(GuildDeleteDate);
    data << int32(GuildLevel);
    data << uint32(Customizations.size());
    for (uint32 i = 0; i < 2; ++i)
    {
        data << uint8(PartyType[i]);
    }
    data << uint8(NumBankSlots);
    data << uint8(NativeSex);
    data << uint8(Inebriation);
    data << uint8(PvpTitle);
    data << uint8(ArenaFaction);
    data << uint8(PvpRank);
    data << int32(Field_88);
    data << uint32(DuelTeam);
    data << int32(GuildTimeStamp);
    if (fieldVisibilityFlags.HasFlag(UpdateFieldFlag::PartyMember))
    {
        for (uint32 i = 0; i < 25; ++i)
        {
            QuestLog[i].WriteCreate(data, owner, receiver);
        }
    }
    for (uint32 i = 0; i < 19; ++i)
    {
        VisibleItems[i].WriteCreate(data, owner, receiver);
    }
    data << int32(PlayerTitle);
    data << int32(FakeInebriation);
    data << uint32(VirtualPlayerRealm);
    data << uint32(CurrentSpecID);
    data << int32(TaxiMountAnimKitID);
    for (uint32 i = 0; i < 6; ++i)
    {
        data << float(AvgItemLevel[i]);
    }
    data << uint8(CurrentBattlePetBreedQuality);
    data << int32(HonorLevel);
    data << int64(LogoutTime);
    data << uint32(ArenaCooldowns.size());
    for (uint32 i = 0; i < 32; ++i)
    {
        ForcedReactions[i].WriteCreate(data, owner, receiver);
    }
    data << int32(Field_13C);
    data << int32(Field_140);
    data << int32(CurrentBattlePetSpeciesID);
    data << uint32(VisualItemReplacements.size());
    for (uint32 i = 0; i < 19; ++i)
    {
        data << uint32(Field_3120[i]);
    }
    PersonalTabard->WriteCreate(data, owner, receiver);
    for (uint32 i = 0; i < Customizations.size(); ++i)
    {
        Customizations[i].WriteCreate(data, owner, receiver);
    }
    for (uint32 i = 0; i < ArenaCooldowns.size(); ++i)
    {
        ArenaCooldowns[i].WriteCreate(data, owner, receiver);
    }
    for (uint32 i = 0; i < VisualItemReplacements.size(); ++i)
    {
        data << int32(VisualItemReplacements[i]);
    }
    data.WriteBits(Name->size(), 6);
    data.WriteBits(DeclinedNames.has_value(), 1);
    data << DungeonScore;
    data.WriteString(Name);
    if (DeclinedNames.has_value())
    {
        DeclinedNames->WriteCreate(data, owner, receiver);
    }
    data.FlushBits();
}

void UFPlayerData::WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Player const* owner, Player const* receiver) const
{
    Mask allowedMaskForTarget({ 0xFFFFFFFFu, 0x000003FFu, 0xFFFFFFF0u, 0xFFFFFFFFu, 0x000FFFFFu });
    AppendAllowedFieldsMaskForFlag(allowedMaskForTarget, fieldVisibilityFlags);
    WriteUpdate(data, _changesMask & allowedMaskForTarget, false, owner, receiver);
}

void UFPlayerData::AppendAllowedFieldsMaskForFlag(Mask& allowedMaskForTarget, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags)
{
    if (fieldVisibilityFlags.HasFlag(UpdateFieldFlag::PartyMember))
        allowedMaskForTarget |= std::array<uint32, 5>{ 0x00000000u, 0xFFFFFC00u, 0x0000000Fu, 0x00000000u, 0x00000000u };
}

void UFPlayerData::FilterDisallowedFieldsMaskForFlag(Mask& changesMask, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags)
{
    Mask allowedMaskForTarget({ 0xFFFFFFFFu, 0x000003FFu, 0xFFFFFFF0u, 0xFFFFFFFFu, 0x000FFFFFu });
    AppendAllowedFieldsMaskForFlag(allowedMaskForTarget, fieldVisibilityFlags);
    changesMask &= allowedMaskForTarget;
}

void UFPlayerData::WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Player const* owner, Player const* receiver) const
{
    data.WriteBits(changesMask.GetBlocksMask(0), 5);
    for (uint32 i = 0; i < 5; ++i)
        if (changesMask.GetBlock(i))
            data.WriteBits(changesMask.GetBlock(i), 32);

    bool noQuestLogChangesMask = IsQuestLogChangesMaskSkipped();
    data.WriteBit(noQuestLogChangesMask);
    if (changesMask[0])
    {
        if (changesMask[1])
        {
            if (!ignoreNestedChangesMask)
                Customizations.WriteUpdateMask(data);
            else
                WriteCompleteDynamicFieldUpdateMask(Customizations.size(), data);
        }
        if (changesMask[2])
        {
            if (!ignoreNestedChangesMask)
                ArenaCooldowns.WriteUpdateMask(data);
            else
                WriteCompleteDynamicFieldUpdateMask(ArenaCooldowns.size(), data);
        }
        if (changesMask[3])
        {
            if (!ignoreNestedChangesMask)
                VisualItemReplacements.WriteUpdateMask(data);
            else
                WriteCompleteDynamicFieldUpdateMask(VisualItemReplacements.size(), data);
        }
    }
    data.FlushBits();
    if (changesMask[0])
    {
        if (changesMask[1])
        {
            for (uint32 i = 0; i < Customizations.size(); ++i)
            {
                if (Customizations.HasChanged(i) || ignoreNestedChangesMask)
                {
                    Customizations[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
                }
            }
        }
        if (changesMask[2])
        {
            for (uint32 i = 0; i < ArenaCooldowns.size(); ++i)
            {
                if (ArenaCooldowns.HasChanged(i) || ignoreNestedChangesMask)
                {
                    ArenaCooldowns[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
                }
            }
        }
        if (changesMask[3])
        {
            for (uint32 i = 0; i < VisualItemReplacements.size(); ++i)
            {
                if (VisualItemReplacements.HasChanged(i) || ignoreNestedChangesMask)
                {
                    data << int32(VisualItemReplacements[i]);
                }
            }
        }
        if (changesMask[4])
        {
            data << DuelArbiter;
        }
        if (changesMask[5])
        {
            data << WowAccount;
        }
        if (changesMask[6])
        {
            data << BnetAccount;
        }
        if (changesMask[7])
        {
            data << uint64(GuildClubMemberID);
        }
        if (changesMask[8])
        {
            data << LootTargetGUID;
        }
        if (changesMask[9])
        {
            data << uint32(PlayerFlags);
        }
        if (changesMask[10])
        {
            data << uint32(PlayerFlagsEx);
        }
        if (changesMask[11])
        {
            data << uint32(GuildRankID);
        }
        if (changesMask[12])
        {
            data << uint32(GuildDeleteDate);
        }
        if (changesMask[13])
        {
            data << int32(GuildLevel);
        }
        if (changesMask[14])
        {
            data << uint8(NumBankSlots);
        }
        if (changesMask[15])
        {
            data << uint8(NativeSex);
        }
        if (changesMask[16])
        {
            data << uint8(Inebriation);
        }
        if (changesMask[17])
        {
            data << uint8(PvpTitle);
        }
        if (changesMask[18])
        {
            data << uint8(ArenaFaction);
        }
        if (changesMask[19])
        {
            data << uint8(PvpRank);
        }
        if (changesMask[20])
        {
            data << int32(Field_88);
        }
        if (changesMask[21])
        {
            data << uint32(DuelTeam);
        }
        if (changesMask[22])
        {
            data << int32(GuildTimeStamp);
        }
        if (changesMask[23])
        {
            data << int32(PlayerTitle);
        }
        if (changesMask[24])
        {
            data << int32(FakeInebriation);
        }
        if (changesMask[25])
        {
            data << uint32(VirtualPlayerRealm);
        }
        if (changesMask[26])
        {
            data << uint32(CurrentSpecID);
        }
        if (changesMask[27])
        {
            data << int32(TaxiMountAnimKitID);
        }
        if (changesMask[28])
        {
            data << uint8(CurrentBattlePetBreedQuality);
        }
        if (changesMask[29])
        {
            data << int32(HonorLevel);
        }
        if (changesMask[30])
        {
            data << int64(LogoutTime);
        }
    }
    if (changesMask[32])
    {
        if (changesMask[33])
        {
            data << int32(Field_13C);
        }
        if (changesMask[34])
        {
            data << int32(Field_140);
        }
        if (changesMask[35])
        {
            data << int32(CurrentBattlePetSpeciesID);
        }
        if (changesMask[38])
        {
            PersonalTabard->WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
        }
    }
    if (changesMask[0])
    {
        if (changesMask[31])
        {
            data.WriteBits(Name->size(), 6);
        }
    }
    if (changesMask[32])
    {
        data.WriteBits(DeclinedNames.has_value(), 1);
        data.FlushBits();
        if (changesMask[36])
        {
            data << DungeonScore;
        }
    }
    if (changesMask[0])
    {
        if (changesMask[31])
        {
            data.WriteString(Name);
        }
    }
    if (changesMask[32])
    {
        if (changesMask[37])
        {
            if (DeclinedNames.has_value())
            {
                DeclinedNames->WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
            }
        }
    }
    if (changesMask[39])
    {
        for (uint32 i = 0; i < 2; ++i)
        {
            if (changesMask[40 + i])
            {
                data << uint8(PartyType[i]);
            }
        }
    }
    if (changesMask[42])
    {
        for (uint32 i = 0; i < 25; ++i)
        {
            if (changesMask[43 + i])
            {
                if (noQuestLogChangesMask)
                    QuestLog[i].WriteCreate(data, owner, receiver);
                else
                    QuestLog[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
            }
        }
    }
    if (changesMask[68])
    {
        for (uint32 i = 0; i < 19; ++i)
        {
            if (changesMask[69 + i])
            {
                VisibleItems[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
            }
        }
    }
    if (changesMask[88])
    {
        for (uint32 i = 0; i < 6; ++i)
        {
            if (changesMask[89 + i])
            {
                data << float(AvgItemLevel[i]);
            }
        }
    }
    if (changesMask[95])
    {
        for (uint32 i = 0; i < 32; ++i)
        {
            if (changesMask[96 + i])
            {
                ForcedReactions[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
            }
        }
    }
    if (changesMask[128])
    {
        for (uint32 i = 0; i < 19; ++i)
        {
            if (changesMask[129 + i])
            {
                data << uint32(Field_3120[i]);
            }
        }
    }
    data.FlushBits();
}

void UFPlayerData::ClearChangesMask()
{
    Base::ClearChangesMask(Customizations);
    Base::ClearChangesMask(ArenaCooldowns);
    Base::ClearChangesMask(VisualItemReplacements);
    Base::ClearChangesMask(DuelArbiter);
    Base::ClearChangesMask(WowAccount);
    Base::ClearChangesMask(BnetAccount);
    Base::ClearChangesMask(GuildClubMemberID);
    Base::ClearChangesMask(LootTargetGUID);
    Base::ClearChangesMask(PlayerFlags);
    Base::ClearChangesMask(PlayerFlagsEx);
    Base::ClearChangesMask(GuildRankID);
    Base::ClearChangesMask(GuildDeleteDate);
    Base::ClearChangesMask(GuildLevel);
    Base::ClearChangesMask(NumBankSlots);
    Base::ClearChangesMask(NativeSex);
    Base::ClearChangesMask(Inebriation);
    Base::ClearChangesMask(PvpTitle);
    Base::ClearChangesMask(ArenaFaction);
    Base::ClearChangesMask(PvpRank);
    Base::ClearChangesMask(Field_88);
    Base::ClearChangesMask(DuelTeam);
    Base::ClearChangesMask(GuildTimeStamp);
    Base::ClearChangesMask(PlayerTitle);
    Base::ClearChangesMask(FakeInebriation);
    Base::ClearChangesMask(VirtualPlayerRealm);
    Base::ClearChangesMask(CurrentSpecID);
    Base::ClearChangesMask(TaxiMountAnimKitID);
    Base::ClearChangesMask(CurrentBattlePetBreedQuality);
    Base::ClearChangesMask(HonorLevel);
    Base::ClearChangesMask(LogoutTime);
    Base::ClearChangesMask(Name);
    Base::ClearChangesMask(Field_13C);
    Base::ClearChangesMask(Field_140);
    Base::ClearChangesMask(CurrentBattlePetSpeciesID);
    Base::ClearChangesMask(DungeonScore);
    Base::ClearChangesMask(DeclinedNames);
    Base::ClearChangesMask(PersonalTabard);
    Base::ClearChangesMask(PartyType);
    Base::ClearChangesMask(QuestLog);
    Base::ClearChangesMask(VisibleItems);
    Base::ClearChangesMask(AvgItemLevel);
    Base::ClearChangesMask(ForcedReactions);
    Base::ClearChangesMask(Field_3120);
    _changesMask.ResetAll();
}

void UFCorpseData::WriteCreate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Corpse const* owner, Player const* receiver) const
{
    data << uint32(DynamicFlags);
    data << Owner;
    data << PartyGUID;
    data << GuildGUID;
    data << uint32(DisplayID);
    for (uint32 i = 0; i < 19; ++i)
    {
        data << uint32(Items[i]);
    }
    data << uint8(RaceID);
    data << uint8(Sex);
    data << uint8(Class);
    data << uint32(Customizations.size());
    data << uint32(Flags);
    data << int32(FactionTemplate);
    for (uint32 i = 0; i < Customizations.size(); ++i)
    {
        Customizations[i].WriteCreate(data, owner, receiver);
    }
}

void UFCorpseData::WriteUpdate(ByteBuffer& data, EnumFlag<UpdateFieldFlag> fieldVisibilityFlags, Corpse const* owner, Player const* receiver) const
{
    WriteUpdate(data, _changesMask, false, owner, receiver);
}

void UFCorpseData::WriteUpdate(ByteBuffer& data, Mask const& changesMask, bool ignoreNestedChangesMask, Corpse const* owner, Player const* receiver) const
{
    data.WriteBits(changesMask.GetBlocksMask(0), 1);
    if (changesMask.GetBlock(0))
        data.WriteBits(changesMask.GetBlock(0), 32);

    if (changesMask[0])
    {
        if (changesMask[1])
        {
            if (!ignoreNestedChangesMask)
                Customizations.WriteUpdateMask(data);
            else
                WriteCompleteDynamicFieldUpdateMask(Customizations.size(), data);
        }
    }
    data.FlushBits();
    if (changesMask[0])
    {
        if (changesMask[1])
        {
            for (uint32 i = 0; i < Customizations.size(); ++i)
            {
                if (Customizations.HasChanged(i) || ignoreNestedChangesMask)
                {
                    Customizations[i].WriteUpdate(data, ignoreNestedChangesMask, owner, receiver);
                }
            }
        }
        if (changesMask[2])
        {
            data << uint32(DynamicFlags);
        }
        if (changesMask[3])
        {
            data << Owner;
        }
        if (changesMask[4])
        {
            data << PartyGUID;
        }
        if (changesMask[5])
        {
            data << GuildGUID;
        }
        if (changesMask[6])
        {
            data << uint32(DisplayID);
        }
        if (changesMask[7])
        {
            data << uint8(RaceID);
        }
        if (changesMask[8])
        {
            data << uint8(Sex);
        }
        if (changesMask[9])
        {
            data << uint8(Class);
        }
        if (changesMask[10])
        {
            data << uint32(Flags);
        }
        if (changesMask[11])
        {
            data << int32(FactionTemplate);
        }
    }
    if (changesMask[12])
    {
        for (uint32 i = 0; i < 19; ++i)
        {
            if (changesMask[13 + i])
            {
                data << uint32(Items[i]);
            }
        }
    }
}

void UFCorpseData::ClearChangesMask()
{
    Base::ClearChangesMask(Customizations);
    Base::ClearChangesMask(DynamicFlags);
    Base::ClearChangesMask(Owner);
    Base::ClearChangesMask(PartyGUID);
    Base::ClearChangesMask(GuildGUID);
    Base::ClearChangesMask(DisplayID);
    Base::ClearChangesMask(RaceID);
    Base::ClearChangesMask(Sex);
    Base::ClearChangesMask(Class);
    Base::ClearChangesMask(Flags);
    Base::ClearChangesMask(FactionTemplate);
    Base::ClearChangesMask(Items);
    _changesMask.ResetAll();
}
