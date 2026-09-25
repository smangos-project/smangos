#ifndef MANGOS_CLASSIC_MISC_H
#define MANGOS_CLASSIC_MISC_H

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"

struct ClassicPing
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PING;

    uint32 Serial = 0;
    uint32 Latency = 0;

    void Write(ByteBuffer& data) const
    {
        data << Serial;
        data << Latency;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 8)
            return false;
        data >> Serial;
        data >> Latency;
        return true;
    }
};

struct ClassicPong
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_PONG;

    uint32 Serial = 0;

    void Write(ByteBuffer& data) const { data << Serial; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4)
            return false;
        data >> Serial;
        return true;
    }
};

// Empty body.
struct ClassicKeepAlive
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_KEEP_ALIVE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct ClassicTimeSyncRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TIME_SYNC_REQUEST;

    uint32 SequenceIndex = 0;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> SequenceIndex;
        return true;
    }

    void Write(ByteBuffer& data) const { data << SequenceIndex; }
};

// Field order: sequenceIndex then clientTime.
struct ClassicTimeSyncResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TIME_SYNC_RESPONSE;

    uint32 SequenceIndex = 0;
    uint32 ClientTime = 0;

    void Write(ByteBuffer& data) const
    {
        data << SequenceIndex;
        data << ClientTime;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 2)
            return false;
        data >> SequenceIndex >> ClientTime;
        return true;
    }
};

// A single IdleLogout bit, flushed to its own byte; always false from a bot.
struct ClassicLogoutRequest
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_LOGOUT_REQUEST;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(false); // idleLogout
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data.ReadBit(); // idleLogout
        data.ResetBitPos();
        return true;
    }
};

// Empty body. Arrives on the Realm connection even when the logout request was sent on the
// Instance one.
struct ClassicLogoutComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGOUT_COMPLETE;

    // May carry a SwitchGameMode bit on the wire; unread here since nothing needs it and every
    // packet is length-framed, so a trailing byte is safe to ignore.
    bool Read(ByteBuffer&) { return true; }
    void Write(ByteBuffer&) const {}
};

#endif
