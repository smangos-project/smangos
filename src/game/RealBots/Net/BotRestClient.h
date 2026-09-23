#ifndef REALBOTS_BOTRESTCLIENT_H
#define REALBOTS_BOTRESTCLIENT_H

#include "BotTlsStream.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include <functional>
#include <memory>
#include <string>

// The bot's half of vibemangos's LoginRESTService (src/bnetserver/REST/LoginRESTService.cpp):
// hand-rolled HTTP-over-TLS carrying the Battle.net SRP6 (v1) login exchange (bot-phase-1.md,
// "LoginPortal"). One BotRestClient is one TLS connection, and the whole SRP login/srp/ ->
// login/ exchange happens on that same connection -- LoginRESTService's ConnectionSrpSession is
// tied to the connection itself, not any ticket/session id, so a second connection can't finish
// a challenge started on this one.
class BotRestClient : public std::enable_shared_from_this<BotRestClient>
{
    public:
        struct LoginResult
        {
            bool Success = false;
            std::string LoginTicket;
            std::string Error;
        };
        using LoginCallback = std::function<void(LoginResult)>;

        static std::shared_ptr<BotRestClient> Create(IO::Networking::AsyncSocket socket);

        // Connects (TLS, no certificate verification) then runs the two-step SRP login:
        // POST /bnetserver/login/srp/ (account_name) for a challenge, then POST
        // /bnetserver/login/ (public_A, client_evidence_M1) for the ticket -- matching what
        // AuthenticationService::VerifyWebCredentials later expects to be handed.
        void Login(std::string const& accountName, std::string const& password, LoginCallback callback);

        void Close();

    private:
        explicit BotRestClient(IO::Networking::AsyncSocket socket);

        struct HttpResponse
        {
            int Status = 0;
            std::string Body;
        };
        using HttpCallback = std::function<void(bool ok, HttpResponse response)>;

        void PostFormInputs(std::string const& path, std::string const& jsonBody, HttpCallback callback);
        void ReadLine(std::shared_ptr<std::string> line, std::function<void(bool ok, std::string line)> callback);
        void ReadStatusAndHeaders(std::function<void(bool ok, int status, size_t contentLength)> callback);
        void ReadBody(size_t contentLength, std::function<void(bool ok, std::string body)> callback);

        void OnConnected(bool success);
        void OnSrpChallengeResponse(bool ok, HttpResponse response);
        void OnSrpProofResponse(bool ok, HttpResponse response);

        void Fail(std::string const& error);
        void Succeed(std::string const& ticket);

        std::shared_ptr<BotTlsStream> m_stream;
        std::string m_accountName;
        std::string m_password;
        LoginCallback m_callback;
        BnetSRP6 m_srp;
};

#endif
