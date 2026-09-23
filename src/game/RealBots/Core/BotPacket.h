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

// A packet in flight, opcode plus body, independent of any concrete Classic* type.
// Protocol/ClassicPackets attaches meaning to Opcode once it exists.
struct BotPacket
{
    uint32 Opcode = 0;
    ByteBuffer Data;
};

#endif
