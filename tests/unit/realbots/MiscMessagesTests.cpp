// Round trips for the rest of the modern Misc.h leftovers: /who is, emotes, tutorials and
// account data.
#include "Packets/MiscMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("EmoteRequest round-trips and accepts the login-time empty body", "[packets]")
{
    EmoteRequest packet;
    packet.EmoteId = 1; // EMOTE_ONESHOT_WAVE
    RoundTrip(packet, [](EmoteRequest const& a, EmoteRequest const& b) { CHECK(a.EmoteId == b.EmoteId); });

    ByteBuffer empty;
    EmoteRequest parsed;
    REQUIRE(parsed.Read(empty));
    CHECK(parsed.EmoteId == 0);

    // Every strict prefix of the 4-byte body except the empty one must still be rejected.
    ByteBuffer full;
    packet.Write(full);
    for (size_t length = 1; length < full.size(); ++length)
    {
        ByteBuffer truncated;
        truncated.append(full.contents(), length);
        EmoteRequest rejected;
        INFO("truncated to " << length << " of " << full.size() << " bytes");
        CHECK_FALSE(rejected.Read(truncated));
    }
}

TEST_CASE("TextEmoteRequest round-trips with and without visual kits", "[packets]")
{
    TextEmoteRequest packet;
    packet.TargetGuid = { 0x1234, 0 };
    packet.EmoteId = 20;
    packet.SoundIndex = 1;
    RoundTrip(packet, [](TextEmoteRequest const& a, TextEmoteRequest const& b)
    {
        CHECK(a.TargetGuid == b.TargetGuid);
        CHECK(a.EmoteId == b.EmoteId);
        CHECK(a.SoundIndex == b.SoundIndex);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("TutorialRequest round-trips update, clear and reset actions", "[packets]")
{
    TutorialRequest update;
    update.Action = TutorialRequest::ActionUpdate;
    update.TutorialBit = 42;
    RoundTrip(update, [](TutorialRequest const& a, TutorialRequest const& b)
    {
        CHECK(a.Action == b.Action);
        CHECK(a.TutorialBit == b.TutorialBit);
    });
    RejectsEveryTruncation(update);

    TutorialRequest clear;
    clear.Action = TutorialRequest::ActionClear;
    RoundTrip(clear, [](TutorialRequest const& a, TutorialRequest const& b) { CHECK(a.Action == b.Action); });
    RejectsEveryTruncation(clear);

    TutorialRequest reset;
    reset.Action = TutorialRequest::ActionReset;
    RoundTrip(reset, [](TutorialRequest const& a, TutorialRequest const& b) { CHECK(a.Action == b.Action); });
    RejectsEveryTruncation(reset);
}

TEST_CASE("WhoIsRequest round-trips", "[packets]")
{
    WhoIsRequest packet;
    packet.CharacterName = "Realbots";
    RoundTrip(packet, [](WhoIsRequest const& a, WhoIsRequest const& b) { CHECK(a.CharacterName == b.CharacterName); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AccountDataRequest round-trips", "[packets]")
{
    AccountDataRequest packet;
    packet.PlayerGuid = { 0x4321, 0 };
    packet.DataType = 3;
    RoundTrip(packet, [](AccountDataRequest const& a, AccountDataRequest const& b)
    {
        CHECK(a.PlayerGuid == b.PlayerGuid);
        CHECK(a.DataType == b.DataType);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AccountDataUpdate round-trips with and without a payload", "[packets]")
{
    AccountDataUpdate empty;
    empty.PlayerGuid = { 0x5555, 0 };
    empty.Time = 1700000000;
    empty.DecompressedSize = 0;
    empty.DataType = 7;
    RoundTrip(empty, [](AccountDataUpdate const& a, AccountDataUpdate const& b)
    {
        CHECK(a.PlayerGuid == b.PlayerGuid);
        CHECK(a.Time == b.Time);
        CHECK(a.DecompressedSize == b.DecompressedSize);
        CHECK(a.DataType == b.DataType);
        CHECK(a.CompressedData.empty());
        CHECK(b.CompressedData.empty());
    });
    RejectsEveryTruncation(empty);

    AccountDataUpdate payload;
    payload.PlayerGuid = { 0x6666, 0 };
    payload.Time = 1700000001;
    payload.DecompressedSize = 100;
    payload.DataType = 1;
    payload.CompressedData = { 1, 2, 3, 4, 5 };
    RoundTrip(payload, [](AccountDataUpdate const& a, AccountDataUpdate const& b) { CHECK(a.CompressedData == b.CompressedData); });
    RejectsEveryTruncation(payload);
}

TEST_CASE("EmoteNotify round-trips", "[packets]")
{
    EmoteNotify packet;
    packet.UnitGuid = { 0x7777, 0 };
    packet.EmoteId = 66;
    RoundTrip(packet, [](EmoteNotify const& a, EmoteNotify const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.EmoteId == b.EmoteId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("TextEmoteNotify round-trips", "[packets]")
{
    TextEmoteNotify packet;
    packet.SourceGuid = { 0x8888, 0 };
    packet.EmoteId = 20;
    packet.SoundIndex = 1;
    packet.TargetGuid = { 0x9999, 0 };
    RoundTrip(packet, [](TextEmoteNotify const& a, TextEmoteNotify const& b)
    {
        CHECK(a.SourceGuid == b.SourceGuid);
        CHECK(a.SourceAccountGuid == b.SourceAccountGuid);
        CHECK(a.EmoteId == b.EmoteId);
        CHECK(a.SoundIndex == b.SoundIndex);
        CHECK(a.TargetGuid == b.TargetGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("WhoIsResult round-trips", "[packets]")
{
    WhoIsResult packet;
    packet.Message = "Realbots's account is Unknown, e-mail: Unknown, last ip: Unknown";
    RoundTrip(packet, [](WhoIsResult const& a, WhoIsResult const& b) { CHECK(a.Message == b.Message); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AccountDataUpdateResult round-trips with and without a payload", "[packets]")
{
    AccountDataUpdateResult empty;
    empty.Timestamp = 1700000000;
    empty.DecompressedLength = 0;
    empty.PlayerGuid = { 0xaaaa, 0 };
    empty.DataType = 2;
    RoundTrip(empty, [](AccountDataUpdateResult const& a, AccountDataUpdateResult const& b)
    {
        CHECK(a.Timestamp == b.Timestamp);
        CHECK(a.DecompressedLength == b.DecompressedLength);
        CHECK(a.PlayerGuid == b.PlayerGuid);
        CHECK(a.DataType == b.DataType);
        CHECK(a.CompressedData.empty());
        CHECK(b.CompressedData.empty());
    });
    RejectsEveryTruncation(empty);

    AccountDataUpdateResult payload;
    payload.Timestamp = 1700000002;
    payload.DecompressedLength = 100;
    payload.PlayerGuid = { 0xbbbb, 0 };
    payload.DataType = 5;
    payload.CompressedData = { 9, 8, 7, 6 };
    RoundTrip(payload, [](AccountDataUpdateResult const& a, AccountDataUpdateResult const& b) { CHECK(a.CompressedData == b.CompressedData); });
    RejectsEveryTruncation(payload);
}

TEST_CASE("SetPvP round-trips both values", "[packets]")
{
    for (bool enable : { false, true })
    {
        SetPvP packet;
        packet.EnablePvP = enable;
        RoundTrip(packet, [](SetPvP const& a, SetPvP const& b) { CHECK(a.EnablePvP == b.EnablePvP); });
        RejectsEveryTruncation(packet);
    }
}

TEST_CASE("RequestPlayedTime round-trips", "[packets]")
{
    RequestPlayedTime packet;
    packet.TriggerScriptEvent = true;
    RoundTrip(packet, [](RequestPlayedTime const& a, RequestPlayedTime const& b) { CHECK(a.TriggerScriptEvent == b.TriggerScriptEvent); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PlayedTimeResult round-trips", "[packets]")
{
    PlayedTimeResult packet;
    packet.TotalTime = 360000;
    packet.LevelTime = 7200;
    packet.TriggerEvent = true;
    RoundTrip(packet, [](PlayedTimeResult const& a, PlayedTimeResult const& b)
    {
        CHECK(a.TotalTime == b.TotalTime);
        CHECK(a.LevelTime == b.LevelTime);
        CHECK(a.TriggerEvent == b.TriggerEvent);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SetActionButton round-trips a set and a clear", "[packets]")
{
    for (uint32 action : { 0u, 0x01000005u })
    {
        SetActionButton packet;
        packet.Action = action;
        packet.Index = 11;
        RoundTrip(packet, [](SetActionButton const& a, SetActionButton const& b)
        {
            CHECK(a.Action == b.Action);
            CHECK(a.Index == b.Index);
        });
        RejectsEveryTruncation(packet);
    }
}

TEST_CASE("SetActionBarToggles round-trips", "[packets]")
{
    SetActionBarToggles packet;
    packet.Mask = 0x3;
    RoundTrip(packet, [](SetActionBarToggles const& a, SetActionBarToggles const& b) { CHECK(a.Mask == b.Mask); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("GameObjectUse round-trips", "[packets]")
{
    GameObjectUse packet;
    packet.Guid = { 0x4242, 0 };
    RoundTrip(packet, [](GameObjectUse const& a, GameObjectUse const& b) { CHECK(a.Guid == b.Guid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("AreaTrigger round-trips", "[packets]")
{
    for (bool entered : { false, true })
    {
        AreaTrigger packet;
        packet.TriggerId = 3646;
        packet.Entered = entered;
        packet.FromClient = true;
        RoundTrip(packet, [](AreaTrigger const& a, AreaTrigger const& b)
        {
            CHECK(a.TriggerId == b.TriggerId);
            CHECK(a.Entered == b.Entered);
            CHECK(a.FromClient == b.FromClient);
        });
        RejectsEveryTruncation(packet);
    }
}
