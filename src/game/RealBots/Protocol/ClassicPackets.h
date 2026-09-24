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

    void Write(ByteBuffer& data) const
    {
        data.append(DosChallenge.data(), DosChallenge.size());
        data.append(ServerChallenge.data(), ServerChallenge.size());
        data << DosZeroBits;
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

    void Write(ByteBuffer& data) const
    {
        data.append(Signature.data(), Signature.size());
        data << uint8(Enabled ? 0x80 : 0x00); // one bit, flushed to its own byte
    }
};

// vibemangos: src/game/Server/WorldSocket.cpp _HandleEnterEncryptedModeAck() (empty body)
struct ClassicEnterEncryptedModeAck
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// vibemangos: src/game/Server/WorldSocket.cpp SendConnectToInstance(). Sent right after
// CMSG_PLAYER_LOGIN, unconditionally -- confirmed identical in TrinityCore cata_classic's
// WorldSession::HandlePlayerLoginOpcode -- redirecting the client to open a second ("Instance")
// world connection for SMSG_LOGIN_VERIFY_WORLD and all further gameplay traffic; the original
// ("Realm") connection stays open for what still routes through it (SMSG_PONG,
// SMSG_LOGOUT_COMPLETE). Only AddressType 1 (IPv4) is parsed -- the only value either reference
// server ever sends. The RSA signature isn't verified: that only matters to a real client
// deciding whether to trust the redirect, and RsaSign here has no verify half, only sign.
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

// ConnectToSerial values (vibemangos: src/game/Server/WorldSocketConnectionType.h; identical in
// TrinityCore cata_classic's Packets/AuthenticationPackets.h): SMSG_CONNECT_TO carries the attempt it
// belongs to, and CMSG_CONNECT_TO_FAILED echoes the one that failed.
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1 = 17;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2 = 35;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3 = 53;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4 = 71;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5 = 89;

// vibemangos: src/game/Server/Packets/Character.cpp ConnectToFailed::ReadFromWorldPacket() (serial,
// then connection type) and WorldSession::HandleConnectToFailedOpcode(); TrinityCore cata_classic:
// AuthenticationPackets.cpp ConnectToFailed::Read() and WorldSocket::HandleConnectToFailed(). The
// escalation is identical (a failure of attempt N asks for attempt N+1, up to five, then the login is
// aborted), but the two DISAGREE on the field order: vibemangos reads Serial (u32) then Con (u8),
// TrinityCore reads Con (u8) then Serial (u32). This follows vibemangos: it is the server the bots
// are proven against first and the tree this is ported into; TrinityCore's may be a 4.4.x-vs-1.15
// difference or a bug in either. Check against a captured real 1.15.6 client before trusting it
// (bot-phase-2.md, open question "real-client ground truth"). Sent on the Realm connection.
struct ClassicConnectToFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CONNECT_TO_FAILED;

    uint32 Serial = 0;     // the ClassicConnectTo::Serial of the attempt that failed
    uint8 Connection = 0;  // the ClassicConnectTo::ConnectionType of that attempt

    void Write(ByteBuffer& data) const
    {
        data << Serial;
        data << Connection;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(Serial) + sizeof(Connection))
            return false;
        data >> Serial >> Connection;
        return true;
    }
};

// vibemangos: src/game/Server/WorldSocket.cpp _HandleAuthContinuedSession() (read order mirrored
// here). Sent on the Instance connection in place of CMSG_AUTH_SESSION: it reuses the world
// session key the Realm connection's CMSG_AUTH_SESSION already established rather than a fresh
// SRP6/join-session-key exchange (see WorldHandshakeCrypto::ComputeContinuedSessionDigest).
struct ClassicAuthContinuedSession
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_AUTH_CONTINUED_SESSION;

    uint64 DosResponse = 0; // see ClassicAuthSession::DosResponse -- read but never validated
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

// vibemangos: src/game/Server/WorldSocket.cpp SendResumeComms() (empty body) -- sent on the
// Instance connection right after CMSG_ENTER_ENCRYPTED_MODE_ACK, in place of SMSG_AUTH_RESPONSE
// (which only ever goes out on the Realm connection).
struct ClassicResumeComms
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_RESUME_COMMS;

    bool Read(ByteBuffer&) { return true; }
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

// CharSelect/EnteringWorld packets (bot-phase-1.md, "Login" steps 5-6). Each cites its
// vibemangos source (src/game/Server/Packets/Character.h/.cpp).

// Packed-guid128: the wire format every modern packet uses for an ObjectGuid -- a two-byte
// (lowMask, highMask) header, then only the non-zero bytes each mask marks (vibemangos:
// ObjectGuid::AppendPackedGuid128/ReadPackedGuid128, ObjectGuid.cpp). The bot has no ObjectGuid
// class of its own (bot-plan.md: touches no server runtime state) -- this is the raw 128-bit
// value, Low/High named exactly as the wire calls them.
struct ClassicGuid
{
    uint64 Low = 0;
    uint64 High = 0;

    bool IsEmpty() const { return Low == 0 && High == 0; }

    void Write(ByteBuffer& data) const
    {
        uint8 lowBytes[8], highBytes[8];
        uint8 lowMask = 0, highMask = 0;
        uint8 loSize = 0, hiSize = 0;
        for (uint8 i = 0; i < 8; ++i)
        {
            uint8 b = uint8(Low >> (i * 8));
            if (b)
            {
                lowMask |= uint8(1 << i);
                lowBytes[loSize++] = b;
            }
        }
        for (uint8 i = 0; i < 8; ++i)
        {
            uint8 b = uint8(High >> (i * 8));
            if (b)
            {
                highMask |= uint8(1 << i);
                highBytes[hiSize++] = b;
            }
        }

        data << lowMask << highMask;
        for (uint8 i = 0; i < loSize; ++i)
            data << lowBytes[i];
        for (uint8 i = 0; i < hiSize; ++i)
            data << highBytes[i];
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2)
            return false;
        uint8 lowMask, highMask;
        data >> lowMask >> highMask;

        Low = 0;
        High = 0;
        for (uint8 i = 0; i < 8; ++i)
            if (lowMask & uint8(1 << i))
            {
                if (data.size() - data.rpos() < 1)
                    return false;
                uint8 b;
                data >> b;
                Low |= uint64(b) << (i * 8);
            }
        for (uint8 i = 0; i < 8; ++i)
            if (highMask & uint8(1 << i))
            {
                if (data.size() - data.rpos() < 1)
                    return false;
                uint8 b;
                data >> b;
                High |= uint64(b) << (i * 8);
            }
        return true;
    }
};

// ResponseCodes (vibemangos: src/game/SharedDefines.h) -- only the one value CharSelect needs.
constexpr uint8 CLASSIC_CHAR_CREATE_SUCCESS = 24;

// vibemangos: src/game/Handlers/CharacterHandler.cpp HandleCharEnumOpcode (NullClientPacket --
// empty body).
struct ClassicEnumCharacters
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENUM_CHARACTERS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// vibemangos: src/game/Server/Packets/Character.cpp EnumCharactersResult::AppendBodyTo(). Only
// enough is parsed to know whether at least one character exists and, if so, its guid -- the rest
// of the body (race/class/appearance/mail data/race-unlock list/...) is real, reverse-engineered
// wire shape (see that file's own extensive citations) but not needed to drive CharSelect, and
// leaving it unread is safe: every packet is length-framed, not delimited by how much of it gets
// consumed (the same reasoning ClassicAuthResponse::Read above already relies on).
struct ClassicEnumCharactersResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENUM_CHARACTERS_RESULT;

    uint32 CharacterCount = 0;
    ClassicGuid FirstCharacterGuid;

    bool Read(ByteBuffer& data)
    {
        // 9 header bits (Success, Realmless, IsDeletedCharacters, IsNewPlayerRestrictionSkipped,
        // IsNewPlayerRestricted, IsNewPlayer, IsTrialAccountRestricted,
        // DisabledClassesMask.HasValue, DontCreateCharacterDisplays), flushed to a byte.
        for (int i = 0; i < 9; ++i)
            data.ReadBit();
        data.ResetBitPos();

        // characters.size(), then 7 more uint32/int32 fields before the per-character list
        // starts (RegionwideCharacters.Count, MaxCharacterLevel, RaceUnlockData.Count,
        // UnlockedConditionalAppearances.Count, RaceLimitDisables.Count, WarbandGroups.Count,
        // DisabledClassesMask value) -- see EnumCharactersResult::AppendBodyTo's own field order.
        if (data.size() - data.rpos() < sizeof(uint32) * 8)
            return false;
        data >> CharacterCount;
        for (int i = 0; i < 7; ++i)
        {
            uint32 skip;
            data >> skip;
        }

        if (CharacterCount > 0)
            return FirstCharacterGuid.Read(data);
        return true;
    }

    // Only the parsed subset (Success set, CharacterCount, the seven skipped fields zeroed, the
    // first guid): a real server's per-character data (name, appearance, ...) follows the guid,
    // and Read ignores it.
    void Write(ByteBuffer& data) const
    {
        for (int i = 0; i < 9; ++i)
            data.WriteBit(i == 0);
        data.FlushBits();

        data << CharacterCount;
        for (int i = 0; i < 7; ++i)
            data << uint32(0);

        if (CharacterCount > 0)
            FirstCharacterGuid.Write(data);
    }
};

// vibemangos: src/game/Server/Packets/Character.cpp CharCreate::ReadFromWorldPacket(). The bot
// sends no ChrCustomization choices (CustomizationCount = 0); the server's
// ConvertModernCustomizationsToLegacy() maps that to skin/face/hairStyle/hairColor/facialHair all
// 0 -- a valid, if plain, default appearance, which is all Phase 1 needs (a character that
// exists).
struct ClassicCreateCharacter
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CREATE_CHARACTER;

    std::string Name;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Gender = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(uint32(Name.size()), 6);
        data.WriteBit(false); // hasTemplateSet
        data.WriteBit(false); // IsTrialBoost
        data.WriteBit(false); // UseNPE
        data.WriteBit(false); // HardcoreSelfFound
        data.FlushBits();

        data << Race;
        data << Class;
        data << Gender;
        data << uint32(0); // customizationCount
        data << int32(0);  // timerunningSeasonId

        data.append(reinterpret_cast<uint8 const*>(Name.data()), Name.size());
        // No templateSetId (hasTemplateSet is false) and no customization pairs (count is 0).
    }

    // Reads what Write produces; a body with a template set or customization pairs is rejected
    // rather than skipped, since this side never produces one.
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2) // 6 + 4 bits, flushed to two bytes
            return false;

        uint32 nameLength = data.ReadBits(6);
        bool hasTemplateSet = data.ReadBit();
        data.ReadBit(); // IsTrialBoost
        data.ReadBit(); // UseNPE
        data.ReadBit(); // HardcoreSelfFound
        data.ResetBitPos();
        if (hasTemplateSet)
            return false;

        if (data.size() - data.rpos() < sizeof(Race) + sizeof(Class) + sizeof(Gender) + sizeof(uint32) + sizeof(int32) + nameLength)
            return false;

        uint32 customizationCount = 0;
        int32 timerunningSeasonId = 0;
        data >> Race >> Class >> Gender >> customizationCount >> timerunningSeasonId;
        if (customizationCount != 0)
            return false;

        Name = data.ReadString(nameLength);
        return true;
    }
};

// vibemangos: src/game/Server/Packets/Character.cpp CharCreateResponse::AppendBodyTo().
struct ClassicCreateCharResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CREATE_CHAR;

    uint8 Result = 0; // ResponseCodes; CLASSIC_CHAR_CREATE_SUCCESS on success
    ClassicGuid Guid; // the new character's guid on success, empty otherwise

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        return Guid.Read(data);
    }

    void Write(ByteBuffer& data) const
    {
        data << Result;
        Guid.Write(data);
    }
};

// vibemangos: src/game/Server/Packets/Character.cpp PlayerLogin::ReadFromWorldPacket().
struct ClassicPlayerLogin
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_PLAYER_LOGIN;

    ClassicGuid Guid;
    // Terrain visibility distance the client requests; read but not applied to anything
    // server-side yet (vibemangos's own comment on PlayerLogin::farClip).
    float FarClip = 0.0f;

    void Write(ByteBuffer& data) const
    {
        Guid.Write(data);
        data << FarClip;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Guid.Read(data) || data.size() - data.rpos() < sizeof(float))
            return false;
        data >> FarClip;
        return true;
    }
};

// vibemangos: src/game/Server/Packets/Character.cpp LoginVerifyWorld::AppendBodyTo(). Reaching
// this is bot-phase-1.md's InWorld marker; SMSG_UPDATE_OBJECT itself is out of scope for Phase 1
// ("parsing SMSG_UPDATE_OBJECT" is explicitly excluded) and is received and skipped.
struct ClassicLoginVerifyWorld
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGIN_VERIFY_WORLD;

    uint32 MapId = 0;
    float X = 0.0f, Y = 0.0f, Z = 0.0f, O = 0.0f;
    uint32 Reason = 0;

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) * 2 + sizeof(float) * 4)
            return false;
        data >> MapId >> X >> Y >> Z >> O >> Reason;
        return true;
    }

    void Write(ByteBuffer& data) const { data << MapId << X << Y << Z << O << Reason; }
};

// Keepalive/session-liveness packets a bot's default handling drives on its own, independent of
// whatever task is on top of the stack (bot-phase-1.md, "Default handling").

// vibemangos: src/game/Handlers/MiscHandler.cpp -- CLASSIC_CMSG_KEEP_ALIVE is bound to
// Handle_NULL (cata's Handle_EarlyProccess): the body is never parsed server-side, so an empty
// one (what a real client also sends -- there is nothing to say) is enough.
struct ClassicKeepAlive
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_KEEP_ALIVE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// vibemangos: src/game/Server/Packets/Misc.cpp TimeSyncRequest::AppendBodyTo().
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

// vibemangos: src/game/Server/Packets/Misc.cpp TimeSyncResponse::ReadFromWorldPacket() (field
// order: sequenceIndex then clientTime).
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

// vibemangos: src/game/Server/Packets/Misc.cpp LogoutRequest::ReadFromWorldPacket() -- a single
// IdleLogout bit, flushed to its own byte; the bot is never idle-logging-out, so it's always false.
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

// vibemangos: src/game/Server/Packets/Misc.cpp LogoutComplete::AppendBodyTo() (empty body).
// Routed PacketRoute::Realm (Opcodes.cpp) -- arrives on the Realm connection even when the
// logout request itself was sent on the Instance one.
struct ClassicLogoutComplete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_LOGOUT_COMPLETE;

    bool Read(ByteBuffer&) { return true; }
    void Write(ByteBuffer&) const {}
};

#endif
