// The player's own state the server sends as the character enters the world (bot-phase-4.md,
// slice A): each packet arrives, on the connection its opcode routes to, and parses with nothing
// left over.
#include "Packets/CharScreen.h"
#include "Packets/PlayerSetup.h"
#include "Packets/SpellBook.h"
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
