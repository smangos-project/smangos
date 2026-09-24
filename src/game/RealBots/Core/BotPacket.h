#ifndef REALBOTS_BOTPACKET_H
#define REALBOTS_BOTPACKET_H

#include "Common.h"
#include "ByteBuffer.h"

// Which way a packet is moving through a bot's task hooks.
enum class BotPacketDir
{
    Incoming,
    Outgoing,
};

// What a task's OnPacket hook wants done with a packet it was shown.
enum class BotPacketAction
{
    Continue,   // let it keep moving: to the next task down, then to default handling
    Handled,    // stop here; default handling does not run
    Drop,       // stop here, and (for Outgoing) never put it on the wire
};

// The two world connections a bot can hold (bot-phase-1.md, "Login"): the Realm connection the
// handshake and character screen run on, and the Instance connection SMSG_CONNECT_TO redirects the
// world session to. Most SMSGs route to one of them by opcode; the client sends on Instance once it
// exists (see ClassicSendRoute).
enum class BotConnectionKind
{
    Realm,
    Instance,
};

// A packet in flight, opcode plus body, independent of any concrete Classic* type.
// Protocol/ClassicPackets attaches meaning to Opcode once it exists.
struct BotPacket
{
    uint32 Opcode = 0;
    ByteBuffer Data;
    // Incoming: the connection it arrived on. Outgoing: the connection it is bound for (what
    // hooks see in OnPacket before it is written).
    BotConnectionKind Connection = BotConnectionKind::Realm;
};

#endif
