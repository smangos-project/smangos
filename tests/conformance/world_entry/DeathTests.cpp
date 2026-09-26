// Dying, being offered a resurrection, releasing the spirit and coming back, driven by a GM bot.
#include "Packets/CombatMessages.h"
#include "Packets/DeathMessages.h"
#include "TestBot.h"
#include "TestCommands.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

namespace
{
size_t CountReleaseLocs(TestBot const& bot, std::function<bool(DeathReleaseLoc const&)> const& predicate)
{
    size_t count = 0;
    for (BotPacket packet : bot.Received())
    {
        if (packet.Opcode != DeathReleaseLoc::Opcode)
            continue;
        packet.Data.rpos(0);
        DeathReleaseLoc parsed;
        if (parsed.Read(packet.Data) && predicate(parsed))
            ++count;
    }
    return count;
}

bool ClearsMarker(DeathReleaseLoc const& p) { return p.MapId == -1; }
bool SetsMarker(DeathReleaseLoc const& p) { return p.MapId >= 0; }
}

TEST_CASE("death: a killed bot is offered a resurrection spell and accepts it", "[gameplay]")
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
        // Pooled characters are saved as they were left, so a victim may start dead or far away.
        SayCommand(gm, ".revive " + victim.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".namego " + victim.GetCharacterName());
        co_await TestDelay(2500);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(victimGuid).has_value(); }, 10000));

        size_t const cleared = CountReleaseLocs(victim, ClearsMarker);

        SetSelection select;
        select.Target = victimGuid;
        gm.Send(select);
        co_await TestDelay(500);

        SayCommand(gm, ".namedie " + victim.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".cast 2006 triggered"); // Resurrection, rank 1

        auto offer = co_await victim.Packet<ResurrectRequest>(20000, [&](ResurrectRequest const& p) { return p.Offerer == gmGuid; });
        REQUIRE(offer);
        CHECK(offer->SpellId == 2006);

        ResurrectResponse accept;
        accept.Resurrecter = offer->Offerer;
        accept.Response = 0;
        victim.Send(accept);

        REQUIRE(co_await TestWaitUntil([&]() { return CountReleaseLocs(victim, ClearsMarker) > cleared; }, 10000));
    });
}

TEST_CASE("death: a killed bot releases its spirit and a GM revives it", "[gameplay]")
{
    if (ConformanceRun::GmTarget().empty())
        SKIP("the target has no game master accounts (" << ConformanceRun::Target() << "-gm)");

    RunTest([]() -> TestRoutine
    {
        TestBot gm(TestBotOptions().Gm());
        TestBot victim;
        REQUIRE(co_await gm.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await victim.Stage(BotStage::InWorld, 30000));

        std::string const victimName = victim.GetCharacterName();
        // Pooled characters are saved as they were left, so a victim may start dead or far away.
        SayCommand(gm, ".revive " + victim.GetCharacterName());
        co_await TestDelay(1000);
        SayCommand(gm, ".namego " + victim.GetCharacterName());
        co_await TestDelay(2500);
        REQUIRE(co_await TestWaitUntil([&]() { return gm.GetKnownObject(victim.GetCharacterGuid()).has_value(); }, 10000));

        size_t const delays = victim.CountReceived(CorpseReclaimDelay::Opcode);
        size_t const cleared = CountReleaseLocs(victim, ClearsMarker);

        SayCommand(gm, ".namedie " + victimName);
        co_await TestDelay(3000);

        RepopRequest release;
        victim.Send(release);

        REQUIRE(co_await TestWaitUntil([&]() { return victim.CountReceived(CorpseReclaimDelay::Opcode) > delays; }, 10000));
        REQUIRE(co_await TestWaitUntil([&]() { return CountReleaseLocs(victim, SetsMarker) > 0; }, 10000));

        SayCommand(gm, ".revive " + victimName);
        REQUIRE(co_await TestWaitUntil([&]() { return CountReleaseLocs(victim, ClearsMarker) > cleared; }, 10000));

        // The character is saved where it stands; bring it back so later scenarios find it beside the others.
        SayCommand(gm, ".namego " + victimName);
        co_await TestDelay(2000);
    });
}
