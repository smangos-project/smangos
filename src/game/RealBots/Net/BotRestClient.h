#ifndef REALBOTS_BOTRESTCLIENT_H
#define REALBOTS_BOTRESTCLIENT_H

#include "Crypto/Authentication/BnetSRP6.h"
#include "Tls/TlsStream.h"
#include <functional>
#include <memory>
#include <string>

// Hand-rolled HTTP-over-TLS carrying the Battle.net SRP6 (v1) login exchange. One BotRestClient is
// one TLS connection running the whole SRP login/srp/ -> login/ exchange, but the server resolves
// the in-progress challenge from the session cookie the first response sets, not the connection --
// the proof request must carry it back (see m_sessionCookie).
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

        struct HttpResponse
        {
            int Status = 0;
            std::string Body;
        };
        using HttpCallback = std::function<void(bool ok, HttpResponse response)>;

        static std::shared_ptr<BotRestClient> Create(IO::Networking::AsyncSocket socket);

        // Connects (TLS, no certificate verification) then runs the two-step SRP login:
        // POST /bnetserver/login/srp/ (account_name) for a challenge, then POST
        // /bnetserver/login/ (public_A, client_evidence_M1) for the ticket -- matching what
        // AuthenticationService::VerifyWebCredentials later expects to be handed.
        void Login(std::string const& accountName, std::string const& password, LoginCallback callback);

        // The TLS connect on its own, for callers that send their own request bodies afterward
        // (PostFormInputs) rather than running the SRP login this class otherwise drives.
        void Connect(std::function<void(bool success)> callback);

        // Posts `jsonBody` verbatim to `path` -- whatever it contains, well-formed or not (design
        // principle: "any task or test can send any packet, typed or raw, malformed on purpose").
        // Login() is just this called twice with the SRP exchange's own bodies.
        void PostFormInputs(std::string const& path, std::string const& jsonBody, HttpCallback callback);

        void Close();

    private:
        explicit BotRestClient(IO::Networking::AsyncSocket socket);

        void ReadLine(std::shared_ptr<std::string> line, std::function<void(bool ok, std::string line)> callback);
        void ReadStatusAndHeaders(std::function<void(bool ok, int status, size_t contentLength)> callback);
        void ReadBody(size_t contentLength, std::function<void(bool ok, std::string body)> callback);

        void OnConnected(bool success);
        void OnSrpChallengeResponse(bool ok, HttpResponse response);
        void OnSrpProofResponse(bool ok, HttpResponse response);

        void Fail(std::string const& error);
        void Succeed(std::string const& ticket);

        std::shared_ptr<TlsStream> m_stream;
        std::string m_accountName;
        std::string m_password;
        LoginCallback m_callback;
        BnetSRP6 m_srp;
        // The server resolves the in-progress SRP challenge from this, not the connection: the
        // proof POST must carry back whatever Set-Cookie the challenge response sent, or the
        // server sees a fresh, un-challenged session and rejects the proof.
        std::string m_sessionCookie;
};

#endif
