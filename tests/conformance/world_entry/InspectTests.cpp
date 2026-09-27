// The inspect window. A GM bot inspects a plain bot, `.namego`-ing to it first to guarantee it's
// in INSPECT_DISTANCE regardless of where the plain bot's pooled character was left by an
// earlier scenario.
#include "Packets/InspectMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("inspect: a bot inspects another and gets its equipment/honor tab", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot inspector(TestBotOptions().Gm());
        TestBot target;
        REQUIRE(co_await inspector.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await target.Stage(BotStage::InWorld, 30000));

        ClassicGuid const targetGuid = target.GetCharacterGuid();
        SayCommand(inspector, ".namego " + target.GetCharacterName());
        REQUIRE(co_await TestWaitUntil([&]() { return inspector.GetKnownObject(targetGuid).has_value(); }, 10000));

        Inspect inspect;
        inspect.TargetGuid = targetGuid;
        inspector.Send(inspect);

        auto const result = co_await inspector.Packet<InspectResult>(10000);
        REQUIRE(result);
        CHECK(result->DisplayInfo.PlayerGuid == targetGuid);
        CHECK(result->DisplayInfo.Name == target.GetCharacterName());
        CHECK_FALSE(result->HasGuildData); // guild data is deferred to the guild slice (F8g)

        InspectPvp inspectPvp;
        inspectPvp.TargetGuid = targetGuid;
        inspector.Send(inspectPvp);

        auto const pvpResult = co_await inspector.Packet<InspectPvpResult>(10000);
        REQUIRE(pvpResult);
        CHECK(pvpResult->PlayerGuid == targetGuid);
        CHECK(pvpResult->Brackets.empty());
        CHECK(pvpResult->ArenaTeams.size() == 3); // vanilla has no arenas: three always-empty team slots

        QueryInspectAchievements query;
        query.TargetGuid = targetGuid;
        inspector.Send(query); // consumed no-op; just checking it doesn't kick the bot
        co_await TestDelay(500);
    });
}
