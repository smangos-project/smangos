// Round trips for the modern quest wire.
#include "Packets/QuestMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
ClassicGuid TestGuid(uint64 low) { return { low, 0x2000000000000000ULL }; }
}

TEST_CASE("Quest giver interaction requests round-trip", "[packets]")
{
    QuestgiverStatusQuery status;
    status.Guid = TestGuid(5);
    RoundTrip(status, [](QuestgiverStatusQuery const& a, QuestgiverStatusQuery const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(status);

    QuestgiverHello hello;
    hello.Guid = TestGuid(5);
    RoundTrip(hello, [](QuestgiverHello const& a, QuestgiverHello const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(hello);

    QuestgiverQueryQuest query;
    query.Guid = TestGuid(5);
    query.QuestId = 783;
    RoundTrip(query, [](QuestgiverQueryQuest const& a, QuestgiverQueryQuest const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
    });
    RejectsEveryTruncation(query);

    QuestgiverAcceptQuest accept;
    accept.Guid = TestGuid(5);
    accept.QuestId = 783;
    RoundTrip(accept, [](QuestgiverAcceptQuest const& a, QuestgiverAcceptQuest const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
    });
    RejectsEveryTruncation(accept);

    QuestgiverCompleteQuest complete;
    complete.Guid = TestGuid(5);
    complete.QuestId = 783;
    RoundTrip(complete, [](QuestgiverCompleteQuest const& a, QuestgiverCompleteQuest const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
    });
    RejectsEveryTruncation(complete);

    QuestgiverRequestReward request;
    request.Guid = TestGuid(5);
    request.QuestId = 783;
    RoundTrip(request, [](QuestgiverRequestReward const& a, QuestgiverRequestReward const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
    });
    RejectsEveryTruncation(request);

    QuestGiverCloseQuest close;
    close.QuestId = 783;
    RoundTrip(close, [](QuestGiverCloseQuest const& a, QuestGiverCloseQuest const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(close);
}

TEST_CASE("QuestgiverChooseReward round-trips an item id", "[packets]")
{
    QuestgiverChooseReward choose;
    choose.Guid = TestGuid(5);
    choose.QuestId = 783;
    choose.RewardItemId = 2589;
    RoundTrip(choose, [](QuestgiverChooseReward const& a, QuestgiverChooseReward const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.RewardItemId == b.RewardItemId);
    });
    RejectsEveryTruncation(choose);
}

TEST_CASE("Quest log, confirm-accept and party-share requests round-trip", "[packets]")
{
    QuestLogRemoveQuest remove;
    remove.Slot = 3;
    RoundTrip(remove, [](QuestLogRemoveQuest const& a, QuestLogRemoveQuest const& b) { CHECK(a.Slot == b.Slot); });
    RejectsEveryTruncation(remove);

    QuestConfirmAccept confirm;
    confirm.QuestId = 783;
    RoundTrip(confirm, [](QuestConfirmAccept const& a, QuestConfirmAccept const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(confirm);

    PushQuestToParty push;
    push.QuestId = 783;
    RoundTrip(push, [](PushQuestToParty const& a, PushQuestToParty const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(push);

    QuestPushResult result;
    result.Guid = TestGuid(5);
    result.QuestId = 783;
    result.Msg = 2;
    RoundTrip(result, [](QuestPushResult const& a, QuestPushResult const& b)
    {
        CHECK(a.Guid == b.Guid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.Msg == b.Msg);
    });
    RejectsEveryTruncation(result);
}

TEST_CASE("QuestGiverStatus round-trips a modern status bitmask", "[packets]")
{
    QuestGiverStatus status;
    status.NpcGuid = TestGuid(5);
    status.Status = 0x400;  // Quest
    RoundTrip(status, [](QuestGiverStatus const& a, QuestGiverStatus const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.Status == b.Status);
    });
    RejectsEveryTruncation(status);
}

TEST_CASE("QuestLogFull and QuestUpdateComplete round-trip", "[packets]")
{
    QuestLogFull full;
    RoundTrip(full, [](QuestLogFull const&, QuestLogFull const&) {});

    QuestUpdateComplete complete;
    complete.QuestId = 783;
    RoundTrip(complete, [](QuestUpdateComplete const& a, QuestUpdateComplete const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(complete);
}

TEST_CASE("QuestConfirmAcceptResponse and QuestPushResultResponse round-trip", "[packets]")
{
    QuestConfirmAcceptResponse confirm;
    confirm.QuestId = 783;
    confirm.InitiatedBy = TestGuid(5);
    confirm.QuestTitle = "A Threat Within";
    RoundTrip(confirm, [](QuestConfirmAcceptResponse const& a, QuestConfirmAcceptResponse const& b)
    {
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.InitiatedBy == b.InitiatedBy);
        CHECK(a.QuestTitle == b.QuestTitle);
    });
    RejectsEveryTruncation(confirm);

    QuestPushResultResponse push;
    push.SenderGuid = TestGuid(5);
    push.Result = 2;
    push.QuestTitle = "A Threat Within";
    RoundTrip(push, [](QuestPushResultResponse const& a, QuestPushResultResponse const& b)
    {
        CHECK(a.SenderGuid == b.SenderGuid);
        CHECK(a.Result == b.Result);
        CHECK(a.QuestTitle == b.QuestTitle);
    });
    RejectsEveryTruncation(push);
}

TEST_CASE("QuestUpdateAddCredit round-trips a kill and an item objective", "[packets]")
{
    QuestUpdateAddCredit kill;
    kill.VictimGuid = TestGuid(9);
    kill.QuestId = 783;
    kill.ObjectId = 6;
    kill.Count = 1;
    kill.Required = 3;
    kill.ObjectiveType = 0;
    RoundTrip(kill, [](QuestUpdateAddCredit const& a, QuestUpdateAddCredit const& b)
    {
        CHECK(a.VictimGuid == b.VictimGuid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.ObjectId == b.ObjectId);
        CHECK(a.Count == b.Count);
        CHECK(a.Required == b.Required);
        CHECK(a.ObjectiveType == b.ObjectiveType);
    });
    RejectsEveryTruncation(kill);

    QuestUpdateAddCredit item;
    item.QuestId = 783;
    item.ObjectId = 2589;
    item.Count = 1;
    item.Required = 1;
    item.ObjectiveType = 1;
    RoundTrip(item, [](QuestUpdateAddCredit const& a, QuestUpdateAddCredit const& b)
    {
        CHECK(a.ObjectId == b.ObjectId);
        CHECK(a.ObjectiveType == b.ObjectiveType);
    });
    RejectsEveryTruncation(item);
}

TEST_CASE("QuestGiverQuestList round-trips several quests and a greeting", "[packets]")
{
    QuestGiverQuestList list;
    list.NpcGuid = TestGuid(5);
    list.GreetEmoteDelay = 1000;
    list.GreetEmoteType = 5;
    list.Greeting = "Welcome, traveler.";

    GossipQuestEntry& first = list.Quests.emplace_back();
    first.QuestId = 783;
    first.QuestType = 2;
    first.QuestLevel = 12;
    first.QuestFlags[0] = 8;
    first.Repeatable = false;
    first.Title = "A Threat Within";

    GossipQuestEntry& second = list.Quests.emplace_back();
    second.QuestId = 784;
    second.QuestType = 4;
    second.QuestLevel = 13;
    second.QuestFlags[0] = 8;
    second.Repeatable = true;
    second.Title = "The Fargodeep Mine";

    RoundTrip(list, [](QuestGiverQuestList const& a, QuestGiverQuestList const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.GreetEmoteDelay == b.GreetEmoteDelay);
        CHECK(a.GreetEmoteType == b.GreetEmoteType);
        CHECK(a.Greeting == b.Greeting);
        REQUIRE(a.Quests.size() == b.Quests.size());
        for (size_t i = 0; i < a.Quests.size(); ++i)
        {
            CHECK(a.Quests[i].QuestId == b.Quests[i].QuestId);
            CHECK(a.Quests[i].QuestType == b.Quests[i].QuestType);
            CHECK(a.Quests[i].QuestLevel == b.Quests[i].QuestLevel);
            CHECK(a.Quests[i].QuestFlags[0] == b.Quests[i].QuestFlags[0]);
            CHECK(a.Quests[i].Repeatable == b.Quests[i].Repeatable);
            CHECK(a.Quests[i].Title == b.Quests[i].Title);
        }
    });
    RejectsEveryTruncation(list);

    QuestGiverQuestList bare;
    bare.NpcGuid = TestGuid(6);
    RoundTrip(bare, [](QuestGiverQuestList const&, QuestGiverQuestList const& b) { CHECK(b.Quests.empty()); });
    RejectsEveryTruncation(bare);
}

namespace
{
QuestRewardsBlock TestRewards()
{
    QuestRewardsBlock rewards;
    rewards.ItemCount = 1;
    rewards.Items[0].ItemId = 2101;
    rewards.Items[0].Quantity = 1;
    rewards.ChoiceItemCount = 2;
    rewards.ChoiceItems[0].ItemId = 2201;
    rewards.ChoiceItems[0].Quantity = 1;
    rewards.ChoiceItems[1].ItemId = 2202;
    rewards.ChoiceItems[1].Quantity = 1;
    rewards.Money = 500;
    rewards.Xp = 250;
    rewards.Factions[0].FactionId = 69;
    rewards.Factions[0].Value = 500;
    rewards.SpellCompletionId = 0;
    return rewards;
}

void CheckRewards(QuestRewardsBlock const& a, QuestRewardsBlock const& b)
{
    CHECK(a.ItemCount == b.ItemCount);
    CHECK(a.Items[0].ItemId == b.Items[0].ItemId);
    CHECK(a.Items[0].Quantity == b.Items[0].Quantity);
    CHECK(a.ChoiceItemCount == b.ChoiceItemCount);
    CHECK(a.ChoiceItems[0].ItemId == b.ChoiceItems[0].ItemId);
    CHECK(a.ChoiceItems[1].ItemId == b.ChoiceItems[1].ItemId);
    CHECK(a.Money == b.Money);
    CHECK(a.Xp == b.Xp);
    CHECK(a.Factions[0].FactionId == b.Factions[0].FactionId);
    CHECK(a.Factions[0].Value == b.Factions[0].Value);
}
}

TEST_CASE("QuestRewardsBlock round-trips items, choices, money and reputation", "[packets]")
{
    struct RewardsHolder
    {
        QuestRewardsBlock Rewards;
        void Write(ByteBuffer& data) const { Rewards.Write(data); }
        bool Read(ByteBuffer& data) { return Rewards.Read(data); }
    };

    RewardsHolder holder;
    holder.Rewards = TestRewards();
    RoundTrip(holder, [](RewardsHolder const& a, RewardsHolder const& b) { CheckRewards(a.Rewards, b.Rewards); });
    RejectsEveryTruncation(holder);
}

TEST_CASE("QuestGiverQuestDetails round-trips objectives and rewards", "[packets]")
{
    QuestGiverQuestDetails details;
    details.NpcGuid = TestGuid(5);
    details.QuestId = 783;
    details.QuestFlags[0] = 8;
    details.SuggestedPartyMembers = 0;
    details.DetailsEmote[0] = 1;
    details.DetailsEmoteDelay[0] = 500;

    QuestGiverQuestDetailsObjective& monster = details.Objectives.emplace_back();
    monster.Id = 0;
    monster.Type = 0;
    monster.ObjectId = 6;
    monster.Amount = 3;

    QuestGiverQuestDetailsObjective& item = details.Objectives.emplace_back();
    item.Id = 1;
    item.Type = 1;
    item.ObjectId = 2589;
    item.Amount = 2;

    details.Rewards = TestRewards();
    details.Title = "A Threat Within";
    details.DescriptionText = "Kobolds have been spotted nearby.";
    details.LogDescription = "Slay the kobolds.";
    details.AutoLaunched = true;

    RoundTrip(details, [](QuestGiverQuestDetails const& a, QuestGiverQuestDetails const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.QuestFlags[0] == b.QuestFlags[0]);
        CHECK(a.DetailsEmote[0] == b.DetailsEmote[0]);
        CHECK(a.DetailsEmoteDelay[0] == b.DetailsEmoteDelay[0]);
        REQUIRE(a.Objectives.size() == b.Objectives.size());
        for (size_t i = 0; i < a.Objectives.size(); ++i)
        {
            CHECK(a.Objectives[i].Id == b.Objectives[i].Id);
            CHECK(a.Objectives[i].Type == b.Objectives[i].Type);
            CHECK(a.Objectives[i].ObjectId == b.Objectives[i].ObjectId);
            CHECK(a.Objectives[i].Amount == b.Objectives[i].Amount);
        }
        CheckRewards(a.Rewards, b.Rewards);
        CHECK(a.Title == b.Title);
        CHECK(a.DescriptionText == b.DescriptionText);
        CHECK(a.LogDescription == b.LogDescription);
        CHECK(a.AutoLaunched == b.AutoLaunched);
    });
    RejectsEveryTruncation(details);

    QuestGiverQuestDetails bare;
    bare.NpcGuid = TestGuid(6);
    bare.QuestId = 1;
    RoundTrip(bare, [](QuestGiverQuestDetails const&, QuestGiverQuestDetails const& b) { CHECK(b.Objectives.empty()); });
    RejectsEveryTruncation(bare);
}

TEST_CASE("QuestGiverOfferReward round-trips emotes and rewards", "[packets]")
{
    QuestGiverOfferReward offer;
    offer.NpcGuid = TestGuid(5);
    offer.QuestId = 783;
    offer.QuestFlags[0] = 8;
    offer.SuggestedPartyMembers = 0;

    QuestDescEmoteEntry& emote = offer.Emotes.emplace_back();
    emote.Type = 1;
    emote.Delay = 0;

    offer.Rewards = TestRewards();
    offer.Title = "A Threat Within";
    offer.OfferRewardText = "You have done well.";
    offer.AutoLaunched = true;

    RoundTrip(offer, [](QuestGiverOfferReward const& a, QuestGiverOfferReward const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.QuestFlags[0] == b.QuestFlags[0]);
        REQUIRE(a.Emotes.size() == b.Emotes.size());
        CHECK(a.Emotes[0].Type == b.Emotes[0].Type);
        CHECK(a.Emotes[0].Delay == b.Emotes[0].Delay);
        CheckRewards(a.Rewards, b.Rewards);
        CHECK(a.Title == b.Title);
        CHECK(a.OfferRewardText == b.OfferRewardText);
        CHECK(a.AutoLaunched == b.AutoLaunched);
    });
    RejectsEveryTruncation(offer);

    QuestGiverOfferReward bare;
    bare.NpcGuid = TestGuid(6);
    bare.QuestId = 1;
    RoundTrip(bare, [](QuestGiverOfferReward const&, QuestGiverOfferReward const& b) { CHECK(b.Emotes.empty()); });
    RejectsEveryTruncation(bare);
}

TEST_CASE("QuestGiverRequestItems round-trips a collect list and completion status", "[packets]")
{
    QuestGiverRequestItems request;
    request.NpcGuid = TestGuid(5);
    request.QuestId = 783;
    request.QuestFlags[0] = 8;
    request.SuggestedPartyMembers = 0;
    request.MoneyToGet = 0;
    request.CompEmoteType = 1;

    QuestGiverRequestItemsObjective& collect = request.Collect.emplace_back();
    collect.ObjectId = 2589;
    collect.Amount = 4;

    request.IsComplete = true;
    request.Title = "A Threat Within";
    request.RequestItemsText = "Bring me the kobold hides.";

    RoundTrip(request, [](QuestGiverRequestItems const& a, QuestGiverRequestItems const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.QuestFlags[0] == b.QuestFlags[0]);
        CHECK(a.MoneyToGet == b.MoneyToGet);
        CHECK(a.CompEmoteType == b.CompEmoteType);
        REQUIRE(a.Collect.size() == b.Collect.size());
        CHECK(a.Collect[0].ObjectId == b.Collect[0].ObjectId);
        CHECK(a.Collect[0].Amount == b.Collect[0].Amount);
        CHECK(a.IsComplete == b.IsComplete);
        CHECK(a.Title == b.Title);
        CHECK(a.RequestItemsText == b.RequestItemsText);
    });
    RejectsEveryTruncation(request);

    QuestGiverRequestItems incomplete;
    incomplete.NpcGuid = TestGuid(6);
    incomplete.QuestId = 1;
    incomplete.IsComplete = false;
    RoundTrip(incomplete, [](QuestGiverRequestItems const&, QuestGiverRequestItems const& b)
    {
        CHECK_FALSE(b.IsComplete);
        CHECK(b.Collect.empty());
    });
    RejectsEveryTruncation(incomplete);
}

TEST_CASE("QuestGiverQuestComplete round-trips xp and money", "[packets]")
{
    QuestGiverQuestComplete complete;
    complete.QuestId = 783;
    complete.XpReward = 250;
    complete.MoneyReward = 500;
    RoundTrip(complete, [](QuestGiverQuestComplete const& a, QuestGiverQuestComplete const& b)
    {
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.XpReward == b.XpReward);
        CHECK(a.MoneyReward == b.MoneyReward);
    });
    RejectsEveryTruncation(complete);
}

TEST_CASE("Quest failure toasts round-trip", "[packets]")
{
    QuestGiverInvalidQuest invalid;
    invalid.Reason = 23;
    invalid.ReasonText = "not enough money";
    RoundTrip(invalid, [](QuestGiverInvalidQuest const& a, QuestGiverInvalidQuest const& b)
    {
        CHECK(a.Reason == b.Reason);
        CHECK(a.ContributionRewardId == b.ContributionRewardId);
        CHECK(a.SendErrorMessage == b.SendErrorMessage);
        CHECK(a.ReasonText == b.ReasonText);
    });
    RejectsEveryTruncation(invalid);

    QuestGiverQuestFailed failed;
    failed.QuestId = 783;
    failed.Reason = 4;
    RoundTrip(failed, [](QuestGiverQuestFailed const& a, QuestGiverQuestFailed const& b)
    {
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(failed);

    QuestUpdateFailed updateFailed;
    updateFailed.QuestId = 783;
    RoundTrip(updateFailed, [](QuestUpdateFailed const& a, QuestUpdateFailed const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(updateFailed);

    QuestUpdateFailedTimer timerFailed;
    timerFailed.QuestId = 783;
    RoundTrip(timerFailed, [](QuestUpdateFailedTimer const& a, QuestUpdateFailedTimer const& b) { CHECK(a.QuestId == b.QuestId); });
    RejectsEveryTruncation(timerFailed);
}

TEST_CASE("QuestGiverStatusMultiple round-trips several givers", "[packets]")
{
    RoundTrip(QuestGiverStatusMultipleQuery(), [](QuestGiverStatusMultipleQuery const&, QuestGiverStatusMultipleQuery const&) {});

    QuestGiverStatusMultiple multiple;
    multiple.Givers.resize(2);
    multiple.Givers[0].Guid = TestGuid(5);
    multiple.Givers[0].Status = 0x400;
    multiple.Givers[1].Guid = TestGuid(9);
    multiple.Givers[1].Status = 0x1000;
    RoundTrip(multiple, [](QuestGiverStatusMultiple const& a, QuestGiverStatusMultiple const& b)
    {
        REQUIRE(a.Givers.size() == b.Givers.size());
        for (size_t i = 0; i < a.Givers.size(); ++i)
        {
            CHECK(a.Givers[i].Guid == b.Givers[i].Guid);
            CHECK(a.Givers[i].Status == b.Givers[i].Status);
        }
    });
    RejectsEveryTruncation(multiple);

    QuestGiverStatusMultiple none;
    RoundTrip(none, [](QuestGiverStatusMultiple const&, QuestGiverStatusMultiple const& b) { CHECK(b.Givers.empty()); });
}

TEST_CASE("QueryQuestInfo and its response round-trip", "[packets]")
{
    QueryQuestInfo query;
    query.QuestId = 783;
    query.QuestGiver = TestGuid(5);
    RoundTrip(query, [](QueryQuestInfo const& a, QueryQuestInfo const& b)
    {
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.QuestGiver == b.QuestGiver);
    });
    RejectsEveryTruncation(query);

    QueryQuestInfoResponse denied;
    denied.QuestId = 99999;
    RoundTrip(denied, [](QueryQuestInfoResponse const& a, QueryQuestInfoResponse const& b)
    {
        CHECK(a.QuestId == b.QuestId);
        CHECK_FALSE(b.Allow);
    });
    RejectsEveryTruncation(denied);

    QueryQuestInfoResponse info;
    info.QuestId = 783;
    info.Allow = true;
    info.QuestType = 2;
    info.QuestLevel = 1;
    info.MinLevel = 1;
    info.SortId = 9;
    info.SuggestedGroupNum = 3;
    info.RewardNextQuest = 7;
    info.RewardXPDifficulty = 3;
    info.RewardXPMultiplier = 1.25f;
    info.RewardMoney = 500;
    info.RewardSpell = 1234;
    info.Flags = 0x8;
    info.RewardItems[1] = { 2589, 2 };
    info.ChoiceItems[0] = { 2092, 1, 1234 };
    info.PoiContinent = 0;
    info.PoiX = -8913.5f;
    info.PoiY = -133.25f;
    info.PoiPriority = 2;
    info.Factions[2] = { 72, 250 };
    info.TimeAllowed = 3600;
    info.Objectives.resize(2);
    info.Objectives[0] = { 0, 0, 6, 8, "Kobolds slain", 0 };
    info.Objectives[1] = { 1, 1, 750, 4, "", 4 };
    info.LogTitle = "Kobold Camp Cleanup";
    info.LogDescription = "Kill eight kobolds";
    info.QuestDescription = "They are stealing my candles.";
    info.QuestCompletionLog = "Return to Deputy Willem";
    RoundTrip(info, [](QueryQuestInfoResponse const& a, QueryQuestInfoResponse const& b)
    {
        CHECK(b.Allow);
        CHECK(a.QuestId == b.QuestId);
        CHECK(a.QuestType == b.QuestType);
        CHECK(a.QuestLevel == b.QuestLevel);
        CHECK(a.MinLevel == b.MinLevel);
        CHECK(a.SortId == b.SortId);
        CHECK(a.SuggestedGroupNum == b.SuggestedGroupNum);
        CHECK(a.RewardNextQuest == b.RewardNextQuest);
        CHECK(a.RewardXPDifficulty == b.RewardXPDifficulty);
        CHECK(a.RewardXPMultiplier == b.RewardXPMultiplier);
        CHECK(a.RewardMoney == b.RewardMoney);
        CHECK(a.RewardSpell == b.RewardSpell);
        CHECK(a.Flags == b.Flags);
        CHECK(a.RewardItems[1].ItemId == b.RewardItems[1].ItemId);
        CHECK(a.RewardItems[1].Quantity == b.RewardItems[1].Quantity);
        CHECK(a.ChoiceItems[0].ItemId == b.ChoiceItems[0].ItemId);
        CHECK(a.ChoiceItems[0].DisplayId == b.ChoiceItems[0].DisplayId);
        CHECK(a.PoiX == b.PoiX);
        CHECK(a.PoiY == b.PoiY);
        CHECK(a.PoiPriority == b.PoiPriority);
        CHECK(a.Factions[2].FactionId == b.Factions[2].FactionId);
        CHECK(a.Factions[2].Value == b.Factions[2].Value);
        CHECK(a.TimeAllowed == b.TimeAllowed);
        REQUIRE(a.Objectives.size() == b.Objectives.size());
        for (size_t i = 0; i < a.Objectives.size(); ++i)
        {
            CHECK(a.Objectives[i].Id == b.Objectives[i].Id);
            CHECK(a.Objectives[i].Type == b.Objectives[i].Type);
            CHECK(a.Objectives[i].ObjectId == b.Objectives[i].ObjectId);
            CHECK(a.Objectives[i].Amount == b.Objectives[i].Amount);
            CHECK(a.Objectives[i].Description == b.Objectives[i].Description);
            CHECK(a.Objectives[i].StorageIndex == b.Objectives[i].StorageIndex);
        }
        CHECK(a.LogTitle == b.LogTitle);
        CHECK(a.LogDescription == b.LogDescription);
        CHECK(a.QuestDescription == b.QuestDescription);
        CHECK(a.QuestCompletionLog == b.QuestCompletionLog);
    });
    RejectsEveryTruncation(info);
}
