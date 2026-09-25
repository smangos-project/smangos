// Round trips for friends, ignores and the contact list that replaced them. Uses the raw
// SocialFlag/FriendStatus/FriendsResult values (SocialMgr.h) rather than including it: this target
// doesn't link `game`, and these are just bits on the wire as far as the struct is concerned.
#include "Packets/Social.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

namespace
{
    constexpr uint32 kSocialFlagFriend = 0x01;
    constexpr uint32 kSocialFlagIgnored = 0x02;
    constexpr uint8 kFriendStatusOnline = 1;
    constexpr uint8 kFriendStatusAfk = 2;
    constexpr uint8 kFriendAddedOnline = 0x06;
}

TEST_CASE("SendContactList round-trips", "[packets]")
{
    SendContactList out;
    out.Flags = kSocialFlagFriend | kSocialFlagIgnored;

    RoundTrip(out, [](SendContactList const& a, SendContactList const& b) { CHECK(a.Flags == b.Flags); });
    RejectsEveryTruncation(out);
}

TEST_CASE("SetContactNotes round-trips", "[packets]")
{
    SetContactNotes out;
    out.Player.Guid.Low = 0x42;
    out.Player.Guid.High = 0x0800000000000000ull;
    out.Player.VirtualRealmAddress = 0;
    out.Notes = "raid healer";

    RoundTrip(out, [](SetContactNotes const& a, SetContactNotes const& b)
    {
        CHECK(a.Player.Guid.Low == b.Player.Guid.Low);
        CHECK(a.Player.Guid.High == b.Player.Guid.High);
        CHECK(a.Player.VirtualRealmAddress == b.Player.VirtualRealmAddress);
        CHECK(a.Notes == b.Notes);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("AddFriend round-trips a name and a note", "[packets]")
{
    AddFriend out;
    out.FriendName = "Cointon";
    out.Notes = "raid healer";

    RoundTrip(out, [](AddFriend const& a, AddFriend const& b)
    {
        CHECK(a.FriendName == b.FriendName);
        CHECK(a.Notes == b.Notes);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("DelFriend round-trips", "[packets]")
{
    DelFriend out;
    out.Player.Guid.Low = 0x99;
    out.Player.Guid.High = 0x0800000000000000ull;

    RoundTrip(out, [](DelFriend const& a, DelFriend const& b)
    {
        CHECK(a.Player.Guid.Low == b.Player.Guid.Low);
        CHECK(a.Player.Guid.High == b.Player.Guid.High);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("AddIgnore round-trips a name", "[packets]")
{
    AddIgnore out;
    out.IgnoreName = "Griefer";

    RoundTrip(out, [](AddIgnore const& a, AddIgnore const& b) { CHECK(a.IgnoreName == b.IgnoreName); });
    RejectsEveryTruncation(out);
}

TEST_CASE("DelIgnore round-trips", "[packets]")
{
    DelIgnore out;
    out.Player.Guid.Low = 0x17;
    out.Player.Guid.High = 0x0800000000000000ull;

    RoundTrip(out, [](DelIgnore const& a, DelIgnore const& b)
    {
        CHECK(a.Player.Guid.Low == b.Player.Guid.Low);
        CHECK(a.Player.Guid.High == b.Player.Guid.High);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ContactList round-trips an empty list", "[packets]")
{
    ContactList out;
    out.Flags = kSocialFlagFriend | kSocialFlagIgnored;

    RoundTrip(out, [](ContactList const& a, ContactList const& b)
    {
        CHECK(a.Flags == b.Flags);
        CHECK(b.Contacts.empty());
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ContactList round-trips friends and ignores with notes", "[packets]")
{
    ContactList out;
    out.Flags = kSocialFlagFriend | kSocialFlagIgnored;

    ContactInfo friendContact;
    friendContact.Guid.Low = 0x42;
    friendContact.Guid.High = 0x0800000000000000ull;
    friendContact.TypeFlags = kSocialFlagFriend;
    friendContact.Notes = "raid healer";
    friendContact.Status = kFriendStatusOnline;
    friendContact.Area = 12;
    friendContact.Level = 60;
    friendContact.ClassId = 5;
    out.Contacts.push_back(friendContact);

    ContactInfo ignoreContact;
    ignoreContact.Guid.Low = 0x99;
    ignoreContact.Guid.High = 0x0800000000000000ull;
    ignoreContact.TypeFlags = kSocialFlagIgnored;
    out.Contacts.push_back(ignoreContact);

    RoundTrip(out, [](ContactList const& a, ContactList const& b)
    {
        CHECK(a.Flags == b.Flags);
        REQUIRE(a.Contacts.size() == b.Contacts.size());
        for (size_t i = 0; i < a.Contacts.size(); ++i)
        {
            CHECK(a.Contacts[i].Guid.Low == b.Contacts[i].Guid.Low);
            CHECK(a.Contacts[i].Guid.High == b.Contacts[i].Guid.High);
            CHECK(a.Contacts[i].TypeFlags == b.Contacts[i].TypeFlags);
            CHECK(a.Contacts[i].Notes == b.Contacts[i].Notes);
            CHECK(a.Contacts[i].Status == b.Contacts[i].Status);
            CHECK(a.Contacts[i].Area == b.Contacts[i].Area);
            CHECK(a.Contacts[i].Level == b.Contacts[i].Level);
            CHECK(a.Contacts[i].ClassId == b.Contacts[i].ClassId);
        }
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("FriendStatusUpdate round-trips", "[packets]")
{
    FriendStatusUpdate out;
    out.Result = kFriendAddedOnline;
    out.FriendGuid.Low = 0x42;
    out.FriendGuid.High = 0x0800000000000000ull;
    out.Notes = "raid healer";
    out.Status = kFriendStatusAfk;
    out.Area = 12;
    out.Level = 60;
    out.ClassId = 5;

    RoundTrip(out, [](FriendStatusUpdate const& a, FriendStatusUpdate const& b)
    {
        CHECK(a.Result == b.Result);
        CHECK(a.FriendGuid.Low == b.FriendGuid.Low);
        CHECK(a.FriendGuid.High == b.FriendGuid.High);
        CHECK(a.Notes == b.Notes);
        CHECK(a.Status == b.Status);
        CHECK(a.Area == b.Area);
        CHECK(a.Level == b.Level);
        CHECK(a.ClassId == b.ClassId);
    });
    RejectsEveryTruncation(out);
}
