// Round trips for the packets that gained their second half in Phase 2 (port-plan.md: "every
// Classic* packet struct has both Read() and Write(), with a round-trip unit test"): what the bot
// only sent gained a Read (the server port and the fake world server in tests/support need it), and
// what the bot only received gained a Write. ClassicPacketsTests.cpp keeps the wire-shape checks
// from Phase 1.
#include "Packets/Classic/Auth.h"
#include "Packets/Classic/Misc.h"
#include "Packets/Classic/Guid.h"
#include "Packets/Classic/Character.h"
#include <catch_amalgamated.hpp>

namespace
{
// Writes `out`, reads the result back into a fresh T, and hands both to `check`; also checks that
// Read consumed exactly what Write produced.
template<typename T, typename Check>
void RoundTrip(T const& out, Check check)
{
    ByteBuffer data;
    out.Write(data);

    T in;
    REQUIRE(in.Read(data));
    CHECK(data.rpos() == data.size());
    check(out, in);
}

// Every strict prefix of a valid body must be rejected, never crash and never read past the end.
template<typename T>
void RejectsEveryTruncation(T const& valid)
{
    ByteBuffer full;
    valid.Write(full);

    for (size_t length = 0; length < full.size(); ++length)
    {
        ByteBuffer truncated;
        if (length)
            truncated.append(full.contents(), length);

        T in;
        INFO("truncated to " << length << " of " << full.size() << " bytes");
        CHECK_FALSE(in.Read(truncated));
    }
}

template<size_t N>
std::array<uint8, N> Pattern(uint8 seed)
{
    std::array<uint8, N> out{};
    for (size_t i = 0; i < N; ++i)
        out[i] = uint8(seed + i * 7);
    return out;
}
}

TEST_CASE("ClassicAuthChallenge round-trips", "[packets]")
{
    ClassicAuthChallenge out;
    out.DosChallenge = Pattern<32>(1);
    out.ServerChallenge = Pattern<32>(9);
    out.DosZeroBits = 4;

    RoundTrip(out, [](ClassicAuthChallenge const& a, ClassicAuthChallenge const& b)
    {
        CHECK(a.DosChallenge == b.DosChallenge);
        CHECK(a.ServerChallenge == b.ServerChallenge);
        CHECK(a.DosZeroBits == b.DosZeroBits);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicAuthSession round-trips, including the variable-length ticket", "[packets]")
{
    ClassicAuthSession out;
    out.DosResponse = 0x1122334455667788ull;
    out.RegionId = 3;
    out.BattlegroupId = 4;
    out.RealmId = 5;
    out.LocalChallenge = Pattern<32>(2);
    out.Digest = Pattern<24>(3);
    out.UseIPv6 = true;
    out.RealmJoinTicket = "12345";

    RoundTrip(out, [](ClassicAuthSession const& a, ClassicAuthSession const& b)
    {
        CHECK(a.DosResponse == b.DosResponse);
        CHECK(a.RegionId == b.RegionId);
        CHECK(a.BattlegroupId == b.BattlegroupId);
        CHECK(a.RealmId == b.RealmId);
        CHECK(a.LocalChallenge == b.LocalChallenge);
        CHECK(a.Digest == b.Digest);
        CHECK(a.UseIPv6 == b.UseIPv6);
        CHECK(a.RealmJoinTicket == b.RealmJoinTicket);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicAuthSession::Read rejects a ticket length that runs past the body", "[packets]")
{
    ClassicAuthSession out;
    out.RealmJoinTicket = "abc";
    ByteBuffer data;
    out.Write(data);

    // Overwrite the uint32 ticket length (just before the 3 ticket bytes) with something huge.
    size_t const lengthOffset = data.size() - out.RealmJoinTicket.size() - sizeof(uint32);
    data.put<uint32>(lengthOffset, 0xFFFFFFFFu);

    ClassicAuthSession in;
    CHECK_FALSE(in.Read(data));
}

TEST_CASE("ClassicEnterEncryptedMode round-trips both values of Enabled", "[packets]")
{
    for (bool enabled : { true, false })
    {
        ClassicEnterEncryptedMode out;
        out.Signature = Pattern<64>(5);
        out.Enabled = enabled;

        RoundTrip(out, [](ClassicEnterEncryptedMode const& a, ClassicEnterEncryptedMode const& b)
        {
            CHECK(a.Signature == b.Signature);
            CHECK(a.Enabled == b.Enabled);
        });
    }
    RejectsEveryTruncation(ClassicEnterEncryptedMode{});
}

TEST_CASE("ClassicEnterEncryptedModeAck, ClassicResumeComms, ClassicEnumCharacters, ClassicKeepAlive and ClassicLogoutComplete are empty bodies", "[packets]")
{
    auto emptyBody = [](auto packet)
    {
        ByteBuffer data;
        packet.Write(data);
        CHECK(data.size() == 0);
        CHECK(packet.Read(data));
    };
    emptyBody(ClassicEnterEncryptedModeAck{});
    emptyBody(ClassicResumeComms{});
    emptyBody(ClassicEnumCharacters{});
    emptyBody(ClassicKeepAlive{});
    emptyBody(ClassicLogoutComplete{});
}

TEST_CASE("ClassicConnectTo round-trips", "[packets]")
{
    ClassicConnectTo out;
    out.Signature = Pattern<256>(6);
    out.AddressIPv4 = { 127, 0, 0, 1 };
    out.Port = 8086;
    out.Serial = 77;
    out.ConnectionType = 1;
    out.Key = 0xAABBCCDD11223344ull;

    RoundTrip(out, [](ClassicConnectTo const& a, ClassicConnectTo const& b)
    {
        CHECK(a.Signature == b.Signature);
        CHECK(a.AddressIPv4 == b.AddressIPv4);
        CHECK(a.Port == b.Port);
        CHECK(a.Serial == b.Serial);
        CHECK(a.ConnectionType == b.ConnectionType);
        CHECK(a.Key == b.Key);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicAuthContinuedSession round-trips", "[packets]")
{
    ClassicAuthContinuedSession out;
    out.DosResponse = 9;
    out.Key = 0x0102030405060708ull;
    out.LocalChallenge = Pattern<32>(11);
    out.Digest = Pattern<24>(12);

    RoundTrip(out, [](ClassicAuthContinuedSession const& a, ClassicAuthContinuedSession const& b)
    {
        CHECK(a.DosResponse == b.DosResponse);
        CHECK(a.Key == b.Key);
        CHECK(a.LocalChallenge == b.LocalChallenge);
        CHECK(a.Digest == b.Digest);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicAuthResponse round-trips its parsed subset", "[packets]")
{
    ClassicAuthResponse out;
    out.Result = 12;
    out.SuccessInfoPresent = true;
    out.WaitInfoPresent = false;

    ByteBuffer data;
    out.Write(data);

    ClassicAuthResponse in;
    REQUIRE(in.Read(data));
    CHECK(in.Result == 12);
    CHECK(in.SuccessInfoPresent);
    CHECK_FALSE(in.WaitInfoPresent);
}

TEST_CASE("ClassicEnumCharactersResult round-trips its parsed subset", "[packets]")
{
    ClassicEnumCharactersResult out;
    out.CharacterCount = 2;
    out.FirstCharacterGuid = ClassicGuid{ 0x1122334455ull, 0x9900ull };

    RoundTrip(out, [](ClassicEnumCharactersResult const& a, ClassicEnumCharactersResult const& b)
    {
        CHECK(a.CharacterCount == b.CharacterCount);
        CHECK(a.FirstCharacterGuid.Low == b.FirstCharacterGuid.Low);
        CHECK(a.FirstCharacterGuid.High == b.FirstCharacterGuid.High);
    });

    ClassicEnumCharactersResult empty;
    RoundTrip(empty, [](ClassicEnumCharactersResult const&, ClassicEnumCharactersResult const& b)
    {
        CHECK(b.CharacterCount == 0);
        CHECK(b.FirstCharacterGuid.IsEmpty());
    });
}

TEST_CASE("ClassicCreateCharacter round-trips", "[packets]")
{
    ClassicCreateCharacter out;
    out.Name = "Botto";
    out.Race = 1;
    out.Class = 4;
    out.Gender = 1;

    RoundTrip(out, [](ClassicCreateCharacter const& a, ClassicCreateCharacter const& b)
    {
        CHECK(a.Name == b.Name);
        CHECK(a.Race == b.Race);
        CHECK(a.Class == b.Class);
        CHECK(a.Gender == b.Gender);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicCreateCharacter::Read rejects a template set and customization pairs", "[packets]")
{
    ByteBuffer withTemplate;
    withTemplate.WriteBits(1, 6);
    withTemplate.WriteBit(true); // hasTemplateSet
    withTemplate.WriteBit(false);
    withTemplate.WriteBit(false);
    withTemplate.WriteBit(false);
    withTemplate.FlushBits();
    withTemplate << uint8(1) << uint8(1) << uint8(0) << uint32(0) << int32(0) << uint8('A');
    ClassicCreateCharacter in;
    CHECK_FALSE(in.Read(withTemplate));

    ByteBuffer withCustomization;
    withCustomization.WriteBits(1, 6);
    withCustomization.WriteBit(false);
    withCustomization.WriteBit(false);
    withCustomization.WriteBit(false);
    withCustomization.WriteBit(false);
    withCustomization.FlushBits();
    withCustomization << uint8(1) << uint8(1) << uint8(0) << uint32(1) << int32(0) << uint8('A');
    ClassicCreateCharacter in2;
    CHECK_FALSE(in2.Read(withCustomization));
}

TEST_CASE("ClassicCreateCharResponse round-trips", "[packets]")
{
    ClassicCreateCharResponse out;
    out.Result = CLASSIC_CHAR_CREATE_SUCCESS;
    out.Guid = ClassicGuid{ 0xABCDEFull, 0 };

    RoundTrip(out, [](ClassicCreateCharResponse const& a, ClassicCreateCharResponse const& b)
    {
        CHECK(a.Result == b.Result);
        CHECK(a.Guid.Low == b.Guid.Low);
        CHECK(a.Guid.High == b.Guid.High);
    });
}

TEST_CASE("ClassicPlayerLogin round-trips", "[packets]")
{
    ClassicPlayerLogin out;
    out.Guid = ClassicGuid{ 0x55AA55ull, 0x1000ull };
    out.FarClip = 533.0f;

    RoundTrip(out, [](ClassicPlayerLogin const& a, ClassicPlayerLogin const& b)
    {
        CHECK(a.Guid.Low == b.Guid.Low);
        CHECK(a.Guid.High == b.Guid.High);
        CHECK(a.FarClip == b.FarClip);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicLoginVerifyWorld round-trips", "[packets]")
{
    ClassicLoginVerifyWorld out;
    out.MapId = 1;
    out.X = -618.5f;
    out.Y = -4251.75f;
    out.Z = 38.7f;
    out.O = 1.25f;
    out.Reason = 0;

    RoundTrip(out, [](ClassicLoginVerifyWorld const& a, ClassicLoginVerifyWorld const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.O == b.O);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("ClassicTimeSyncRequest and ClassicTimeSyncResponse round-trip", "[packets]")
{
    ClassicTimeSyncRequest request;
    request.SequenceIndex = 21;
    RoundTrip(request, [](ClassicTimeSyncRequest const& a, ClassicTimeSyncRequest const& b) { CHECK(a.SequenceIndex == b.SequenceIndex); });
    RejectsEveryTruncation(request);

    ClassicTimeSyncResponse response;
    response.SequenceIndex = 21;
    response.ClientTime = 987654;
    RoundTrip(response, [](ClassicTimeSyncResponse const& a, ClassicTimeSyncResponse const& b)
    {
        CHECK(a.SequenceIndex == b.SequenceIndex);
        CHECK(a.ClientTime == b.ClientTime);
    });
    RejectsEveryTruncation(response);
}

TEST_CASE("ClassicLogoutRequest round-trips and rejects an empty body", "[packets]")
{
    RoundTrip(ClassicLogoutRequest{}, [](ClassicLogoutRequest const&, ClassicLogoutRequest const&) {});
    RejectsEveryTruncation(ClassicLogoutRequest{});
}

TEST_CASE("ClassicConnectToFailed round-trips, connection type first then the serial", "[packets]")
{
    ClassicConnectToFailed out;
    out.Serial = CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3;
    out.Connection = 1;

    RoundTrip(out, [](ClassicConnectToFailed const& a, ClassicConnectToFailed const& b)
    {
        CHECK(a.Serial == b.Serial);
        CHECK(a.Connection == b.Connection);
    });
    RejectsEveryTruncation(out);

    // Connection type byte, then the serial (little-endian).
    ByteBuffer data;
    out.Write(data);
    REQUIRE(data.size() == 5);
    CHECK(data.contents()[0] == 1);
    CHECK(data.contents()[1] == 53);
}

TEST_CASE("the ConnectToSerial attempts escalate the way both reference servers number them", "[packets]")
{
    CHECK(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1 == 17);
    CHECK(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2 == 35);
    CHECK(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3 == 53);
    CHECK(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4 == 71);
    CHECK(CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5 == 89);
}

namespace
{
ClassicCharacterInfo MakeCharacter(uint64 guidLow, std::string name, uint8 race, uint8 cls, uint8 level)
{
    ClassicCharacterInfo c;
    c.Guid = ClassicGuid{ guidLow, 0x4000 };
    c.ListPosition = 2;
    c.Race = race;
    c.Gender = 1;
    c.Class = cls;
    c.Level = level;
    c.Map = 1;
    c.Zone = 14;
    c.X = -618.5f;
    c.Y = -4251.75f;
    c.Z = 38.7f;
    c.CharFlags = 0x20;
    c.FirstLogin = true;
    c.Name = std::move(name);
    return c;
}
}

TEST_CASE("ClassicEnumCharactersResult round-trips every character's fixed-layout block", "[packets]")
{
    ClassicEnumCharactersResult out;
    out.Characters.push_back(MakeCharacter(0x111111, "Botto", 1, 1, 12));
    out.Characters.push_back(MakeCharacter(0x222222, "Zqwertyui", 4, 8, 60));

    ByteBuffer data;
    out.Write(data);

    ClassicEnumCharactersResult in;
    REQUIRE(in.Read(data));
    REQUIRE(in.Characters.size() == 2);
    CHECK(in.CharacterCount == 2);
    CHECK(in.FirstCharacterGuid.Low == 0x111111);

    for (size_t i = 0; i < 2; ++i)
    {
        ClassicCharacterInfo const& a = out.Characters[i];
        ClassicCharacterInfo const& b = in.Characters[i];
        CHECK(a.Guid.Low == b.Guid.Low);
        CHECK(a.Guid.High == b.Guid.High);
        CHECK(a.ListPosition == b.ListPosition);
        CHECK(a.Race == b.Race);
        CHECK(a.Gender == b.Gender);
        CHECK(a.Class == b.Class);
        CHECK(a.Level == b.Level);
        CHECK(a.Map == b.Map);
        CHECK(a.Zone == b.Zone);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.CharFlags == b.CharFlags);
        CHECK(a.FirstLogin == b.FirstLogin);
        CHECK(a.Name == b.Name);
    }
}

TEST_CASE("ClassicEnumCharactersResult: a full-layout body cut short falls back to the count and first guid", "[packets]")
{
    ClassicEnumCharactersResult out;
    out.Characters.push_back(MakeCharacter(0x333333, "Botto", 1, 1, 5));
    out.Characters.push_back(MakeCharacter(0x444444, "Other", 1, 1, 5));

    ByteBuffer full;
    out.Write(full);

    // Cut off inside the second character.
    ByteBuffer cut;
    cut.append(full.contents(), full.size() - 40);

    ClassicEnumCharactersResult in;
    REQUIRE(in.Read(cut));
    CHECK(in.Characters.empty());
    CHECK(in.CharacterCount == 2);
    CHECK(in.FirstCharacterGuid.Low == 0x333333);
}

TEST_CASE("ClassicEnumCharactersResult: a character with mail senders is not walked, only counted", "[packets]")
{
    ClassicEnumCharactersResult out;
    out.Characters.push_back(MakeCharacter(0x555555, "Mailed", 1, 1, 5));

    ByteBuffer data;
    out.Write(data);
    // The last twelve bytes are Flags4, MailSenders.Count, MailSenderTypes.Count: make it non-zero.
    data.put<uint32>(data.size() - 8, 3);

    ClassicEnumCharactersResult in;
    REQUIRE(in.Read(data));
    CHECK(in.Characters.empty());
    CHECK(in.FirstCharacterGuid.Low == 0x555555);
}

TEST_CASE("ClassicCharDelete, ClassicDeleteCharResponse and ClassicCharacterLoginFailed round-trip", "[packets]")
{
    ClassicCharDelete del;
    del.Guid = ClassicGuid{ 0xABC123, 0x4000 };
    RoundTrip(del, [](ClassicCharDelete const& a, ClassicCharDelete const& b)
    {
        CHECK(a.Guid.Low == b.Guid.Low);
        CHECK(a.Guid.High == b.Guid.High);
    });
    RejectsEveryTruncation(del);

    ClassicDeleteCharResponse response;
    response.Result = CLASSIC_CHAR_DELETE_SUCCESS;
    RoundTrip(response, [](ClassicDeleteCharResponse const& a, ClassicDeleteCharResponse const& b) { CHECK(a.Result == b.Result); });
    RejectsEveryTruncation(response);

    ClassicCharacterLoginFailed failed;
    failed.Reason = 1;
    RoundTrip(failed, [](ClassicCharacterLoginFailed const& a, ClassicCharacterLoginFailed const& b) { CHECK(a.Reason == b.Reason); });
    RejectsEveryTruncation(failed);
}

TEST_CASE("the character-screen result codes are the client's own", "[packets]")
{
    CHECK(CLASSIC_CHAR_CREATE_SUCCESS == 24);
    CHECK(CLASSIC_CHAR_CREATE_NAME_IN_USE == 27);
    CHECK(CLASSIC_CHAR_DELETE_SUCCESS == 65);
    CHECK_FALSE(ClassicIsCharNameResult(CLASSIC_CHAR_CREATE_NAME_IN_USE));
    CHECK_FALSE(ClassicIsCharNameResult(CLASSIC_CHAR_NAME_SUCCESS));
    CHECK(ClassicIsCharNameResult(CLASSIC_CHAR_NAME_TOO_SHORT));
    CHECK(ClassicIsCharNameResult(CLASSIC_CHAR_NAME_INVALID_CHARACTER));
}
