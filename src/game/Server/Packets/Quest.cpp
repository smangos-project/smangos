#include "Quest.h"

#include "ObjectMgr.h"
#include "QuestDef.h"

void WorldPackets::Quest::QueryQuest::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> questEntry;
}

// --- Server Packets ---

size_t WorldPackets::Quest::QuestGiverQuestFailed::EstimateFinalSize() const
{
    return sizeof(questId) +
           sizeof(reason);
}

void WorldPackets::Quest::QuestGiverQuestFailed::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << questId;
    buffer << reason;
}

size_t WorldPackets::Quest::QuestUpdateFailed::EstimateFinalSize() const
{
    return sizeof(questId);
}

void WorldPackets::Quest::QuestUpdateFailed::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << questId;
}

size_t WorldPackets::Quest::QuestUpdateFailedTimer::EstimateFinalSize() const
{
    return sizeof(questId);
}

void WorldPackets::Quest::QuestUpdateFailedTimer::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << questId;
}

size_t WorldPackets::Quest::QuestGiverQuestInvalid::EstimateFinalSize() const
{
    return sizeof(msg);
}

void WorldPackets::Quest::QuestGiverQuestInvalid::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << msg;
}

size_t WorldPackets::Quest::QuestQueryResponse::EstimateFinalSize() const
{
    // Only an estimate, the localized strings may have a different length.
    return sizeof(uint32) + /*quest id*/
           sizeof(uint32) + /*quest method*/
           sizeof(uint32) + /*quest level*/
           sizeof(uint32) + /*zone or sort*/
           sizeof(uint32) + /*type*/
           sizeof(uint32) + /*rep objective faction*/
           sizeof(uint32) + /*rep objective value*/
           sizeof(uint32) + /*required oposite rep faction*/
           sizeof(uint32) + /*required oposite rep value*/
           sizeof(uint32) + /*next quest in chain*/
           sizeof(uint32) + /*reward money*/
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
           sizeof(uint32) + /*reward money max level*/
#endif
           sizeof(uint32) + /*reward spell*/
           sizeof(uint32) + /*source item id*/
           sizeof(uint32) + /*quest flags*/
           sizeof(uint32) * 2 * QUEST_REWARDS_COUNT + /*id, count*/
           sizeof(uint32) * 2 * QUEST_REWARD_CHOICES_COUNT + /*id, count*/
           sizeof(uint32) + /*point map id*/
           sizeof(float) + /*point x*/
           sizeof(float) + /*point y*/
           sizeof(uint32) + /*point opt*/
           quest->GetTitle().size() + sizeof(char) + /*null terminator*/
           quest->GetObjectives().size() + sizeof(char) + /*null terminator*/
           quest->GetDetails().size() + sizeof(char) + /*null terminator*/
           quest->GetEndText().size() + sizeof(char) + /*null terminator*/
           sizeof(uint32) * 4 * QUEST_OBJECTIVES_COUNT + /*creature or go id, count, item id, count*/
           sizeof(char) * QUEST_OBJECTIVES_COUNT + /*objective text null terminators*/
           quest->ObjectiveText[0].size() +
           quest->ObjectiveText[1].size() +
           quest->ObjectiveText[2].size() +
           quest->ObjectiveText[3].size();
}

void WorldPackets::Quest::QuestQueryResponse::AppendBodyTo(ByteBuffer& buffer) const
{
    char const* Title = quest->GetTitle().c_str();
    size_t titleLen = quest->GetTitle().length();
    char const* Details = quest->GetDetails().c_str();
    size_t detailsLen = quest->GetDetails().length();
    char const* Objectives = quest->GetObjectives().c_str();
    size_t objectivesLen = quest->GetObjectives().length();
    char const* EndText = quest->GetEndText().c_str();
    size_t endTextLen = quest->GetEndText().length();
    char const* ObjectiveText[QUEST_OBJECTIVES_COUNT];
    size_t objectiveTextLen[QUEST_OBJECTIVES_COUNT];

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        ObjectiveText[i] = quest->ObjectiveText[i].c_str();
        objectiveTextLen[i] = quest->ObjectiveText[i].length();
    }

    int const loc_idx = sessionDbLocaleIndex;
    if (loc_idx >= 0)
    {
        if (QuestLocale const* ql = sObjectMgr.GetQuestLocale(quest->GetQuestId()))
        {
            if (ql->Title.size() >static_cast<size_t>(loc_idx) && !ql->Title[loc_idx].empty())
            {
                Title = ql->Title[loc_idx].c_str();
                titleLen = ql->Title[loc_idx].length();
            }
            if (ql->Details.size() > static_cast<size_t>(loc_idx) && !ql->Details[loc_idx].empty())
            {
                Details = ql->Details[loc_idx].c_str();
                detailsLen = ql->Details[loc_idx].length();
            }
            if (ql->Objectives.size() > static_cast<size_t>(loc_idx) && !ql->Objectives[loc_idx].empty())
            {
                Objectives = ql->Objectives[loc_idx].c_str();
                objectivesLen = ql->Objectives[loc_idx].length();
            }
            if (ql->EndText.size() > static_cast<size_t>(loc_idx) && !ql->EndText[loc_idx].empty())
            {
                EndText = ql->EndText[loc_idx].c_str();
                endTextLen = ql->EndText[loc_idx].length();
            }

            for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                if (ql->ObjectiveText[i].size() >static_cast<size_t>(loc_idx) && !ql->ObjectiveText[i][loc_idx].empty())
                {
                    ObjectiveText[i] = ql->ObjectiveText[i][loc_idx].c_str();
                    objectiveTextLen[i] = ql->ObjectiveText[i][loc_idx].length();
                }
            }
        }
    }

    buffer << quest->GetQuestId();                   // quest id
    buffer << quest->GetQuestMethod();               // Accepted values: 0, 1 or 2. 0==IsAutoComplete() (skip objectives/details)
    buffer << quest->GetQuestLevel();                // may be 0, static data, in other cases must be used dynamic level: Player::GetQuestLevelForPlayer
    buffer << quest->GetZoneOrSort();                // zone or sort to display in quest log

    buffer << quest->GetType();

    buffer << quest->GetRepObjectiveFaction();       // shown in quest log as part of quest objective
    buffer << static_cast<uint32>(quest->GetRepObjectiveValue());         // shown in quest log as part of quest objective

    buffer << static_cast<uint32>(0);                // RequiredOpositeRepFaction
    buffer << static_cast<uint32>(0);                // RequiredOpositeRepValue, required faction value with another (oposite) faction (objective)

    buffer << quest->GetNextQuestInChain();          // client will request this quest from NPC, if not 0

    if (quest->HasQuestFlag(QUEST_FLAGS_HIDDEN_REWARDS))
        buffer << static_cast<uint32>(0);            // Hide money rewarded
    else
        buffer << static_cast<uint32>(quest->GetRewOrReqMoney());

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
    buffer << quest->GetRewMoneyMaxLevel();          // used in XP calculation at client
#endif
    buffer << quest->GetRewSpell();                  // reward spell, this spell will display (icon) (casted if RewSpellCast==0)

    buffer << quest->GetSrcItemId();                 // source item id
    buffer << quest->GetQuestFlags();                // quest flags

    int iI;

    if (quest->HasQuestFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; ++iI)
            buffer << static_cast<uint32>(0) << static_cast<uint32>(0);
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; ++iI)
            buffer << static_cast<uint32>(0) << static_cast<uint32>(0);
    }
    else
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; ++iI)
        {
            buffer << quest->RewItemId[iI];
            buffer << quest->RewItemCount[iI];
        }
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; ++iI)
        {
            buffer << quest->RewChoiceItemId[iI];
            buffer << quest->RewChoiceItemCount[iI];
        }
    }

    buffer << quest->GetPointMapId();
    buffer << quest->GetPointX();
    buffer << quest->GetPointY();
    buffer << quest->GetPointOpt();

    buffer.append(Title, titleLen + 1);
    buffer.append(Objectives, objectivesLen + 1);
    buffer.append(Details, detailsLen + 1);
    buffer.append(EndText, endTextLen + 1);

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; ++iI)
    {
        if (quest->ReqCreatureOrGOId[iI] < 0)
        {
            // client expected gameobject template id in form (id|0x80000000)
            buffer << static_cast<uint32>(quest->ReqCreatureOrGOId[iI] * -1 | 0x80000000);
        }
        else
        {
            buffer << static_cast<uint32>(quest->ReqCreatureOrGOId[iI]);
        }
        buffer << quest->ReqCreatureOrGOCount[iI];
        buffer << quest->ReqItemId[iI];
        buffer << quest->ReqItemCount[iI];
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; ++iI)
        buffer.append(ObjectiveText[iI], objectiveTextLen[iI] + 1);
}
