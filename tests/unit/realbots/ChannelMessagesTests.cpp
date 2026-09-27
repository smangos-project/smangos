// Round trips for the modern channel wire.
#include "Packets/ChannelMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("JoinChannel round-trips", "[packets]")
{
    JoinChannel packet;
    packet.ChatChannelId = 1;
    packet.CreateVoiceSession = true;
    packet.Internal = false;
    packet.ChannelName = "General";
    packet.Password = "secret";
    RoundTrip(packet, [](JoinChannel const& a, JoinChannel const& b)
    {
        CHECK(a.ChatChannelId == b.ChatChannelId);
        CHECK(a.CreateVoiceSession == b.CreateVoiceSession);
        CHECK(a.Internal == b.Internal);
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.Password == b.Password);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("LeaveChannel round-trips", "[packets]")
{
    LeaveChannel packet;
    packet.ZoneChannelId = 2;
    packet.ChannelName = "Trade";
    RoundTrip(packet, [](LeaveChannel const& a, LeaveChannel const& b)
    {
        CHECK(a.ZoneChannelId == b.ZoneChannelId);
        CHECK(a.ChannelName == b.ChannelName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelList round-trips", "[packets]")
{
    ChannelList packet;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](ChannelList const& a, ChannelList const& b) { CHECK(a.ChannelName == b.ChannelName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelDisplayList round-trips", "[packets]")
{
    ChannelDisplayList packet;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](ChannelDisplayList const& a, ChannelDisplayList const& b) { CHECK(a.ChannelName == b.ChannelName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelOwner round-trips", "[packets]")
{
    ChannelOwner packet;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](ChannelOwner const& a, ChannelOwner const& b) { CHECK(a.ChannelName == b.ChannelName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelAnnouncements round-trips", "[packets]")
{
    ChannelAnnouncements packet;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](ChannelAnnouncements const& a, ChannelAnnouncements const& b) { CHECK(a.ChannelName == b.ChannelName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelDeclineInvite round-trips", "[packets]")
{
    ChannelDeclineInvite packet;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](ChannelDeclineInvite const& a, ChannelDeclineInvite const& b) { CHECK(a.ChannelName == b.ChannelName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelPassword round-trips", "[packets]")
{
    ChannelPassword packet;
    packet.ChannelName = "RealbotsTest";
    packet.Password = "secret";
    RoundTrip(packet, [](ChannelPassword const& a, ChannelPassword const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.Password == b.Password);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelSetOwner round-trips", "[packets]")
{
    ChannelSetOwner packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelSetOwner const& a, ChannelSetOwner const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelModerator round-trips", "[packets]")
{
    ChannelModerator packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelModerator const& a, ChannelModerator const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelUnmoderator round-trips", "[packets]")
{
    ChannelUnmoderator packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelUnmoderator const& a, ChannelUnmoderator const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelInvite round-trips", "[packets]")
{
    ChannelInvite packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelInvite const& a, ChannelInvite const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelKick round-trips", "[packets]")
{
    ChannelKick packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelKick const& a, ChannelKick const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelBan round-trips", "[packets]")
{
    ChannelBan packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelBan const& a, ChannelBan const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelUnban round-trips", "[packets]")
{
    ChannelUnban packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelUnban const& a, ChannelUnban const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelSilenceAll round-trips", "[packets]")
{
    ChannelSilenceAll packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelSilenceAll const& a, ChannelSilenceAll const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelUnsilenceAll round-trips", "[packets]")
{
    ChannelUnsilenceAll packet;
    packet.ChannelName = "RealbotsTest";
    packet.PlayerName = "Realbot";
    RoundTrip(packet, [](ChannelUnsilenceAll const& a, ChannelUnsilenceAll const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.PlayerName == b.PlayerName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelNotify round-trips a plain notice", "[packets]")
{
    ChannelNotify packet;
    packet.Type = 0x00; // CHAT_JOINED_NOTICE
    packet.ChannelName = "RealbotsTest";
    packet.Sender = "Realbot";
    packet.SenderGuid = { 0x1111, 0 };
    packet.SenderAccountGuid = { 0x2222, 0 };
    packet.TargetGuid = { 0x3333, 0 };
    packet.ChatChannelId = 1;
    RoundTrip(packet, [](ChannelNotify const& a, ChannelNotify const& b)
    {
        CHECK(a.Type == b.Type);
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.Sender == b.Sender);
        CHECK(a.SenderGuid == b.SenderGuid);
        CHECK(a.SenderAccountGuid == b.SenderAccountGuid);
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.ChatChannelId == b.ChatChannelId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelNotify round-trips a mode-change notice with the flag tail", "[packets]")
{
    ChannelNotify packet;
    packet.Type = ChannelNotify::ModeChangeNotice;
    packet.ChannelName = "RealbotsTest";
    packet.SenderGuid = { 0x4444, 0 };
    packet.OldFlags = 0;
    packet.NewFlags = 2;
    RoundTrip(packet, [](ChannelNotify const& a, ChannelNotify const& b)
    {
        CHECK(a.Type == b.Type);
        CHECK(a.OldFlags == b.OldFlags);
        CHECK(a.NewFlags == b.NewFlags);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelNotifyJoined round-trips", "[packets]")
{
    ChannelNotifyJoined packet;
    packet.ChannelName = "RealbotsTest";
    packet.ChannelFlags = 0x18;
    packet.ChatChannelId = 0;
    packet.InstanceId = 0;
    packet.ChannelGuid = { 0x5555, 0 };
    RoundTrip(packet, [](ChannelNotifyJoined const& a, ChannelNotifyJoined const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.ChannelWelcomeMsg == b.ChannelWelcomeMsg);
        CHECK(a.ChannelFlags == b.ChannelFlags);
        CHECK(a.ChatChannelId == b.ChatChannelId);
        CHECK(a.InstanceId == b.InstanceId);
        CHECK(a.ChannelGuid == b.ChannelGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelNotifyLeft round-trips", "[packets]")
{
    ChannelNotifyLeft packet;
    packet.ChannelName = "RealbotsTest";
    packet.ChatChannelId = 0;
    packet.Suspended = false;
    RoundTrip(packet, [](ChannelNotifyLeft const& a, ChannelNotifyLeft const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.ChatChannelId == b.ChatChannelId);
        CHECK(a.Suspended == b.Suspended);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ChannelListResponse round-trips with no and with several members", "[packets]")
{
    ChannelListResponse packet;
    packet.Display = true;
    packet.ChannelName = "RealbotsTest";
    packet.ChannelFlags = 0x18;
    RoundTrip(packet, [](ChannelListResponse const& a, ChannelListResponse const& b)
    {
        CHECK(a.Display == b.Display);
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.ChannelFlags == b.ChannelFlags);
        CHECK(a.Members.size() == b.Members.size());
    });
    RejectsEveryTruncation(packet);

    packet.Members.push_back({ ClassicGuid{ 0x6666, 0 }, 0, 1 });
    packet.Members.push_back({ ClassicGuid{ 0x7777, 0 }, 0, 2 });
    RoundTrip(packet, [](ChannelListResponse const& a, ChannelListResponse const& b)
    {
        REQUIRE(a.Members.size() == b.Members.size());
        for (size_t i = 0; i < a.Members.size(); ++i)
        {
            CHECK(a.Members[i].Guid == b.Members[i].Guid);
            CHECK(a.Members[i].VirtualRealmAddress == b.Members[i].VirtualRealmAddress);
            CHECK(a.Members[i].Flags == b.Members[i].Flags);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("UserlistAdd round-trips", "[packets]")
{
    UserlistAdd packet;
    packet.AddedUserGuid = { 0x8888, 0 };
    packet.UserFlags = 1;
    packet.ChannelFlags = 0x18;
    packet.ChannelId = 1;
    packet.ChannelName = "General";
    RoundTrip(packet, [](UserlistAdd const& a, UserlistAdd const& b)
    {
        CHECK(a.AddedUserGuid == b.AddedUserGuid);
        CHECK(a.UserFlags == b.UserFlags);
        CHECK(a.ChannelFlags == b.ChannelFlags);
        CHECK(a.ChannelId == b.ChannelId);
        CHECK(a.ChannelName == b.ChannelName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("UserlistRemove round-trips", "[packets]")
{
    UserlistRemove packet;
    packet.RemovedUserGuid = { 0x9999, 0 };
    packet.ChannelFlags = 0x18;
    packet.ChannelId = 1;
    packet.ChannelName = "General";
    RoundTrip(packet, [](UserlistRemove const& a, UserlistRemove const& b)
    {
        CHECK(a.RemovedUserGuid == b.RemovedUserGuid);
        CHECK(a.ChannelFlags == b.ChannelFlags);
        CHECK(a.ChannelId == b.ChannelId);
        CHECK(a.ChannelName == b.ChannelName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("UserlistUpdate round-trips", "[packets]")
{
    UserlistUpdate packet;
    packet.UpdatedUserGuid = { 0xaaaa, 0 };
    packet.UserFlags = 2;
    packet.ChannelFlags = 0;
    packet.ChannelId = 0;
    packet.ChannelName = "RealbotsTest";
    RoundTrip(packet, [](UserlistUpdate const& a, UserlistUpdate const& b)
    {
        CHECK(a.UpdatedUserGuid == b.UpdatedUserGuid);
        CHECK(a.UserFlags == b.UserFlags);
        CHECK(a.ChannelFlags == b.ChannelFlags);
        CHECK(a.ChannelId == b.ChannelId);
        CHECK(a.ChannelName == b.ChannelName);
    });
    RejectsEveryTruncation(packet);
}
