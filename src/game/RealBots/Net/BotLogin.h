#ifndef REALBOTS_BOTLOGIN_H
#define REALBOTS_BOTLOGIN_H

#include "BotBnetLogin.h"
#include "BotCharSelect.h"
#include "BotStage.h"
#include "BotWorldHandshake.h"
#include "IO/Context/IoContext.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

// Composes BotBnetLogin -> BotWorldHandshake -> BotCharSelect into bot-phase-1.md's whole "Login"
// sequence: account name and password in, a connected BotWorldConnection sitting InWorld out.
// Each step already exists and is independently verified against fakes; this is just the plain
// callback chain joining them, the same relationship BotBnetLogin itself has to
// BotRestClient/BotBnetConnection.
//
// Not yet a BotRoutine -- ctx.Packet/ctx.Run still aren't wired to real networking, which needs a
// RealBotMgr worker thread to resume onto (bot-phase-1.md step 5/6). This is the callback chain
// the eventual `login` BotRoutine will drive once that exists.
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

            // BotWorldHandshake::Options
            bool VerifyServerSignature = true;

            // BotCharSelect::Options
            bool CreateIfNone = true;
            std::string CharacterName;
            uint8 Race = 0;
            uint8 Class = 0;
            uint8 Gender = 0;
            float FarClip = 533.0f;

            // Holds the whole chain at this stage instead of continuing past it
            // (bot-phase-1.md, "Login": "stop_at=<stage>", the framework's per-stage awaitable
            // granularity). InWorld (the default) means "don't stop early". Threaded down into
            // whichever of BotBnetLogin/BotCharSelect owns that stage's own step; WorldConnecting
            // and WorldHandshake are checked here instead, since they sit at the seam between
            // BotBnetLogin and BotWorldHandshake/BotCharSelect rather than inside either one.
            BotStage StopAt = BotStage::InWorld;
        };

        struct Result
        {
            bool Success = false;
            // Set instead of the rest of a successful Result when Options::StopAt held the
            // chain before it reached InWorld; never set together with an error. Connection/
            // RealmConnection/CharacterGuid are filled in as far as the stopped-at stage got.
            bool Stopped = false;
            // The Instance connection once CLASSIC_SMSG_CONNECT_TO redirected to one (see
            // BotCharSelect::Result); falls back to the Realm connection if no redirect happened.
            std::shared_ptr<BotWorldConnection> Connection;
            // The original ("Realm") connection, kept alive alongside Connection above -- null
            // if no redirect happened (Connection is already the Realm connection then).
            std::shared_ptr<BotWorldConnection> RealmConnection;
            ClassicGuid CharacterGuid;
            std::string Error;
        };
        using LoginCallback = std::function<void(Result)>;

        static std::shared_ptr<BotLogin> Create(IO::IoContext* ioContext);

        void Login(Options options, LoginCallback callback);

    private:
        explicit BotLogin(IO::IoContext* ioContext);

        void OnBnetLogin(BotBnetLogin::Result result);
        void OnWorldHandshake(BotWorldHandshake::Result result);
        void OnCharSelect(BotCharSelect::Result result);

        void Fail(std::string const& error);
        void Succeed(std::shared_ptr<BotWorldConnection> connection, std::shared_ptr<BotWorldConnection> realmConnection, ClassicGuid const& guid);
        void StopEarly(std::shared_ptr<BotWorldConnection> connection = nullptr, std::shared_ptr<BotWorldConnection> realmConnection = nullptr, ClassicGuid const& guid = ClassicGuid());

        IO::IoContext* m_ioContext;
        Options m_options;
        LoginCallback m_callback;
};

#endif
