// The taxi wire: flight master interaction and starting a flight.
//
// A live scenario exercising the flight-master-facing opcodes (TaxiQueryAvailableNodes,
// ShowTaxiNodes, a successful ActivateTaxi) needs a real flight master's guid and a known-good
// destination node, neither of which this suite can determine without a database query (see
// cur-step.md item 2d) -- ".learn all_mytaxis" was tried here, but it only fires NewTaxiPath the
// first time a node is learned, and the pooled GM account's taxi mask is persistent across runs,
// so the scenario only passed once. The rejection path below needs no such state and is asserted
// live; the rest of the wire is covered by TaxiMessagesTests.cpp's round trips.
#include "Packets/TaxiMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("taxi: activating a flight at a vendor the bot can't reach is rejected", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // The bot's own guid is never a flight master, so this is rejected deterministically
        // regardless of what is actually near it.
        ActivateTaxi request;
        request.Vendor = bot.GetCharacterGuid();
        request.Node = 1;
        bot.Send(request);

        auto reply = co_await bot.Packet<ActivateTaxiReply>(10000);
        REQUIRE(reply);
        CHECK(reply->Reply == 4); // ERR_TAXITOOFARAWAY
    });
}
