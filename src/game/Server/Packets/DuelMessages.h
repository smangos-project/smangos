#ifndef MANGOS_PACKETS_DUELMESSAGES_H
#define MANGOS_PACKETS_DUELMESSAGES_H

// The modern duel wire: challenge lookup, the challenge itself, its answer, and the countdown,
// boundary and result messages. Light like Packets/DeathMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <string>

constexpr uint32 CLASSIC_MAX_DUEL_NAME_LENGTH = 63;

// Asks whether a player may be challenged before the client offers the option.
struct CanDuel
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CAN_DUEL;

    ClassicGuid Target;
    bool ToTheDeath = false;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        data.WriteBit(ToTheDeath);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        ToTheDeath = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// The client's answer to a challenge: accept, decline, or forfeit a duel under way.
struct DuelResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_DUEL_RESPONSE;

    ClassicGuid Arbiter;
    bool Accepted = false;
    bool Forfeited = false;

    void Write(ByteBuffer& data) const
    {
        Arbiter.Write(data);
        data.WriteBit(Accepted);
        data.WriteBit(Forfeited);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Arbiter.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Accepted = data.ReadBit();
        Forfeited = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

struct CanDuelResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CAN_DUEL_RESULT;

    ClassicGuid Target;
    bool Result = false;

    void Write(ByteBuffer& data) const
    {
        Target.Write(data);
        data.WriteBit(Result);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Target.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Result = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// A challenge, sent to both duellists. The arbiter is the flag object between them.
struct DuelRequested
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_REQUESTED;

    ClassicGuid Arbiter;
    ClassicGuid RequestedBy;
    ClassicGuid RequestedByAccount;
    bool ToTheDeath = false;

    void Write(ByteBuffer& data) const
    {
        Arbiter.Write(data);
        RequestedBy.Write(data);
        RequestedByAccount.Write(data);
        data.WriteBit(ToTheDeath);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!Arbiter.Read(data) || !RequestedBy.Read(data) || !RequestedByAccount.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        ToTheDeath = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Seconds before the duel begins, in milliseconds on the wire.
struct DuelCountdown
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_COUNTDOWN;

    uint32 Countdown = 0;

    void Write(ByteBuffer& data) const { data << Countdown; }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Countdown;
        return true;
    }
};

// A duellist left the arena radius.
struct DuelOutOfBounds
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_OUT_OF_BOUNDS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// A duellist came back inside the arena radius.
struct DuelInBounds
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_IN_BOUNDS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// The duel ended. `Started` is false when it was called off before the countdown finished.
struct DuelCompleteNotice
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_COMPLETE;

    bool Started = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(Started);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Started = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Who won. The beaten player's name comes first on the wire.
struct DuelWinner
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DUEL_WINNER;

    std::string BeatenName;
    std::string WinnerName;
    uint32 BeatenVirtualRealmAddress = 0;
    uint32 WinnerVirtualRealmAddress = 0;
    bool Fled = false;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(BeatenName.length()), 6);
        data.WriteBits(uint32(WinnerName.length()), 6);
        data.WriteBit(Fled);
        data.FlushBits();
        data << BeatenVirtualRealmAddress;
        data << WinnerVirtualRealmAddress;
        data.WriteString(BeatenName);
        data.WriteString(WinnerName);
    }

    bool Read(ByteBuffer& data)
    {
        // 13 bits, flushed to two bytes, then the two realm addresses
        if (data.size() - data.rpos() < 2 + 2 * sizeof(uint32))
            return false;
        uint32 const beatenLength = data.ReadBits(6);
        uint32 const winnerLength = data.ReadBits(6);
        Fled = data.ReadBit();
        data.ResetBitPos();
        data >> BeatenVirtualRealmAddress >> WinnerVirtualRealmAddress;
        if (data.size() - data.rpos() < beatenLength + winnerLength)
            return false;
        BeatenName = data.ReadString(beatenLength);
        WinnerName = data.ReadString(winnerLength);
        return true;
    }
};

#endif
