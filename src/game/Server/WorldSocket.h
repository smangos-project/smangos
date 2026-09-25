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

#ifndef MANGOS_GAME_SERVER_WORLDSOCKET_H
#define MANGOS_GAME_SERVER_WORLDSOCKET_H

#include "IO/Networking/AsyncSocket.h"
#include "Crypto/Encryption/PacketCrypt.h"
#include "ClassicPacketFraming.h"
#include "Protocol/ClassicOpcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "IO/Timer/TimerHandle.h"
#include <array>

class WorldSocketMgr;

// The world socket's wire format: the V2 banner handshake, then every packet framed as
// ClassicPacketHeader + PacketCrypt ciphertext (see ClassicPacketFraming), from the very first
// SMSG_AUTH_CHALLENGE on -- there is no unencrypted vanilla-style header phase. PacketCrypt itself
// is a no-op cipher (framing/counters still run, but the tag is never checked) until CMSG_AUTH_SESSION
// succeeds and CMSG_ENTER_ENCRYPTED_MODE_ACK arms it.
class WorldSocket final : public std::enable_shared_from_this<WorldSocket>
{
    friend WorldSocketMgr;

private:
    enum class HandlerResult
    {
        Okay,
        Fail,
    };

    // Where a connection is in the handshake; gates which opcode is expected next.
    enum class HandshakeStage
    {
        AwaitingAuthSession,
        AwaitingEncryptedModeAck,
        Done,
    };

    /// Whether the listener that accepted this socket was the Realm or Instance one (decision 5);
    /// gates which of CMSG_AUTH_SESSION/CMSG_AUTH_CONTINUED_SESSION is accepted.
    ClassicConnectionType m_connectionType;

    /// Time in which the last ping was received
    std::chrono::system_clock::time_point m_lastPingTime;

    /// Keep track of over-speed pings, to prevent ping flood.
    uint32 m_overSpeedPings;

    /// AES-256-GCM cipher over every packet after the header (see ClassicPacketFraming); a no-op
    /// until CMSG_ENTER_ENCRYPTED_MODE_ACK arms it with the handshake's derived key.
    PacketCrypt m_crypt;

    /// Session to which received packets are routed
    WorldSession* m_Session; // TODO: Rename me to m_session

    HandshakeStage m_stage;

    /// Sent in SMSG_AUTH_CHALLENGE; combined with the client's own LocalChallenge to derive the
    /// session key and encryption key (WorldHandshakeCrypto).
    std::array<uint8, 32> m_serverChallenge;

    /// Derived once CMSG_AUTH_SESSION's digest checks out; arms m_crypt once the client acks
    /// SMSG_ENTER_ENCRYPTED_MODE.
    std::array<uint8, 32> m_encryptKey{};

    /// Starting the recv loop
    void Start();

    void SendServerBanner();
    void ReadClientBanner();
    void SendAuthChallenge();

    /// process one incoming packet.
    void ReadNextHeader();
    void ReadBody(uint32 bodySize, std::array<uint8, 12> tag);

    /// Encrypt and write to queue
    void HandleResultOfAsyncWrite(IO::NetworkError const& error, std::shared_ptr<ByteBuffer> const& alreadyAllocatedBuffer);

    void OnIoError(IO::NetworkError const& error, char const* where);

    HandlerResult HandleCompletePacket(std::vector<uint8> plaintext);

    /// Called on CMSG_AUTH_SESSION.
    HandlerResult HandleAuthSession(ByteBuffer& data);
    HandlerResult FailAuthSession(char const* reason);

    void SendEnterEncryptedMode();
    /// Called on CMSG_ENTER_ENCRYPTED_MODE_ACK.
    HandlerResult HandleEnterEncryptedModeAck(ByteBuffer& data);

    /// Called on CMSG_PING.
    HandlerResult HandlePing(ByteBuffer& data);

    /// Called on CMSG_TIME_SYNC_RESPONSE, pairing it back to the session's own pending request.
    HandlerResult HandleTimeSyncResponse(ByteBuffer& data);

    /// Character-screen opcodes (port-plan.md step 4); parsed here, handled on the session.
    HandlerResult HandleEnumCharacters(ByteBuffer& data);
    HandlerResult HandleCreateCharacter(ByteBuffer& data);
    HandlerResult HandleCharDelete(ByteBuffer& data);
    HandlerResult HandlePlayerLogin(ByteBuffer& data);

    /// Called on CMSG_AUTH_CONTINUED_SESSION (the Instance connection's own handshake): looks the
    /// session up by the packed key CMSG_PLAYER_LOGIN's SMSG_CONNECT_TO handed out, checks the
    /// digest against the Realm connection's existing session key, and attaches this socket as the
    /// session's Instance one.
    HandlerResult HandleAuthContinuedSession(ByteBuffer& data);
    HandlerResult FailAuthContinuedSession(char const* reason);

    /// Called on CMSG_CONNECT_TO_FAILED (Realm connection, port step 5).
    HandlerResult HandleConnectToFailed(ByteBuffer& data);

    /// Called on CMSG_LOGOUT_REQUEST.
    HandlerResult HandleLogoutRequest(ByteBuffer& data);

    void SendRaw(uint32 opcode, ByteBuffer const& body);

    template<typename T>
    void SendClassic(T const& packet)
    {
        ByteBuffer body;
        packet.Write(body);
        SendRaw(uint32(T::Opcode), body);
    }

    // Wraps [opcode, body] in CLASSIC_SMSG_COMPRESSED_PACKET on this connection's own deflate
    // stream. Empty if compression isn't worth it (small body) or fails.
    std::vector<uint8> BuildCompressedPlaintext(uint32 opcode, uint8 const* body, size_t bodySize);

    // The outgoing deflate window CLASSIC_SMSG_COMPRESSED_PACKET packets share for this
    // connection's whole lifetime; only its deflate half is used server-side.
    ClassicPacketFraming m_outboundFraming;

    std::shared_ptr<IO::Timer::TimerHandle> m_handshakeTimeout; // nullptr once the handshake completes, or if the feature is disabled

    std::mutex m_sendQueueLock;
    std::queue<WorldPacket> m_sendQueue;
    std::atomic_flag m_sendQueueIsRunning;

    IO::Networking::AsyncSocket m_socket;
    std::string m_remoteIpAddressStringAfterProxy; // might differ from `m_socket.m_descriptor` if behind proxy

    static std::string const ServerBanner;
    static std::string const ClientBanner;

public:
    explicit WorldSocket(IO::Networking::AsyncSocket socket, ClassicConnectionType connectionType = ClassicConnectionType::Realm);
    /// The destructor will automatically close the socket
    ~WorldSocket();
    WorldSocket(WorldSocket const&) = delete;
    WorldSocket& operator=(WorldSocket const&) = delete;
    WorldSocket(WorldSocket&&) = delete;
    WorldSocket& operator=(WorldSocket&&) = delete;

    void SendPacket(WorldPacket packet);

    void FinalizeSession()
    {
        m_Session = nullptr;
    }

    inline ClassicConnectionType GetConnectionType() const { return m_connectionType; }

    // ----- Exposing `m_socket` features -----
    inline std::string const& GetRemoteIpString() const { return m_remoteIpAddressStringAfterProxy; }
    inline bool IsClosing() const { return m_socket.IsClosing(); }
    void CloseSocket();
};

#endif // MANGOS_GAME_SERVER_WORLDSOCKET_H
