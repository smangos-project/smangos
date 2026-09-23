#ifndef REALBOTS_BOTINSTANCECONNECT_H
#define REALBOTS_BOTINSTANCECONNECT_H

#include "BotWorldConnection.h"
#include "IO/Context/IoContext.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

// Drives the second ("Instance") world connection CLASSIC_SMSG_CONNECT_TO redirects the client
// to (bot-phase-1.md's Login step 6, "EnteringWorld -> InWorld"; see BotCharSelect's own comment
// on why this exists -- confirmed mandatory, not a vibemangos quirk, by TrinityCore cata_classic's
// WorldSession::HandlePlayerLoginOpcode doing the exact same unconditional redirect). Same
// V2-banner/AUTH_CHALLENGE/ENTER_ENCRYPTED_MODE dance as BotWorldHandshake, but
// CMSG_AUTH_CONTINUED_SESSION replaces CMSG_AUTH_SESSION (reusing the Realm connection's
// already-established session key instead of a fresh join-session-key exchange) and
// SMSG_RESUME_COMMS replaces SMSG_AUTH_RESPONSE at the end (vibemangos's
// _HandleEnterEncryptedModeAck: SMSG_AUTH_RESPONSE only ever goes out on the Realm connection).
//
// No CMSG_CONNECT_TO_FAILED retry yet (bot-phase-1.md's minimal first-contact scope): a failed
// Instance connect just fails the whole CharSelect/Login chain.
// TODO(port): the up-to-5-attempt retry vibemangos's/TrinityCore's
// WorldSession::HandleConnectToFailedOpcode implement.
class BotInstanceConnect : public std::enable_shared_from_this<BotInstanceConnect>
{
    public:
        struct Options
        {
            std::string WorldAddress;
            uint16_t WorldPort = 0;
            std::array<uint8, 40> SessionKey{}; // BotWorldHandshake::Result::SessionKey
            uint64_t PackedKey = 0; // ClassicConnectTo::Key
            bool VerifyServerSignature = true;
        };

        struct Result
        {
            bool Success = false;
            std::shared_ptr<BotWorldConnection> Connection;
            std::string Error;
        };
        using ConnectCallback = std::function<void(Result)>;

        static std::shared_ptr<BotInstanceConnect> Create(IO::IoContext* ioContext);

        void Connect(Options options, ConnectCallback callback);

    private:
        enum class Step
        {
            WaitingAuthChallenge,
            WaitingEnterEncryptedMode,
            WaitingResumeComms,
            Done,
        };

        explicit BotInstanceConnect(IO::IoContext* ioContext);

        void OpenSocket();
        void OnPacket(BotPacket packet);
        void OnDisconnect();

        void HandleAuthChallenge(BotPacket& packet);
        void HandleEnterEncryptedMode(BotPacket& packet);
        void HandleResumeComms(BotPacket& packet);

        void Fail(std::string const& error);
        void Succeed();

        IO::IoContext* m_ioContext;
        Options m_options;
        ConnectCallback m_callback;

        std::shared_ptr<BotWorldConnection> m_connection;
        Step m_step = Step::WaitingAuthChallenge;

        std::array<uint8, 32> m_localChallenge{};
        std::array<uint8, 32> m_serverChallenge{};
        std::array<uint8, 32> m_encryptKey{};
};

#endif
