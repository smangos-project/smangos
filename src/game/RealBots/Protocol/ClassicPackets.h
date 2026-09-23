#ifndef REALBOTS_CLASSICPACKETS_H
#define REALBOTS_CLASSICPACKETS_H

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include <array>
#include <string>
#include <vector>

// Handshake packets for BotWorldConnection's WorldConnecting/WorldHandshake stages
// (bot-phase-1.md, "Login"). Each cites its vibemangos source (src/game/Server/WorldSocket.cpp);
// there is no separate TrinityCore cata_classic checkout in this environment for these, so
// vibemangos's own citations to it (see WorldSocket.cpp comments) stand in.
//
// vibemangos: src/game/Server/WorldSocket.cpp ReadClientBanner() (SMSG_AUTH_CHALLENGE send)
struct ClassicAuthChallenge
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUTH_CHALLENGE;

    std::array<uint8, 32> DosChallenge{};
    std::array<uint8, 32> ServerChallenge{};
    uint8 DosZeroBits = 0;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < DosChallenge.size() + ServerChallenge.size() + 1)
            return false;
        data.read(DosChallenge.data(), DosChallenge.size());
        data.read(ServerChallenge.data(), ServerChallenge.size());
        data >> DosZeroBits;
        return true;
    }
};

// vibemangos: src/game/Server/WorldSocket.cpp _HandleAuthSession() (read order mirrored here)
struct ClassicAuthSession
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTH_SESSION;

    // Anti-DOS proof-of-work response to AuthChallenge's DosChallenge/DosZeroBits. vibemangos's
    // _HandleAuthSession() reads this field but never validates it (only the auth-check digest
    // below is checked), so a placeholder is enough to reach InWorld against it; a real client's
    // proof-of-work algorithm is not implemented here.
    uint64 DosResponse = 0;
    uint32 RegionId = 1;
    uint32 BattlegroupId = 1;
    uint32 RealmId = 0;
    std::array<uint8, 32> LocalChallenge{};
    // Truncated (24-byte) auth-check digest -- see WorldHandshakeCrypto::ComputeAuthCheckDigest.
    std::array<uint8, 24> Digest{};
    bool UseIPv6 = false;
    std::string RealmJoinTicket; // this server's join ticket is the account id as a decimal string

    void Write(ByteBuffer& data) const
    {
        data << DosResponse;
        data << RegionId;
        data << BattlegroupId;
        data << RealmId;
        data.append(LocalChallenge.data(), LocalChallenge.size());
        data.append(Digest.data(), Digest.size());
        // Single WriteBit()+FlushBits() on the wire this mirrors is exactly one byte with only
        // the top bit meaningful -- equivalent to and simpler than reproducing the bit-stream call.
        data << uint8(UseIPv6 ? 0x80 : 0x00);
        data << uint32(RealmJoinTicket.size());
        data.WriteString(RealmJoinTicket);
    }
};

// vibemangos: src/game/Server/WorldSocket.cpp SendEnterEncryptedMode()
struct ClassicEnterEncryptedMode
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENTER_ENCRYPTED_MODE;

    std::array<uint8, 64> Signature{};
    bool Enabled = false;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < Signature.size() + 1)
            return false;
        data.read(Signature.data(), Signature.size());
        uint8 enabledByte = 0;
        data >> enabledByte;
        Enabled = (enabledByte & 0x80) != 0;
        return true;
    }
};

// vibemangos: src/game/Server/WorldSocket.cpp _HandleEnterEncryptedModeAck() (empty body)
struct ClassicEnterEncryptedModeAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK;

    void Write(ByteBuffer&) const {}
};

// vibemangos: src/game/Server/WorldSocket.cpp SendAuthResponse(). Only the fields bot-phase-1.md's
// WorldHandshake stage needs are parsed (Result, whether SuccessInfo was sent); the rest of the
// body (virtual realm list, character templates, ...) is left unread, which is safe -- every
// packet is length-framed by BotWorldConnection, not delimited by how much of it gets consumed.
struct ClassicAuthResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_AUTH_RESPONSE;

    uint32 Result = 0; // BattlenetRpcErrorCode; 0 == Ok
    bool SuccessInfoPresent = false;
    bool WaitInfoPresent = false;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        data >> Result;
        SuccessInfoPresent = data.ReadBit();
        WaitInfoPresent = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// bot-examples.md #1
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

#endif
