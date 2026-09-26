// Challenging another bot to a duel, answering, and calling it off, driven by a GM bot.
#include "Packets/CombatMessages.h"
#include "Packets/DuelMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("duel: a challenge is offered, accepted and called off", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot victim;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await victim.Stage(BotStage::InWorld, 30000));

        ClassicGuid const gmGuid = gm.GetCharacterGuid();
        ClassicGuid const victimGuid = victim.GetCharacterGuid();
        // Pooled characters are saved as they were left, so the victim may start dead or far away.
        SayCommand(gm, ".revive " + victim.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".namego " + victim.GetCharacterName());
        co_await TestDelay(2500);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(victimGuid).has_value(); }, 10000));

        CanDuel ask;
        ask.Target = victimGuid;
        gm.Send(ask);
        auto answer = co_await gm.WaitReceived<CanDuelResult>(10000, [&](CanDuelResult const& p) { return p.Target == victimGuid; });
        REQUIRE(answer);
        CHECK(gm.FindReceived<CanDuelResult>([&](CanDuelResult const& p) { return p.Target == victimGuid; })->Result);

        SetSelection select;
        select.Target = victimGuid;
        gm.Send(select);
        co_await TestDelay(500);
        SayCommand(gm, ".cast 7266 triggered"); // Duel

        REQUIRE(co_await victim.WaitReceived<DuelRequested>(10000, [&](DuelRequested const& p) { return p.RequestedBy == gmGuid; }));
        REQUIRE(co_await gm.WaitReceived<DuelRequested>(10000, [&](DuelRequested const& p) { return p.RequestedBy == gmGuid; }));
        ClassicGuid const arbiter = victim.FindReceived<DuelRequested>([&](DuelRequested const& p) { return p.RequestedBy == gmGuid; })->Arbiter;
        CHECK(!arbiter.IsEmpty());

        DuelResponse accept;
        accept.Arbiter = arbiter;
        accept.Accepted = true;
        victim.Send(accept);
        REQUIRE(co_await victim.WaitReceived<DuelCountdown>(10000));
        REQUIRE(co_await gm.WaitReceived<DuelCountdown>(10000));
        CHECK(gm.FindReceived<DuelCountdown>([](DuelCountdown const&) { return true; })->Countdown == 3000);

        // Forfeiting before the countdown ends calls the duel off for both.
        DuelResponse forfeit;
        forfeit.Arbiter = arbiter;
        forfeit.Forfeited = true;
        victim.Send(forfeit);
        REQUIRE(co_await victim.WaitReceived<DuelCompleteNotice>(10000));
        REQUIRE(co_await gm.WaitReceived<DuelCompleteNotice>(10000));
        CHECK(!gm.FindReceived<DuelCompleteNotice>([](DuelCompleteNotice const&) { return true; })->Started);
    });
}
