#include "Packets/ClassicClient.h"
#include "Packets/Classic/Character.h"
#include "Packets/CharScreen.h"
#include "Packets/Session.h"
#include "Packets/Auth.h"

#include <catch_amalgamated.hpp>

namespace
{
template<typename T>
WorldPacket BuildPacket(T const& body)
{
    ByteBuffer bytes;
    body.Write(bytes);
    WorldPacket packet(uint32(T::Opcode), bytes.size());
    if (!bytes.empty())
        packet.append(bytes.contents(), bytes.size());
    return packet;
}
}

TEST_CASE("ClassicClient<T> exposes T's own opcode before any read", "[classicclient]")
{
    CHECK(ClassicClient<ClassicEnumCharacters>().GetOpcode() == uint32(CLASSIC_CMSG_ENUM_CHARACTERS));
    CHECK(ClassicClient<CharCreate>().GetOpcode() == uint32(CLASSIC_CMSG_CREATE_CHARACTER));
    CHECK(ClassicClient<CharDelete>().GetOpcode() == uint32(CLASSIC_CMSG_CHAR_DELETE));
    CHECK(ClassicClient<PlayerLogin>().GetOpcode() == uint32(CLASSIC_CMSG_PLAYER_LOGIN));
    CHECK(ClassicClient<ConnectToFailed>().GetOpcode() == uint32(CLASSIC_CMSG_CONNECT_TO_FAILED));
    CHECK(ClassicClient<ClassicLogoutRequest>().GetOpcode() == uint32(CLASSIC_CMSG_LOGOUT_REQUEST));
    CHECK(ClassicClient<TimeSyncResponse>().GetOpcode() == uint32(CLASSIC_CMSG_TIME_SYNC_RESPONSE));
}

TEST_CASE("A well-formed ClassicEnumCharacters body is valid and fully consumed", "[classicclient]")
{
    WorldPacket wire = BuildPacket(ClassicEnumCharacters{});
    ClassicClient<ClassicEnumCharacters> packet;
    packet.ReadFromWorldPacket(wire);
    CHECK(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
}

TEST_CASE("A well-formed CharCreate body round-trips through ClassicClient", "[classicclient]")
{
    CharCreate body;
    body.Name = "Testchar";
    body.Race = 1;
    body.Class = 4;
    body.Gender = 0;

    WorldPacket wire = BuildPacket(body);
    ClassicClient<CharCreate> packet;
    packet.ReadFromWorldPacket(wire);
    REQUIRE(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
    CHECK(packet.Body.Name == body.Name);
    CHECK(packet.Body.Race == body.Race);
    CHECK(packet.Body.Class == body.Class);
    CHECK(packet.Body.Gender == body.Gender);
}

TEST_CASE("A well-formed CharDelete body round-trips through ClassicClient", "[classicclient]")
{
    CharDelete body;
    body.Guid.Low = 0x42;
    body.Guid.High = uint64(2) << 58;

    WorldPacket wire = BuildPacket(body);
    ClassicClient<CharDelete> packet;
    packet.ReadFromWorldPacket(wire);
    REQUIRE(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
    CHECK(packet.Body.Guid.Low == body.Guid.Low);
    CHECK(packet.Body.Guid.High == body.Guid.High);
}

TEST_CASE("A well-formed PlayerLogin body round-trips through ClassicClient", "[classicclient]")
{
    PlayerLogin body;
    body.Guid.Low = 7;
    body.FarClip = 500.0f;

    WorldPacket wire = BuildPacket(body);
    ClassicClient<PlayerLogin> packet;
    packet.ReadFromWorldPacket(wire);
    REQUIRE(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
    CHECK(packet.Body.Guid.Low == body.Guid.Low);
    CHECK(packet.Body.FarClip == body.FarClip);
}

TEST_CASE("A well-formed ConnectToFailed body round-trips through ClassicClient", "[classicclient]")
{
    ConnectToFailed body;
    body.Connection = 1;
    body.Serial = 3;

    WorldPacket wire = BuildPacket(body);
    ClassicClient<ConnectToFailed> packet;
    packet.ReadFromWorldPacket(wire);
    REQUIRE(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
    CHECK(packet.Body.Connection == body.Connection);
    CHECK(packet.Body.Serial == body.Serial);
}

TEST_CASE("A well-formed ClassicLogoutRequest body is valid and fully consumed", "[classicclient]")
{
    WorldPacket wire = BuildPacket(ClassicLogoutRequest{});
    ClassicClient<ClassicLogoutRequest> packet;
    packet.ReadFromWorldPacket(wire);
    CHECK(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
}

TEST_CASE("A well-formed TimeSyncResponse body round-trips through ClassicClient", "[classicclient]")
{
    TimeSyncResponse body;
    body.SequenceIndex = 9;
    body.ClientTime = 123456;

    WorldPacket wire = BuildPacket(body);
    ClassicClient<TimeSyncResponse> packet;
    packet.ReadFromWorldPacket(wire);
    REQUIRE(packet.IsValid());
    CHECK(wire.rpos() == wire.size());
    CHECK(packet.Body.SequenceIndex == body.SequenceIndex);
    CHECK(packet.Body.ClientTime == body.ClientTime);
}

TEST_CASE("A truncated body is rejected instead of thrown or silently accepted", "[classicclient]")
{
    WorldPacket connectToFailedWire{uint32(CLASSIC_CMSG_CONNECT_TO_FAILED)};
    ClassicClient<ConnectToFailed> connectToFailed;
    connectToFailed.ReadFromWorldPacket(connectToFailedWire);
    CHECK_FALSE(connectToFailed.IsValid());

    WorldPacket timeSyncWire{uint32(CLASSIC_CMSG_TIME_SYNC_RESPONSE)};
    ClassicClient<TimeSyncResponse> timeSync;
    timeSync.ReadFromWorldPacket(timeSyncWire);
    CHECK_FALSE(timeSync.IsValid());

    WorldPacket playerLoginWire{uint32(CLASSIC_CMSG_PLAYER_LOGIN)};
    ClassicClient<PlayerLogin> playerLogin;
    playerLogin.ReadFromWorldPacket(playerLoginWire);
    CHECK_FALSE(playerLogin.IsValid());

    WorldPacket logoutRequestWire{uint32(CLASSIC_CMSG_LOGOUT_REQUEST)};
    ClassicClient<ClassicLogoutRequest> logoutRequest;
    logoutRequest.ReadFromWorldPacket(logoutRequestWire);
    CHECK_FALSE(logoutRequest.IsValid());
}
