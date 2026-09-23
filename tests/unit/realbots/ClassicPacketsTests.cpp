// Round-trip / wire-shape coverage for the CharSelect/EnteringWorld packets in ClassicPackets.h
// (bot-phase-1.md's packet rule: every Classic* struct gets a round-trip test). CMSG structs
// (ClassicEnumCharacters, ClassicCreateCharacter, ClassicPlayerLogin) only have Write() -- the
// bot uses one direction, per bot-examples.md -- so those are checked against hand-verified wire
// bytes instead of round-tripped through a Read() that doesn't exist on this side.
#include "ClassicPackets.h"
#include <catch_amalgamated.hpp>

TEST_CASE("ClassicGuid round-trips, including a partially-empty mask", "[packets]")
{
    ClassicGuid out;
    out.Low = 0x0000A200000000CDull;  // bytes 0 and 5 of Low are non-zero
    out.High = 0;                     // High entirely zero -- highMask should end up 0, no bytes

    ByteBuffer data;
    out.Write(data);
    CHECK(data.size() == 2 + 2); // lowMask + highMask + 2 non-zero low bytes + 0 high bytes

    ClassicGuid in;
    REQUIRE(in.Read(data));
    CHECK(in.Low == out.Low);
    CHECK(in.High == out.High);
    CHECK_FALSE(in.IsEmpty());
}

TEST_CASE("ClassicGuid IsEmpty is true only when both halves are zero", "[packets]")
{
    CHECK(ClassicGuid{}.IsEmpty());
    CHECK_FALSE(ClassicGuid{ 1, 0 }.IsEmpty());
    CHECK_FALSE(ClassicGuid{ 0, 1 }.IsEmpty());
}

TEST_CASE("ClassicEnumCharacters::Write produces an empty body", "[packets]")
{
    ByteBuffer data;
    ClassicEnumCharacters{}.Write(data);
    CHECK(data.size() == 0);
}

namespace
{
// Builds a body shaped like EnumCharactersResult::AppendBodyTo's own field order (see
// Character.cpp): 9 header bits flushed, characters.size(), 7 more skipped fields, then
// characterCount packed guids each followed by `trailingBytesPerCharacter` bytes of
// deliberately-unparsed per-character data (name/appearance/mail block/...), matching how the
// real packet keeps going well past what ClassicEnumCharactersResult::Read consumes.
ByteBuffer BuildEnumCharactersResultBody(uint32 characterCount, std::vector<ClassicGuid> const& guids, size_t trailingBytesPerCharacter)
{
    ByteBuffer data;
    for (int i = 0; i < 9; ++i)
        data.WriteBit(i == 0); // only "Success" true, rest false -- values besides CharacterCount/guid aren't read anyway
    data.FlushBits();

    data << characterCount;
    for (int i = 0; i < 7; ++i)
        data << uint32(0xAAAAAAAA); // deliberately non-zero: Read must skip these without using them

    for (uint32 i = 0; i < characterCount; ++i)
    {
        guids[i].Write(data);
        for (size_t b = 0; b < trailingBytesPerCharacter; ++b)
            data << uint8(0xFF);
    }
    return data;
}
}

TEST_CASE("ClassicEnumCharactersResult::Read extracts the count and first guid, ignoring the rest", "[packets]")
{
    std::vector<ClassicGuid> guids = { ClassicGuid{ 0x1122334455667788ull, 0 }, ClassicGuid{ 0xAABBCCDDull, 0 } };
    ByteBuffer data = BuildEnumCharactersResultBody(2, guids, 40); // trailing bytes never consumed by Read

    ClassicEnumCharactersResult result;
    REQUIRE(result.Read(data));
    CHECK(result.CharacterCount == 2);
    CHECK(result.FirstCharacterGuid.Low == guids[0].Low);
    CHECK(result.FirstCharacterGuid.High == guids[0].High);
}

TEST_CASE("ClassicEnumCharactersResult::Read succeeds with zero characters and no guid to read", "[packets]")
{
    ByteBuffer data = BuildEnumCharactersResultBody(0, {}, 0);

    ClassicEnumCharactersResult result;
    REQUIRE(result.Read(data));
    CHECK(result.CharacterCount == 0);
    CHECK(result.FirstCharacterGuid.IsEmpty());
}

TEST_CASE("ClassicEnumCharactersResult::Read rejects a body truncated before the count fields", "[packets]")
{
    ByteBuffer data;
    for (int i = 0; i < 9; ++i)
        data.WriteBit(false);
    data.FlushBits();
    data << uint32(1); // characters.size() present, but none of the 7 fields after it

    ClassicEnumCharactersResult result;
    CHECK_FALSE(result.Read(data));
}

TEST_CASE("ClassicCreateCharacter::Write matches the expected bit-packed wire shape", "[packets]")
{
    ClassicCreateCharacter create;
    create.Name = "Botto";
    create.Race = 1;
    create.Class = 4;
    create.Gender = 0;

    ByteBuffer data;
    create.Write(data);

    // Decode it back by hand, the way a server-side Read() would (mirroring
    // CharCreate::ReadFromWorldPacket's own field order).
    uint32 nameLength = data.ReadBits(6);
    bool hasTemplateSet = data.ReadBit();
    bool isTrialBoost = data.ReadBit();
    bool useNPE = data.ReadBit();
    bool hardcoreSelfFound = data.ReadBit();
    data.ResetBitPos();

    CHECK(nameLength == create.Name.size());
    CHECK_FALSE(hasTemplateSet);
    CHECK_FALSE(isTrialBoost);
    CHECK_FALSE(useNPE);
    CHECK_FALSE(hardcoreSelfFound);

    uint8 race, class_, gender;
    uint32 customizationCount;
    int32 timerunningSeasonId;
    data >> race >> class_ >> gender >> customizationCount >> timerunningSeasonId;
    CHECK(race == create.Race);
    CHECK(class_ == create.Class);
    CHECK(gender == create.Gender);
    CHECK(customizationCount == 0);
    CHECK(timerunningSeasonId == 0);

    std::string name = data.ReadString(nameLength);
    CHECK(name == create.Name);
    CHECK(data.rpos() == data.size()); // nothing else was written -- no templateSetId, no customizations
}

TEST_CASE("ClassicCreateCharResponse::Read parses the result and guid", "[packets]")
{
    ClassicGuid newCharacter{ 0x42ull, 0 };

    ByteBuffer data;
    data << CLASSIC_CHAR_CREATE_SUCCESS;
    newCharacter.Write(data);

    ClassicCreateCharResponse response;
    REQUIRE(response.Read(data));
    CHECK(response.Result == CLASSIC_CHAR_CREATE_SUCCESS);
    CHECK(response.Guid.Low == newCharacter.Low);
}

TEST_CASE("ClassicCreateCharResponse::Read rejects an empty body", "[packets]")
{
    ByteBuffer data;
    ClassicCreateCharResponse response;
    CHECK_FALSE(response.Read(data));
}

TEST_CASE("ClassicPlayerLogin::Write matches the expected guid-then-float wire shape", "[packets]")
{
    ClassicPlayerLogin login;
    login.Guid = ClassicGuid{ 0x99ull, 0 };
    login.FarClip = 533.0f;

    ByteBuffer data;
    login.Write(data);

    ClassicGuid decodedGuid;
    REQUIRE(decodedGuid.Read(data));
    CHECK(decodedGuid.Low == login.Guid.Low);

    float farClip;
    data >> farClip;
    CHECK(farClip == login.FarClip);
    CHECK(data.rpos() == data.size());
}

TEST_CASE("ClassicLoginVerifyWorld::Read round-trips a world location", "[packets]")
{
    ByteBuffer data;
    data << uint32(1) << 100.0f << -200.5f << 50.25f << 3.14f << uint32(0);

    ClassicLoginVerifyWorld verify;
    REQUIRE(verify.Read(data));
    CHECK(verify.MapId == 1);
    CHECK(verify.X == 100.0f);
    CHECK(verify.Y == -200.5f);
    CHECK(verify.Z == 50.25f);
    CHECK(verify.O == 3.14f);
    CHECK(verify.Reason == 0);
}

TEST_CASE("ClassicLoginVerifyWorld::Read rejects a truncated body", "[packets]")
{
    ByteBuffer data;
    data << uint32(1) << 100.0f;

    ClassicLoginVerifyWorld verify;
    CHECK_FALSE(verify.Read(data));
}
