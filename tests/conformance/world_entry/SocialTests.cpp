// Friends, ignores and the contact list that replaced them (bot-phase-4.md, slice B).
#include "Packets/Social.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <algorithm>
#include <optional>

namespace
{
// Raw SocialFlag/FriendsResult/FriendStatus values (SocialMgr.h): this target doesn't link `game`.
constexpr uint32_t kSocialFlagFriend = 0x01;
constexpr uint32_t kSocialFlagIgnored = 0x02;
constexpr uint8_t kFriendAddedOnline = 0x06;
constexpr uint8_t kFriendAlready = 0x08;
constexpr uint8_t kFriendStatusOnline = 1;
// FriendsResult values SendFriendStatus's login/logout broadcast uses (never an AddFriend result):
// a pool account may already have other friends from earlier runs, and one of them logging in
// around the same time broadcasts one of these with the same FriendGuid as an unrelated request.
constexpr uint8_t kFriendResultOnlineBroadcast = 0x02;
constexpr uint8_t kFriendResultOfflineBroadcast = 0x03;

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

TEST_CASE("world_entry: the login burst carries the character's contact list", "[protocol]")
{
    // Not asserting an empty list: the bot account pool is reused across runs, so a leased
    // account may already carry friends/ignores from an earlier run.
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await bot.WaitReceived(ContactList::Opcode, 10000));

        std::optional<ContactList> list = ParsedExactly<ContactList>(bot, BotConnectionKind::Realm);
        REQUIRE(list);
        CHECK(list->Flags == (kSocialFlagFriend | kSocialFlagIgnored));
    });
}

TEST_CASE("social: adding a friend by name lists them in the contact list", "[protocol]")
{
    // Not asserting FRIEND_ADDED_ONLINE specifically: the reused account pool means botA and botB
    // may already be friends from an earlier run, in which case AddFriend reports FRIEND_ALREADY
    // instead -- either way, botB must end up in botA's requested contact list.
    RunTest([]() -> TestRoutine
    {
        TestBot botA;
        TestBot botB;
        REQUIRE(co_await botA.Stage(BotStage::InWorld, 30000));
        REQUIRE(co_await botB.Stage(BotStage::InWorld, 30000));

        AddFriend addFriend;
        addFriend.FriendName = botB.GetCharacterName();
        botA.Send(addFriend);

        // Filtered by FriendGuid and excluding the broadcast-only result codes: if botA already had
        // botB as a friend from an earlier run against this reused pool, botB's own login moments
        // ago broadcasts a FRIEND_ONLINE status with the same FriendGuid, racing our actual response.
        ClassicGuid const friendGuid = botB.GetCharacterGuid();
        auto status = co_await botA.Packet<FriendStatusUpdate>(10000, [friendGuid](FriendStatusUpdate const& s)
        {
            return s.FriendGuid.Low == friendGuid.Low && s.FriendGuid.High == friendGuid.High
                && s.Result != kFriendResultOnlineBroadcast && s.Result != kFriendResultOfflineBroadcast;
        });
        REQUIRE(status);
        CHECK((status->Result == kFriendAddedOnline || status->Result == kFriendAlready));
        CHECK(status->FriendGuid.Low == botB.GetCharacterGuid().Low);
        CHECK(status->FriendGuid.High == botB.GetCharacterGuid().High);

        SendContactList request;
        request.Flags = kSocialFlagFriend;
        botA.Send(request);

        auto list = co_await botA.Packet<ContactList>(10000, [friendGuid](ContactList const& l)
        {
            return std::any_of(l.Contacts.begin(), l.Contacts.end(), [friendGuid](ContactInfo const& c)
            {
                return c.Guid.Low == friendGuid.Low && c.Guid.High == friendGuid.High;
            });
        });
        REQUIRE(list);
        CHECK(list->Flags == kSocialFlagFriend);

        auto found = std::find_if(list->Contacts.begin(), list->Contacts.end(), [friendGuid](ContactInfo const& c)
        {
            return c.Guid.Low == friendGuid.Low && c.Guid.High == friendGuid.High;
        });
        REQUIRE(found != list->Contacts.end());
        CHECK((found->TypeFlags & kSocialFlagFriend) != 0);
        CHECK(found->Status == kFriendStatusOnline);
    });
}
