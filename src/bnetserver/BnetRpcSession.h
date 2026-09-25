#ifndef BNETSERVER_BNETRPCSESSION_H
#define BNETSERVER_BNETRPCSESSION_H

#include "BnetAccountStore.h"
#include "Common.h"
#include "Tls/TlsStream.h"

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace google { namespace protobuf { class Message; } }
namespace bgs { namespace protocol { class Header; } }

// What the RPC services need to know about the client on this connection.
struct BnetRpcClientState
{
    // From the Logon call.
    bool logonReceived = false;
    std::string platform;
    std::string locale;
    int32 build = 0;

    // From VerifyWebCredentials: the account behind the login ticket.
    nonstd::optional<BnetAccount> account;

    // From the realm-list ticket request: which game account the client is using, and its half of
    // the world session key.
    BnetGameAccount const* gameAccount = nullptr;
    std::array<uint8, 32> clientSecret{};
    bool hasClientSecret = false;
    uint32 clientPlatform = 0;
    uint32 clientArch = 0;
    uint32 clientType = 0;

    bool IsAuthed() const { return account.has_value(); }
};

// One connection to the Battle.net RPC port: TLS, then framed RPC calls dispatched to
// BnetServices. Unknown calls are answered with an error; only malformed framing closes the
// connection.
class BnetRpcSession : public std::enable_shared_from_this<BnetRpcSession>
{
    public:
        static std::shared_ptr<BnetRpcSession> Create(std::unique_ptr<IO::Networking::AsyncSocket> socket, SSL_CTX* sslCtx, std::string clientIp);

        void Start();

        std::string const& GetClientIp() const { return m_clientIp; }
        BnetRpcClientState& GetState() { return m_state; }

        // Called once the client has proven its login ticket; connections that never do are dropped
        // after a short deadline.
        void MarkAuthenticated() { m_authenticated = true; }

        // Runs `action` once the response to the call being handled has been queued, so a push a
        // call causes follows its response instead of preceding it.
        void AfterResponse(std::function<void()> action) { m_afterResponse.push_back(std::move(action)); }

        // An unprompted call to a service the client exports (AuthenticationListener, ...).
        void SendPush(uint32 serviceHash, uint32 methodId, google::protobuf::Message const& message);

        // Closes once everything already queued for the client has been written.
        void CloseAfterWrites();

    private:
        BnetRpcSession(std::shared_ptr<TlsStream> stream, std::string clientIp);

        void ReadFrame();
        void OnHeader(std::shared_ptr<std::vector<char>> const& headerBytes);
        void OnPayload(bgs::protocol::Header const& header, std::string payload);
        void Write(std::vector<uint8> frame, bool closeAfter);
        void SendResponse(uint32 token, uint32 status, std::string const& payload);
        void ScheduleIdleCheck();
        void Touch();

        std::shared_ptr<TlsStream> m_stream;
        std::string m_clientIp;
        BnetRpcClientState m_state;
        uint32 m_pushToken = 0;
        std::vector<std::function<void()>> m_afterResponse;
        std::atomic<int64> m_lastActivityMs{0};
        std::atomic<bool> m_authenticated{false};
        int64 m_connectedAtMs = 0;
};

#endif
