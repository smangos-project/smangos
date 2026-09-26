// What a bot learns about the objects around it from SMSG_UPDATE_OBJECT.
#include "Packets/ChatMessages.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <cmath>

constexpr float kNearbyYards = 300.0f;

static float DistanceBetween(BotKnownObject const& a, BotKnownObject const& b)
{
    float const dx = a.X - b.X;
    float const dy = a.Y - b.Y;
    float const dz = a.Z - b.Z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

static TestWaitUntil WaitForObject(TestBot const& bot, ClassicGuid guid, uint32_t timeoutMs)
{
    return TestWaitUntil([&bot, guid]() { return bot.GetKnownObject(guid).has_value(); }, timeoutMs);
}

TEST_CASE("world_entry: two bots in one place see each other as players", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot botA;
        TestBot botB;
        REQUIRE(co_await botA.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await botB.Stage(BotStage::InWorld, 30000));

        REQUIRE(co_await WaitForObject(botA, botB.GetCharacterGuid(), 10000));
        REQUIRE(co_await WaitForObject(botB, botA.GetCharacterGuid(), 10000));

        std::optional<BotKnownObject> const bSeenByA = botA.GetKnownObject(botB.GetCharacterGuid());
        std::optional<BotKnownObject> const aSeenByB = botB.GetKnownObject(botA.GetCharacterGuid());
        REQUIRE(bSeenByA);
        REQUIRE(aSeenByB);
        CHECK(bSeenByA->ObjectType == BOT_TYPEID_PLAYER);
        CHECK(aSeenByB->ObjectType == BOT_TYPEID_PLAYER);

        // Each bot's own create block carries its own position, so it is what the other one saw.
        std::optional<BotKnownObject> const aSelf = botA.GetKnownObject(botA.GetCharacterGuid());
        std::optional<BotKnownObject> const bSelf = botB.GetKnownObject(botB.GetCharacterGuid());
        REQUIRE(aSelf);
        REQUIRE(bSelf);
        CHECK(aSelf->ObjectType == BOT_TYPEID_ACTIVE_PLAYER);
        CHECK(DistanceBetween(*bSeenByA, *bSelf) < 5.0f);
        CHECK(DistanceBetween(*aSeenByB, *aSelf) < 5.0f);
    });
}

TEST_CASE("world_entry: a bot sees creatures near where it enters the world", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        ClassicGuid const self = bot.GetCharacterGuid();
        REQUIRE(co_await WaitForObject(bot, self, 10000));
        std::optional<BotKnownObject> const me = bot.GetKnownObject(self);
        REQUIRE(me);

        // Creatures arrive in the visibility sweep that follows the login, not with the player's
        // own create block.
        bool found = false;
        for (int attempt = 0; attempt < 50 && !found; ++attempt)
        {
            for (BotKnownObject const& object : bot.GetKnownObjects())
            {
                if (object.ObjectType == BOT_TYPEID_UNIT && DistanceBetween(object, *me) < kNearbyYards)
                    found = true;
            }
            if (!found)
                co_await TestDelay(100);
        }
        CHECK(found);
    });
}

TEST_CASE("world_entry: a bot sees another bot's values change", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot botA;
        TestBot botB;
        REQUIRE(co_await botA.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await botB.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await WaitForObject(botB, botA.GetCharacterGuid(), 10000));

        // Setting AFK changes the player's flags, which everyone who sees them is told about.
        ClassicGuid const guid = botA.GetCharacterGuid();
        uint32_t const before = botB.GetKnownObject(guid)->ValuesUpdates;

        ChatMessageAfk afk;
        afk.Message = "away";
        botA.Send(afk);

        REQUIRE(co_await TestWaitUntil([&botB, guid, before]()
        {
            std::optional<BotKnownObject> const seen = botB.GetKnownObject(guid);
            return seen && seen->ValuesUpdates > before;
        }, 10000));
    });
}

TEST_CASE("world_entry: a bot stops knowing an object that leaves the world", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot botA;
        TestBot botB;
        REQUIRE(co_await botA.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await botB.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await WaitForObject(botA, botB.GetCharacterGuid(), 10000));

        ClassicGuid const guid = botB.GetCharacterGuid();
        botB.Disconnect(true);

        REQUIRE(co_await TestWaitUntil([&botA, guid]() { return !botA.GetKnownObject(guid).has_value(); }, 20000));
    });
}
