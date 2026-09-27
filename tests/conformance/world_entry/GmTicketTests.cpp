// The Help UI's status polls and the right-click player-report form, driven by two plain bots.
#include "Packets/GmTicketMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("gm ticket: system status, case status and a player report round-trip", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot reporter;
        TestBot offender;
        REQUIRE(co_await reporter.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await offender.Stage(BotStage::InWorld, 30000));

        reporter.Send(GetSystemStatus());
        REQUIRE(co_await reporter.WaitReceived<SystemStatus>(10000));
        CHECK(reporter.FindReceived<SystemStatus>()->Status == SystemStatus::Enabled); // GMTickets.Enable defaults to true

        reporter.Send(GetCaseStatus());
        REQUIRE(co_await reporter.WaitReceived<CaseStatus>(10000));

        Complaint report;
        report.ComplaintType = Complaint::Chat;
        report.OffenderGuid = offender.GetCharacterGuid();
        report.ChatCommand = 1;
        report.ChatChannelId = 0;
        report.ChatMessageLog = "spam spam spam";
        reporter.Send(report);

        REQUIRE(co_await reporter.WaitReceived<ComplaintResult>(10000));
        auto const result = reporter.FindReceived<ComplaintResult>();
        REQUIRE(result);
        CHECK(result->ComplaintType == Complaint::Chat);
        CHECK(result->Result == 1);
    });
}
