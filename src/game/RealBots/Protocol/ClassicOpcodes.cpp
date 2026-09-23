#include "ClassicOpcodes.h"

// Name lookup for the packet log (bot-phase-1.md, "Packet log") -- covers every opcode this
// framework currently knows about; anything else falls back to its hex value.
char const* ClassicOpcodeName(uint32 opcode)
{
    switch (opcode)
    {
        case CLASSIC_CMSG_AUTH_SESSION: return "CMSG_AUTH_SESSION";
        case CLASSIC_CMSG_AUTH_CONTINUED_SESSION: return "CMSG_AUTH_CONTINUED_SESSION";
        case CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK: return "CMSG_ENTER_ENCRYPTED_MODE_ACK";
        case CLASSIC_CMSG_PING: return "CMSG_PING";
        case CLASSIC_CMSG_LOG_DISCONNECT: return "CMSG_LOG_DISCONNECT";
        case CLASSIC_CMSG_KEEP_ALIVE: return "CMSG_KEEP_ALIVE";
        case CLASSIC_CMSG_ENUM_CHARACTERS: return "CMSG_ENUM_CHARACTERS";
        case CLASSIC_CMSG_PLAYER_LOGIN: return "CMSG_PLAYER_LOGIN";
        case CLASSIC_CMSG_CREATE_CHARACTER: return "CMSG_CREATE_CHARACTER";
        case CLASSIC_CMSG_CONNECT_TO_FAILED: return "CMSG_CONNECT_TO_FAILED";
        case CLASSIC_CMSG_LOGOUT_REQUEST: return "CMSG_LOGOUT_REQUEST";
        case CLASSIC_CMSG_TIME_SYNC_RESPONSE: return "CMSG_TIME_SYNC_RESPONSE";

        case CLASSIC_SMSG_AUTH_CHALLENGE: return "SMSG_AUTH_CHALLENGE";
        case CLASSIC_SMSG_ENTER_ENCRYPTED_MODE: return "SMSG_ENTER_ENCRYPTED_MODE";
        case CLASSIC_SMSG_RESUME_COMMS: return "SMSG_RESUME_COMMS";
        case CLASSIC_SMSG_CONNECT_TO: return "SMSG_CONNECT_TO";
        case CLASSIC_SMSG_AUTH_RESPONSE: return "SMSG_AUTH_RESPONSE";
        case CLASSIC_SMSG_PONG: return "SMSG_PONG";
        case CLASSIC_SMSG_COMPRESSED_PACKET: return "SMSG_COMPRESSED_PACKET";
        case CLASSIC_SMSG_ENUM_CHARACTERS_RESULT: return "SMSG_ENUM_CHARACTERS_RESULT";
        case CLASSIC_SMSG_LOGIN_VERIFY_WORLD: return "SMSG_LOGIN_VERIFY_WORLD";
        case CLASSIC_SMSG_CREATE_CHAR: return "SMSG_CREATE_CHAR";
        case CLASSIC_SMSG_LOGOUT_COMPLETE: return "SMSG_LOGOUT_COMPLETE";
        case CLASSIC_SMSG_TIME_SYNC_REQUEST: return "SMSG_TIME_SYNC_REQUEST";

        default: return "UNKNOWN";
    }
}
