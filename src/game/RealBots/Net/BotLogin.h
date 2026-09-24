#ifndef REALBOTS_BOTLOGIN_H
#define REALBOTS_BOTLOGIN_H

#include "BotBnetLogin.h"
#include "BotStage.h"
#include "IO/Context/IoContext.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// The Battle.net half of bot-phase-1.md's "Login": account name and password in, a world server
// address, join key and ticket out (LoginPortal -> LoginRpc -> RealmSelect, all BotBnetLogin's).
// It runs its network work on the bot's IoContext and reports on an IO thread, which is why the
// login routine drives it through ctx.Login (the completion is marshalled onto the bot's own
// thread there). Everything after RealmSelect -- the world handshake, the character screen, the
// Instance redirect -- is not here: those are routines on the bot itself (Net/BotWorldLogin.h), so
// their packets go through the bot's normal dispatch.
class BotLogin : public std::enable_shared_from_this<BotLogin>
{
    public:
        struct Options
        {
            // BotBnetLogin::Options
            std::string LoginAddress;
            uint16_t RestPort = 0;
            uint16_t RpcPort = 0;
            std::string AccountName;
            std::string Password;
            std::string RealmName;
            uint32_t RealmId = 0;
            int32_t ClientBuild = 0;
            uint32_t ConnectTimeoutMs = 10000;
            std::string LoginTicketOverride; // BotBnetLogin::Options::LoginTicketOverride
            // BotBnetLogin::Options::OnStage: LoginRpc and RealmSelect as they begin.
            std::function<void(BotStage)> OnStage;

            // Holds the login as this stage begins, before any of its own work (bot-phase-1.md,
            // "Login": "stop_at=<stage>"). Only the stages this class owns are checked here:
            // LoginPortal (before any network call), LoginRpc (the portal login done), RealmSelect
            // (the RPC logon done) and WorldConnecting (the realm joined, no world socket opened
            // yet). InWorld (the default) means "don't stop early"; the later stages are the
            // world routines' (Net/BotWorldLogin.h).
            BotStage StopAt = BotStage::InWorld;
        };

        struct Result
        {
            bool Success = false;
            // Set instead of the rest of a successful Result when Options::StopAt held the login
            // here; never set together with an error.
            bool Stopped = false;
            std::string LoginTicket; // BotBnetLogin::Result::LoginTicket
            std::string WorldAddress;
            uint16_t WorldPort = 0;
            // BotBnetLogin::Result::SessionKey: what the world handshake's digests are keyed with.
            std::vector<uint8> SessionKey;
            std::string RealmJoinTicket;
            std::string Error;
        };
        using LoginCallback = std::function<void(Result)>;

        static std::shared_ptr<BotLogin> Create(IO::IoContext* ioContext);

        void Login(Options options, LoginCallback callback);

    private:
        explicit BotLogin(IO::IoContext* ioContext);

        void OnBnetLogin(BotBnetLogin::Result result);

        void Fail(std::string const& error);
        void Finish(Result result);

        IO::IoContext* m_ioContext;
        Options m_options;
        LoginCallback m_callback;
};

#endif
