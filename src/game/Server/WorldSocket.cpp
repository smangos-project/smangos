/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 * Copyright (C) 2017-2024 VMaNGOS Project <https://github.com/vmangos>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "WorldSession.h"

#include "World.h"
#include "AccountMgr.h"
#include "SharedDefines.h"
#include "Protocol/ClassicOpcodes.h"
#include "Protocol/Opcodes.h"
#include "Packets/Auth.h"
#include "Packets/Session.h"
#include "Crypto/Authentication/WorldHandshakeCrypto.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "Database/SqlPreparedStatement.h"
#include "Database/DatabaseEnv.h"
#include "Config/Config.h"
#include "Timer.h"
#include "Util.h"
#include "Errors.h"

#include "IO/Timer/AsyncSystemTimer.h"

#include <algorithm>
#include <cstring>
#include <memory>

namespace
{
// The proof-of-work and session-key challenges SMSG_AUTH_CHALLENGE hands out; strong randomness
// isn't load-bearing for the (unverified) DoS challenge, but the session/encrypt keys derived from
// ServerChallenge are, so both use the same generator.
std::array<uint8, 32> RandomChallenge32()
{
    BigNumber bn;
    bn.SetRand(32 * 8);
    std::vector<uint8> bytes = bn.AsByteArray(32, false);

    std::array<uint8, 32> out{};
    std::copy(bytes.begin(), bytes.end(), out.begin());
    return out;
}

// Below this size, wrapping a packet in CLASSIC_SMSG_COMPRESSED_PACKET costs more than it saves.
constexpr size_t ClassicCompressionThreshold = 0x400;
}

std::string const WorldSocket::ServerBanner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2";
std::string const WorldSocket::ClientBanner = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2";

WorldSocket::WorldSocket(IO::Networking::AsyncSocket socket, ClassicConnectionType connectionType)
    : m_connectionType(connectionType),
      m_socket(std::move(socket)),
      m_lastPingTime(std::chrono::system_clock::time_point::min()),
      m_overSpeedPings(0),
      m_crypt(PacketCryptRole::Server),
      m_Session(nullptr),
      m_stage(HandshakeStage::AwaitingAuthSession),
      m_serverChallenge(RandomChallenge32()),
      m_remoteIpAddressStringAfterProxy(m_socket.GetRemoteIpString())
{
    m_sendQueueIsRunning.clear(); // there is no atomic_flag::constructor on windows to initialize it with false by default (and if left out, linux is uninitialized and will fail randomly)
}

WorldSocket::~WorldSocket()
{
    CloseSocket();
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] Connection closed", GetRemoteIpString().c_str());

    if (m_handshakeTimeout)
    {
        m_handshakeTimeout->Cancel();
    }
}

void WorldSocket::OnIoError(IO::NetworkError const& error, char const* where)
{
    if (error.GetErrorType() != IO::NetworkError::ErrorType::SocketClosed || !IsClosing()) // only print error if it's not "normal close" related
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: IoError while %s: %s", GetRemoteIpString().c_str(), where, error.ToString().c_str());
    CloseSocket();
}

void WorldSocket::SendServerBanner()
{
    auto banner = std::make_shared<ByteBuffer>();
    banner->append((uint8 const*)ServerBanner.c_str(), ServerBanner.length());
    banner->append((uint8 const*)"\n", 1);

    m_socket.Write({ std::shared_ptr<ByteBuffer const>(banner) }, [self = shared_from_this()](IO::NetworkError const& error) -> void
    {
        if (error)
        {
            self->OnIoError(error, "sending the server banner");
            return;
        }

        self->ReadClientBanner();
    });
}

void WorldSocket::ReadClientBanner()
{
    size_t const expectedSize = ClientBanner.length() + 1;
    auto buffer = std::make_shared<std::vector<char>>(expectedSize);

    m_socket.Read(buffer->data(), buffer->size(), [self = shared_from_this(), buffer, expectedSize](IO::NetworkError const& error, std::size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error, "reading the client banner");
            return;
        }

        if (std::memcmp(buffer->data(), ClientBanner.data(), ClientBanner.length()) != 0 || (*buffer)[expectedSize - 1] != '\n')
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: bad client banner", self->GetRemoteIpString().c_str());
            self->CloseSocket();
            return;
        }

        self->SendAuthChallenge();
    });
}

void WorldSocket::SendAuthChallenge()
{
    AuthChallenge challenge;
    challenge.DosChallenge = RandomChallenge32();
    challenge.ServerChallenge = m_serverChallenge;
    challenge.DosZeroBits = 0;
    SendClassic(challenge);

    ReadNextHeader();
}

void WorldSocket::ReadNextHeader()
{
    auto header = std::make_shared<PacketHeader>();

    m_socket.Read(reinterpret_cast<char*>(header.get()), sizeof(PacketHeader), [self = shared_from_this(), header](IO::NetworkError const& error, std::size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error, "reading a packet header");
            return;
        }

        if (!header->IsValidSize())
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: client sent an oversized packet (size = %u)", self->GetRemoteIpString().c_str(), header->Size);
            self->CloseSocket();
            return;
        }

        std::array<uint8, 12> tag;
        std::memcpy(tag.data(), header->Tag, tag.size());
        self->ReadBody(header->Size, tag);
    });
}

void WorldSocket::ReadBody(uint32 bodySize, std::array<uint8, 12> tag)
{
    auto payload = std::make_shared<std::vector<uint8>>(bodySize);

    m_socket.Read(reinterpret_cast<char*>(payload->data()), payload->size(), [self = shared_from_this(), payload, tag](IO::NetworkError const& error, std::size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error, "reading a packet body");
            return;
        }

        AES_GCM::Tag rawTag;
        std::memcpy(rawTag, tag.data(), sizeof(rawTag));
        if (!self->m_crypt.DecryptRecv(payload->data(), payload->size(), rawTag))
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket: failed to decrypt/verify a packet (size %zu)", self->GetRemoteIpString().c_str(), payload->size());
            self->CloseSocket();
            return;
        }

        if (self->HandleCompletePacket(std::move(*payload)) == HandlerResult::Okay && !self->IsClosing())
            self->ReadNextHeader();
    });
}

WorldSocket::HandlerResult WorldSocket::HandleCompletePacket(std::vector<uint8> plaintext)
{
    if (IsClosing())
        return HandlerResult::Fail;

    uint32 opcode;
    uint8 const* bodyStart;
    size_t bodySize;
    if (!PacketFraming::ParsePlaintext(plaintext, opcode, bodyStart, bodySize))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket: packet too small to contain an opcode", GetRemoteIpString().c_str());
        CloseSocket();
        return HandlerResult::Fail;
    }

    ByteBuffer data;
    if (bodySize)
        data.append(bodyStart, bodySize);
    data.rpos(0);

    try
    {
        switch (opcode)
        {
            case CLASSIC_CMSG_AUTH_SESSION:
                if (m_stage != HandshakeStage::AwaitingAuthSession || m_connectionType != ClassicConnectionType::Realm)
                {
                    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: unexpected CMSG_AUTH_SESSION", GetRemoteIpString().c_str());
                    CloseSocket();
                    return HandlerResult::Fail;
                }
                return HandleAuthSession(data);

            case CLASSIC_CMSG_AUTH_CONTINUED_SESSION:
                if (m_stage != HandshakeStage::AwaitingAuthSession || m_connectionType != ClassicConnectionType::Instance)
                {
                    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: unexpected CMSG_AUTH_CONTINUED_SESSION", GetRemoteIpString().c_str());
                    CloseSocket();
                    return HandlerResult::Fail;
                }
                return HandleAuthContinuedSession(data);

            case CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK:
                if (m_stage != HandshakeStage::AwaitingEncryptedModeAck)
                {
                    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: unexpected CMSG_ENTER_ENCRYPTED_MODE_ACK", GetRemoteIpString().c_str());
                    CloseSocket();
                    return HandlerResult::Fail;
                }
                return HandleEnterEncryptedModeAck(data);

            case CLASSIC_CMSG_PING:
                if (m_Session == nullptr)
                {
                    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: CMSG_PING before authentication", GetRemoteIpString().c_str());
                    CloseSocket();
                    return HandlerResult::Fail;
                }
                return HandlePing(data);

            default:
                if (m_Session == nullptr)
                {
                    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: opcode 0x%X sent before authentication", GetRemoteIpString().c_str(), opcode);
                    CloseSocket();
                    return HandlerResult::Fail;
                }

                if (!LookupOpcodeHandler(opcode).impl.has_value())
                {
                    // A real client sends plenty of opcodes nothing here handles yet; logged quietly
                    // and dropped, never fatal to the connection.
                    sLog.Out(LOG_NETWORK, LOG_LVL_DEBUG, "[%s] WorldSocket: unhandled opcode 0x%X (%s)", GetRemoteIpString().c_str(), opcode, ClassicOpcodeName(opcode));
                    return HandlerResult::Okay;
                }

                {
                    auto packet = std::make_unique<WorldPacket>(opcode, bodySize);
                    if (bodySize)
                        packet->append(bodyStart, bodySize);
                    packet->FillPacketTime(WorldTimer::getMSTime());
                    m_Session->QueueBinaryPacket(packet);
                }
                return HandlerResult::Okay;
        }
    }
    catch (ByteBufferException&)
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket: malformed opcode 0x%X body, packet dropped", GetRemoteIpString().c_str(), opcode);
        return HandlerResult::Okay;
    }
}

WorldSocket::HandlerResult WorldSocket::FailAuthSession(char const* reason)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: CMSG_AUTH_SESSION rejected: %s", GetRemoteIpString().c_str(), reason);
    CloseSocket();
    return HandlerResult::Fail;
}

WorldSocket::HandlerResult WorldSocket::HandleAuthSession(ByteBuffer& data)
{
    AuthSession session;
    if (!session.Read(data))
        return FailAuthSession("malformed packet");

    // This server's join ticket (BnetServices.cpp's JoinRealm) is just the account id as a decimal
    // string: there's one game account per Battle.net account, so nothing else needs to travel in
    // it, and the client treats the ticket as opaque either way.
    uint32 ticketAccountId;
    try
    {
        size_t consumed = 0;
        unsigned long long parsed = std::stoull(session.RealmJoinTicket, &consumed);
        if (consumed != session.RealmJoinTicket.size() || parsed > UINT32_MAX)
            return FailAuthSession("realm join ticket is not a plain account id");
        ticketAccountId = uint32(parsed);
    }
    catch (std::exception const&)
    {
        return FailAuthSession("realm join ticket is not a plain account id");
    }

    auto accountQueryResult = LoginDatabase.PQuery(
        "SELECT a.`id`, a.`username`, a.`session_key_bnet`, aa.`gmLevel`, a.`mutetime`, a.`locale`, "
        "a.`flags`, a.`email`, a.`email_verif`, "
        "ab.`unbandate` > UNIX_TIMESTAMP() OR ab.`unbandate` = ab.`bandate` "
        "FROM `account` a "
        "LEFT JOIN `account_access` aa ON a.`id` = aa.`id` AND aa.`RealmID` IN (-1, %u) "
        "LEFT JOIN `account_banned` ab ON a.`id` = ab.`id` AND ab.`active` = 1 "
        "WHERE a.`id` = %u ORDER BY aa.`RealmID` DESC LIMIT 1", realmID, ticketAccountId);
    if (!accountQueryResult)
        return FailAuthSession("realm join ticket names an unknown account");

    Field* fields = accountQueryResult->Fetch();
    uint32 const accountId = fields[0].GetUInt32();
    std::string const username = fields[1].GetCppString();
    std::string const sessionKeyHex = fields[2].GetCppString();
    AccountTypes security = fields[3].GetString() ? AccountTypes(fields[3].GetUInt32()) : SEC_PLAYER;
    if (security > SEC_ADMINISTRATOR) // prevent invalid security settings in DB
        security = SEC_ADMINISTRATOR;
    time_t const mutetime = time_t(fields[4].GetUInt64());
    LocaleConstant locale = LocaleConstant(fields[5].GetUInt8());
    if (locale >= MAX_LOCALE)
        locale = LOCALE_enUS;
    uint32 const accFlags = fields[6].GetUInt32();
    std::string const email = fields[7].GetCppString();
    bool const verifiedEmail = fields[8].GetBool() || email.empty(); // treat no email as verified (created from console)
    bool const isBanned = fields[9].GetBool();

    if (sessionKeyHex.empty() || (sessionKeyHex.size() % 2) != 0)
        return FailAuthSession("account has no world session key (never completed a Battle.net realm join)");

    std::vector<uint8> joinSessionKey(sessionKeyHex.size() / 2);
    HexStrToByteArray(sessionKeyHex, joinSessionKey.data());

    if (isBanned || sAccountMgr.IsIPBanned(GetRemoteIpString()))
        return FailAuthSession("account or IP banned");

    AccountTypes const allowedAccountType = sWorld.GetPlayerSecurityLimit();
    if (allowedAccountType > SEC_PLAYER && security < allowedAccountType)
        return FailAuthSession("server is locked to a higher security level");

    std::array<uint8, 24> const expectedDigest = WorldHandshakeCrypto::ComputeAuthCheckDigest(joinSessionKey, session.LocalChallenge, m_serverChallenge);
    if (expectedDigest != session.Digest)
        return FailAuthSession("auth-check digest mismatch");

    std::array<uint8, 40> const sessionKey = WorldHandshakeCrypto::ComputeSessionKey(joinSessionKey, session.LocalChallenge, m_serverChallenge);
    m_encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(sessionKey, session.LocalChallenge, m_serverChallenge);

    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "WorldSocket::HandleAuthSession: Client '%s' authenticated successfully from %s.",
              username.c_str(), GetRemoteIpString().c_str());

    m_Session = new WorldSession(accountId, this->shared_from_this(), security, mutetime, locale);
    m_Session->SetUsername(username);
    m_Session->SetGameBuild(58797);
    m_Session->SetAccountFlags(accFlags);
    m_Session->SetVerifiedEmail(verifiedEmail);
    m_Session->SetWorldSessionKeyBytes(sessionKey);
    m_Session->LoadGlobalAccountData();
    m_Session->LoadTutorialsData();
    sAccountMgr.UpdateAccountData(accountId, username, email, verifiedEmail, security);

    sWorld.AddSession(m_Session);

    m_stage = HandshakeStage::AwaitingEncryptedModeAck;
    SendEnterEncryptedMode();
    return HandlerResult::Okay;
}

void WorldSocket::SendEnterEncryptedMode()
{
    EnterEncryptedMode enter;
    enter.Enabled = true;

    std::array<uint8, 64> const toSign = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(m_encryptKey, enter.Enabled);

    Ed25519 ed25519;
    if (!ed25519.LoadFromByteArray(WorldHandshakeCrypto::EnterEncryptedModePrivateKey()))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket: failed to load the Ed25519 key", GetRemoteIpString().c_str());
        CloseSocket();
        return;
    }

    std::vector<uint8> const context(WorldHandshakeCrypto::EnableEncryptionContext().begin(), WorldHandshakeCrypto::EnableEncryptionContext().end());
    std::vector<uint8> const signature = ed25519.SignWithContext(toSign, context);
    if (signature.size() != enter.Signature.size())
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket: failed to sign SMSG_ENTER_ENCRYPTED_MODE", GetRemoteIpString().c_str());
        CloseSocket();
        return;
    }
    std::copy(signature.begin(), signature.end(), enter.Signature.begin());

    SendClassic(enter);
}

WorldSocket::HandlerResult WorldSocket::HandleEnterEncryptedModeAck(ByteBuffer& data)
{
    EnterEncryptedModeAck ack;
    if (!ack.Read(data))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: malformed CMSG_ENTER_ENCRYPTED_MODE_ACK", GetRemoteIpString().c_str());
        CloseSocket();
        return HandlerResult::Fail;
    }

    if (m_handshakeTimeout)
    {
        m_handshakeTimeout->Cancel();
        m_handshakeTimeout = nullptr;
    }

    // Every packet from here on (starting with SMSG_AUTH_RESPONSE/SMSG_RESUME_COMMS below) is
    // encrypted; the ack itself was decrypted above while still unarmed, exactly as the client sent it.
    m_crypt.Init(m_encryptKey);
    m_stage = HandshakeStage::Done;

    if (m_connectionType == ClassicConnectionType::Instance)
    {
        SendClassic(ResumeComms{});
        return HandlerResult::Okay;
    }

    SendAuthResponseAndGlueScreen();

    return HandlerResult::Okay;
}

void WorldSocket::SendAuthResponseAndGlueScreen()
{
    std::string realmName;
    if (std::unique_ptr<QueryResult> result = LoginDatabase.PQuery("SELECT `name` FROM `realmlist` WHERE `id` = '%u'", realmID))
        realmName = result->Fetch()[0].GetCppString();

    AuthSuccessInfo info;
    info.RealmAddress = (uint32(1) << 24) | (uint32(1) << 16) | uint16(realmID);
    info.RealmName = realmName;
    info.Time = int64(time(nullptr));
    info.RaceClasses =
    {
        { 1, { 1, 2, 4, 5, 8, 9 } },
        { 2, { 1, 3, 4, 7, 9 } },
        { 3, { 1, 2, 3, 4, 5 } },
        { 4, { 1, 3, 4, 5, 11 } },
        { 5, { 1, 4, 5, 8, 9 } },
        { 6, { 1, 3, 7, 11 } },
        { 7, { 1, 4, 8, 9 } },
        { 8, { 1, 3, 4, 5, 7, 8 } },
    };

    AuthResponse response;
    response.Result = 0; // Ok
    response.SuccessInfoPresent = true;
    ByteBuffer authBody;
    response.Write(authBody);
    info.Write(authBody);
    SendRaw(CLASSIC_SMSG_AUTH_RESPONSE, authBody);

    ByteBuffer timeZone;
    std::string const zone = "Europe/Paris";
    timeZone.WriteBits(uint32(zone.size()), 7);
    timeZone.WriteBits(uint32(zone.size()), 7);
    timeZone.FlushBits();
    timeZone.WriteString(zone);
    timeZone.WriteString(zone);
    SendRaw(CLASSIC_SMSG_SET_TIME_ZONE_INFORMATION, timeZone);

    ByteBuffer features;
    for (int i = 0; i < 20; ++i)
        features.WriteBit(false);   // Bpay store, undelete, commerce, boost, live region and crash reporter flags
    features.WriteBit(true);        // EuropaTicketSystemStatus present
    for (int i = 0; i < 11; ++i)
        features.WriteBit(false);   // NameReservationOnly .. AccountLockedByExport
    features.WriteBits(uint32(0), 11); // RealmHiddenAlert length
    features.WriteBit(true);        // BNSendWhisperUseV2Services
    features.WriteBit(true);        // BNSendGameDataUseV2Services
    features.WriteBit(false);       // CharacterSelectListModeRealmless
    features.FlushBits();
    for (int i = 0; i < 4; ++i)
        features.WriteBit(true);    // TicketsEnabled, BugsEnabled, ComplaintsEnabled, SuggestionsEnabled
    features.FlushBits();
    features << uint32(10) << uint32(60000) << uint32(1) << uint32(111111); // SavedThrottleObjectState
    features << uint32(0);  // TokenPollTimeSeconds
    features << uint32(0);  // KioskSessionMinutes
    features << int64(0);   // TokenBalanceAmount
    features << int32(sWorld.getConfig(CONFIG_UINT32_CHARACTERS_PER_REALM));
    features << uint32(0);  // LiveRegionCharacterCopySourceRegions
    features << uint32(0);  // BpayStoreProductDeliveryDelay
    features << int32(0);   // ActiveBoostType
    features << int32(0);   // TrialBoostType
    features << int32(0);   // MinimumExpansionLevel
    features << int32(0);   // MaximumExpansionLevel
    features << int32(0);   // ContentSetID
    features << uint32(0);  // GameRuleValues
    features << int32(0);   // ActiveTimerunningSeasonID
    features << int32(0);   // RemainingTimerunningSeasonSeconds
    features << int16(50);  // MaxPlayerGuidLookupsPerRequest
    features << int16(600); // NameLookupTelemetryInterval
    features << uint32(10); // NotFoundCacheTimeSeconds
    features << uint32(0);  // DebugTimeEvents
    features << int32(0);   // MostRecentTimeEventID
    features << uint32(0);  // EventRealmQueues
    SendRaw(CLASSIC_SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN, features);

    ByteBuffer cacheVersion;
    cacheVersion << uint32(0);
    SendRaw(CLASSIC_SMSG_CACHE_VERSION, cacheVersion);

    ByteBuffer hotfixes;
    hotfixes << info.RealmAddress;
    hotfixes << int32(0);
    SendRaw(CLASSIC_SMSG_AVAILABLE_HOTFIXES, hotfixes);

    ByteBuffer connectionStatus;
    connectionStatus.WriteBits(uint32(1), 2); // State
    connectionStatus.WriteBit(true);          // SuppressNotification
    connectionStatus.FlushBits();
    SendRaw(CLASSIC_SMSG_BATTLE_NET_CONNECTION_STATUS, connectionStatus);
}

WorldSocket::HandlerResult WorldSocket::HandlePing(ByteBuffer& data)
{
    Ping ping;
    if (!ping.Read(data))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: malformed CMSG_PING, dropped", GetRemoteIpString().c_str());
        return HandlerResult::Okay;
    }

    if (m_lastPingTime == std::chrono::system_clock::time_point::min())
        m_lastPingTime = std::chrono::system_clock::now();              // for 1st ping
    else
    {
        auto now = std::chrono::system_clock::now();
        std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastPingTime);
        m_lastPingTime = now;

        // The modern client's own keepalive cadence is a few seconds, well under the ~30s a
        // vanilla client pinged at; the flood threshold has to follow it or every real session
        // trips it within a minute of being in the world.
        if (seconds.count() < 3)
        {
            ++m_overSpeedPings;

            uint32 maxAllowedOverspeedPings = sWorld.getConfig(CONFIG_UINT32_MAX_OVERSPEED_PINGS);
            if (maxAllowedOverspeedPings && m_overSpeedPings > maxAllowedOverspeedPings)
            {
                if (m_Session && m_Session->GetSecurity() == SEC_PLAYER)
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "WorldSocket::HandlePing: Player kicked for overspeeded pings address = %s", GetRemoteIpString().c_str());
                    CloseSocket();
                    return HandlerResult::Fail;
                }
            }
        }
        else
        {
            m_overSpeedPings = 0;
        }
    }

    m_Session->SetLatency(ping.Latency);

    Pong pong;
    pong.Serial = ping.Serial;
    SendClassic(pong);
    return HandlerResult::Okay;
}

WorldSocket::HandlerResult WorldSocket::FailAuthContinuedSession(char const* reason)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] WorldSocket: CMSG_AUTH_CONTINUED_SESSION rejected: %s", GetRemoteIpString().c_str(), reason);
    CloseSocket();
    return HandlerResult::Fail;
}

WorldSocket::HandlerResult WorldSocket::HandleAuthContinuedSession(ByteBuffer& data)
{
    AuthContinuedSession packet;
    if (!packet.Read(data))
        return FailAuthContinuedSession("malformed packet");

    ConnectToKey const key = ConnectToKey::Unpack(packet.Key);
    if (key.ConnectionType != uint8(ClassicConnectionType::Instance))
        return FailAuthContinuedSession("key is not for the Instance connection");

    WorldSession* session = WorldSession::FindPendingInstanceConnect(packet.Key);
    if (!session || session->GetAccountId() != key.AccountId || session->GetInstanceSocket() != nullptr)
        return FailAuthContinuedSession("no pending Instance connect for that key");

    std::array<uint8, 40> const& sessionKey = session->GetWorldSessionKeyBytes();
    std::array<uint8, 24> const expectedDigest = WorldHandshakeCrypto::ComputeContinuedSessionDigest(sessionKey, packet.Key, packet.LocalChallenge, m_serverChallenge);
    if (expectedDigest != packet.Digest)
        return FailAuthContinuedSession("auth-check digest mismatch");

    m_encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(sessionKey, packet.LocalChallenge, m_serverChallenge);

    m_Session = session;
    m_Session->SetInstanceSocket(shared_from_this());

    m_stage = HandshakeStage::AwaitingEncryptedModeAck;
    SendEnterEncryptedMode();
    return HandlerResult::Okay;
}

void WorldSocket::SendRaw(uint32 opcode, ByteBuffer const& body)
{
    WorldPacket packet(opcode, body.size());
    if (!body.empty())
        packet.append(body.contents(), body.size());
    SendPacket(std::move(packet));
}

std::vector<uint8> WorldSocket::BuildCompressedPlaintext(uint32 opcode, uint8 const* body, size_t bodySize)
{
    if (bodySize <= ClassicCompressionThreshold || opcode == CLASSIC_SMSG_COMPRESSED_PACKET)
        return {};

    std::vector<uint8> const inner = PacketFraming::BuildPlaintext(opcode, body, bodySize);
    std::vector<uint8> compressed;
    if (!m_outboundFraming.Deflate(inner.data(), inner.size(), compressed))
        return {};

    CompressedPacketHeader const header = PacketFraming::BuildCompressedHeader(
        inner.data(), inner.size(), compressed.data(), compressed.size());

    std::vector<uint8> outerBody(sizeof(header) + compressed.size());
    std::memcpy(outerBody.data(), &header, sizeof(header));
    std::memcpy(outerBody.data() + sizeof(header), compressed.data(), compressed.size());

    return PacketFraming::BuildPlaintext(CLASSIC_SMSG_COMPRESSED_PACKET, outerBody.data(), outerBody.size());
}

void WorldSocket::SendPacket(WorldPacket packet)
{
    if (IsClosing())
        return;

    // We don't want to allocate or encrypt anything inside the world thread, so we move everything to the IO thread.
    m_sendQueueLock.lock();
    if (m_sendQueue.size() > 1024) // There should never be so many packets queued up. The socket is probably not responding.
    {
        m_sendQueueLock.unlock();
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] Send queue is full. Disconnecting.", GetRemoteIpString().c_str());
        CloseSocket();
        return;
    }
    m_sendQueue.push(std::move(packet));
    // The flag must be set under the same lock that guards the queue: HandleResultOfAsyncWrite
    // only clears it while holding the lock with an empty queue, so our push is either seen by
    // the running loop or we observe the cleared flag and start a new one. Otherwise a packet
    // pushed between its empty-check and clear would strand in the queue until the next send.
    bool const alreadyRunning = m_sendQueueIsRunning.test_and_set();
    m_sendQueueLock.unlock();

    // Start AsyncProcessingSendQueue which take things from the queue
    if (alreadyRunning)
        return; // already running

    m_socket.EnterIoContext([self = shared_from_this()](IO::NetworkError error)
    {
        self->HandleResultOfAsyncWrite(error, std::make_shared<ByteBuffer>());
    });
}

void WorldSocket::HandleResultOfAsyncWrite(IO::NetworkError const& error, std::shared_ptr<ByteBuffer> const& alreadyAllocatedBuffer)
{
    if (error)
    {
        if (error.GetErrorType() != IO::NetworkError::ErrorType::SocketClosed || !IsClosing()) // only print error if it's not "normal close" related
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] WorldSocket::HandleResultOfAsyncWrite: IoError: %s", GetRemoteIpString().c_str(), error.ToString().c_str());
            CloseSocket(); // This call to CloseSocket is actually necessary for once, so that others can see that this socket is not usable anymore
        }

        m_sendQueueIsRunning.clear();
        return;
    }

    m_sendQueueLock.lock();
    if (m_sendQueue.empty())
    {
        // Must be cleared while holding the queue lock, see comment in SendPacket().
        m_sendQueueIsRunning.clear();
        m_sendQueueLock.unlock();
        return;
    }
    m_sendQueueLock.unlock();

    // Combine all packets into `alreadyAllocatedBuffer`, each with its own header/tag: PacketCrypt
    // needs exactly one EncryptSend() call per packet to keep its counter in lockstep with the
    // client's, so packets can be batched into one write() but never share one cipher call.
    alreadyAllocatedBuffer->clear();
    while (!m_sendQueue.empty())
    {
        m_sendQueueLock.lock();
        if (m_sendQueue.empty()) // re-check after we locked the queue if it's really not empty
        {
            m_sendQueueLock.unlock();
            break;
        }
        WorldPacket packet = std::move(m_sendQueue.front());
        m_sendQueue.pop();
        m_sendQueueLock.unlock();

        uint8 const* body = packet.empty() ? nullptr : packet.contents();
        std::vector<uint8> plaintext = BuildCompressedPlaintext(packet.GetOpcode(), body, packet.size());
        if (plaintext.empty())
            plaintext = PacketFraming::BuildPlaintext(packet.GetOpcode(), body, packet.size());

        PacketHeader header{};
        header.Size = uint32(plaintext.size());

        AES_GCM::Tag tag;
        m_crypt.EncryptSend(plaintext.data(), plaintext.size(), tag);
        std::memcpy(header.Tag, tag, sizeof(header.Tag));

        alreadyAllocatedBuffer->append((uint8 const*)&header, sizeof(header));
        if (!plaintext.empty())
            alreadyAllocatedBuffer->append(plaintext.data(), plaintext.size());
    }

    m_socket.Write({ alreadyAllocatedBuffer /* dont move, re-used in lambda */ }, [self = shared_from_this(), alreadyAllocatedBuffer](IO::NetworkError const& error)
    {
        self->HandleResultOfAsyncWrite(error, alreadyAllocatedBuffer);
    });
}

void WorldSocket::Start()
{
    // Start the handshake timeout: a connection that hasn't finished the whole V2 handshake (up
    // through CMSG_ENTER_ENCRYPTED_MODE_ACK) within this bound is closed (bot-phase-3.md step C.2 --
    // neither reference has one).
    if (int secs = sConfig.GetIntDefault("Network.TimeoutSecsIfNoAuth", 10))
    {
        m_handshakeTimeout = sAsyncSystemTimer.ScheduleFunctionOnce(std::chrono::seconds(secs), [this]()
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_DETAIL, "[%s] Connection did not finish the world handshake in time. Closing socket...", this->GetRemoteIpString().c_str());
            // It's correct that we capture _this_ and not a shared_ptr, since the timer will be canceled in destructor
            this->CloseSocket();
        });
    }

    SendServerBanner();
}

void WorldSocket::CloseSocket()
{
    m_socket.CloseSocket();
}
