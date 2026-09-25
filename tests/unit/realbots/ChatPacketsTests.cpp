// Round trips for the 13 modern CMSG_CHAT_MESSAGE_* opcodes and the SMSG_CHAT that replaced
// SMSG_MESSAGECHAT.
#include "Packets/ChatMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("ChatMessageSay round-trips", "[packets]")
{
    ChatMessageSay out;
    out.Language = 7;
    out.Message = "hello there";

    RoundTrip(out, [](ChatMessageSay const& a, ChatMessageSay const& b)
    {
        CHECK(a.Language == b.Language);
        CHECK(a.Message == b.Message);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ChatMessageParty/Raid/RaidWarning/InstanceChat round-trip like ChatMessageSay", "[packets]")
{
    ChatMessageParty party;
    party.Language = 1;
    party.Message = "party time";
    RoundTrip(party, [](ChatMessageParty const& a, ChatMessageParty const& b)
    {
        CHECK(a.Language == b.Language);
        CHECK(a.Message == b.Message);
    });

    ChatMessageRaid raid;
    raid.Message = "raid time";
    RoundTrip(raid, [](ChatMessageRaid const& a, ChatMessageRaid const& b) { CHECK(a.Message == b.Message); });

    ChatMessageRaidWarning warning;
    warning.Message = "incoming";
    RoundTrip(warning, [](ChatMessageRaidWarning const& a, ChatMessageRaidWarning const& b) { CHECK(a.Message == b.Message); });

    ChatMessageInstanceChat instance;
    instance.Message = "lfg";
    RoundTrip(instance, [](ChatMessageInstanceChat const& a, ChatMessageInstanceChat const& b) { CHECK(a.Message == b.Message); });
}

TEST_CASE("ChatMessageYell round-trips with no secure-gate bit", "[packets]")
{
    ChatMessageYell out;
    out.Language = 7;
    out.Message = "for the horde";

    ByteBuffer data;
    out.Write(data);
    CHECK(data.size() == sizeof(uint32) + 2 + out.Message.size()); // no extra IsSecure byte

    RoundTrip(out, [](ChatMessageYell const& a, ChatMessageYell const& b)
    {
        CHECK(a.Language == b.Language);
        CHECK(a.Message == b.Message);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ChatMessageGuild/Officer round-trip like ChatMessageYell", "[packets]")
{
    ChatMessageGuild guild;
    guild.Message = "raid tonight";
    RoundTrip(guild, [](ChatMessageGuild const& a, ChatMessageGuild const& b) { CHECK(a.Message == b.Message); });

    ChatMessageOfficer officer;
    officer.Message = "invite this one";
    RoundTrip(officer, [](ChatMessageOfficer const& a, ChatMessageOfficer const& b) { CHECK(a.Message == b.Message); });
}

TEST_CASE("ChatMessageEmote round-trips with no language field", "[packets]")
{
    ChatMessageEmote out;
    out.Message = "dances";

    RoundTrip(out, [](ChatMessageEmote const& a, ChatMessageEmote const& b) { CHECK(a.Message == b.Message); });
    RejectsEveryTruncation(out);
}

TEST_CASE("ChatMessageAfk/Dnd round-trip like ChatMessageEmote", "[packets]")
{
    ChatMessageAfk afk;
    afk.Message = "afk";
    RoundTrip(afk, [](ChatMessageAfk const& a, ChatMessageAfk const& b) { CHECK(a.Message == b.Message); });

    ChatMessageDnd dnd;
    dnd.Message = "dnd";
    RoundTrip(dnd, [](ChatMessageDnd const& a, ChatMessageDnd const& b) { CHECK(a.Message == b.Message); });
}

TEST_CASE("ChatMessageWhisper round-trips, including an empty target/message", "[packets]")
{
    ChatMessageWhisper out;
    out.Language = 7;
    out.TargetName = "Cointon";
    out.Message = "hey";

    RoundTrip(out, [](ChatMessageWhisper const& a, ChatMessageWhisper const& b)
    {
        CHECK(a.Language == b.Language);
        CHECK(a.TargetName == b.TargetName);
        CHECK(a.Message == b.Message);
    });
    RejectsEveryTruncation(out);

    ChatMessageWhisper empty;
    RoundTrip(empty, [](ChatMessageWhisper const&, ChatMessageWhisper const& b)
    {
        CHECK(b.TargetName.empty());
        CHECK(b.Message.empty());
    });
}

TEST_CASE("ChatMessageChannel round-trips", "[packets]")
{
    ChatMessageChannel out;
    out.Language = 7;
    out.ChannelName = "General";
    out.Message = "wts stuff";

    RoundTrip(out, [](ChatMessageChannel const& a, ChatMessageChannel const& b)
    {
        CHECK(a.Language == b.Language);
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.Message == b.Message);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("Chat (SMSG_CHAT) round-trips a say line", "[packets]")
{
    Chat out;
    out.MsgType = 0x01; // modern CHAT_MSG_SAY
    out.Language = 7;
    out.SenderGuid.Low = 0x42;
    out.SenderGuid.High = 0x0800000000000000ull;
    out.ChatFlags = 0;
    out.SenderName = "Cointon";
    out.Message = "hello there";

    RoundTrip(out, [](Chat const& a, Chat const& b)
    {
        CHECK(a.MsgType == b.MsgType);
        CHECK(a.Language == b.Language);
        CHECK(a.SenderGuid.Low == b.SenderGuid.Low);
        CHECK(a.SenderGuid.High == b.SenderGuid.High);
        CHECK(a.ChatFlags == b.ChatFlags);
        CHECK(a.SenderName == b.SenderName);
        CHECK(a.Message == b.Message);
        CHECK(b.ChannelGuid.IsEmpty());
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("Chat (SMSG_CHAT) round-trips a channel line with a channel guid", "[packets]")
{
    Chat out;
    out.MsgType = 0x11; // modern CHAT_MSG_CHANNEL
    out.Language = 7;
    out.ChannelName = "General";
    out.Message = "wts stuff";
    out.ChannelGuid.Low = 0x99;
    out.ChannelGuid.High = 0x0800000000000000ull;

    RoundTrip(out, [](Chat const& a, Chat const& b)
    {
        CHECK(a.ChannelName == b.ChannelName);
        CHECK(a.Message == b.Message);
        CHECK(a.ChannelGuid.Low == b.ChannelGuid.Low);
        CHECK(a.ChannelGuid.High == b.ChannelGuid.High);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("Chat (SMSG_CHAT) round-trips a system message with no sender", "[packets]")
{
    Chat out;
    out.MsgType = 0x00; // modern CHAT_MSG_SYSTEM
    out.Language = 0;
    out.Message = "Welcome to the server";

    RoundTrip(out, [](Chat const& a, Chat const& b)
    {
        CHECK(a.MsgType == b.MsgType);
        CHECK(a.Message == b.Message);
        CHECK(b.SenderGuid.IsEmpty());
        CHECK(b.SenderName.empty());
    });
    RejectsEveryTruncation(out);
}
