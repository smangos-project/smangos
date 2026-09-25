#ifndef MANGOS_PACKETS_SESSION_H
#define MANGOS_PACKETS_SESSION_H

// Connection-housekeeping wire structs (ping/keepalive/time-sync/logout) for the world socket.
// Kept out of Packets/Misc.h deliberately: that header pulls in ObjectGuid.h and the rest of the
// WorldPackets machinery, which RealBots must not depend on (see RealBots/README.md) -- these
// structs stay as light as Packets/Auth.h.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"

struct Ping
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

struct Pong
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
struct KeepAlive
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_KEEP_ALIVE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct TimeSyncRequest
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
struct TimeSyncResponse
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

// A single IdleLogout bit, flushed to its own byte; always false from a bot. Keeps the Classic
// prefix: WorldSession::LogoutRequest(time_t) is a live, unrelated vanilla method, and the bare
// name would collide with it wherever this struct and WorldSession are both in scope.
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

// A LogoutResult (0 == Ok) then an Instant bit, flushed to its own byte.
struct LogoutResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGOUT_RESPONSE;

    int32 Reason = 0;
    bool Instant = false;

    void Write(ByteBuffer& data) const
    {
        data << Reason;
        data.WriteBit(Instant);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32) + 1)
            return false;
        data >> Reason;
        Instant = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Empty body. Arrives on the Realm connection even when the logout request was sent on the
// Instance one.
struct LogoutComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGOUT_COMPLETE;

    // May carry a SwitchGameMode bit on the wire; unread here since nothing needs it and every
    // packet is length-framed, so a trailing byte is safe to ignore.
    bool Read(ByteBuffer&) { return true; }
    void Write(ByteBuffer&) const {}
};

#endif
