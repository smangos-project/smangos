// "char_screen/" scenarios (port-plan.md step 4: enum, create, delete, login), against
// ConformanceRun::Target(). A bot held at StopAt(CharSelect) is authenticated and encrypted on the
// Realm connection and has asked the character screen nothing, so each scenario drives the screen
// by hand. Scenarios that create characters use a random name and delete what they made (also when
// an assertion fails), so reruns and parallel targets never collide and the account's own character
// is never touched.
#include "ClassicPackets.h"
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

ClassicCreateCharacter MakeCreate(std::string name)
{
    ClassicCreateCharacter create;
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

// Deletes `guid` when the scope ends, whatever happened before (an assertion that throws included),
// so a failing scenario does not leave its test character behind. Sent without waiting: the bot is
// about to go away, and the delete is processed before its connection closes.
struct DeleteOnExit
{
    TestBot& Bot;
    ClassicGuid Guid;
    bool Armed = false;

    ~DeleteOnExit()
    {
        if (Armed)
        {
            ClassicCharDelete del;
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
        auto created = co_await bot.Packet<ClassicCreateCharResponse>(10000);
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

        ClassicCharDelete del;
        del.Guid = created->Guid;
        bot.Send(del);
        auto deleted = co_await bot.Packet<ClassicDeleteCharResponse>(10000);
        REQUIRE(deleted);
        CHECK(deleted->Result == CLASSIC_CHAR_DELETE_SUCCESS);
        cleanup.Armed = false;

        bot.Send(ClassicEnumCharacters{});
        auto afterDelete = co_await bot.Packet<ClassicEnumCharactersResult>(10000);
        REQUIRE(afterDelete);
        CHECK_FALSE(Contains(*afterDelete, created->Guid));
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
        auto first = co_await bot.Packet<ClassicCreateCharResponse>(10000);
        REQUIRE(first);
        if (IsFullResult(first->Result))
            SKIP("the account has no free character slot (result " << int(first->Result) << ")");
        REQUIRE(first->Result == CLASSIC_CHAR_CREATE_SUCCESS);
        DeleteOnExit cleanup{ bot, first->Guid, true };

        bot.Send(MakeCreate(name));
        auto second = co_await bot.Packet<ClassicCreateCharResponse>(10000);
        REQUIRE(second);
        CHECK(second->Result == CLASSIC_CHAR_CREATE_NAME_IN_USE);
        CHECK(second->Guid.IsEmpty());
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
            auto response = co_await bot.Packet<ClassicCreateCharResponse>(10000);
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
        ClassicPlayerLogin login;
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
