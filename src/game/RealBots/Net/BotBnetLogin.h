#ifndef REALBOTS_BOTBNETLOGIN_H
#define REALBOTS_BOTBNETLOGIN_H

#include "BotBnetConnection.h"
#include "BotRestClient.h"
#include "BotStage.h"
#include "IO/Context/IoContext.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Composes BotRestClient (LoginPortal) and BotBnetConnection (LoginRpc, RealmSelect) into the
// full sequence bot-phase-1.md's "Login" describes up through JoinRealm: account name/password
// in, a world server address + session key out. Not yet a BotRoutine coroutine (ctx.Bnet and the
// rest of the coroutine layer aren't wired to real networking);
// this is the plain callback-based network step the eventual LoginRoutine will drive, same
// relationship BotRestClient/BotBnetConnection already have to their own callers.
//
// Steps, each cited against vibemangos's src/bnetserver (Services/*.cpp, Server/BnetSession.cpp):
//   LoginPortal : BotRestClient::Login (already exists)
//   LoginRpc    : ConnectionService.Connect -> AuthenticationService.Logon ->
//                 AuthenticationService.VerifyWebCredentials -> wait for the
//                 AuthenticationListener.OnLogonComplete push
//   RealmSelect : GameUtilitiesService.ProcessClientRequest three times (RealmListTicket,
//                 RealmList, RealmJoin) -- see GameUtilitiesService.cpp for the exact
//                 attribute/JSON shapes this mirrors
class BotBnetLogin : public std::enable_shared_from_this<BotBnetLogin>
{
    public:
        struct Options
        {
            std::string LoginAddress;
            uint16_t RestPort = 0;
            uint16_t RpcPort = 0;
            std::string AccountName;
            std::string Password;
            // Realm selection: RealmName wins when non-empty, else RealmId when non-zero, else
            // the first realm the server lists (bot-phase-1.md's RealmSelect args).
            std::string RealmName;
            uint32_t RealmId = 0;
            int32_t ClientBuild = 0;
            // How long each of the two connects (REST portal, RPC) may take.
            uint32_t ConnectTimeoutMs = 10000;
            // Used at the RPC step instead of the ticket the portal issued (a forged or reused one,
            // for scenarios); empty uses the real one.
            std::string LoginTicketOverride;
            // Called as each stage begins after LoginPortal (LoginRpc once the portal login is
            // done, RealmSelect once the RPC logon is complete), on whichever IO thread got there;
            // before StopAt is checked, so a held login has reported the stage it holds at.
            std::function<void(BotStage)> OnStage;
            // Holds as LoginRpc or RealmSelect begins, instead of continuing (bot-phase-1.md,
            // "Login": "stop_at=<stage>"): after the portal login, or after the RPC logon. Realm
            // select completing is just this call's normal success, so only these two stages need
            // a check here -- BotLogin (the composer) handles StopAt for LoginPortal and
            // WorldConnecting itself.
            BotStage StopAt = BotStage::InWorld;
        };

        struct Result
        {
            bool Success = false;
            // Set instead of the rest of a successful Result when Options::StopAt held this
            // call before it finished; never set together with an error.
            bool Stopped = false;
            // The login ticket the portal issued (not any override), set once the portal login
            // succeeded, including on a held (Stopped) result.
            std::string LoginTicket;
            std::string WorldAddress;
            uint16_t WorldPort = 0;
            // clientSecret(32) || serverSecret(32) -- the joinSessionKey WorldHandshakeCrypto's
            // ComputeAuthCheckDigest/ComputeSessionKey take, per GameUtilitiesService.cpp's
            // JoinRealm ("World session key = client-supplied secret || server-generated secret").
            std::vector<uint8> SessionKey;
            // An opaque blob the world socket echoes back as ClassicAuthSession::RealmJoinTicket;
            // never parsed here, so its exact shape (which varies by server) doesn't matter.
            std::string RealmJoinTicket;
            std::string Error;
        };
        using LoginCallback = std::function<void(Result)>;

        static std::shared_ptr<BotBnetLogin> Create(IO::IoContext* ioContext);

        void Login(Options options, LoginCallback callback);

    private:
        explicit BotBnetLogin(IO::IoContext* ioContext);

        void ConnectRest();
        void OnRestLogin(BotRestClient::LoginResult result);

        void ConnectRpc();
        void OnRpcConnected(bool success);
        void OnConnectServiceResponse(bool success, uint32_t status, std::string payload);
        void OnLogonResponse(bool success, uint32_t status, std::string payload);
        void OnVerifyWebCredentialsResponse(bool success, uint32_t status, std::string payload);
        void OnPush(uint32_t serviceHash, uint32_t methodId, std::string const& payload);
        void MaybeStartRealmSelect();

        void RequestRealmListTicket();
        void OnRealmListTicketResponse(bool success, uint32_t status, std::string payload);
        void RequestRealmList();
        void OnRealmListResponse(bool success, uint32_t status, std::string payload);
        void RequestRealmJoin(uint32_t realmAddress);
        void OnRealmJoinResponse(bool success, uint32_t status, std::string payload);

        void Fail(std::string const& error);
        void Succeed(Result result);
        void StopEarly();

        IO::IoContext* m_ioContext;
        Options m_options;
        LoginCallback m_callback;

        std::shared_ptr<BotRestClient> m_rest;
        std::shared_ptr<BotBnetConnection> m_rpc;
        std::string m_loginTicket;       // what the RPC step sends (the portal's, or the override)
        std::string m_portalTicket;      // what the portal issued

        bool m_verifyOk = false;
        bool m_logonCompletePushed = false;
        uint32_t m_logonErrorCode = 0;
        // The account's first game account id and region, from OnLogonComplete's push
        // (LogonResult::game_account_id/connected_region) -- GetRealmListTicket needs the id to
        // resolve which account is asking.
        uint32_t m_gameAccountId = 0;
        uint32_t m_gameAccountRegion = 1;

        std::vector<uint8> m_clientSecret;
};

#endif
