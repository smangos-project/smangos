#ifndef MANGOS_PACKETS_QUESTMESSAGES_H
#define MANGOS_PACKETS_QUESTMESSAGES_H

// The modern quest wire: giver status, the quest-giver-only quest list, details/offer/request-items
// windows, the accept/complete/reward round trip, log housekeeping, party sharing and objective
// credit. Light like Packets/CombatMessages.h, so RealBots can include it.
//
// Bit-packing note: several of these bodies (QuestGiverQuestDetails, QuestGiverOfferReward,
// QuestGiverRequestItems, QuestRewardsBlock's choice items) pack a real client's several adjacent
// bit-fields into one region before a byte-aligned flush; here each is instead flushed right after
// it's written. This is a self-consistent simplification, not a byte-for-byte match to a real
// client -- everything in this phase is unverified against one anyway (see bot-phase-4.md's Risks).

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include "Packets/ItemMessages.h"
#include <array>
#include <cstring>
#include <string>
#include <vector>

constexpr uint32 CLASSIC_QUEST_REWARD_ITEM_COUNT = 4;
constexpr uint32 CLASSIC_QUEST_REWARD_CHOICE_COUNT = 6;
constexpr uint32 CLASSIC_QUEST_REWARD_FACTION_COUNT = 5;
constexpr uint32 CLASSIC_QUEST_REWARD_CURRENCY_COUNT = 4;
constexpr uint32 CLASSIC_QUEST_EMOTE_COUNT = 4;
constexpr uint32 CLASSIC_MAX_QUEST_OBJECTIVES = 16;
constexpr uint32 CLASSIC_MAX_QUEST_COLLECT_ITEMS = 16;
constexpr uint32 CLASSIC_MAX_QUEST_LIST_ENTRIES = 32;

// ==== Client packets ====

struct QuestgiverStatusQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_STATUS_QUERY;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

struct QuestgiverHello
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_HELLO;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// Trailing RespondToGiver bit is parsed, unused: this engine always answers to the giver guid.
struct QuestgiverQueryQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_QUERY_QUEST;

    ClassicGuid Guid;
    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId;
        data.WriteBit(false);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5)
            return false;
        data >> QuestId;
        data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Trailing StartCheat bit is parsed, unused: this engine never accepts with the cheat flag.
struct QuestgiverAcceptQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_ACCEPT_QUEST;

    ClassicGuid Guid;
    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId;
        data.WriteBit(false);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5)
            return false;
        data >> QuestId;
        data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Trailing FromScript bit is parsed, unused.
struct QuestgiverCompleteQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_COMPLETE_QUEST;

    ClassicGuid Guid;
    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId;
        data.WriteBit(false);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5)
            return false;
        data >> QuestId;
        data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct QuestgiverRequestReward
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_REQUEST_REWARD;

    ClassicGuid Guid;
    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

// The client identifies its chosen reward by item id (embedded in a full ItemInstance), not by a
// bare reward-slot index; HandleQuestgiverChooseRewardOpcode resolves the slot once it has the
// quest template in hand. The LootItemType bits and the ItemInstance's bonus/modifier fields are
// always item/none for this engine's reward choices; a body that sets either is rejected on read.
// The wire's Quantity always comes from the quest template, not the client, so it's parsed and
// discarded.
struct QuestgiverChooseReward
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_CHOOSE_REWARD;

    ClassicGuid Guid;
    uint32 QuestId = 0;
    uint32 RewardItemId = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId;
        data.WriteBits(0, 2);
        data.FlushBits();
        ClassicItemInstance item;
        item.ItemId = int32(RewardItemId);
        item.Write(data);
        data << uint32(0);  // Quantity
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        if (data.size() - data.rpos() < 1)
            return false;
        uint32 const lootType = data.ReadBits(2);
        data.ResetBitPos();
        if (lootType != 0)
            return false;
        ClassicItemInstance item;
        if (!item.Read(data))
            return false;
        RewardItemId = uint32(item.ItemId);
        if (data.size() - data.rpos() < 4)
            return false;
        uint32 quantity;
        data >> quantity;
        return true;
    }
};

// Replaces the vanilla "cancel" click: the client now names the quest it's closing. Parsed, unused
// -- this engine's PlayerTalkClass::CloseGossip() already tears down whatever window is open.
struct QuestGiverCloseQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_CLOSE_QUEST;

    int32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << QuestId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

struct QuestLogRemoveQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_LOG_REMOVE_QUEST;

    uint8 Slot = 0;

    void Write(ByteBuffer& data) const { data << Slot; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Slot;
        return true;
    }
};

struct QuestConfirmAccept
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_CONFIRM_ACCEPT;

    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << QuestId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

struct PushQuestToParty
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PUSH_QUEST_TO_PARTY;

    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << QuestId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

struct QuestPushResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_PUSH_RESULT;

    ClassicGuid Guid;
    uint32 QuestId = 0;
    uint8 Msg = 0;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << QuestId << Msg;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data))
            return false;
        if (data.size() - data.rpos() < 5)
            return false;
        data >> QuestId >> Msg;
        return true;
    }
};

// ==== Server packets ====

struct QuestGiverStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_STATUS;

    ClassicGuid NpcGuid;
    uint64 Status = 0;  // a bitmask (TrinityCore's QuestGiverStatus), not the engine's 0-7 dialog status

    void Write(ByteBuffer& data) const
    {
        NpcGuid.Write(data);
        data << Status;
    }

    bool Read(ByteBuffer& data)
    {
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 8)
            return false;
        data >> Status;
        return true;
    }
};

struct QuestLogFull
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_LOG_FULL;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct QuestUpdateComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_UPDATE_COMPLETE;

    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << int32(QuestId); }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        int32 questId;
        data >> questId;
        QuestId = uint32(questId);
        return true;
    }
};

struct QuestConfirmAcceptResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_CONFIRM_ACCEPT;

    uint32 QuestId = 0;
    ClassicGuid InitiatedBy;
    std::string QuestTitle;

    void Write(ByteBuffer& data) const
    {
        data << QuestId;
        InitiatedBy.Write(data);
        data.WriteBits(uint32(QuestTitle.length()), 10);
        data.FlushBits();
        data.WriteString(QuestTitle);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        if (!InitiatedBy.Read(data))
            return false;
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(10);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        QuestTitle = data.ReadString(length);
        return true;
    }
};

struct QuestPushResultResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_PUSH_RESULT;

    ClassicGuid SenderGuid;
    uint8 Result = 0;
    std::string QuestTitle;

    void Write(ByteBuffer& data) const
    {
        SenderGuid.Write(data);
        data << Result;
        data.WriteBits(uint32(QuestTitle.length()), 9);
        data.FlushBits();
        data.WriteString(QuestTitle);
    }

    bool Read(ByteBuffer& data)
    {
        if (!SenderGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const length = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        QuestTitle = data.ReadString(length);
        return true;
    }
};

struct QuestUpdateAddCredit
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_UPDATE_ADD_CREDIT;

    ClassicGuid VictimGuid;
    uint32 QuestId = 0;
    int32 ObjectId = 0;   // creature/gameobject entry (a gameobject entry is OR'd with 0x80000000)
    uint16 Count = 0;
    uint16 Required = 0;
    uint8 ObjectiveType = 0;  // 0 monster, 1 item, 2 gameobject

    void Write(ByteBuffer& data) const
    {
        VictimGuid.Write(data);
        data << int32(QuestId) << ObjectId << Count << Required << ObjectiveType;
    }

    bool Read(ByteBuffer& data)
    {
        if (!VictimGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 13)
            return false;
        int32 questId;
        data >> questId >> ObjectId >> Count >> Required >> ObjectiveType;
        QuestId = uint32(questId);
        return true;
    }
};

// One quest offered by a pure quest-giver (no gossip options) with more than one quest to hand
// out. TrinityCore's own SMSG_QUEST_GIVER_QUEST_LIST_MESSAGE carries only greeting text variants,
// not per-quest fields -- the quest list itself seems to always ride the gossip window there
// (Packets/NpcMessages.h's GossipMessage/GossipQuestEntry, already used for every gossip-flagged
// NPC). This struct instead follows vibemangos's shape (cross-checked against HermesProxy's
// QuestGiverQuestListMessage.Write()), kept for the "no gossip flag, several quests" case this
// engine's Player::SendPreparedQuest already special-cases. README dispute: TrinityCore vs
// vibemangos on what this opcode actually carries.
struct QuestGiverQuestListEntry
{
    uint32 QuestId = 0;
    int32 QuestType = 0;   // 0 auto-complete, 2 not taken, 4 already in the log
    int32 QuestLevel = 0;
    int32 QuestFlags[3] = {};
    bool Repeatable = false;
    std::string Title;

    void Write(ByteBuffer& data) const
    {
        data << QuestId;
        data << int32(0);  // content tuning
        data << QuestType << QuestLevel;
        data << QuestFlags[0] << QuestFlags[1] << QuestFlags[2];
        data.WriteBit(Repeatable);
        data.WriteBits(uint32(Title.length()), 9);
        data.FlushBits();
        data.WriteString(Title);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 30)
            return false;
        int32 contentTuning;
        data >> QuestId >> contentTuning >> QuestType >> QuestLevel;
        data >> QuestFlags[0] >> QuestFlags[1] >> QuestFlags[2];
        Repeatable = data.ReadBit();
        uint32 const length = data.ReadBits(9);
        data.ResetBitPos();
        if (contentTuning)
            return false;
        if (data.size() - data.rpos() < length)
            return false;
        Title = data.ReadString(length);
        return true;
    }
};

struct QuestGiverQuestList
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_QUEST_LIST_MESSAGE;

    ClassicGuid NpcGuid;
    uint32 GreetEmoteDelay = 0;
    uint32 GreetEmoteType = 0;
    std::string Greeting;
    std::vector<QuestGiverQuestListEntry> Quests;

    void Write(ByteBuffer& data) const
    {
        NpcGuid.Write(data);
        data << GreetEmoteDelay << GreetEmoteType;
        data << int32(Quests.size());
        data.WriteBits(uint32(Greeting.length()), 11);
        data.FlushBits();
        for (QuestGiverQuestListEntry const& quest : Quests)
            quest.Write(data);
        data.WriteString(Greeting);
    }

    bool Read(ByteBuffer& data)
    {
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 14)
            return false;
        int32 count;
        data >> GreetEmoteDelay >> GreetEmoteType >> count;
        uint32 const greetingLength = data.ReadBits(11);
        data.ResetBitPos();
        if (count < 0 || uint32(count) > CLASSIC_MAX_QUEST_LIST_ENTRIES)
            return false;
        Quests.assign(uint32(count), QuestGiverQuestListEntry());
        for (QuestGiverQuestListEntry& quest : Quests)
            if (!quest.Read(data))
                return false;
        if (data.size() - data.rpos() < greetingLength)
            return false;
        Greeting = data.ReadString(greetingLength);
        return true;
    }
};

struct QuestRewardItemEntry
{
    uint32 ItemId = 0;
    uint32 Quantity = 0;
};

struct QuestRewardFactionEntry
{
    uint32 FactionId = 0;
    int32 Value = 0;
};

// The nested "QuestRewards" block shared by QuestGiverQuestDetails and QuestGiverOfferReward.
// Fixed-size arrays match the modern protocol's own reward-item/choice/reputation/currency
// counts (4/6/5/4), which happen to equal this engine's own QUEST_REWARDS_COUNT/
// QUEST_REWARD_CHOICES_COUNT/QUEST_REPUTATIONS_COUNT constants. Currencies, artifact xp/category,
// honor, title, faction flags/override/cap, spell display icons, skill line, treasure picker and
// boost-spell rewards have no equivalent system in this engine and are always empty/zero; a body
// that sets one is rejected on read.
struct QuestRewardsBlock
{
    std::array<QuestRewardItemEntry, CLASSIC_QUEST_REWARD_ITEM_COUNT> Items{};
    uint32 ChoiceItemCount = 0;
    uint32 ItemCount = 0;
    int32 Money = 0;
    uint32 Xp = 0;
    std::array<QuestRewardFactionEntry, CLASSIC_QUEST_REWARD_FACTION_COUNT> Factions{};
    uint32 SpellCompletionId = 0;
    std::array<QuestRewardItemEntry, CLASSIC_QUEST_REWARD_CHOICE_COUNT> ChoiceItems{};

    void Write(ByteBuffer& data) const
    {
        for (QuestRewardItemEntry const& item : Items)
            data << item.ItemId << item.Quantity;
        for (uint32 i = 0; i < CLASSIC_QUEST_REWARD_CURRENCY_COUNT; ++i)
            data << uint32(0) << uint32(0) << uint32(0);  // CurrencyID, CurrencyQty, BonusQty

        data << ChoiceItemCount << ItemCount;
        data << Money;
        data << Xp;
        data << uint64(0);  // ArtifactXP
        data << uint32(0);  // ArtifactCategoryID
        data << uint32(0);  // Honor
        data << uint32(0);  // Title
        data << uint32(0);  // FactionFlags
        for (QuestRewardFactionEntry const& faction : Factions)
            data << faction.FactionId << faction.Value << int32(0) << int32(0);  // Override, CapIn
        for (uint32 i = 0; i < 3; ++i)
            data << int32(0);  // SpellCompletionDisplayID[3]
        data << SpellCompletionId;
        data << uint32(0);  // SkillLineID
        data << uint32(0);  // NumSkillUps
        data << uint32(0);  // TreasurePickerID.size()

        for (QuestRewardItemEntry const& choice : ChoiceItems)
        {
            data.WriteBits(0, 2);  // LootItemType::Item
            data.FlushBits();
            ClassicItemInstance item;
            item.ItemId = int32(choice.ItemId);
            item.Write(data);
            data << choice.Quantity;
        }
        data.WriteBit(false);  // IsBoostSpell
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < CLASSIC_QUEST_REWARD_ITEM_COUNT * 8)
            return false;
        for (QuestRewardItemEntry& item : Items)
            data >> item.ItemId >> item.Quantity;

        if (data.size() - data.rpos() < CLASSIC_QUEST_REWARD_CURRENCY_COUNT * 12)
            return false;
        for (uint32 i = 0; i < CLASSIC_QUEST_REWARD_CURRENCY_COUNT; ++i)
        {
            uint32 currencyId, currencyQty, bonusQty;
            data >> currencyId >> currencyQty >> bonusQty;
            if (currencyId || currencyQty || bonusQty)
                return false;
        }

        if (data.size() - data.rpos() < 16)
            return false;
        data >> ChoiceItemCount >> ItemCount >> Money >> Xp;
        if (ChoiceItemCount > CLASSIC_QUEST_REWARD_CHOICE_COUNT || ItemCount > CLASSIC_QUEST_REWARD_ITEM_COUNT)
            return false;

        if (data.size() - data.rpos() < 24)
            return false;
        uint64 artifactXp;
        uint32 artifactCategory, honor, title, factionFlags;
        data >> artifactXp >> artifactCategory >> honor >> title >> factionFlags;
        if (artifactXp || artifactCategory || honor || title || factionFlags)
            return false;

        if (data.size() - data.rpos() < CLASSIC_QUEST_REWARD_FACTION_COUNT * 16)
            return false;
        for (QuestRewardFactionEntry& faction : Factions)
        {
            int32 override_, capIn;
            data >> faction.FactionId >> faction.Value >> override_ >> capIn;
            if (override_ || capIn)
                return false;
        }

        if (data.size() - data.rpos() < 16)
            return false;
        int32 displaySpell[3];
        for (int32& value : displaySpell)
            data >> value;
        if (displaySpell[0] || displaySpell[1] || displaySpell[2])
            return false;
        data >> SpellCompletionId;

        if (data.size() - data.rpos() < 12)
            return false;
        uint32 skillLine, numSkillUps, treasurePicker;
        data >> skillLine >> numSkillUps >> treasurePicker;
        if (skillLine || numSkillUps || treasurePicker)
            return false;

        for (QuestRewardItemEntry& choice : ChoiceItems)
        {
            if (data.size() - data.rpos() < 1)
                return false;
            uint32 const lootType = data.ReadBits(2);
            data.ResetBitPos();
            if (lootType != 0)
                return false;
            ClassicItemInstance item;
            if (!item.Read(data))
                return false;
            choice.ItemId = uint32(item.ItemId);
            if (data.size() - data.rpos() < 4)
                return false;
            data >> choice.Quantity;
        }

        if (data.size() - data.rpos() < 1)
            return false;
        bool const isBoostSpell = data.ReadBit();
        data.ResetBitPos();
        return !isBoostSpell;
    }
};

struct QuestGiverQuestDetailsObjective
{
    uint32 Id = 0;
    int32 Type = 0;       // 0 monster, 1 item, 2 gameobject
    int32 ObjectId = 0;   // creature/item/gameobject entry (a gameobject entry is OR'd with 0x80000000)
    int32 Amount = 0;
};

struct QuestGiverQuestDetails
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_QUEST_DETAILS;

    ClassicGuid NpcGuid;
    uint32 QuestId = 0;
    uint32 QuestFlags[3] = {};
    int32 SuggestedPartyMembers = 0;
    std::array<uint32, CLASSIC_QUEST_EMOTE_COUNT> DetailsEmote{};
    std::array<uint32, CLASSIC_QUEST_EMOTE_COUNT> DetailsEmoteDelay{};
    std::vector<QuestGiverQuestDetailsObjective> Objectives;
    QuestRewardsBlock Rewards;
    std::string Title;
    std::string DescriptionText;
    std::string LogDescription;
    bool AutoLaunched = false;

    void Write(ByteBuffer& data) const
    {
        NpcGuid.Write(data);
        ClassicGuid().Write(data);  // InformUnit -- no equivalent system
        data << QuestId;
        data << int32(0);  // QuestPackageID
        data << int32(0);  // PortraitGiver
        data << int32(0);  // PortraitGiverMount
        data << int32(0);  // PortraitGiverModelSceneID
        data << int32(0);  // PortraitTurnIn
        data << QuestFlags[0] << QuestFlags[1] << QuestFlags[2];
        data << SuggestedPartyMembers;
        data << uint32(0);  // LearnSpells.size()
        data << uint32(DetailsEmote.size());
        data << uint32(Objectives.size());
        // QuestStartItemID -- always 0: a nonzero value here made a real client believe the quest
        // was auto-accepted via item use and broke its accept UI (vibemangos's live finding).
        data << int32(0);
        data << int32(0);  // QuestInfoID
        data << int32(0);  // QuestSessionBonus
        data << int32(0);  // QuestGiverCreatureID
        data << uint32(0);  // ConditionalDescriptionText.size()

        for (size_t i = 0; i < DetailsEmote.size(); ++i)
        {
            data << int32(DetailsEmote[i]);
            data << DetailsEmoteDelay[i];
        }
        for (QuestGiverQuestDetailsObjective const& obj : Objectives)
        {
            data << obj.Id;
            data << obj.Type;
            data << obj.ObjectId;
            data << obj.Amount;
        }

        data.WriteBits(uint32(Title.length()), 9);
        data.FlushBits();
        data.WriteBits(uint32(DescriptionText.length()), 12);
        data.FlushBits();
        data.WriteBits(uint32(LogDescription.length()), 12);
        data.FlushBits();
        data.WriteBit(AutoLaunched);
        data.WriteBit(false);  // FromContentPush
        data.WriteBit(false);  // ReplayQuest
        data.WriteBit(false);  // ResetByScheduler
        data.WriteBit(false);  // StartCheat
        data.WriteBit(false);  // DisplayPopup
        data.FlushBits();

        Rewards.Write(data);

        data.WriteString(Title);
        data.WriteString(DescriptionText);
        data.WriteString(LogDescription);
    }

    bool Read(ByteBuffer& data)
    {
        ClassicGuid informUnit;
        if (!NpcGuid.Read(data) || !informUnit.Read(data))
            return false;
        if (!informUnit.IsEmpty())
            return false;
        if (data.size() - data.rpos() < 72)
            return false;
        int32 questPackage, portraitGiver, portraitMount, portraitScene, portraitTurnIn;
        uint32 learnSpellCount, emoteCount, objectiveCount;
        int32 startItem, questInfoId, sessionBonus, giverCreatureId;
        uint32 conditionalCount;
        data >> QuestId >> questPackage >> portraitGiver >> portraitMount >> portraitScene >> portraitTurnIn;
        data >> QuestFlags[0] >> QuestFlags[1] >> QuestFlags[2];
        data >> SuggestedPartyMembers;
        data >> learnSpellCount >> emoteCount >> objectiveCount;
        data >> startItem >> questInfoId >> sessionBonus >> giverCreatureId;
        data >> conditionalCount;
        if (questPackage || portraitGiver || portraitMount || portraitScene || portraitTurnIn)
            return false;
        if (learnSpellCount || startItem || questInfoId || sessionBonus || giverCreatureId || conditionalCount)
            return false;
        if (emoteCount != DetailsEmote.size() || objectiveCount > CLASSIC_MAX_QUEST_OBJECTIVES)
            return false;

        if (data.size() - data.rpos() < emoteCount * 8)
            return false;
        for (uint32 i = 0; i < emoteCount; ++i)
        {
            int32 emote;
            data >> emote;
            DetailsEmote[i] = uint32(emote);
            data >> DetailsEmoteDelay[i];
        }

        if (data.size() - data.rpos() < objectiveCount * 16)
            return false;
        Objectives.assign(objectiveCount, QuestGiverQuestDetailsObjective());
        for (QuestGiverQuestDetailsObjective& obj : Objectives)
        {
            int32 id;
            data >> id;
            obj.Id = uint32(id);
            data >> obj.Type >> obj.ObjectId >> obj.Amount;
        }

        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const titleLength = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const descLength = data.ReadBits(12);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const logDescLength = data.ReadBits(12);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 1)
            return false;
        AutoLaunched = data.ReadBit();
        bool const fromContentPush = data.ReadBit();
        bool const replayQuest = data.ReadBit();
        bool const resetByScheduler = data.ReadBit();
        bool const startCheat = data.ReadBit();
        bool const displayPopup = data.ReadBit();
        data.ResetBitPos();
        if (fromContentPush || replayQuest || resetByScheduler || startCheat || displayPopup)
            return false;

        if (!Rewards.Read(data))
            return false;

        if (data.size() - data.rpos() < titleLength + descLength + logDescLength)
            return false;
        Title = data.ReadString(titleLength);
        DescriptionText = data.ReadString(descLength);
        LogDescription = data.ReadString(logDescLength);
        return true;
    }
};

struct QuestDescEmoteEntry
{
    int32 Type = 0;
    uint32 Delay = 0;
};

struct QuestGiverOfferReward
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_OFFER_REWARD_MESSAGE;

    ClassicGuid NpcGuid;
    uint32 QuestId = 0;
    uint32 QuestFlags[3] = {};
    int32 SuggestedPartyMembers = 0;
    std::vector<QuestDescEmoteEntry> Emotes;
    QuestRewardsBlock Rewards;
    std::string Title;
    std::string OfferRewardText;
    bool AutoLaunched = false;

    void Write(ByteBuffer& data) const
    {
        Rewards.Write(data);
        data << int32(Emotes.size());
        NpcGuid.Write(data);
        data << int32(QuestFlags[0]) << int32(QuestFlags[1]) << int32(QuestFlags[2]);
        data << int32(0);  // QuestGiverCreatureID
        data << int32(QuestId);
        data << SuggestedPartyMembers;
        data << int32(0);  // QuestInfoID
        for (QuestDescEmoteEntry const& emote : Emotes)
            data << emote.Type << emote.Delay;
        data.WriteBit(AutoLaunched);
        data.WriteBit(false);  // unused
        data.WriteBit(false);  // ResetByScheduler
        data.FlushBits();

        data << int32(0);  // QuestPackageID
        data << int32(0);  // PortraitGiver
        data << int32(0);  // PortraitGiverMount
        data << int32(0);  // PortraitGiverModelSceneID
        data << int32(0);  // PortraitTurnIn
        data << int32(0);  // QuestGiverCreatureID (outer)
        data << uint32(0);  // ConditionalRewardText.size()

        data.WriteBits(uint32(Title.length()), 9);
        data.FlushBits();
        data.WriteBits(uint32(OfferRewardText.length()), 12);
        data.FlushBits();

        data.WriteString(Title);
        data.WriteString(OfferRewardText);
    }

    bool Read(ByteBuffer& data)
    {
        if (!Rewards.Read(data))
            return false;
        if (data.size() - data.rpos() < 4)
            return false;
        int32 emoteCount;
        data >> emoteCount;
        if (emoteCount < 0 || uint32(emoteCount) > CLASSIC_QUEST_EMOTE_COUNT)
            return false;
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 28)
            return false;
        int32 questFlags[3], giverCreatureId, questId, questInfoId;
        data >> questFlags[0] >> questFlags[1] >> questFlags[2];
        QuestFlags[0] = uint32(questFlags[0]);
        QuestFlags[1] = uint32(questFlags[1]);
        QuestFlags[2] = uint32(questFlags[2]);
        data >> giverCreatureId;
        data >> questId;
        QuestId = uint32(questId);
        data >> SuggestedPartyMembers;
        data >> questInfoId;
        if (giverCreatureId || questInfoId)
            return false;

        if (data.size() - data.rpos() < uint32(emoteCount) * 8)
            return false;
        Emotes.assign(uint32(emoteCount), QuestDescEmoteEntry());
        for (QuestDescEmoteEntry& emote : Emotes)
            data >> emote.Type >> emote.Delay;

        if (data.size() - data.rpos() < 1)
            return false;
        AutoLaunched = data.ReadBit();
        bool const unused = data.ReadBit();
        bool const resetByScheduler = data.ReadBit();
        data.ResetBitPos();
        if (unused || resetByScheduler)
            return false;

        if (data.size() - data.rpos() < 28)
            return false;
        int32 questPackage, portraitGiver, portraitMount, portraitScene, portraitTurnIn, giverCreatureId2;
        uint32 conditionalCount;
        data >> questPackage >> portraitGiver >> portraitMount >> portraitScene >> portraitTurnIn >> giverCreatureId2;
        data >> conditionalCount;
        if (questPackage || portraitGiver || portraitMount || portraitScene || portraitTurnIn || giverCreatureId2 || conditionalCount)
            return false;

        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const titleLength = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const textLength = data.ReadBits(12);
        data.ResetBitPos();

        if (data.size() - data.rpos() < titleLength + textLength)
            return false;
        Title = data.ReadString(titleLength);
        OfferRewardText = data.ReadString(textLength);
        return true;
    }
};

struct QuestGiverRequestItemsObjective
{
    uint32 ObjectId = 0;
    uint32 Amount = 0;
};

struct QuestGiverRequestItems
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_REQUEST_ITEMS;

    ClassicGuid NpcGuid;
    uint32 QuestId = 0;
    uint32 QuestFlags[3] = {};
    int32 SuggestedPartyMembers = 0;
    int32 MoneyToGet = 0;
    uint32 CompEmoteType = 0;
    std::vector<QuestGiverRequestItemsObjective> Collect;
    bool IsComplete = false;
    std::string Title;
    std::string RequestItemsText;

    void Write(ByteBuffer& data) const
    {
        data << int32(Collect.size());
        data << int32(0);  // Currency.size()
        NpcGuid.Write(data);
        data << QuestFlags[0] << QuestFlags[1] << QuestFlags[2];
        // StatusFlags -- reverse-derived from a legacy->modern translator's status-bit decode
        // (vibemangos finding): every "must be true-ish" bit the vanilla body exposed folds into
        // this one field, 223 when complete, 219 when not.
        data << int32(IsComplete ? 223 : 219);
        data << int32(0);  // QuestGiverCreatureID
        data << int32(QuestId);
        data << int32(0);  // CompEmoteDelay
        data << int32(CompEmoteType);
        data << SuggestedPartyMembers;
        data << MoneyToGet;
        data << int32(0);  // QuestInfoID

        for (QuestGiverRequestItemsObjective const& obj : Collect)
            data << int32(obj.ObjectId) << int32(obj.Amount) << uint32(0);

        data.WriteBit(false);  // AutoLaunched
        data.WriteBit(false);  // ResetByScheduler
        data.FlushBits();

        data << int32(0);  // QuestGiverCreatureID (outer)
        data << uint32(0);  // ConditionalCompletionText.size()

        data.WriteBits(uint32(Title.length()), 9);
        data.FlushBits();
        data.WriteBits(uint32(RequestItemsText.length()), 12);
        data.FlushBits();

        data.WriteString(Title);
        data.WriteString(RequestItemsText);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 8)
            return false;
        int32 collectCount, currencyCount;
        data >> collectCount >> currencyCount;
        if (collectCount < 0 || uint32(collectCount) > CLASSIC_MAX_QUEST_COLLECT_ITEMS || currencyCount != 0)
            return false;
        if (!NpcGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 44)
            return false;
        int32 statusFlags, giverCreatureId, questId, compEmoteDelay, compEmoteType, questInfoId;
        data >> QuestFlags[0] >> QuestFlags[1] >> QuestFlags[2];
        data >> statusFlags >> giverCreatureId >> questId >> compEmoteDelay >> compEmoteType;
        data >> SuggestedPartyMembers >> MoneyToGet >> questInfoId;
        if (giverCreatureId || compEmoteDelay || questInfoId)
            return false;
        if (statusFlags != 223 && statusFlags != 219)
            return false;
        IsComplete = statusFlags == 223;
        QuestId = uint32(questId);
        CompEmoteType = uint32(compEmoteType);

        if (data.size() - data.rpos() < uint32(collectCount) * 12)
            return false;
        Collect.assign(uint32(collectCount), QuestGiverRequestItemsObjective());
        for (QuestGiverRequestItemsObjective& obj : Collect)
        {
            int32 objectId, amount;
            uint32 flags;
            data >> objectId >> amount >> flags;
            if (flags)
                return false;
            obj.ObjectId = uint32(objectId);
            obj.Amount = uint32(amount);
        }

        if (data.size() - data.rpos() < 1)
            return false;
        bool const autoLaunched = data.ReadBit();
        bool const resetByScheduler = data.ReadBit();
        data.ResetBitPos();
        if (autoLaunched || resetByScheduler)
            return false;

        if (data.size() - data.rpos() < 8)
            return false;
        int32 giverCreatureId2;
        uint32 conditionalCount;
        data >> giverCreatureId2 >> conditionalCount;
        if (giverCreatureId2 || conditionalCount)
            return false;

        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const titleLength = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < 2)
            return false;
        uint32 const textLength = data.ReadBits(12);
        data.ResetBitPos();

        if (data.size() - data.rpos() < titleLength + textLength)
            return false;
        Title = data.ReadString(titleLength);
        RequestItemsText = data.ReadString(textLength);
        return true;
    }
};

// The quest turn-in confirmation toast (gold/xp gained, and at most one highlighted bonus item).
// HARDCODE: this engine has no notion of a single "highlighted" reward item distinct from the
// full reward list already shown in QuestGiverOfferReward, so the item/launch fields are always
// empty/false.
struct QuestGiverQuestComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_QUEST_COMPLETE;

    uint32 QuestId = 0;
    uint32 XpReward = 0;
    int64 MoneyReward = 0;

    void Write(ByteBuffer& data) const
    {
        data << int32(QuestId);
        data << int32(XpReward);
        data << MoneyReward;
        data << int32(0);  // SkillLineIDReward
        data << int32(0);  // NumSkillUpsReward
        data.WriteBit(false);  // UseQuestReward
        data.WriteBit(false);  // LaunchGossip
        data.WriteBit(false);  // LaunchQuest
        data.WriteBit(false);  // HideChatMessage
        data.FlushBits();
        ClassicItemInstance item;
        item.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 24)
            return false;
        int32 questId, xp, skillLine, numSkillUps;
        data >> questId >> xp >> MoneyReward >> skillLine >> numSkillUps;
        if (skillLine || numSkillUps)
            return false;
        QuestId = uint32(questId);
        XpReward = uint32(xp);
        if (data.size() - data.rpos() < 1)
            return false;
        bool const useQuestReward = data.ReadBit();
        bool const launchGossip = data.ReadBit();
        bool const launchQuest = data.ReadBit();
        bool const hideChatMessage = data.ReadBit();
        data.ResetBitPos();
        if (useQuestReward || launchGossip || launchQuest || hideChatMessage)
            return false;
        ClassicItemInstance item;
        if (!item.Read(data))
            return false;
        return item.ItemId == 0;
    }
};

// ==== Failure toasts ====

// The reason is the modern QuestFailedReason numbering, not the engine's INVALIDREASON_*.
struct QuestGiverInvalidQuest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_INVALID_QUEST;

    uint32 Reason = 0;
    int32 ContributionRewardId = 0;
    bool SendErrorMessage = true;
    std::string ReasonText;

    void Write(ByteBuffer& data) const
    {
        data << Reason << ContributionRewardId;
        data.WriteBit(SendErrorMessage);
        data.WriteBits(uint32(ReasonText.length()), 9);
        data.FlushBits();
        data.WriteString(ReasonText);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 10)
            return false;
        data >> Reason >> ContributionRewardId;
        SendErrorMessage = data.ReadBit();
        uint32 const length = data.ReadBits(9);
        data.ResetBitPos();
        if (data.size() - data.rpos() < length)
            return false;
        ReasonText = data.ReadString(length);
        return true;
    }
};

// Reason is an inventory result for the reasons the client has text for (inventory full, duplicate item).
struct QuestGiverQuestFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_QUEST_FAILED;

    uint32 QuestId = 0;
    uint32 Reason = 0;

    void Write(ByteBuffer& data) const { data << QuestId << Reason; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 8)
            return false;
        data >> QuestId >> Reason;
        return true;
    }
};

struct QuestUpdateFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_UPDATE_FAILED;

    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << QuestId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

struct QuestUpdateFailedTimer
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_UPDATE_FAILED_TIMER;

    uint32 QuestId = 0;

    void Write(ByteBuffer& data) const { data << QuestId; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return true;
    }
};

// ==== Area-wide giver status ====

struct QuestGiverStatusMultipleQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUEST_GIVER_STATUS_MULTIPLE_QUERY;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct QuestGiverStatusEntry
{
    ClassicGuid Guid;
    uint64 Status = 0;
};

struct QuestGiverStatusMultiple
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUEST_GIVER_STATUS_MULTIPLE;

    std::vector<QuestGiverStatusEntry> Givers;

    void Write(ByteBuffer& data) const
    {
        data << int32(Givers.size());
        for (QuestGiverStatusEntry const& giver : Givers)
        {
            giver.Guid.Write(data);
            data << giver.Status;
        }
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        int32 count;
        data >> count;
        if (count < 0 || uint32(count) > data.size())
            return false;
        Givers.assign(uint32(count), QuestGiverStatusEntry());
        for (QuestGiverStatusEntry& giver : Givers)
        {
            if (!giver.Guid.Read(data))
                return false;
            if (data.size() - data.rpos() < 8)
                return false;
            data >> giver.Status;
        }
        return true;
    }
};

// ==== Quest log lookup by id ====

struct QueryQuestInfo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_QUERY_QUEST_INFO;

    int32 QuestId = 0;
    ClassicGuid QuestGiver;

    void Write(ByteBuffer& data) const
    {
        data << QuestId;
        QuestGiver.Write(data);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> QuestId;
        return QuestGiver.Read(data);
    }
};

struct QuestInfoObjective
{
    uint32 Id = 0;
    uint8 Type = 0;  // 0 creature, 1 item, 2 game object
    int32 ObjectId = 0;
    int32 Amount = 0;
    std::string Description;
};

struct QuestInfoRewardItem
{
    int32 ItemId = 0;
    int32 Quantity = 0;
};

struct QuestInfoChoiceItem
{
    int32 ItemId = 0;
    int32 Quantity = 0;
    int32 DisplayId = 0;
};

struct QuestInfoFaction
{
    int32 FactionId = 0;
    int32 Value = 0;
};

// The quest log's own lookup of a quest's full record. Scaling, artifact, currency, portrait, sound
// kit, area group, treasure picker, conditional text and visual effect fields have no equivalent
// here and are always zero/empty; a body that sets one is rejected on read.
struct QueryQuestInfoResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_QUERY_QUEST_INFO_RESPONSE;

    uint32 QuestId = 0;
    bool Allow = false;

    int32 QuestType = 0;
    int32 QuestLevel = 0;
    int32 MinLevel = 0;
    int32 SortId = 0;
    int32 SuggestedGroupNum = 0;
    int32 RewardNextQuest = 0;
    int32 RewardMoney = 0;
    int32 RewardSpell = 0;
    uint32 Flags = 0;
    std::array<QuestInfoRewardItem, CLASSIC_QUEST_REWARD_ITEM_COUNT> RewardItems{};
    std::array<QuestInfoChoiceItem, CLASSIC_QUEST_REWARD_CHOICE_COUNT> ChoiceItems{};
    int32 PoiContinent = 0;
    float PoiX = 0.0f;
    float PoiY = 0.0f;
    int32 PoiPriority = 0;
    std::array<QuestInfoFaction, CLASSIC_QUEST_REWARD_FACTION_COUNT> Factions{};
    int64 TimeAllowed = 0;
    std::vector<QuestInfoObjective> Objectives;
    std::string LogTitle;
    std::string LogDescription;
    std::string QuestDescription;
    std::string QuestCompletionLog;

    void Write(ByteBuffer& data) const
    {
        data << QuestId;
        data.WriteBit(Allow);
        data.FlushBits();
        if (!Allow)
            return;

        data << int32(QuestId) << QuestType << QuestLevel;
        data << int32(0) << int32(255) << int32(0);  // scaling faction group, max scaling level, package id
        data << MinLevel << SortId;
        data << int32(0);  // QuestInfoID
        data << SuggestedGroupNum << RewardNextQuest;
        data << int32(0) << 1.0f;  // RewardXPDifficulty, RewardXPMultiplier
        data << RewardMoney;
        data << int32(0) << 1.0f;  // RewardMoneyDifficulty, RewardMoneyMultiplier
        data << int32(0);          // RewardBonusMoney
        for (uint32 i = 0; i < 3; ++i)
            data << int32(0);      // RewardDisplaySpell
        data << RewardSpell;
        data << int32(0) << 0.0f;  // RewardHonor, RewardKillHonor
        data << int32(0) << 0.0f << int32(0);  // artifact xp difficulty, multiplier, category
        data << int32(0);          // StartItem
        data << Flags << uint32(0) << uint32(0);

        for (QuestInfoRewardItem const& item : RewardItems)
            data << item.ItemId << item.Quantity << int32(0) << int32(0);  // ItemDrop, ItemDropQuantity
        for (QuestInfoChoiceItem const& item : ChoiceItems)
            data << item.ItemId << item.Quantity << item.DisplayId;

        data << PoiContinent << PoiX << PoiY << PoiPriority;
        data << int32(0) << int32(0) << int32(0) << int32(0);  // title, arena points, skill line, skill ups
        data << int32(0) << int32(0) << int32(0) << int32(0);  // portraits

        for (QuestInfoFaction const& faction : Factions)
            data << faction.FactionId << faction.Value << int32(0) << int32(0);  // Override, CapIn
        data << int32(0);  // RewardFactionFlags
        for (uint32 i = 0; i < CLASSIC_QUEST_REWARD_CURRENCY_COUNT; ++i)
            data << int32(0) << int32(0);  // currency id, quantity
        data << int32(0) << int32(0);      // accepted and complete sound kits
        data << int32(0);                  // AreaGroupID
        data << TimeAllowed;

        data << uint32(Objectives.size());
        data << uint64(~0ull);  // AllowableRaces
        data << uint32(0) << uint32(0);  // treasure picker lists
        data << int32(0);       // Expansion
        data << int32(0);       // QuestGiverCreatureID
        data << uint32(0) << uint32(0);  // conditional description and completion log

        data.WriteBits(uint32(LogTitle.length()), 9);
        data.WriteBits(uint32(LogDescription.length()), 12);
        data.WriteBits(uint32(QuestDescription.length()), 12);
        data.WriteBits(0, 9);   // AreaDescription
        data.WriteBits(0, 10);  // PortraitGiverText
        data.WriteBits(0, 8);   // PortraitGiverName
        data.WriteBits(0, 10);  // PortraitTurnInText
        data.WriteBits(0, 8);   // PortraitTurnInName
        data.WriteBits(uint32(QuestCompletionLog.length()), 11);
        data.WriteBit(false);   // ReadyForTranslation
        data.FlushBits();

        for (QuestInfoObjective const& objective : Objectives)
        {
            data << objective.Id << int32(objective.Type) << int8(0);  // StorageIndex
            data << objective.ObjectId << objective.Amount;
            data << uint32(0) << uint32(0) << 0.0f;  // Flags, Flags2, ProgressBarWeight
            data << int32(0);                        // VisualEffects.size()
            data.WriteBits(uint32(objective.Description.length()), 8);
            data.FlushBits();
            data.WriteString(objective.Description);
        }

        data.WriteString(LogTitle);
        data.WriteString(LogDescription);
        data.WriteString(QuestDescription);
        data.WriteString(QuestCompletionLog);
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 5)
            return false;
        data >> QuestId;
        Allow = data.ReadBit();
        data.ResetBitPos();
        if (!Allow)
            return true;

        // 119 fixed words precede the bit fields; a short body fails here instead of inside the reads.
        if (data.size() - data.rpos() < 119 * 4)
            return false;

        bool ok = true;
        auto word = [&data]() { int32 value; data >> value; return value; };
        auto zero = [&](uint32 count)
        {
            for (uint32 i = 0; i < count; ++i)
                ok = ok && word() == 0;
        };

        int32 const questId = word();
        QuestType = word();
        QuestLevel = word();
        ok = ok && uint32(questId) == QuestId;
        ok = ok && word() == 0;    // scaling faction group
        ok = ok && word() == 255;  // max scaling level
        zero(1);                   // package id
        MinLevel = word();
        SortId = word();
        zero(1);                   // QuestInfoID
        SuggestedGroupNum = word();
        RewardNextQuest = word();
        zero(1);
        word();                    // RewardXPMultiplier
        RewardMoney = word();
        zero(1);
        word();                    // RewardMoneyMultiplier
        zero(4);                   // RewardBonusMoney, RewardDisplaySpell[3]
        RewardSpell = word();
        zero(1);
        word();                    // RewardKillHonor
        zero(1);
        word();                    // artifact xp multiplier
        zero(2);                   // artifact category, StartItem
        Flags = uint32(word());
        zero(2);                   // FlagsEx, FlagsEx2

        for (QuestInfoRewardItem& item : RewardItems)
        {
            item.ItemId = word();
            item.Quantity = word();
            zero(2);               // ItemDrop, ItemDropQuantity
        }
        for (QuestInfoChoiceItem& item : ChoiceItems)
        {
            item.ItemId = word();
            item.Quantity = word();
            item.DisplayId = word();
        }

        PoiContinent = word();
        int32 const poiX = word();
        int32 const poiY = word();
        std::memcpy(&PoiX, &poiX, sizeof(float));
        std::memcpy(&PoiY, &poiY, sizeof(float));
        PoiPriority = word();
        zero(8);                   // title, arena points, skill line, skill ups, portraits

        for (QuestInfoFaction& faction : Factions)
        {
            faction.FactionId = word();
            faction.Value = word();
            zero(2);               // Override, CapIn
        }
        zero(1 + CLASSIC_QUEST_REWARD_CURRENCY_COUNT * 2 + 3);  // faction flags, currencies, sound kits, area group
        int32 const timeLow = word();
        int32 const timeHigh = word();
        TimeAllowed = int64(uint64(uint32(timeHigh)) << 32 | uint32(timeLow));

        uint32 const objectiveCount = uint32(word());
        word();
        word();                    // AllowableRaces
        zero(2 + 1 + 1 + 2);       // treasure pickers, Expansion, QuestGiverCreatureID, conditional text
        if (!ok || objectiveCount > CLASSIC_MAX_QUEST_OBJECTIVES)
            return false;

        if (data.size() - data.rpos() < 12)
            return false;
        uint32 const logTitleLength = data.ReadBits(9);
        uint32 const logDescriptionLength = data.ReadBits(12);
        uint32 const descriptionLength = data.ReadBits(12);
        uint32 const areaDescriptionLength = data.ReadBits(9);
        uint32 const portraitLengths = data.ReadBits(10) + data.ReadBits(8) + data.ReadBits(10) + data.ReadBits(8);
        uint32 const completionLogLength = data.ReadBits(11);
        bool const readyForTranslation = data.ReadBit();
        data.ResetBitPos();
        if (areaDescriptionLength || portraitLengths || readyForTranslation)
            return false;

        Objectives.assign(objectiveCount, QuestInfoObjective());
        for (QuestInfoObjective& objective : Objectives)
        {
            if (data.size() - data.rpos() < 34)
                return false;
            int32 type;
            int8 storageIndex;
            uint32 flags, flags2;
            float progressBarWeight;
            int32 visualEffects;
            data >> objective.Id >> type >> storageIndex >> objective.ObjectId >> objective.Amount;
            data >> flags >> flags2 >> progressBarWeight >> visualEffects;
            if (type < 0 || type > 255 || visualEffects != 0)
                return false;
            objective.Type = uint8(type);
            uint32 const length = data.ReadBits(8);
            data.ResetBitPos();
            if (data.size() - data.rpos() < length)
                return false;
            objective.Description = data.ReadString(length);
        }

        if (data.size() - data.rpos() < uint64(logTitleLength) + logDescriptionLength + descriptionLength + completionLogLength)
            return false;
        LogTitle = data.ReadString(logTitleLength);
        LogDescription = data.ReadString(logDescriptionLength);
        QuestDescription = data.ReadString(descriptionLength);
        QuestCompletionLog = data.ReadString(completionLogLength);
        return true;
    }
};

#endif
