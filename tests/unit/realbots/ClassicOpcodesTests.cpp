#include "ClassicOpcodes.h"

#include <catch_amalgamated.hpp>
#include <cstring>

TEST_CASE("ClassicOpcodeName names known opcodes without the CLASSIC_ prefix", "[classicopcodes]")
{
    CHECK(std::strcmp(ClassicOpcodeName(CLASSIC_CMSG_AUTH_SESSION), "CMSG_AUTH_SESSION") == 0);
    CHECK(std::strcmp(ClassicOpcodeName(CLASSIC_SMSG_LOGIN_VERIFY_WORLD), "SMSG_LOGIN_VERIFY_WORLD") == 0);
    CHECK(std::strcmp(ClassicOpcodeName(CLASSIC_SMSG_ZONE_UNDER_ATTACK), "SMSG_ZONE_UNDER_ATTACK") == 0);
}

TEST_CASE("ClassicOpcodeName answers UNKNOWN for values outside the table", "[classicopcodes]")
{
    CHECK(std::strcmp(ClassicOpcodeName(0), "UNKNOWN") == 0);
    CHECK(std::strcmp(ClassicOpcodeName(0xFFFFFFFFu), "UNKNOWN") == 0);
}

// The vanilla opcode enum lives below 0x10000; the table's own values must never reach into
// that range, so both can share one uint32 opcode space without overlap. The lowest classic
// table starts at 0x2E0000, so nothing below that may be named.
TEST_CASE("No classic opcode overlaps the vanilla opcode range", "[classicopcodes]")
{
    uint32 firstOverlap = 0;
    bool overlaps = false;
    for (uint32 opcode = 0; opcode < 0x2E0000; ++opcode)
    {
        if (std::strcmp(ClassicOpcodeName(opcode), "UNKNOWN") != 0)
        {
            overlaps = true;
            firstOverlap = opcode;
            break;
        }
    }
    INFO("first overlapping opcode: 0x" << std::hex << firstOverlap);
    CHECK_FALSE(overlaps);
}

TEST_CASE("ClassicSendRoute sends handshake and glue-screen SMSGs on Realm", "[classicopcodes]")
{
    CHECK(ClassicSendRoute(CLASSIC_SMSG_AUTH_CHALLENGE) == ClassicConnectionType::Realm);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_ENTER_ENCRYPTED_MODE) == ClassicConnectionType::Realm);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_AUTH_RESPONSE) == ClassicConnectionType::Realm);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_CONNECT_TO) == ClassicConnectionType::Realm);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_ENUM_CHARACTERS_RESULT) == ClassicConnectionType::Realm);
}

TEST_CASE("ClassicSendRoute sends world-state SMSGs on Instance", "[classicopcodes]")
{
    CHECK(ClassicSendRoute(CLASSIC_SMSG_LOGIN_VERIFY_WORLD) == ClassicConnectionType::Instance);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_TIME_SYNC_REQUEST) == ClassicConnectionType::Instance);
    CHECK(ClassicSendRoute(CLASSIC_SMSG_ATTACK_START) == ClassicConnectionType::Instance);
}

TEST_CASE("ClassicSendRoute defaults to Realm for a CMSG or an unknown opcode", "[classicopcodes]")
{
    CHECK(ClassicSendRoute(CLASSIC_CMSG_PLAYER_LOGIN) == ClassicConnectionType::Realm);
    CHECK(ClassicSendRoute(0xFFFFFFFFu) == ClassicConnectionType::Realm);
}
