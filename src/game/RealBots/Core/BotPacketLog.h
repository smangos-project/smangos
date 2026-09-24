#ifndef REALBOTS_BOTPACKETLOG_H
#define REALBOTS_BOTPACKETLOG_H

#include "BotPacket.h"
#include "ByteBuffer.h"
#include <fstream>
#include <string>

// Writes one bot's traffic to its own file under RealBots.PacketLogDir when RealBots.PacketLog is
// enabled -- the main debugging tool while bringing the protocol up (bot-phase-1.md, "Talking to
// the server" -> "Packet log"). Not thread-safe by itself: a RealBot only ever touches its own
// log from its own worker thread, same as every other piece of per-bot state.
class BotPacketLog
{
    public:
        BotPacketLog(std::string const& dir, std::string const& botName);

        void Write(uint32 timeMs, BotPacketDir dir, BotConnectionKind connection, uint32 opcode, ByteBuffer const& data, bool compressed = false);

    private:
        std::ofstream m_file;
};

#endif
