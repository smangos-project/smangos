// Round trips for the modern mail wire.
#include "Packets/MailMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("SendMail round-trips with and without an attachment", "[packets]")
{
    SendMail packet;
    packet.MailboxGuid = { 1, 0 };
    packet.StationeryId = 41;
    packet.SendMoney = 1234;
    packet.Cod = 5678;
    packet.Target = "Recipient";
    packet.Subject = "Hello";
    packet.Body = "A letter";
    RoundTrip(packet, [](SendMail const& a, SendMail const& b)
    {
        CHECK(a.MailboxGuid == b.MailboxGuid);
        CHECK(a.StationeryId == b.StationeryId);
        CHECK(a.SendMoney == b.SendMoney);
        CHECK(a.Cod == b.Cod);
        CHECK(a.Target == b.Target);
        CHECK(a.Subject == b.Subject);
        CHECK(a.Body == b.Body);
        CHECK(a.Attachments.empty() == b.Attachments.empty());
    });
    RejectsEveryTruncation(packet);

    packet.Attachments.push_back({ 0, ClassicGuid{ 99, 0 } });
    RoundTrip(packet, [](SendMail const& a, SendMail const& b)
    {
        REQUIRE(a.Attachments.size() == b.Attachments.size());
        CHECK(a.Attachments[0].Position == b.Attachments[0].Position);
        CHECK(a.Attachments[0].ItemGuid == b.Attachments[0].ItemGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailReturnToSender round-trips", "[packets]")
{
    MailReturnToSender packet;
    packet.MailId = 42;
    packet.SenderGuid = { 7, 0 };
    RoundTrip(packet, [](MailReturnToSender const& a, MailReturnToSender const& b)
    {
        CHECK(a.MailId == b.MailId);
        CHECK(a.SenderGuid == b.SenderGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailMarkAsRead round-trips", "[packets]")
{
    MailMarkAsRead packet;
    packet.MailboxGuid = { 1, 0 };
    packet.MailId = 42;
    RoundTrip(packet, [](MailMarkAsRead const& a, MailMarkAsRead const& b)
    {
        CHECK(a.MailboxGuid == b.MailboxGuid);
        CHECK(a.MailId == b.MailId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailTakeItem round-trips", "[packets]")
{
    MailTakeItem packet;
    packet.MailboxGuid = { 1, 0 };
    packet.MailId = 42;
    packet.AttachId = 99;
    RoundTrip(packet, [](MailTakeItem const& a, MailTakeItem const& b)
    {
        CHECK(a.MailboxGuid == b.MailboxGuid);
        CHECK(a.MailId == b.MailId);
        CHECK(a.AttachId == b.AttachId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailTakeMoney round-trips", "[packets]")
{
    MailTakeMoney packet;
    packet.MailboxGuid = { 1, 0 };
    packet.MailId = 42;
    packet.Money = 100;
    RoundTrip(packet, [](MailTakeMoney const& a, MailTakeMoney const& b)
    {
        CHECK(a.MailboxGuid == b.MailboxGuid);
        CHECK(a.MailId == b.MailId);
        CHECK(a.Money == b.Money);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GetMailList round-trips", "[packets]")
{
    GetMailList packet;
    packet.MailboxGuid = { 1, 0 };
    RoundTrip(packet, [](GetMailList const& a, GetMailList const& b) { CHECK(a.MailboxGuid == b.MailboxGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailDelete round-trips", "[packets]")
{
    MailDelete packet;
    packet.MailId = 42;
    packet.DeleteReason = 1;
    RoundTrip(packet, [](MailDelete const& a, MailDelete const& b)
    {
        CHECK(a.MailId == b.MailId);
        CHECK(a.DeleteReason == b.DeleteReason);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailCreateTextItem round-trips", "[packets]")
{
    MailCreateTextItem packet;
    packet.MailboxGuid = { 1, 0 };
    packet.MailId = 42;
    RoundTrip(packet, [](MailCreateTextItem const& a, MailCreateTextItem const& b)
    {
        CHECK(a.MailboxGuid == b.MailboxGuid);
        CHECK(a.MailId == b.MailId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("QueryNextMailTime round-trips", "[packets]")
{
    RoundTrip(QueryNextMailTime(), [](QueryNextMailTime const&, QueryNextMailTime const&) {});
    RejectsEveryTruncation(QueryNextMailTime());
}

TEST_CASE("ItemTextQuery round-trips", "[packets]")
{
    ItemTextQuery packet;
    packet.Id = { 55, 0 };
    RoundTrip(packet, [](ItemTextQuery const& a, ItemTextQuery const& b) { CHECK(a.Id == b.Id); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailCommandResult round-trips", "[packets]")
{
    MailCommandResult packet;
    packet.MailId = 42;
    packet.Command = 1;
    packet.ErrorCode = 0;
    packet.BagResult = 0;
    packet.AttachId = 99;
    packet.QtyInInventory = 3;
    RoundTrip(packet, [](MailCommandResult const& a, MailCommandResult const& b)
    {
        CHECK(a.MailId == b.MailId);
        CHECK(a.Command == b.Command);
        CHECK(a.ErrorCode == b.ErrorCode);
        CHECK(a.BagResult == b.BagResult);
        CHECK(a.AttachId == b.AttachId);
        CHECK(a.QtyInInventory == b.QtyInInventory);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ReceivedMail round-trips", "[packets]")
{
    ReceivedMail packet;
    packet.Delay = 1.5f;
    RoundTrip(packet, [](ReceivedMail const& a, ReceivedMail const& b) { CHECK(a.Delay == b.Delay); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ItemTextQueryResponse round-trips valid and invalid", "[packets]")
{
    ItemTextQueryResponse packet;
    packet.Id = { 55, 0 };
    packet.Valid = true;
    packet.Text = "Some readable text";
    RoundTrip(packet, [](ItemTextQueryResponse const& a, ItemTextQueryResponse const& b)
    {
        CHECK(a.Id == b.Id);
        CHECK(a.Valid == b.Valid);
        CHECK(a.Text == b.Text);
    });
    RejectsEveryTruncation(packet);

    ItemTextQueryResponse invalid;
    invalid.Id = { 55, 0 };
    invalid.Valid = false;
    RoundTrip(invalid, [](ItemTextQueryResponse const& a, ItemTextQueryResponse const& b)
    {
        CHECK(a.Id == b.Id);
        CHECK(a.Valid == b.Valid);
        CHECK(a.Text == b.Text);
    });
    RejectsEveryTruncation(invalid);
}

TEST_CASE("MailQueryNextTimeResult round-trips with normal and non-player senders", "[packets]")
{
    MailQueryNextTimeResult packet;
    packet.NextMailTime = 0.0f;

    MailQueryNextTimeResult::Entry normal;
    normal.SenderGuid = { 7, 0 };
    normal.TimeLeft = 10.0f;
    normal.StationeryId = 41;
    packet.Mails.push_back(normal);

    MailQueryNextTimeResult::Entry auctionEntry;
    auctionEntry.AltSenderId = 123;
    auctionEntry.AltSenderType = 2;
    auctionEntry.TimeLeft = 5.0f;
    packet.Mails.push_back(auctionEntry);

    RoundTrip(packet, [](MailQueryNextTimeResult const& a, MailQueryNextTimeResult const& b)
    {
        CHECK(a.NextMailTime == b.NextMailTime);
        REQUIRE(a.Mails.size() == b.Mails.size());
        for (size_t i = 0; i < a.Mails.size(); ++i)
        {
            CHECK(a.Mails[i].SenderGuid == b.Mails[i].SenderGuid);
            CHECK(a.Mails[i].TimeLeft == b.Mails[i].TimeLeft);
            CHECK(a.Mails[i].AltSenderId == b.Mails[i].AltSenderId);
            CHECK(a.Mails[i].AltSenderType == b.Mails[i].AltSenderType);
            CHECK(a.Mails[i].StationeryId == b.Mails[i].StationeryId);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailListResult round-trips a normal-sender mail with an attachment", "[packets]")
{
    MailListResult packet;
    packet.TotalNumRecords = 1;

    MailListEntry entry;
    entry.MailId = 42;
    entry.SenderType = 0; // MAIL_NORMAL
    entry.SenderGuid = { 7, 0 };
    entry.Cod = 0;
    entry.StationeryId = 41;
    entry.SentMoney = 1000;
    entry.Flags = 0;
    entry.DaysLeft = 2.5f;
    entry.MailTemplateId = 0;
    entry.Subject = "Hello";
    entry.Body = "A letter";

    MailAttachedItem att;
    att.Position = 0;
    att.AttachId = 99;
    att.Item.ItemId = 6948;
    att.Count = 1;
    att.Unlocked = true;
    entry.Attachments.push_back(att);

    packet.Mails.push_back(entry);

    RoundTrip(packet, [](MailListResult const& a, MailListResult const& b)
    {
        CHECK(a.TotalNumRecords == b.TotalNumRecords);
        REQUIRE(a.Mails.size() == b.Mails.size());
        MailListEntry const& ea = a.Mails[0];
        MailListEntry const& eb = b.Mails[0];
        CHECK(ea.MailId == eb.MailId);
        CHECK(ea.SenderType == eb.SenderType);
        CHECK(ea.SenderGuid == eb.SenderGuid);
        CHECK(ea.Cod == eb.Cod);
        CHECK(ea.StationeryId == eb.StationeryId);
        CHECK(ea.SentMoney == eb.SentMoney);
        CHECK(ea.Flags == eb.Flags);
        CHECK(ea.DaysLeft == eb.DaysLeft);
        CHECK(ea.MailTemplateId == eb.MailTemplateId);
        CHECK(ea.Subject == eb.Subject);
        CHECK(ea.Body == eb.Body);
        REQUIRE(ea.Attachments.size() == eb.Attachments.size());
        CHECK(ea.Attachments[0].AttachId == eb.Attachments[0].AttachId);
        CHECK(ea.Attachments[0].Item.ItemId == eb.Attachments[0].Item.ItemId);
        CHECK(ea.Attachments[0].Count == eb.Attachments[0].Count);
        CHECK(ea.Attachments[0].Unlocked == eb.Attachments[0].Unlocked);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("MailListResult round-trips an auction-sender mail with no attachment", "[packets]")
{
    MailListResult packet;
    packet.TotalNumRecords = 1;

    MailListEntry entry;
    entry.MailId = 7;
    entry.SenderType = 2; // MAIL_AUCTION
    entry.AltSenderId = 555;
    entry.Cod = 0;
    entry.StationeryId = 62;
    entry.SentMoney = 0;
    entry.Flags = 0;
    entry.DaysLeft = 1.0f;
    entry.Subject = "Auction won";
    entry.Body = "";
    packet.Mails.push_back(entry);

    RoundTrip(packet, [](MailListResult const& a, MailListResult const& b)
    {
        REQUIRE(a.Mails.size() == b.Mails.size());
        CHECK(a.Mails[0].SenderType == b.Mails[0].SenderType);
        CHECK(a.Mails[0].AltSenderId == b.Mails[0].AltSenderId);
        CHECK(a.Mails[0].Subject == b.Mails[0].Subject);
        CHECK(a.Mails[0].Attachments.empty());
        CHECK(b.Mails[0].Attachments.empty());
    });
    RejectsEveryTruncation(packet);
}
