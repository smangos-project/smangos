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
// ("Realm") connection stays open for what still routes through it (SMSG_LOGOUT_COMPLETE, and the
// answer to anything sent on it: a ping's pong comes back on the socket it went out on). Only AddressType 1 (IPv4) is parsed -- the only value either reference
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

// SMSG_CONNECT_TO carries the attempt it belongs to, and CMSG_CONNECT_TO_FAILED echoes the one
// that failed.
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1 = 17;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2 = 35;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3 = 53;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4 = 71;
constexpr uint32 CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5 = 89;

// The escalation is a failure of attempt N asking for attempt N+1, up to five, then the login is
// aborted. Sent on the Realm connection. Field order (connection type, then serial) is unconfirmed
// against a real 1.15.6 client; see the README's "vibemangos vs TrinityCore" section.
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

// ResponseCodes (vibemangos: src/game/SharedDefines.h; the values are the client's own, the same in
// TrinityCore): the character-screen results a create, a delete and a name check answer with.
constexpr uint8 CLASSIC_CHAR_CREATE_SUCCESS = 24;
constexpr uint8 CLASSIC_CHAR_CREATE_ERROR = 25;
constexpr uint8 CLASSIC_CHAR_CREATE_FAILED = 26;
constexpr uint8 CLASSIC_CHAR_CREATE_NAME_IN_USE = 27;
constexpr uint8 CLASSIC_CHAR_CREATE_SERVER_LIMIT = 30;  // this realm holds no more characters
constexpr uint8 CLASSIC_CHAR_CREATE_ACCOUNT_LIMIT = 31; // this account holds no more
constexpr uint8 CLASSIC_CHAR_DELETE_SUCCESS = 65;
constexpr uint8 CLASSIC_CHAR_DELETE_FAILED = 66;
// Name validation results: CHAR_NAME_FAILURE .. CHAR_NAME_SPACES_DISALLOWED (a create with a bad name
// is answered with one of these in place of a CHAR_CREATE_* code).
constexpr uint8 CLASSIC_CHAR_NAME_SUCCESS = 92;
constexpr uint8 CLASSIC_CHAR_NAME_FAILURE = 93;
constexpr uint8 CLASSIC_CHAR_NAME_NO_NAME = 94;
constexpr uint8 CLASSIC_CHAR_NAME_TOO_SHORT = 95;
constexpr uint8 CLASSIC_CHAR_NAME_TOO_LONG = 96;
constexpr uint8 CLASSIC_CHAR_NAME_INVALID_CHARACTER = 97;
constexpr uint8 CLASSIC_CHAR_NAME_SPACES_DISALLOWED = 109;

inline bool ClassicIsCharNameResult(uint8 result)
{
    return result >= CLASSIC_CHAR_NAME_FAILURE && result <= CLASSIC_CHAR_NAME_SPACES_DISALLOWED;
}

// vibemangos: src/game/Handlers/CharacterHandler.cpp HandleCharEnumOpcode (NullClientPacket --
// empty body).
struct ClassicEnumCharacters
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENUM_CHARACTERS;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

// One entry of SMSG_ENUM_CHARACTERS_RESULT: the fields a bot or scenario wants of a character, out of
// the fixed-layout CharacterInfoBasic block (everything after them -- equipment visuals, tabard,
// professions, ... -- is parsed past and dropped).
struct ClassicCharacterInfo
{
    ClassicGuid Guid;
    uint8 ListPosition = 0;
    uint8 Race = 0;
    uint8 Gender = 0;
    uint8 Class = 0;
    uint8 Level = 0;
    int32 Map = 0;
    int32 Zone = 0;
    float X = 0.0f, Y = 0.0f, Z = 0.0f;
    uint32 CharFlags = 0;
    bool FirstLogin = false;
    std::string Name;
};

// vibemangos: src/game/Server/Packets/Character.cpp EnumCharactersResult::AppendBodyTo(), which
// follows TrinityCore cata_classic's EnumCharactersResult::Write() (the two agree on this layout).
// Read parses every character's fixed-layout block into Characters when the body matches it exactly
// (no mail senders, which would add variable-length data after a character), so scenarios can see
// names and guids. When it does not -- a server with a different layout, or a body cut short --
// Read still succeeds with what CharSelect needs, the count and the first guid, and leaves
// Characters empty: the rest of the body (race-unlock list, ...) is real wire shape but not needed,
// and leaving it unread is safe, every packet being length-framed (the same reasoning
// ClassicAuthResponse::Read above relies on).
struct ClassicEnumCharactersResult
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ENUM_CHARACTERS_RESULT;
    static constexpr uint32 EQUIPMENT_SLOTS = 19; // vibemangos: CHAR_ENUM_EQUIPMENT_SLOTS

    uint32 CharacterCount = 0;
    ClassicGuid FirstCharacterGuid;
    std::vector<ClassicCharacterInfo> Characters; // empty unless every character parsed

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

        Characters.clear();
        if (CharacterCount == 0)
            return true;

        // The full per-character parse if the layout matches; else the lenient subset.
        size_t const listStart = data.rpos();
        if (ReadCharacters(data))
        {
            FirstCharacterGuid = Characters[0].Guid;
            return true;
        }
        Characters.clear();
        data.rpos(listStart);
        return FirstCharacterGuid.Read(data);
    }

    // With Characters set: the whole fixed layout for each (unknown fields zero, no mail); without:
    // only the parsed subset (Success set, CharacterCount, the seven other header fields zeroed, the
    // first guid).
    void Write(ByteBuffer& data) const
    {
        for (int i = 0; i < 9; ++i)
            data.WriteBit(i == 0);
        data.FlushBits();

        data << (Characters.empty() ? CharacterCount : uint32(Characters.size()));
        for (int i = 0; i < 7; ++i)
            data << uint32(0);

        if (Characters.empty())
        {
            if (CharacterCount > 0)
                FirstCharacterGuid.Write(data);
            return;
        }

        for (ClassicCharacterInfo const& c : Characters)
        {
            c.Guid.Write(data);
            data << uint32(0);                 // VirtualRealmAddress
            data << c.ListPosition << c.Race << c.Gender << c.Class;
            data << int16(0);                  // SpecID
            data << uint32(0);                 // customization count
            data << c.Level;
            data << c.Map << c.Zone << c.X << c.Y << c.Z;
            data << uint64(0);                 // GuildClubMemberID
            ClassicGuid().Write(data);         // guild guid
            data << c.CharFlags << uint32(0) << uint32(0); // CharFlags, Flags2, Flags3
            data << uint8(0);                  // unkWod61x
            data << uint32(0) << uint32(0) << uint32(0); // pet display, level, family
            for (uint32 slot = 0; slot < EQUIPMENT_SLOTS; ++slot)
            {
                data << uint32(0) << uint8(0) << uint32(0) << uint8(0) << int32(0) << uint32(0) << uint32(0);
            }
            data << int32(0);                  // Unknown703
            data << uint64(0);                 // LastPlayedTime
            data << int32(0);                  // LastLoginVersion
            for (int i = 0; i < 5; ++i)
                data << int32(-1);             // PersonalTabard
            data << uint32(0) << uint32(0);    // ProfessionIds
            data << int32(0);                  // TimerunningSeasonID
            data << uint32(0);                 // OverrideSelectScreenFileDataID
            data.WriteBits(uint32(c.Name.size()), 6);
            data.WriteBit(c.FirstLogin);
            data.FlushBits();
            data.append(reinterpret_cast<uint8 const*>(c.Name.data()), c.Name.size());
            data.WriteBit(false);              // BoostInProgress
            data.WriteBit(false);              // RpeResetAvailable
            data.WriteBit(false);              // RpeResetQuestClearAvailable
            data.FlushBits();
            data << uint32(0) << uint32(0) << uint32(0); // Flags4, mail senders, mail sender types
        }
    }

    private:
        // Parses CharacterCount characters starting at the current position. False on any shortfall
        // or on a mail-sender count (variable data this does not walk); the caller rewinds.
        bool ReadCharacters(ByteBuffer& data)
        {
            auto const need = [&](size_t bytes) { return data.size() - data.rpos() >= bytes; };
            size_t const equipmentBytes = EQUIPMENT_SLOTS * (4 + 1 + 4 + 1 + 4 + 4 + 4);

            for (uint32 i = 0; i < CharacterCount; ++i)
            {
                ClassicCharacterInfo c;
                if (!c.Guid.Read(data))
                    return false;

                // VirtualRealmAddress(4) ListPosition..Class(4) SpecID(2) customizationCount(4) Level(1)
                // Map(4) Zone(4) X/Y/Z(12) GuildClubMemberID(8)
                if (!need(4 + 4 + 2 + 4 + 1 + 4 + 4 + 12 + 8))
                    return false;
                uint32 virtualRealm; data >> virtualRealm;
                data >> c.ListPosition >> c.Race >> c.Gender >> c.Class;
                int16 spec; data >> spec;
                uint32 customizationCount; data >> customizationCount;
                data >> c.Level >> c.Map >> c.Zone >> c.X >> c.Y >> c.Z;
                uint64 guildClubMember; data >> guildClubMember;

                ClassicGuid guild;
                if (!guild.Read(data))
                    return false;

                // CharFlags, Flags2, Flags3 (12), unkWod61x (1), pet display/level/family (12),
                // the equipment block, Unknown703 (4), LastPlayedTime (8), LastLoginVersion (4),
                // PersonalTabard (20), ProfessionIds (8), TimerunningSeasonID (4),
                // OverrideSelectScreenFileDataID (4)
                if (!need(12 + 1 + 12 + equipmentBytes + 4 + 8 + 4 + 20 + 8 + 4 + 4))
                    return false;
                uint32 flags2, flags3; data >> c.CharFlags >> flags2 >> flags3;
                data.rpos(data.rpos() + 1 + 12 + equipmentBytes + 4 + 8 + 4 + 20 + 8 + 4 + 4);

                if (customizationCount > 1024 || !need(size_t(customizationCount) * 8 + 1))
                    return false;
                data.rpos(data.rpos() + size_t(customizationCount) * 8);

                uint32 nameLength = data.ReadBits(6);
                c.FirstLogin = data.ReadBit();
                data.ResetBitPos();
                if (!need(nameLength + 1 + 12))
                    return false;
                c.Name = data.ReadString(nameLength);

                // Restriction bits (one byte), Flags4, MailSenders.Count, MailSenderTypes.Count.
                data.ReadBit(); data.ReadBit(); data.ReadBit();
                data.ResetBitPos();
                uint32 flags4, mailSenders, mailSenderTypes;
                data >> flags4 >> mailSenders >> mailSenderTypes;
                if (mailSenders != 0 || mailSenderTypes != 0)
                    return false;

                Characters.push_back(std::move(c));
            }
            return true;
        }
};

// vibemangos: src/game/Server/Packets/Character.cpp CharDelete::ReadFromWorldPacket() -- a
// packed-guid128 and nothing else (jimsproxy's CharDelete.Read()).
struct ClassicCharDelete
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_CHAR_DELETE;

    ClassicGuid Guid;

    void Write(ByteBuffer& data) const { Guid.Write(data); }
    bool Read(ByteBuffer& data) { return Guid.Read(data); }
};

// vibemangos: src/game/Server/Packets/Character.cpp CharDeleteResponse::AppendBodyTo() -- one
// ResponseCodes byte (CLASSIC_CHAR_DELETE_SUCCESS on success).
struct ClassicDeleteCharResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_DELETE_CHAR;

    uint8 Result = 0;

    void Write(ByteBuffer& data) const { data << Result; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Result;
        return true;
    }
};

// vibemangos: src/game/Server/Packets/Character.cpp CharacterLoginFailed::AppendBodyTo() -- one
// LoginFailureReason byte (1 is NoWorld, which vibemangos also sends when the Instance connect fails
// five times).
struct ClassicCharacterLoginFailed
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_CHARACTER_LOGIN_FAILED;

    uint8 Reason = 0;

    void Write(ByteBuffer& data) const { data << Reason; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        data >> Reason;
        return true;
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
