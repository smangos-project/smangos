#ifndef MANGOS_CLASSIC_AUTH_H
#define MANGOS_CLASSIC_AUTH_H

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include <array>
#include <string>

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

    void Write(ByteBuffer& data) const
    {
        data.append(DosChallenge.data(), DosChallenge.size());
        data.append(ServerChallenge.data(), ServerChallenge.size());
        data << DosZeroBits;
    }
};

struct ClassicAuthSession
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTH_SESSION;

    // Proof-of-work response to AuthChallenge's DosChallenge/DosZeroBits. Not validated, and no
    // proof-of-work is computed by the sender.
    uint64 DosResponse = 0;
    uint32 RegionId = 1;
    uint32 BattlegroupId = 1;
    uint32 RealmId = 0;
    std::array<uint8, 32> LocalChallenge{};
    // Truncated (24-byte) auth-check digest -- see WorldHandshakeCrypto::ComputeAuthCheckDigest.
    std::array<uint8, 24> Digest{};
    bool UseIPv6 = false;
    std::string RealmJoinTicket; // opaque: whatever the login server issued as Param_RealmJoinTicket

    void Write(ByteBuffer& data) const
    {
        data << DosResponse;
        data << RegionId;
        data << BattlegroupId;
        data << RealmId;
        data.append(LocalChallenge.data(), LocalChallenge.size());
        data.append(Digest.data(), Digest.size());
        // one bit, flushed to its own byte
        data << uint8(UseIPv6 ? 0x80 : 0x00);
        data << uint32(RealmJoinTicket.size());
        data.WriteString(RealmJoinTicket);
    }

    bool Read(ByteBuffer& data)
    {
        size_t const fixedSize = sizeof(DosResponse) + sizeof(RegionId) + sizeof(BattlegroupId) + sizeof(RealmId)
            + LocalChallenge.size() + Digest.size() + sizeof(uint8) + sizeof(uint32);
        if (data.size() - data.rpos() < fixedSize)
            return false;

        data >> DosResponse >> RegionId >> BattlegroupId >> RealmId;
        data.read(LocalChallenge.data(), LocalChallenge.size());
        data.read(Digest.data(), Digest.size());
        uint8 ipv6Byte = 0;
        data >> ipv6Byte;
        UseIPv6 = (ipv6Byte & 0x80) != 0;

        uint32 ticketSize = 0;
        data >> ticketSize;
        if (data.size() - data.rpos() < ticketSize)
            return false;
        RealmJoinTicket = data.ReadString(ticketSize);
        return true;
    }
};

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

    void Write(ByteBuffer& data) const
    {
        data.append(Signature.data(), Signature.size());
        data << uint8(Enabled ? 0x80 : 0x00); // one bit, flushed to its own byte
    }
};

struct ClassicEnterEncryptedModeAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// Sent right after CMSG_PLAYER_LOGIN, redirecting the client to open a second ("Instance") world
// connection for SMSG_LOGIN_VERIFY_WORLD and all further gameplay traffic. The original ("Realm")
// connection stays open for what still routes through it (SMSG_LOGOUT_COMPLETE, and the answer to
// anything sent on it: a pong comes back on the socket its ping went out on). Only address type 1
// (IPv4) is parsed. The RSA signature isn't verified on read: RsaSign has no verify half.
struct ClassicConnectTo
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CONNECT_TO;
    static constexpr uint8 ADDRESS_TYPE_IPV4 = 1;

    std::array<uint8, 256> Signature{};
    std::array<uint8, 4> AddressIPv4{};
    uint16 Port = 0;
    uint32 Serial = 0;
    uint8 ConnectionType = 0;
    uint64 Key = 0;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < Signature.size() + 1)
            return false;
        data.read(Signature.data(), Signature.size());

        uint8 addressType = 0;
        data >> addressType;
        if (addressType != ADDRESS_TYPE_IPV4)
            return false;

        if (data.size() - data.rpos() < AddressIPv4.size() + sizeof(Port) + sizeof(Serial) + sizeof(ConnectionType) + sizeof(Key))
            return false;
        data.read(AddressIPv4.data(), AddressIPv4.size());
        data >> Port;
        data >> Serial;
        data >> ConnectionType;
        data >> Key;
        return true;
    }

    void Write(ByteBuffer& data) const
    {
        data.append(Signature.data(), Signature.size());
        data << uint8(ADDRESS_TYPE_IPV4);
        data.append(AddressIPv4.data(), AddressIPv4.size());
        data << Port;
        data << Serial;
        data << ConnectionType;
        data << Key;
    }
};

// SMSG_CONNECT_TO carries the attempt it belongs to, and CMSG_CONNECT_TO_FAILED echoes the one
// that failed.
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1 = 17;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2 = 35;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3 = 53;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4 = 71;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5 = 89;

// The escalation is a failure of attempt N asking for attempt N+1, up to five, then the login is
// aborted. Sent on the Realm connection. Field order (connection type, then serial) is unconfirmed
// against a real 1.15.6 client.
struct ClassicConnectToFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CONNECT_TO_FAILED;

    uint8 Connection = 0;  // the ClassicConnectTo::ConnectionType of the attempt that failed
    uint32 Serial = 0;     // the ClassicConnectTo::Serial of that attempt

    void Write(ByteBuffer& data) const
    {
        data << Connection;
        data << Serial;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(Connection) + sizeof(Serial))
            return false;
        data >> Connection >> Serial;
        return true;
    }
};

// Sent on the Instance connection in place of CMSG_AUTH_SESSION: it reuses the world session key
// the Realm connection's CMSG_AUTH_SESSION already established rather than a fresh exchange (see
// WorldHandshakeCrypto::ComputeContinuedSessionDigest).
struct ClassicAuthContinuedSession
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTH_CONTINUED_SESSION;

    uint64 DosResponse = 0; // see ClassicAuthSession::DosResponse
    uint64 Key = 0; // ClassicConnectTo::Key, echoed back to identify the pending Instance connection
    std::array<uint8, 32> LocalChallenge{};
    std::array<uint8, 24> Digest{};

    void Write(ByteBuffer& data) const
    {
        data << DosResponse;
        data << Key;
        data.append(LocalChallenge.data(), LocalChallenge.size());
        data.append(Digest.data(), Digest.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(DosResponse) + sizeof(Key) + LocalChallenge.size() + Digest.size())
            return false;
        data >> DosResponse >> Key;
        data.read(LocalChallenge.data(), LocalChallenge.size());
        data.read(Digest.data(), Digest.size());
        return true;
    }
};

// Empty body. Sent on the Instance connection right after CMSG_ENTER_ENCRYPTED_MODE_ACK, in place
// of SMSG_AUTH_RESPONSE (which only ever goes out on the Realm connection).
struct ClassicResumeComms
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_RESUME_COMMS;

    bool Read(ByteBuffer&) { return true; }
    void Write(ByteBuffer&) const {}
};

// Only Result and whether SuccessInfo was sent are parsed; the rest of the body (virtual realm
// list, character templates, ...) is left unread, which is safe: every packet is length-framed,
// not delimited by how much of it gets consumed.
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

    // Only the parsed subset: Result, then the two presence bits. A real server sends SuccessInfo
    // right after when its bit is set; Read ignores it either way, so a Write with the bit set
    // and nothing following still round-trips, but is not a valid real-server packet.
    void Write(ByteBuffer& data) const
    {
        data << Result;
        data.WriteBit(SuccessInfoPresent);
        data.WriteBit(WaitInfoPresent);
        data.FlushBits();
    }
};

#endif
