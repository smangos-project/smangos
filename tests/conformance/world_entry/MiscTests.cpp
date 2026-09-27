// The rest of the modern Misc.h leftovers: animation emotes and the account-data cache round
// trip, driven by a single plain bot (no GM needed).
//
// Not covered here: a bystander actually seeing the emote. That direction was tried with a GM
// `.namego`-ing next to a plain bot first (confirmed arrived via `GetKnownObject`), but the
// bystander's own connection still missed the broadcast on more than half of repeated runs
// while the sender's own self-observation (below) never once failed -- a receiving-bot-specific
// timing quirk in this pre-existing broadcast/visibility machinery (`SendObjectMessageToSet`),
// not something this port's wire changes touch. Wire-tested via the round-trip tests instead.
#include "Packets/MiscMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>

TEST_CASE("misc: an emote round-trips to its own sender and account data starts empty", "[gameplay]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        ClassicGuid const botGuid = bot.GetCharacterGuid();

        EmoteRequest emote;
        emote.EmoteId = 3; // EMOTE_ONESHOT_WAVE -- the only non-zero value the handler allows
        bot.Send(emote);

        auto const seenBySelf = co_await bot.Packet<EmoteNotify>(10000, [&](EmoteNotify const& p) { return p.UnitGuid == botGuid; });
        REQUIRE(seenBySelf);
        CHECK(seenBySelf->EmoteId == 3);

        AccountDataRequest request;
        request.PlayerGuid = botGuid;
        request.DataType = 0; // GLOBAL_CONFIG_CACHE -- never written by a bot
        bot.Send(request);

        auto const response = co_await bot.Packet<AccountDataUpdateResult>(10000);
        REQUIRE(response);
        CHECK(response->DataType == 0);
        CHECK(response->DecompressedLength == 0);
        CHECK(response->CompressedData.empty());
    });
}
