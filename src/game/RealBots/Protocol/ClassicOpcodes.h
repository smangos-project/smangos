#ifndef REALBOTS_CLASSICOPCODES_H
#define REALBOTS_CLASSICOPCODES_H

#include "Common.h"

// Modern (cata-classic-era, 1.15.x) opcodes the world-socket handshake needs. Values verified
// against vibemangos's src/game/Server/Protocol/Opcodes_1_15_6.h -- one shared table for the
// server's handler dispatch and the bots, per port-plan.md ("Build everything for both server
// and client"). Extended as later stages (CharSelect, world entry, ...) need more opcodes.
enum ClassicOpcodes : uint32
{
    CLASSIC_CMSG_AUTH_SESSION              = 0x3A0001,
    CLASSIC_CMSG_AUTH_CONTINUED_SESSION    = 0x3A0002,
    CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK  = 0x3A0003,
    CLASSIC_CMSG_PING                      = 0x3A0004,
    CLASSIC_CMSG_LOG_DISCONNECT            = 0x3A0005,
    CLASSIC_CMSG_KEEP_ALIVE                = 0x3900AB,
    CLASSIC_CMSG_ENUM_CHARACTERS           = 0x390014,
    CLASSIC_CMSG_PLAYER_LOGIN              = 0x390016,
    CLASSIC_CMSG_CREATE_CHARACTER          = 0x39006E,
    CLASSIC_CMSG_CHAR_DELETE               = 0x3900CA,
    CLASSIC_CMSG_CONNECT_TO_FAILED         = 0x390000,
    CLASSIC_CMSG_LOGOUT_REQUEST            = 0x35006A,
    CLASSIC_CMSG_TIME_SYNC_RESPONSE        = 0x37005A,

    CLASSIC_SMSG_AUTH_CHALLENGE            = 0x420000,
    CLASSIC_SMSG_ENTER_ENCRYPTED_MODE      = 0x420001,
    CLASSIC_SMSG_RESUME_COMMS              = 0x420003,
    CLASSIC_SMSG_CONNECT_TO                = 0x420005,
    CLASSIC_SMSG_AUTH_RESPONSE             = 0x3B0001,
    CLASSIC_SMSG_PONG                      = 0x420006,
    CLASSIC_SMSG_COMPRESSED_PACKET         = 0x42000A,
    CLASSIC_SMSG_ENUM_CHARACTERS_RESULT    = 0x3B0018,
    CLASSIC_SMSG_LOGIN_VERIFY_WORLD        = 0x3B0030,
    CLASSIC_SMSG_CREATE_CHAR               = 0x3B019D,
    CLASSIC_SMSG_DELETE_CHAR               = 0x3B019E,
    CLASSIC_SMSG_CHARACTER_LOGIN_FAILED    = 0x3B01A1,
    CLASSIC_SMSG_LOGOUT_COMPLETE           = 0x3B0120,
    CLASSIC_SMSG_TIME_SYNC_REQUEST         = 0x4C0000,

    // The glue-screen packets a server sends on the Realm connection right after
    // SMSG_AUTH_RESPONSE, before the client reaches the character screen. Values confirmed live
    // against vibemangos (opcode only; bodies aren't parsed -- see the README's "vibemangos vs
    // TrinityCore" section).
    CLASSIC_SMSG_SET_TIME_ZONE_INFORMATION         = 0x3B0113,
    CLASSIC_SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN = 0x3B0059,
    CLASSIC_SMSG_CACHE_VERSION                     = 0x3F000E,
    CLASSIC_SMSG_AVAILABLE_HOTFIXES                = 0x3F0001,
    CLASSIC_SMSG_BATTLE_NET_CONNECTION_STATUS      = 0x3B02A2,
};

// Name for the packet log (bot-phase-1.md, "Packet log"); "UNKNOWN" (plus the opcode's own hex
// value, printed alongside it by the caller) for anything not listed above.
char const* ClassicOpcodeName(uint32 opcode);

#endif
