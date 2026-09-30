// "char_screen/" scenarios (port-plan.md step 4: enum, create, delete, login), against
// ConformanceRun::Target(). A bot held at StopAt(CharSelect) is authenticated and encrypted on the
// Realm connection and has asked the character screen nothing, so each scenario drives the screen
// by hand. Scenarios that create characters use a random name and delete what they made (also when
// an assertion fails), so reruns and parallel targets never collide and the account's own character
// is never touched.
#include "Packets/Classic/Guid.h"
#include "Packets/CharScreen.h"
#include "Packets/Classic/Character.h"
#include "Packets/Session.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <random>

namespace
{
// A valid, unique character name: capitalised letters only.
std::string RandomCharacterName()
{
    static constexpr char Letters[] = "abcdefghijklmnopqrstuvwxyz";
    std::random_device device;
    std::string name = "Zq";
    for (int i = 0; i < 8; ++i)
        name += Letters[device() % 26];
    return name;
}

CharCreate MakeCreate(std::string name)
{
    CharCreate create;
    create.Name = std::move(name);
    create.Race = 1;   // human
    create.Class = 1;  // warrior
    create.Gender = 0;
    return create;
}

// A create that could not happen because the account (or realm) is full is not a failure of what
// these scenarios test, and the characters in the way are not theirs to delete: skip, saying so.
bool IsFullResult(uint8 result)
{
    return result == CLASSIC_CHAR_CREATE_SERVER_LIMIT || result == CLASSIC_CHAR_CREATE_ACCOUNT_LIMIT;
}

bool Contains(ClassicEnumCharactersResult const& result, ClassicGuid const& guid)
{
    return std::any_of(result.Characters.begin(), result.Characters.end(),
        [&](ClassicCharacterInfo const& c) { return c.Guid.Low == guid.Low && c.Guid.High == guid.High; });
}

// Best-effort cleanup for a scenario that fails before it can confirm its own delete (an assertion
// that throws included): sent without waiting, so it can race the connection tearing down and never
// run (CMSG_CHAR_DELETE is PACKET_PROCESS_WORLD -- Step 9 -- not handled before the socket closes).
// A scenario on its success path should send its own CharDelete and await CharDeleteResponse instead.
struct DeleteOnExit
{
    TestBot& Bot;
    ClassicGuid Guid;
    bool Armed = false;

    ~DeleteOnExit()
    {
        if (Armed)
        {
            CharDelete del;
            del.Guid = Guid;
            Bot.Send(del);
        }
    }
};
}

TEST_CASE("char_screen: enum lists an existing character", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        bot.Send(ClassicEnumCharacters{});
        auto result = co_await bot.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(result);

        // Every account in a target's pool carries a character (login/world_entry scenarios rely
        // on it), and the list says who they are.
        CHECK(result->CharacterCount >= 1);
        // Without race unlock entries the creation screen greys every race out ("faction imbalance").
        CHECK(result->RaceUnlockCount >= 8);
        REQUIRE_FALSE(result->Characters.empty());
        for (ClassicCharacterInfo const& c : result->Characters)
        {
            CHECK_FALSE(c.Guid.IsEmpty());
            CHECK_FALSE(c.Name.empty());
            CHECK(c.Level >= 1);
        }
    });
}

TEST_CASE("char_screen: a created character shows up in the enum, and a deleted one does not", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        std::string const name = RandomCharacterName();
        bot.Send(MakeCreate(name));
        auto created = co_await bot.Packet<CharCreateResponse>(10000);
        REQUIRE(created);
        if (IsFullResult(created->Result))
            SKIP("the account has no free character slot (result " << int(created->Result) << ")");
        REQUIRE(created->Result == CLASSIC_CHAR_CREATE_SUCCESS);
        REQUIRE_FALSE(created->Guid.IsEmpty());

        DeleteOnExit cleanup{ bot, created->Guid, true };

        bot.Send(ClassicEnumCharacters{});
        auto afterCreate = co_await bot.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(afterCreate);
        REQUIRE_FALSE(afterCreate->Characters.empty());
        CHECK(Contains(*afterCreate, created->Guid));
        CHECK(std::any_of(afterCreate->Characters.begin(), afterCreate->Characters.end(),
            [&](ClassicCharacterInfo const& c) { return c.Name == name; }));

        CharDelete del;
        del.Guid = created->Guid;
        bot.Send(del);
        auto deleted = co_await bot.Packet<CharDeleteResponse>(10000);
        REQUIRE(deleted);
        CHECK(deleted->Result == CLASSIC_CHAR_DELETE_SUCCESS);
        cleanup.Armed = false;

        bot.Send(ClassicEnumCharacters{});
        auto afterDelete = co_await bot.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(afterDelete);
        CHECK_FALSE(Contains(*afterDelete, created->Guid));
    });
}

TEST_CASE("char_screen: a created warrior is listed wearing its starting gear", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        bot.Send(MakeCreate(RandomCharacterName()));
        auto created = co_await bot.Packet<CharCreateResponse>(10000);
        REQUIRE(created);
        if (IsFullResult(created->Result))
            SKIP("the account has no free character slot (result " << int(created->Result) << ")");
        REQUIRE(created->Result == CLASSIC_CHAR_CREATE_SUCCESS);
        DeleteOnExit cleanup{ bot, created->Guid, true };

        bot.Send(ClassicEnumCharacters{});
        auto list = co_await bot.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(list);
        auto const mine = std::find_if(list->Characters.begin(), list->Characters.end(),
            [&](ClassicCharacterInfo const& c) { return c.Guid.Low == created->Guid.Low && c.Guid.High == created->Guid.High; });
        REQUIRE(mine != list->Characters.end());

        // The character screen draws the model from these; all zeros is a naked character.
        size_t const drawn = std::count_if(mine->Equipment.begin(), mine->Equipment.end(),
            [](ClassicEquipmentVisual const& v) { return v.DisplayId != 0 && v.InventoryType != 0; });
        CHECK(drawn >= 3);

        CharDelete del;
        del.Guid = created->Guid;
        bot.Send(del);
        auto deleted = co_await bot.Packet<CharDeleteResponse>(10000);
        REQUIRE(deleted);
        CHECK(deleted->Result == CLASSIC_CHAR_DELETE_SUCCESS);
        cleanup.Armed = false;
    });
}

// Each character is about 640 bytes on the wire, so a few make the list past the size where the
// server wraps it in SMSG_COMPRESSED_PACKET. The bot checks that wrapper's checksums the way the
// client does, so a bad one fails the enum here.
TEST_CASE("char_screen: a character list large enough to be compressed still arrives intact", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        std::vector<ClassicGuid> made;
        for (int i = 0; i < 2; ++i)
        {
            bot.Send(MakeCreate(RandomCharacterName()));
            auto created = co_await bot.Packet<CharCreateResponse>(10000);
            REQUIRE(created);
            if (IsFullResult(created->Result))
            {
                for (ClassicGuid const& guid : made)
                {
                    CharDelete del;
                    del.Guid = guid;
                    bot.Send(del);
                    co_await bot.Packet<CharDeleteResponse>(10000);
                }
                SKIP("the account has no free character slot (result " << int(created->Result) << ")");
            }
            REQUIRE(created->Result == CLASSIC_CHAR_CREATE_SUCCESS);
            made.push_back(created->Guid);
        }

        bot.Send(ClassicEnumCharacters{});
        auto list = co_await bot.Packet<ClassicEnumCharactersResult>(10000);

        for (ClassicGuid const& guid : made)
        {
            CharDelete del;
            del.Guid = guid;
            bot.Send(del);
            auto deleted = co_await bot.Packet<CharDeleteResponse>(10000);
            CHECK(deleted);
        }

        REQUIRE(list);
        REQUIRE(list->Characters.size() >= 3);
        for (ClassicGuid const& guid : made)
            CHECK(Contains(*list, guid));
        CHECK(list->RaceUnlockCount >= 8);
    });
}

TEST_CASE("char_screen: creating a character with a name already in use fails with the name-in-use result", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        std::string const name = RandomCharacterName();
        bot.Send(MakeCreate(name));
        auto first = co_await bot.Packet<CharCreateResponse>(10000);
        REQUIRE(first);
        if (IsFullResult(first->Result))
            SKIP("the account has no free character slot (result " << int(first->Result) << ")");
        REQUIRE(first->Result == CLASSIC_CHAR_CREATE_SUCCESS);
        DeleteOnExit cleanup{ bot, first->Guid, true };

        bot.Send(MakeCreate(name));
        auto second = co_await bot.Packet<CharCreateResponse>(10000);
        REQUIRE(second);
        CHECK(second->Result == CLASSIC_CHAR_CREATE_NAME_IN_USE);
        CHECK(second->Guid.IsEmpty());

        // Confirmed, not fire-and-forget: CMSG_CHAR_DELETE is PACKET_PROCESS_WORLD (Step 9), so a
        // delete sent right before the connection tears down can lose the race against the next
        // world tick and never run. Waiting for the response here is what actually frees the slot.
        CharDelete del;
        del.Guid = first->Guid;
        bot.Send(del);
        auto deleted = co_await bot.Packet<CharDeleteResponse>(10000);
        REQUIRE(deleted);
        CHECK(deleted->Result == CLASSIC_CHAR_DELETE_SUCCESS);
        cleanup.Armed = false;
    });
}

TEST_CASE("char_screen: creating a character with an invalid name fails with a name result", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        // Digits, punctuation and a single letter are none of them names, whichever way a server
        // words the refusal; what it must not do is create the character.
        for (std::string const& bad : { std::string("Zq1234"), std::string("A"), std::string("Bad_Name!") })
        {
            bot.Send(MakeCreate(bad));
            auto response = co_await bot.Packet<CharCreateResponse>(10000);
            REQUIRE(response);
            INFO("name \"" << bad << "\" -> result " << int(response->Result));
            CHECK(response->Result != CLASSIC_CHAR_CREATE_SUCCESS);
            CHECK(ClassicIsCharNameResult(response->Result));
            CHECK(response->Guid.IsEmpty());
        }
    });
}

TEST_CASE("char_screen: CMSG_PLAYER_LOGIN for a guid that is not the account's is refused", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        // A player guid nobody on this account owns.
        PlayerLogin login;
        login.Guid = ClassicGuid{ 0x7FFFFFF0ull, 0x0000000000004000ull };
        login.FarClip = 533.0f;
        bot.Send(login);

        // Refused: the server says so (SMSG_CHARACTER_LOGIN_FAILED) or drops the connection, and
        // in neither case is the world entered. The bot follows no redirect (it is held), so no
        // SMSG_LOGIN_VERIFY_WORLD can reach it either way; what is checked is the refusal itself.
        bool const refused = co_await TestWaitUntil([&bot]()
        {
            return bot.CountReceived(CLASSIC_SMSG_CHARACTER_LOGIN_FAILED) > 0 || bot.GetStage() == BotStage::Failed;
        }, 15000);
        CHECK(refused);
        CHECK(bot.CountReceived(CLASSIC_SMSG_LOGIN_VERIFY_WORLD) == 0);
    });
}

TEST_CASE("char_screen: a logout request from the character screen is ignored", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Nothing is logged in yet at CharSelect, so CLASSIC_CMSG_LOGOUT_REQUEST (STATUS_LOGGEDIN)
        // has no player to act on; the fix for the null-GetPlayer() crash this used to hit is that
        // the opcode is dropped rather than dereferencing one.
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        bot.Send(ClassicLogoutRequest{});
        CHECK(co_await bot.NoPacket(CLASSIC_SMSG_LOGOUT_RESPONSE, 2000));

        // Still there: a ping gets its pong.
        Ping ping;
        ping.Serial = 0x70000001;
        bot.Send(ping);
        auto pong = co_await bot.Packet<Pong>(10000, [](Pong const& p) { return p.Serial == 0x70000001; });
        REQUIRE(pong);
        CHECK(bot.GetStage() == BotStage::CharSelect);

        // The server itself is still healthy, not just this one connection.
        TestBot second;
        CHECK(co_await second.Stage(BotStage::InWorld, 30000));
    });
}
