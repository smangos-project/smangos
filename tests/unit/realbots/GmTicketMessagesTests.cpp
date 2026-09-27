// Round trips for the modern support wire.
#include "Packets/GmTicketMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("AcknowledgeSurvey round-trips", "[packets]")
{
    AcknowledgeSurvey packet;
    packet.CaseId = 42;
    RoundTrip(packet, [](AcknowledgeSurvey const& a, AcknowledgeSurvey const& b) { CHECK(a.CaseId == b.CaseId); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SubmitUserFeedback round-trips", "[packets]")
{
    SubmitUserFeedback packet;
    packet.MapId = 0;
    packet.PosX = 1.5f;
    packet.PosY = -2.5f;
    packet.PosZ = 3.0f;
    packet.Facing = 1.0f;
    packet.Note = "the door won't open";
    packet.IsSuggestion = false;
    RoundTrip(packet, [](SubmitUserFeedback const& a, SubmitUserFeedback const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.PosX == b.PosX);
        CHECK(a.PosY == b.PosY);
        CHECK(a.PosZ == b.PosZ);
        CHECK(a.Facing == b.Facing);
        CHECK(a.Note == b.Note);
        CHECK(a.IsSuggestion == b.IsSuggestion);
    });
    RejectsEveryTruncation(packet);

    SubmitUserFeedback empty;
    empty.IsSuggestion = true;
    RoundTrip(empty, [](SubmitUserFeedback const& a, SubmitUserFeedback const& b)
    {
        CHECK(a.Note == b.Note);
        CHECK(a.IsSuggestion == b.IsSuggestion);
    });
    RejectsEveryTruncation(empty);
}

TEST_CASE("Complaint round-trips every complaint type", "[packets]")
{
    Complaint mail;
    mail.ComplaintType = Complaint::Mail;
    mail.OffenderGuid = { 0x1234, 0 };
    mail.OffenderRealmAddress = 5;
    mail.TimeSinceOffence = 60;
    mail.MailId = 0x1122334455667788ull;
    RoundTrip(mail, [](Complaint const& a, Complaint const& b)
    {
        CHECK(a.ComplaintType == b.ComplaintType);
        CHECK(a.OffenderGuid == b.OffenderGuid);
        CHECK(a.OffenderRealmAddress == b.OffenderRealmAddress);
        CHECK(a.TimeSinceOffence == b.TimeSinceOffence);
        CHECK(a.MailId == b.MailId);
    });
    RejectsEveryTruncation(mail);

    Complaint chat;
    chat.ComplaintType = Complaint::Chat;
    chat.OffenderGuid = { 0x4321, 0 };
    chat.ChatCommand = 3;
    chat.ChatChannelId = 7;
    chat.ChatMessageLog = "spam spam spam";
    RoundTrip(chat, [](Complaint const& a, Complaint const& b)
    {
        CHECK(a.ChatCommand == b.ChatCommand);
        CHECK(a.ChatChannelId == b.ChatChannelId);
        CHECK(a.ChatMessageLog == b.ChatMessageLog);
    });
    RejectsEveryTruncation(chat);

    Complaint calendar;
    calendar.ComplaintType = Complaint::Calendar;
    calendar.OffenderGuid = { 0x5555, 0 };
    calendar.CalendarEventGuid = 0x1122334455ull;
    calendar.CalendarInviteGuid = 0x6677889900ull;
    RoundTrip(calendar, [](Complaint const& a, Complaint const& b)
    {
        CHECK(a.CalendarEventGuid == b.CalendarEventGuid);
        CHECK(a.CalendarInviteGuid == b.CalendarInviteGuid);
    });
    RejectsEveryTruncation(calendar);
}

TEST_CASE("BugReport round-trips", "[packets]")
{
    BugReport packet;
    packet.IsSuggestion = true;
    packet.DiagInfo = "build 58797";
    packet.Text = "the sky is the wrong color";
    RoundTrip(packet, [](BugReport const& a, BugReport const& b)
    {
        CHECK(a.IsSuggestion == b.IsSuggestion);
        CHECK(a.DiagInfo == b.DiagInfo);
        CHECK(a.Text == b.Text);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SystemStatus round-trips", "[packets]")
{
    SystemStatus packet;
    packet.Status = SystemStatus::Enabled;
    RoundTrip(packet, [](SystemStatus const& a, SystemStatus const& b) { CHECK(a.Status == b.Status); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CaseStatus round-trips", "[packets]")
{
    RoundTrip(CaseStatus(), [](CaseStatus const&, CaseStatus const&) {});
    RejectsEveryTruncation(CaseStatus());
}

TEST_CASE("ComplaintResult round-trips", "[packets]")
{
    ComplaintResult packet;
    packet.ComplaintType = 1;
    packet.Result = 1;
    RoundTrip(packet, [](ComplaintResult const& a, ComplaintResult const& b)
    {
        CHECK(a.ComplaintType == b.ComplaintType);
        CHECK(a.Result == b.Result);
    });
    RejectsEveryTruncation(packet);
}
