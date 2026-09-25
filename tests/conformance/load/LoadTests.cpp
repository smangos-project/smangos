// "load/" scenarios: many bots at once, each a real login on an account of its own (bot-phase-2.md,
// "Scenarios" / "Accounts"). Tagged [load]: they need a pool of as many accounts as bots, which a
// target lists (Tags "... load") only when it has provisioned them
// (contrib/realbotrunner/provision-accounts.ps1); with a smaller pool, or a LoadBots cap below what a
// scenario needs, a scenario SKIPs and says how many it wanted. Only the overall bound is asserted;
// the per-stage login times are printed for whoever is tuning a server.
#include "LoadHelpers.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <random>

namespace
{
struct LoadBot
{
    std::unique_ptr<TestBot> Bot;
    std::shared_ptr<StageTimings> Timings;
};

// A valid, unique character name (char_screen/CharScreenTests.cpp's RandomCharacterName): the pool's
// accounts are freshly provisioned and have no character yet, so every load bot creates one.
std::string RandomCharacterName()
{
    static constexpr char Letters[] = "abcdefghijklmnopqrstuvwxyz";
    std::random_device device;
    std::string name = "Zl";
    for (int i = 0; i < 8; ++i)
        name += Letters[device() % 26];
    return name;
}

LoadBot MakeLoadBot()
{
    LoadBot loadBot;
    loadBot.Timings = std::make_shared<StageTimings>();
    std::shared_ptr<StageTimings> timings = loadBot.Timings;
    loadBot.Bot = std::make_unique<TestBot>(TestBotOptions()
        .CharacterName(RandomCharacterName())
        .Race(1)  // human
        .Class(1) // warrior
        .Hook([timings]() { return std::make_unique<StageTimingHook>(timings); }));
    return loadBot;
}

std::vector<std::shared_ptr<StageTimings>> TimingsOf(std::vector<LoadBot> const& bots)
{
    std::vector<std::shared_ptr<StageTimings>> out;
    for (LoadBot const& bot : bots)
        out.push_back(bot.Timings);
    return out;
}
}

TEST_CASE("load: ten bots log in at once", "[protocol][load]")
{
    RequireLoadCapacity(10);
    RunTest([]() -> TestRoutine
    {
        auto const start = std::chrono::steady_clock::now();
        std::vector<LoadBot> bots;
        for (uint32_t i = 0; i < 10; ++i)
            bots.push_back(MakeLoadBot());

        std::vector<bool> inWorld = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Stage(BotStage::InWorld, 60000); });
        CHECK(std::all_of(inWorld.begin(), inWorld.end(), [](bool ok) { return ok; }));
        WARN("time to InWorld: " << Describe(Summarize(TimesToInWorld(TimingsOf(bots), start))) << "\n" << DescribeStageTimes(TimingsOf(bots)));

        for (LoadBot& bot : bots)
            bot.Bot->Disconnect(true);
        std::vector<bool> out = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Disconnected(60000); });
        CHECK(std::all_of(out.begin(), out.end(), [](bool ok) { return ok; }));
    });
}

TEST_CASE("load: fifty bots at once are all in the world within the bound and all log out cleanly", "[protocol][load]")
{
    RequireLoadCapacity(50);
    RunTest([]() -> TestRoutine
    {
        auto const start = std::chrono::steady_clock::now();
        std::vector<LoadBot> bots;
        for (uint32_t i = 0; i < 50; ++i)
            bots.push_back(MakeLoadBot());

        std::vector<bool> inWorld = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Stage(BotStage::InWorld, 120000); });
        CHECK(std::all_of(inWorld.begin(), inWorld.end(), [](bool ok) { return ok; }));
        WARN("time to InWorld: " << Describe(Summarize(TimesToInWorld(TimingsOf(bots), start))) << "\n" << DescribeStageTimes(TimingsOf(bots)));

        for (LoadBot& bot : bots)
            bot.Bot->Disconnect(true);
        std::vector<bool> out = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Disconnected(120000); });
        CHECK(std::all_of(out.begin(), out.end(), [](bool ok) { return ok; }));
        CHECK(std::all_of(bots.begin(), bots.end(), [](LoadBot const& bot) { return bot.Bot->GetDisconnectReason() == BotDisconnectReason::Requested; }));
    });
}

TEST_CASE("load: a hundred bots staggered over twenty seconds are all in the world and all log out cleanly", "[protocol][load]")
{
    RequireLoadCapacity(100);
    RunTest([]() -> TestRoutine
    {
        auto const start = std::chrono::steady_clock::now();
        std::vector<LoadBot> bots;
        for (uint32_t i = 0; i < 100; ++i)
        {
            bots.push_back(MakeLoadBot());
            co_await TestDelay(200); // 100 bots over 20 s
        }

        std::vector<bool> inWorld = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Stage(BotStage::InWorld, 180000); });
        CHECK(std::all_of(inWorld.begin(), inWorld.end(), [](bool ok) { return ok; }));
        WARN("time to InWorld: " << Describe(Summarize(TimesToInWorld(TimingsOf(bots), start))) << "\n" << DescribeStageTimes(TimingsOf(bots)));

        for (LoadBot& bot : bots)
            bot.Bot->Disconnect(true);
        std::vector<bool> out = co_await WhenAll(bots, [](LoadBot& bot) { return bot.Bot->Disconnected(180000); });
        CHECK(std::all_of(out.begin(), out.end(), [](bool ok) { return ok; }));
    });
}
