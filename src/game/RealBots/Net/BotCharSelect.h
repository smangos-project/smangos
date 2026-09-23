#ifndef REALBOTS_BOTCHARSELECT_H
#define REALBOTS_BOTCHARSELECT_H

#include "BotWorldConnection.h"
#include "ClassicPackets.h"
#include "IO/Context/IoContext.h"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

// Drives CharSelect -> EnteringWorld (bot-phase-1.md, "Login" steps 5-6) on top of a connected,
// already-encrypted BotWorldConnection -- typically the one BotWorldHandshake::Result hands
// back. CMSG_ENUM_CHARACTERS -> pick the first character if one exists, else create one (when
// Options::CreateIfNone) using its response guid directly -> CMSG_PLAYER_LOGIN -> the server
// redirects to a second ("Instance") connection via CLASSIC_SMSG_CONNECT_TO (BotInstanceConnect)
// -> SMSG_LOGIN_VERIFY_WORLD there, bot-phase-1.md's InWorld marker. The original ("Realm")
// connection stays open (SMSG_PONG/SMSG_LOGOUT_COMPLETE still route through it) and comes back
// as Result::RealmConnection.
//
// Not yet a BotRoutine (same reason as BotBnetLogin/BotWorldHandshake -- ctx.Packet isn't wired
// to real networking yet, see bot-phase-1-progress-1.md). Takes over the connection's
// packet/disconnect callbacks via SetPacketCallback/SetDisconnectCallback in EnterWorld(); the
// caller must not keep driving the connection through its own callbacks afterward.
class BotCharSelect : public std::enable_shared_from_this<BotCharSelect>
{
    public:
        struct Options
        {
            bool CreateIfNone = true;
            // Only used if a character actually needs creating; must be set (and unique on the
            // target) whenever CreateIfNone might trigger.
            std::string CharacterName;
            uint8 Race = 0;
            uint8 Class = 0;
            uint8 Gender = 0;
            float FarClip = 533.0f;
            // Needed only if the server redirects to an Instance connection: the Realm
            // connection's BotWorldHandshake::Result::SessionKey and whichever
            // VerifyServerSignature choice that handshake used (BotInstanceConnect mirrors it).
            std::array<uint8, 40> SessionKey{};
            bool VerifyServerSignature = true;
        };

        struct Result
        {
            bool Success = false;
            ClassicGuid CharacterGuid;
            // The Instance connection once CLASSIC_SMSG_CONNECT_TO redirected to one (the usual
            // case against vibemangos/TrinityCore); falls back to the original connection if no
            // redirect happened.
            std::shared_ptr<BotWorldConnection> Connection;
            // The original ("Realm") connection, kept alive alongside Connection above -- null
            // if no redirect happened (Connection is already the Realm connection then).
            std::shared_ptr<BotWorldConnection> RealmConnection;
            std::string Error;
        };
        using CharSelectCallback = std::function<void(Result)>;

        static std::shared_ptr<BotCharSelect> Create(IO::IoContext* ioContext, std::shared_ptr<BotWorldConnection> connection);

        void EnterWorld(Options options, CharSelectCallback callback);

    private:
        enum class Step
        {
            WaitingEnumCharacters,
            WaitingCreateCharResponse,
            WaitingLoginVerifyWorld,
            Done,
        };

        explicit BotCharSelect(IO::IoContext* ioContext, std::shared_ptr<BotWorldConnection> connection);

        void OnPacket(BotPacket packet);
        void OnDisconnect();

        void HandleEnumCharactersResult(BotPacket& packet);
        void HandleCreateCharResponse(BotPacket& packet);
        void HandleLoginVerifyWorld(BotPacket& packet);
        void HandleConnectTo(BotPacket& packet);
        void SendPlayerLogin(ClassicGuid const& guid);

        void Fail(std::string const& error);
        void Succeed();

        IO::IoContext* m_ioContext;
        std::shared_ptr<BotWorldConnection> m_connection;
        std::shared_ptr<BotWorldConnection> m_realmConnection;
        Options m_options;
        CharSelectCallback m_callback;

        Step m_step = Step::WaitingEnumCharacters;
        ClassicGuid m_characterGuid;
};

#endif
