// The player's own state the server sends as the character enters the world (bot-phase-4.md,
// slice A): each packet arrives, on the connection its opcode routes to, and parses with nothing
// left over.
#include "Packets/CharScreen.h"
#include "Packets/PlayerSetup.h"
#include "Packets/SpellBook.h"
#include "Packets/WorldStates.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <algorithm>
#include <cmath>

namespace
{
// The last T the bot received, parsed; fails the scenario if it didn't come on `connection` or
// its body has bytes the struct doesn't account for.
template<typename T>
std::optional<T> ParsedExactly(TestBot const& bot, BotConnectionKind connection)
{
    std::optional<BotPacket> raw = bot.LastReceived(T::Opcode);
    if (!raw)
        return std::nullopt;
    CHECK(raw->Connection == connection);

    raw->Data.rpos(0);
    T value;
    if (!value.Read(raw->Data))
        return std::nullopt;
    CHECK(raw->Data.rpos() == raw->Data.size());
    return value;
}
}

TEST_CASE("world_entry: the login burst carries the character's bind point", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(BindPointUpdate::Opcode, 10000));

        std::optional<BindPointUpdate> bind = ParsedExactly<BindPointUpdate>(bot, BotConnectionKind::Instance);
        REQUIRE(bind);
        CHECK(bind->MapId < 10000);
        CHECK(std::isfinite(bind->X));
        CHECK(std::isfinite(bind->Y));
        CHECK(std::isfinite(bind->Z));
        CHECK(bind->AreaId != 0);
    });
}

TEST_CASE("world_entry: the login burst sets the game clock to real time", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(LoginSetTimeSpeed::Opcode, 10000));

        std::optional<LoginSetTimeSpeed> speed = ParsedExactly<LoginSetTimeSpeed>(bot, BotConnectionKind::Instance);
        REQUIRE(speed);
        CHECK(speed->NewSpeed == Catch::Approx(1.0f / 60.0f));

        // A packed date: years since 2000 in the top bits, a month and a minute in range.
        uint32 const year = (speed->GameTime >> 24) & 0x1F;
        uint32 const month = (speed->GameTime >> 20) & 0xF;
        uint32 const minute = speed->GameTime & 0x3F;
        CHECK(year >= 20);
        CHECK(month < 12);
        CHECK(minute < 60);
    });
}

TEST_CASE("world_entry: the login burst carries the account's tutorial flags", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(TutorialFlags::Opcode, 10000));

        CHECK(ParsedExactly<TutorialFlags>(bot, BotConnectionKind::Realm));
    });
}

TEST_CASE("world_entry: the login burst carries weapon and armor proficiencies", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(SetProficiency::Opcode, 10000));
        co_await TestDelay(500);

        bool weapon = false;
        bool armor = false;
        for (BotPacket packet : bot.Received())
        {
            if (packet.Opcode != SetProficiency::Opcode)
                continue;
            REQUIRE(packet.Connection == BotConnectionKind::Instance);

            packet.Data.rpos(0);
            SetProficiency proficiency;
            REQUIRE(proficiency.Read(packet.Data));
            CHECK(packet.Data.rpos() == packet.Data.size());
            weapon |= proficiency.ItemClass == 2;
            armor |= proficiency.ItemClass == 4;
        }
        CHECK(weapon);
        CHECK(armor);
    });
}

TEST_CASE("world_entry: the login burst carries the spell book", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(SendKnownSpells::Opcode, 10000));
        REQUIRE(co_await bot.WaitReceived(SendUnlearnSpells::Opcode, 10000));
        REQUIRE(co_await bot.WaitReceived(SendSpellHistory::Opcode, 10000));

        std::optional<SendKnownSpells> known = ParsedExactly<SendKnownSpells>(bot, BotConnectionKind::Instance);
        REQUIRE(known);
        CHECK(known->InitialLogin);
        // Every class starts with the auto attack.
        CHECK(std::ranges::find(known->KnownSpells, 6603u) != known->KnownSpells.end());

        CHECK(ParsedExactly<SendUnlearnSpells>(bot, BotConnectionKind::Instance));
        CHECK(ParsedExactly<SendSpellHistory>(bot, BotConnectionKind::Instance));
    });
}

TEST_CASE("world_entry: the login burst sends the action buttons once", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(UpdateActionButtons::Opcode, 10000));
        co_await TestDelay(500);

        CHECK(bot.CountReceived(UpdateActionButtons::Opcode) == 1);
        std::optional<UpdateActionButtons> buttons = ParsedExactly<UpdateActionButtons>(bot, BotConnectionKind::Instance);
        REQUIRE(buttons);
        CHECK(buttons->Reason == UpdateActionButtons::REASON_INITIAL);
        CHECK(std::ranges::any_of(buttons->Buttons, [](uint64 button) { return button != 0; }));
    });
}

TEST_CASE("world_entry: the login burst carries the character's reputations", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(InitializeFactions::Opcode, 10000));

        std::optional<InitializeFactions> factions = ParsedExactly<InitializeFactions>(bot, BotConnectionKind::Instance);
        REQUIRE(factions);
        CHECK_FALSE(factions->Factions.empty());
        CHECK(factions->Bonuses.size() == factions->Factions.size());
        CHECK(std::ranges::all_of(factions->Factions, [](InitializeFactions::Faction const& faction) { return faction.FactionId > 0; }));
    });
}

TEST_CASE("world_entry: the login burst carries the account data times for the character", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(AccountDataTimes::Opcode, 10000));

        std::optional<AccountDataTimes> times = ParsedExactly<AccountDataTimes>(bot, BotConnectionKind::Realm);
        REQUIRE(times);
        ClassicGuid const guid = bot.GetCharacterGuid();
        CHECK_FALSE(guid.IsEmpty());
        CHECK(times->PlayerGuid.Low == guid.Low);
        CHECK(times->PlayerGuid.High == guid.High);
        CHECK(times->ServerTime > 1700000000);
    });
}

TEST_CASE("world_entry: the login burst carries the world states for the character's zone", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(InitWorldStates::Opcode, 10000));

        std::optional<InitWorldStates> states = ParsedExactly<InitWorldStates>(bot, BotConnectionKind::Instance);
        REQUIRE(states);
        CHECK(states->MapId < 10000);
        CHECK(states->AreaId != 0);
        CHECK(states->SubareaId != 0);
    });
}

TEST_CASE("world_entry: the login burst carries the weather of the character's zone", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(WeatherUpdate::Opcode, 10000));

        std::optional<WeatherUpdate> weather = ParsedExactly<WeatherUpdate>(bot, BotConnectionKind::Instance);
        REQUIRE(weather);
        CHECK(weather->Intensity >= 0.0f);
        CHECK(weather->Intensity <= 1.0f);
    });
}
