#ifndef REALBOTS_BOTWORLDHANDSHAKE_H
#define REALBOTS_BOTWORLDHANDSHAKE_H

#include "BotWorldConnection.h"
#include "IO/Context/IoContext.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Drives BotWorldConnection through bot-phase-1.md's WorldConnecting/WorldHandshake stages:
// connect -> V2 banner (BotWorldConnection::StartHandshake) -> SMSG_AUTH_CHALLENGE ->
// CMSG_AUTH_SESSION -> SMSG_ENTER_ENCRYPTED_MODE (signature optionally verified) ->
// CMSG_ENTER_ENCRYPTED_MODE_ACK -> encryption armed -> SMSG_AUTH_RESPONSE. Takes exactly what
// BotBnetLogin::Result hands back (world address/port, joinSessionKey, RealmJoinTicket).
//
// Not yet a BotRoutine (see BotBnetLogin's header comment for why); on success the caller gets
// back the still-open, now-encrypted BotWorldConnection to keep driving (CharSelect next).
class BotWorldHandshake : public std::enable_shared_from_this<BotWorldHandshake>
{
    public:
        struct Options
        {
            std::string WorldAddress;
            uint16_t WorldPort = 0;
            std::vector<uint8> SessionKey; // joinSessionKey: BotBnetLogin::Result::SessionKey
            std::string RealmJoinTicket;
            uint32_t RealmId = 0;
            // vibemangos's _HandleAuthSession never checks these two; sent for wire-shape parity.
            uint32_t RegionId = 1;
            uint32_t BattlegroupId = 1;
            bool VerifyServerSignature = true;
        };

        struct Result
        {
            bool Success = false;
            std::shared_ptr<BotWorldConnection> Connection;
            uint32 AuthResult = 0; // BattlenetRpcErrorCode from SMSG_AUTH_RESPONSE; 0 == Ok
            // The 40-byte world session key CMSG_AUTH_SESSION established -- CharSelect needs it
            // again if the server redirects to a second ("Instance") connection via
            // SMSG_CONNECT_TO (BotInstanceConnect's CMSG_AUTH_CONTINUED_SESSION reuses it rather
            // than doing a fresh key exchange).
            std::array<uint8, 40> SessionKey{};
            std::string Error;
        };
        using HandshakeCallback = std::function<void(Result)>;

        static std::shared_ptr<BotWorldHandshake> Create(IO::IoContext* ioContext);

        void Handshake(Options options, HandshakeCallback callback);

    private:
        enum class Step
        {
            WaitingAuthChallenge,
            WaitingEnterEncryptedMode,
            WaitingAuthResponse,
            Done,
        };

        explicit BotWorldHandshake(IO::IoContext* ioContext);

        void Connect();
        void OnPacket(BotPacket packet);
        void OnDisconnect();

        void HandleAuthChallenge(BotPacket& packet);
        void HandleEnterEncryptedMode(BotPacket& packet);
        void HandleAuthResponse(BotPacket& packet);

        void Fail(std::string const& error);
        void Succeed(uint32 authResult);

        IO::IoContext* m_ioContext;
        Options m_options;
        HandshakeCallback m_callback;

        std::shared_ptr<BotWorldConnection> m_connection;
        Step m_step = Step::WaitingAuthChallenge;

        std::array<uint8, 32> m_localChallenge{};
        std::array<uint8, 32> m_serverChallenge{};
        std::array<uint8, 40> m_sessionKey{};
        std::array<uint8, 32> m_encryptKey{};
};

#endif
