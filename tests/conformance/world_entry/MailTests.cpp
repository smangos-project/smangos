// A GM bot mails a plain bot a letter over the live CMSG_SEND_MAIL request/response path, then
// seeds an item mail and a money mail through the ".send items"/".send money" GM commands (both
// still instant delivery -- a player-sent mail carrying an item or money gets the full configured
// MailDeliveryDelay, 3600s in this environment, too long for a scenario to wait on; both commands
// need SEC_ADMINISTRATOR, so the pooled GM account was bumped from SEC_DEVELOPER, matching the
// precedent set for ".npc spawn add" in an earlier slice). The recipient lists, marks read, takes
// the item and the money, copies the letter's body to a readable item, returns the letter to the
// sender, and deletes the money mail.
#include "Packets/MailMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <optional>
#include <vector>

namespace
{
constexpr int32_t LINEN_CLOTH = 2589;

std::vector<ClassicGuid> KnownGameObjects(TestBot const& bot)
{
    std::vector<ClassicGuid> objects;
    for (BotKnownObject const& object : bot.GetKnownObjects())
        if (object.ObjectType == BOT_TYPEID_GAMEOBJECT)
            objects.push_back(object.Guid);
    return objects;
}

std::optional<ClassicGuid> FindNewGameObject(TestBot const& bot, std::vector<ClassicGuid> const& before)
{
    for (ClassicGuid const& guid : KnownGameObjects(bot))
        if (std::find(before.begin(), before.end(), guid) == before.end())
            return guid;
    return std::nullopt;
}
}

TEST_CASE("mail: a letter is sent live, listed, read, its item and money taken, its body copied, returned and deleted", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot partner;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await partner.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        SayCommand(gm, ".namego " + partner.GetCharacterName());
        co_await TestDelay(2500);

        std::vector<ClassicGuid> const before = KnownGameObjects(gm);
        SayCommand(gm, ".gobject tmpadd 32349 120"); // a temporary mailbox, gone in 120s regardless
        REQUIRE(co_await TestWaitUntil([&]() { return FindNewGameObject(gm, before).has_value(); }, 10000));
        ClassicGuid const mailbox = *FindNewGameObject(gm, before);

        SendMail letter;
        letter.MailboxGuid = mailbox;
        letter.StationeryId = 41;
        letter.Target = partner.GetCharacterName();
        letter.Subject = "Hello";
        letter.Body = "A message for you";
        gm.Send(letter);
        auto const sendResult = co_await gm.Packet<MailCommandResult>(5000);
        REQUIRE(sendResult);
        CHECK(sendResult->Command == 0);  // MAIL_SEND
        CHECK(sendResult->ErrorCode == 0); // MAIL_OK

        SayCommand(gm, ".send items " + partner.GetCharacterName() + " \"Cloth\" \"five cloth\" " + std::to_string(LINEN_CLOTH) + ":5");
        SayCommand(gm, ".send money " + partner.GetCharacterName() + " \"Gold\" \"for you\" 500");
        co_await TestDelay(1500);

        GetMailList list;
        list.MailboxGuid = mailbox;
        partner.Send(list);
        auto const mailList = co_await partner.Packet<MailListResult>(5000);
        REQUIRE(mailList);
        // The pooled partner account keeps whatever a previous run of this scenario left behind, so
        // there may be more than 3; mail is listed newest-first (MasterPlayer::AddMail push_front),
        // so the first subject match is always this run's own mail.
        REQUIRE(mailList->Mails.size() >= 3);

        auto const findBySubject = [&](std::string const& subject) -> MailListEntry const*
        {
            for (MailListEntry const& entry : mailList->Mails)
                if (entry.Subject == subject)
                    return &entry;
            return nullptr;
        };

        MailListEntry const* letterMail = findBySubject("Hello");
        MailListEntry const* itemMail = findBySubject("Cloth");
        MailListEntry const* moneyMail = findBySubject("Gold");
        REQUIRE(letterMail);
        REQUIRE(itemMail);
        REQUIRE(moneyMail);
        CHECK(letterMail->Body == "A message for you");
        CHECK(letterMail->SenderGuid == gmGuid);
        REQUIRE(itemMail->Attachments.size() == 1);
        CHECK(itemMail->Attachments[0].Item.ItemId == LINEN_CLOTH);
        CHECK(itemMail->Attachments[0].Count == 5);
        CHECK(moneyMail->SentMoney == 500);

        MailMarkAsRead markRead;
        markRead.MailboxGuid = mailbox;
        markRead.MailId = letterMail->MailId;
        partner.Send(markRead);

        MailTakeItem takeItem;
        takeItem.MailboxGuid = mailbox;
        takeItem.MailId = itemMail->MailId;
        takeItem.AttachId = itemMail->Attachments[0].AttachId;
        partner.Send(takeItem);
        auto const takeItemResult = co_await partner.Packet<MailCommandResult>(5000);
        REQUIRE(takeItemResult);
        CHECK(takeItemResult->Command == 2);  // MAIL_ITEM_TAKEN
        CHECK(takeItemResult->ErrorCode == 0); // MAIL_OK

        MailTakeMoney takeMoney;
        takeMoney.MailboxGuid = mailbox;
        takeMoney.MailId = moneyMail->MailId;
        takeMoney.Money = 500;
        partner.Send(takeMoney);
        auto const takeMoneyResult = co_await partner.Packet<MailCommandResult>(5000);
        REQUIRE(takeMoneyResult);
        CHECK(takeMoneyResult->Command == 1);  // MAIL_MONEY_TAKEN
        CHECK(takeMoneyResult->ErrorCode == 0); // MAIL_OK

        // Not covered live past this point: ItemTextQuery on the created body item. Neither
        // MailCommandResult(MAIL_MADE_PERMANENT) nor any other message on this path carries the new
        // item's guid (StoreItem() here, unlike ".additem"'s StoreNewItem(), sends no ItemPushResult
        // either -- same story as MailTakeItem above), so a bot has no way to learn it, matching
        // PetitionTests.cpp's own note about an item field it can't parse. Wire-tested only
        // (MailMessagesTests.cpp).
        MailCreateTextItem createText;
        createText.MailboxGuid = mailbox;
        createText.MailId = letterMail->MailId;
        partner.Send(createText);
        auto const createTextResult = co_await partner.Packet<MailCommandResult>(5000);
        REQUIRE(createTextResult);
        CHECK(createTextResult->Command == 5);  // MAIL_MADE_PERMANENT
        CHECK(createTextResult->ErrorCode == 0); // MAIL_OK

        MailReturnToSender returnToSender;
        returnToSender.MailId = letterMail->MailId;
        returnToSender.SenderGuid = gmGuid;
        partner.Send(returnToSender);
        auto const returnResult = co_await partner.Packet<MailCommandResult>(5000);
        REQUIRE(returnResult);
        CHECK(returnResult->Command == 3);  // MAIL_RETURNED_TO_SENDER
        CHECK(returnResult->ErrorCode == 0); // MAIL_OK

        MailDelete deleteMoney;
        deleteMoney.MailId = moneyMail->MailId;
        partner.Send(deleteMoney);
        auto const deleteResult = co_await partner.Packet<MailCommandResult>(5000);
        REQUIRE(deleteResult);
        CHECK(deleteResult->Command == 4);  // MAIL_DELETED
        CHECK(deleteResult->ErrorCode == 0); // MAIL_OK

        // No cleanup needed: a ".gobject tmpadd" summon isn't a DB spawn, and it expires on its own.
    });
}
