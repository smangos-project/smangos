#ifndef REALBOTS_BOTCHARSELECT_H
#define REALBOTS_BOTCHARSELECT_H

#include "BotWorldConnection.h"
#include "ClassicPackets.h"
#include <functional>
#include <memory>
#include <string>

// Drives CharSelect -> EnteringWorld (bot-phase-1.md, "Login" steps 5-6) on top of a connected,
// already-encrypted BotWorldConnection -- typically the one BotWorldHandshake::Result hands
// back. CMSG_ENUM_CHARACTERS -> pick the first character if one exists, else create one (when
// Options::CreateIfNone) using its response guid directly -> CMSG_PLAYER_LOGIN ->
// SMSG_LOGIN_VERIFY_WORLD, bot-phase-1.md's InWorld marker.
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
        };

        struct Result
        {
            bool Success = false;
            ClassicGuid CharacterGuid;
            std::string Error;
        };
        using CharSelectCallback = std::function<void(Result)>;

        static std::shared_ptr<BotCharSelect> Create(std::shared_ptr<BotWorldConnection> connection);

        void EnterWorld(Options options, CharSelectCallback callback);

    private:
        enum class Step
        {
            WaitingEnumCharacters,
            WaitingCreateCharResponse,
            WaitingLoginVerifyWorld,
            Done,
        };

        explicit BotCharSelect(std::shared_ptr<BotWorldConnection> connection);

        void OnPacket(BotPacket packet);
        void OnDisconnect();

        void HandleEnumCharactersResult(BotPacket& packet);
        void HandleCreateCharResponse(BotPacket& packet);
        void HandleLoginVerifyWorld(BotPacket& packet);
        void SendPlayerLogin(ClassicGuid const& guid);

        void Fail(std::string const& error);
        void Succeed();

        std::shared_ptr<BotWorldConnection> m_connection;
        Options m_options;
        CharSelectCallback m_callback;

        Step m_step = Step::WaitingEnumCharacters;
        ClassicGuid m_characterGuid;
};

#endif
