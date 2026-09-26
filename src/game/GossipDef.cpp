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

#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Formulas.h"
#include "World.h"
#include "Packets/Classic/GuidMapping.h"
#include "Packets/NpcMessages.h"
#include "Packets/QuestMessages.h"

namespace
{
// The quest list shows a quest as available (2), in the log (4) or ready to hand in (0).
int32 GossipQuestTypeForDialogStatus(uint8 dialogStatus)
{
    switch (dialogStatus)
    {
        case DIALOG_STATUS_INCOMPLETE:
            return 4;
        case DIALOG_STATUS_REWARD_REP:
        case DIALOG_STATUS_REWARD_OLD:
        case DIALOG_STATUS_REWARD2:
            return 0;
        default:
            return 2;
    }
}

// The modern client's quest-giver status is a bitmask (TrinityCore's QuestGiverStatus: Future=0x2,
// Trivial=0x4, Reward=0x20, RepeatableTurnin=0x100, Quest=0x400, RewardCompleteNoPOI=0x800,
// RewardCompletePOI=0x1000), not this engine's sequential 0-7 DIALOG_STATUS_* enum (QuestDef.h).
uint64 ToModernQuestGiverStatus(uint8 dialogStatus)
{
    switch (dialogStatus)
    {
        case DIALOG_STATUS_UNAVAILABLE: return 0x000002;  // Future
        case DIALOG_STATUS_CHAT:        return 0x000004;  // Trivial
        case DIALOG_STATUS_INCOMPLETE:  return 0x000020;  // Reward
        case DIALOG_STATUS_REWARD_REP:  return 0x000100;  // RepeatableTurnin
        case DIALOG_STATUS_AVAILABLE:   return 0x000400;  // Quest
        case DIALOG_STATUS_REWARD_OLD:  return 0x000800;  // RewardCompleteNoPOI (never produced by GetDialogStatus)
        case DIALOG_STATUS_REWARD2:     return 0x001000;  // RewardCompletePOI -- ready to turn in, show map marker
        default:                        return 0x000000;
    }
}
}

GossipMenu::GossipMenu(WorldSession* session) : m_session(session)
{
    m_gItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
    m_gMenuId = 0;
    m_discoveredNode = false;
}

GossipMenu::~GossipMenu()
{
    ClearMenu();
}

void GossipMenu::AddMenuItem(uint8 Icon, std::string const& Message, uint32 dtSender, uint32 dtAction, std::string const& BoxMessage, bool Coded)
{
    MANGOS_ASSERT(m_gItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    GossipMenuItem gItem;

    gItem.m_gIcon       = Icon;
    gItem.m_gMessage    = Message;
    gItem.m_gCoded      = Coded;
    gItem.m_gSender     = dtSender;
    gItem.m_gOptionId   = dtAction;
    gItem.m_gBoxMessage = BoxMessage;

    m_gItems.push_back(gItem);
}

void GossipMenu::AddGossipMenuItemData(int32 action_menu, uint32 action_poi, uint32 action_script)
{
    GossipMenuItemData pItemData;

    pItemData.m_gAction_menu    = action_menu;
    pItemData.m_gAction_poi     = action_poi;
    pItemData.m_gAction_script  = action_script;

    m_gItemsData.push_back(pItemData);
}

void GossipMenu::AddMenuItem(uint8 Icon, std::string const& Message, bool Coded)
{
    AddMenuItem(Icon, Message, 0, 0, "", Coded);
}

void GossipMenu::AddMenuItem(uint8 Icon, char const* Message, bool Coded)
{
    AddMenuItem(Icon, std::string(Message ? Message : ""), Coded);
}

void GossipMenu::AddMenuItem(uint8 Icon, char const* Message, uint32 dtSender, uint32 dtAction, char const* BoxMessage, bool Coded)
{
    AddMenuItem(Icon, std::string(Message ? Message : ""), dtSender, dtAction, std::string(BoxMessage ? BoxMessage : ""), Coded);
}

void GossipMenu::AddMenuItem(uint8 Icon, int32 itemText, uint32 dtSender, uint32 dtAction, int32 boxText, bool Coded)
{
    uint32 loc_idx = m_session->GetSessionDbLocaleIndex();

    char const* item_text = itemText ? (itemText > 0 ? sObjectMgr.GetBroadcastText(itemText, loc_idx, GetMenuSession()->GetPlayer()->GetGender()) : sObjectMgr.GetMangosString(itemText, loc_idx)) : "";
    char const* box_text = boxText ? (boxText > 0 ? sObjectMgr.GetBroadcastText(boxText, loc_idx, GetMenuSession()->GetPlayer()->GetGender()) : sObjectMgr.GetMangosString(boxText, loc_idx)) : "";

    AddMenuItem(Icon, std::string(item_text), dtSender, dtAction, std::string(box_text), Coded);
}

uint32 GossipMenu::MenuItemSender(unsigned int ItemId)
{
    if (ItemId >= m_gItems.size())
        return 0;

    return m_gItems[ ItemId ].m_gSender;
}

uint32 GossipMenu::MenuItemAction(unsigned int ItemId)
{
    if (ItemId >= m_gItems.size())
        return 0;

    return m_gItems[ ItemId ].m_gOptionId;
}

bool GossipMenu::MenuItemCoded(unsigned int ItemId) const
{
    if (ItemId >= m_gItems.size())
        return false;

    return m_gItems[ ItemId ].m_gCoded;
}

void GossipMenu::ClearMenu()
{
    m_gItems.clear();
    m_gItemsData.clear();
    m_gMenuId = 0;
    m_discoveredNode = false;
}

PlayerMenu::PlayerMenu(WorldSession* session) : mGossipMenu(session)
{
}

PlayerMenu::~PlayerMenu()
{
    ClearMenus();
}

void PlayerMenu::ClearMenus()
{
    mGossipMenu.ClearMenu();
    mQuestMenu.ClearMenu();
}

uint32 PlayerMenu::GossipOptionSender(unsigned int Selection)
{
    return mGossipMenu.MenuItemSender(Selection);
}

uint32 PlayerMenu::GossipOptionAction(unsigned int Selection)
{
    return mGossipMenu.MenuItemAction(Selection);
}

bool PlayerMenu::GossipOptionCoded(unsigned int Selection) const
{
    return mGossipMenu.MenuItemCoded(Selection);
}

void PlayerMenu::SendGossipMenu(uint32 textId, ObjectGuid objectGuid)
{
    GossipMessage message;
    message.GossipGuid = ClassicGuidMapping::ToClassic(objectGuid, realmID);
    message.GossipId = int32(objectGuid.GetEntry());
    message.TextId = int32(textId);

    for (uint32 iI = 0; iI < mGossipMenu.MenuItemCount(); ++iI)
    {
        GossipMenuItem const& gItem = mGossipMenu.GetItem(iI);

        GossipOptionData& option = message.Options.emplace_back();
        option.GossipOptionId = int32(iI);
        option.OptionNpc = gItem.m_gIcon;
        option.OptionFlags = gItem.m_gCoded ? 1 : 0;           // makes pop up box password
        option.OrderIndex = int32(iI);
        option.Text = gItem.m_gMessage;
        option.Confirm = gItem.m_gBoxMessage;
    }

    for (uint32 iI = 0; iI < mQuestMenu.MenuItemCount(); ++iI)
    {
        QuestMenuItem const& qItem = mQuestMenu.GetItem(iI);
        uint32 questID = qItem.m_qId;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questID);
        if (!pQuest)
            continue;

        GossipQuestEntry& entry = message.Quests.emplace_back();
        entry.QuestId = int32(questID);
        entry.QuestType = GossipQuestTypeForDialogStatus(qItem.m_qIcon);
        entry.QuestLevel = pQuest->GetQuestLevel();
        entry.QuestFlags[0] = 8;
        entry.Repeatable = pQuest->IsRepeatable();
        entry.Title = pQuest->GetTitle();

        int loc_idx = GetMenuSession()->GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            if (QuestLocale const* ql = sObjectMgr.GetQuestLocale(questID))
                if (ql->Title.size() > (size_t)loc_idx && !ql->Title[loc_idx].empty())
                    entry.Title = ql->Title[loc_idx];
        }
    }

    GetMenuSession()->SendClassic(message);
}

void PlayerMenu::CloseGossip()
{
    GetMenuSession()->SendClassic(GossipComplete());
}

// Outdated
void PlayerMenu::SendPointOfInterest(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, char const* locName)
{
    GossipPoi poi;
    poi.Flags = Flags;
    poi.X = X;
    poi.Y = Y;
    poi.Icon = int32(Icon);
    poi.Importance = int32(Data);
    poi.Name = locName ? locName : "";
    GetMenuSession()->SendClassic(poi);
}

void PlayerMenu::SendPointOfInterest(uint32 poi_id)
{
    PointOfInterest const* poi = sObjectMgr.GetPointOfInterest(poi_id);
    if (!poi)
    {
        sLog.Out(LOG_DBERROR, LOG_LVL_MINIMAL, "Requested send nonexistent POI (Id: %u), ignore.", poi_id);
        return;
    }

    GossipPoi message;
    message.Id = int32(poi_id);
    message.Flags = poi->flags;
    message.X = poi->x;
    message.Y = poi->y;
    message.Icon = int32(poi->icon);
    message.Importance = int32(poi->data);
    message.Name = poi->icon_name;

    int loc_idx = GetMenuSession()->GetSessionDbLocaleIndex();
    if (loc_idx >= 0)
    {
        if (PointOfInterestLocale const* pl = sObjectMgr.GetPointOfInterestLocale(poi_id))
            if (pl->IconName.size() > size_t(loc_idx) && !pl->IconName[loc_idx].empty())
                message.Name = pl->IconName[loc_idx];
    }

    GetMenuSession()->SendClassic(message);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

QuestMenu::QuestMenu()
{
    m_qItems.reserve(16);                                   // can be set for max from most often sizes to speedup push_back and less memory use
}

QuestMenu::~QuestMenu()
{
    ClearMenu();
}

void QuestMenu::AddMenuItem(uint32 QuestId, uint8 Icon)
{
    Quest const* qinfo = sObjectMgr.GetQuestTemplate(QuestId);
    if (!qinfo)
        return;

    MANGOS_ASSERT(m_qItems.size() <= GOSSIP_MAX_MENU_ITEMS);

    QuestMenuItem qItem;

    qItem.m_qId        = QuestId;
    qItem.m_qIcon      = Icon;

    m_qItems.push_back(qItem);
}

bool QuestMenu::HasItem(uint32 questid) const
{
    for (const auto& qItem : m_qItems)
        if (qItem.m_qId == questid)
            return true;
    return false;
}

void QuestMenu::ClearMenu()
{
    m_qItems.clear();
}

namespace
{
// Fills the reward-item/choice-item/faction arrays and money/xp shared by QuestGiverQuestDetails
// and QuestGiverOfferReward. Rewards are always sent in full: this engine's only "hidden rewards"
// behavior was the vanilla QUEST_FLAGS_HIDDEN_REWARDS zero-out, which has no field to carry it here.
QuestRewardsBlock BuildQuestRewardsBlock(Quest const* quest)
{
    QuestRewardsBlock rewards;
    rewards.ItemCount = quest->GetRewItemsCount();
    for (uint32 i = 0; i < QUEST_REWARDS_COUNT; ++i)
    {
        rewards.Items[i].ItemId = quest->RewItemId[i];
        rewards.Items[i].Quantity = quest->RewItemCount[i];
    }
    rewards.ChoiceItemCount = quest->GetRewChoiceItemsCount();
    for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
    {
        rewards.ChoiceItems[i].ItemId = quest->RewChoiceItemId[i];
        rewards.ChoiceItems[i].Quantity = quest->RewChoiceItemCount[i];
    }
    rewards.Money = quest->GetRewOrReqMoney();
    rewards.Xp = quest->XPValue(0);
    for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        rewards.Factions[i].FactionId = quest->RewRepFaction[i];
        rewards.Factions[i].Value = quest->RewRepValue[i];
    }
    rewards.SpellCompletionId = quest->GetRewSpellCast() ? quest->GetRewSpellCast() : quest->GetRewSpell();
    return rewards;
}

std::string LocalizedQuestTitle(Quest const* quest, int locIdx)
{
    std::string title = quest->GetTitle();
    if (locIdx >= 0)
        if (QuestLocale const* ql = sObjectMgr.GetQuestLocale(quest->GetQuestId()))
            if (ql->Title.size() > size_t(locIdx) && !ql->Title[locIdx].empty())
                title = ql->Title[locIdx];
    return title;
}
}

void PlayerMenu::SendQuestGiverQuestList(QuestNpcEmoteInfo fallbackEmote, std::string const& Title, ObjectGuid guid) const
{
    int const locIdx = GetMenuSession()->GetSessionDbLocaleIndex();

    QuestGiverQuestList packet;
    packet.NpcGuid = ClassicGuidMapping::ToClassic(guid, realmID);

    if (QuestGreetingLocale const* questGreeting = sObjectMgr.GetQuestGreetingLocale(guid.GetEntry(), guid.IsAnyTypeCreature() ? 0 : 1))
    {
        if (int32(questGreeting->Content.size()) > locIdx + 1 && !questGreeting->Content[locIdx + 1].empty())
            packet.Greeting = questGreeting->Content[locIdx + 1];
        else
            packet.Greeting = questGreeting->Content[0];
        packet.GreetEmoteDelay = questGreeting->EmoteDelay;
        packet.GreetEmoteType = questGreeting->Emote;
    }
    else
    {
        packet.Greeting = Title;
        packet.GreetEmoteDelay = fallbackEmote.delay;
        packet.GreetEmoteType = fallbackEmote.emote;
    }

    for (uint32 count = 0; count < mQuestMenu.MenuItemCount(); ++count)
    {
        QuestMenuItem const& qmi = mQuestMenu.GetItem(count);
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(qmi.m_qId);
        if (!pQuest)
            continue;

        QuestGiverQuestListEntry& entry = packet.Quests.emplace_back();
        entry.QuestId = pQuest->GetQuestId();
        entry.QuestType = GossipQuestTypeForDialogStatus(qmi.m_qIcon);
        entry.QuestLevel = int32(pQuest->GetQuestLevel());
        entry.QuestFlags[0] = 8;
        entry.Repeatable = pQuest->IsRepeatable();
        entry.Title = LocalizedQuestTitle(pQuest, locIdx);
    }

    GetMenuSession()->SendClassic(packet);
}

void PlayerMenu::SendQuestGiverStatus(uint8 questStatus, ObjectGuid npcGUID) const
{
    QuestGiverStatus packet;
    packet.NpcGuid = ClassicGuidMapping::ToClassic(npcGUID, realmID);
    packet.Status = ToModernQuestGiverStatus(questStatus);
    GetMenuSession()->SendClassic(packet);
}

void PlayerMenu::SendQuestGiverQuestDetails(Quest const* pQuest, ObjectGuid npcGUID, bool ActivateAccept) const
{
    int const locIdx = GetMenuSession()->GetSessionDbLocaleIndex();
    QuestLocale const* ql = locIdx >= 0 ? sObjectMgr.GetQuestLocale(pQuest->GetQuestId()) : nullptr;

    QuestGiverQuestDetails packet;
    packet.NpcGuid = ClassicGuidMapping::ToClassic(npcGUID, realmID);
    packet.QuestId = pQuest->GetQuestId();
    packet.QuestFlags[0] = pQuest->GetQuestFlags();
    packet.SuggestedPartyMembers = int32(pQuest->GetSuggestedPlayers());
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        packet.DetailsEmote[i] = pQuest->DetailsEmote[i];
        packet.DetailsEmoteDelay[i] = pQuest->DetailsEmoteDelay[i];
    }
    for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        if (pQuest->ReqCreatureOrGOId[i] == 0)
            continue;
        int32 const raw = pQuest->ReqCreatureOrGOId[i];
        QuestGiverQuestDetailsObjective& obj = packet.Objectives.emplace_back();
        obj.Id = uint32(packet.Objectives.size() - 1);
        obj.ObjectId = raw > 0 ? raw : ((-raw) | int32(0x80000000));
        obj.Amount = int32(pQuest->ReqCreatureOrGOCount[i]);
        obj.Type = raw > 0 ? 0 : 2;  // monster : gameobject
    }
    for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        if (pQuest->ReqItemId[i] == 0)
            continue;
        QuestGiverQuestDetailsObjective& obj = packet.Objectives.emplace_back();
        obj.Id = uint32(packet.Objectives.size() - 1);
        obj.ObjectId = int32(pQuest->ReqItemId[i]);
        obj.Amount = int32(pQuest->ReqItemCount[i]);
        obj.Type = 1;  // item
    }
    packet.Rewards = BuildQuestRewardsBlock(pQuest);
    packet.Title = LocalizedQuestTitle(pQuest, locIdx);
    packet.DescriptionText = pQuest->GetDetails();
    packet.LogDescription = pQuest->GetObjectives();
    if (ql)
    {
        if (ql->Details.size() > size_t(locIdx) && !ql->Details[locIdx].empty())
            packet.DescriptionText = ql->Details[locIdx];
        if (ql->Objectives.size() > size_t(locIdx) && !ql->Objectives[locIdx].empty())
            packet.LogDescription = ql->Objectives[locIdx];
    }
    packet.AutoLaunched = ActivateAccept; // I have no idea here this is used

    GetMenuSession()->SendClassic(packet);
}

void PlayerMenu::SendQuestGiverOfferReward(Quest const* pQuest, ObjectGuid npcGuid, bool EnableNext) const
{
    int const locIdx = GetMenuSession()->GetSessionDbLocaleIndex();
    QuestLocale const* ql = locIdx >= 0 ? sObjectMgr.GetQuestLocale(pQuest->GetQuestId()) : nullptr;

    QuestGiverOfferReward packet;
    packet.NpcGuid = ClassicGuidMapping::ToClassic(npcGuid, realmID);
    packet.QuestId = pQuest->GetQuestId();
    packet.QuestFlags[0] = pQuest->GetQuestFlags();
    packet.SuggestedPartyMembers = int32(pQuest->GetSuggestedPlayers());
    for (uint32 emoteId : pQuest->OfferRewardEmote)
    {
        if (emoteId == 0)
            break;
        QuestDescEmoteEntry& emote = packet.Emotes.emplace_back();
        emote.Type = int32(pQuest->OfferRewardEmote[packet.Emotes.size() - 1]);
        emote.Delay = pQuest->OfferRewardEmoteDelay[packet.Emotes.size() - 1];
    }
    packet.Rewards = BuildQuestRewardsBlock(pQuest);
    packet.Title = LocalizedQuestTitle(pQuest, locIdx);
    packet.OfferRewardText = pQuest->GetOfferRewardText();
    if (ql && ql->OfferRewardText.size() > size_t(locIdx) && !ql->OfferRewardText[locIdx].empty())
        packet.OfferRewardText = ql->OfferRewardText[locIdx];
    packet.AutoLaunched = EnableNext; // I have no idea here this is used

    GetMenuSession()->SendClassic(packet);
}

void PlayerMenu::SendQuestGiverRequestItems(Quest const* pQuest, ObjectGuid npcGuid, bool isComplete, bool closeOnCancel) const
{
    // We can always call to RequestItems, but this packet only goes out if there are actually
    // items. Otherwise, we'll skip straight to the OfferReward
    if (pQuest->GetRequestItemsText().empty() || ((pQuest->GetReqItemsCount() == 0) && isComplete))
    {
        // Quests that don't require items use the RequestItemsText field to store the text
        // that is shown when you talk to the quest giver while the quest is incomplete.
        // Therefore the text should not be shown for them when the quest is complete.
        // For quests that do require items, it is self explanatory.
        SendQuestGiverOfferReward(pQuest, npcGuid, true);
        return;
    }

    int const locIdx = GetMenuSession()->GetSessionDbLocaleIndex();
    QuestLocale const* ql = locIdx >= 0 ? sObjectMgr.GetQuestLocale(pQuest->GetQuestId()) : nullptr;

    QuestGiverRequestItems packet;
    packet.NpcGuid = ClassicGuidMapping::ToClassic(npcGuid, realmID);
    packet.QuestId = pQuest->GetQuestId();
    packet.QuestFlags[0] = pQuest->GetQuestFlags();
    packet.SuggestedPartyMembers = int32(pQuest->GetSuggestedPlayers());
    packet.MoneyToGet = pQuest->GetRewOrReqMoney() < 0 ? -pQuest->GetRewOrReqMoney() : 0;
    packet.CompEmoteType = isComplete ? pQuest->GetCompleteEmote() : pQuest->GetIncompleteEmote();
    for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        if (!pQuest->ReqItemId[i])
            continue;
        QuestGiverRequestItemsObjective& obj = packet.Collect.emplace_back();
        obj.ObjectId = pQuest->ReqItemId[i];
        obj.Amount = pQuest->ReqItemCount[i];
    }
    packet.IsComplete = isComplete;
    packet.Title = LocalizedQuestTitle(pQuest, locIdx);
    packet.RequestItemsText = pQuest->GetRequestItemsText();
    if (ql && ql->RequestItemsText.size() > size_t(locIdx) && !ql->RequestItemsText[locIdx].empty())
        packet.RequestItemsText = ql->RequestItemsText[locIdx];

    (void)closeOnCancel;  // no modern field carries this -- Escape/gossip cancel already covers it
    GetMenuSession()->SendClassic(packet);
}
