#ifndef REALBOTS_BOTBNETCONNECTION_H
#define REALBOTS_BOTBNETCONNECTION_H

#include "Common.h"
#include "Tls/TlsStream.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace google { namespace protobuf { class Message; } }
namespace bgs { namespace protocol { class Header; } }

// Battle.net RPC over TLS, framed via BnetRpcFraming (bgs::protocol::Header, rpc_types.proto). One
// BotBnetConnection is one TLS connection to RpcPort, carrying every RPC call the login flow makes.
//
// Requests are sent bindless (service_id=0, service_hash=<the actual hash>): the server never
// requires a bind handshake first. Responses come back with service_id=ResponseServiceId, matched
// to a request by token; unprompted pushes (e.g. AuthenticationListener.OnLogonComplete) come back
// with service_id=PushServiceId and are delivered to the push handler instead of a Call callback.
class BotBnetConnection : public std::enable_shared_from_this<BotBnetConnection>
{
    public:
        using ConnectCallback = std::function<void(bool success)>;

        // status is a BnetServer::RpcErrorCode value (0 == Ok); responsePayload is the raw
        // serialized response message (empty on failure, or for calls with no response body).
        using CallCallback = std::function<void(bool success, uint32 status, std::string responsePayload)>;

        // Delivers every push frame (service_id == 0) the server sends: (serviceHash, methodId,
        // payload). Set before Connect(); pushes arriving before it's set are dropped.
        using PushHandler = std::function<void(uint32 serviceHash, uint32 methodId, std::string const& payload)>;

        static std::shared_ptr<BotBnetConnection> Create(IO::Networking::AsyncSocket socket);

        void Connect(ConnectCallback callback);

        void SetPushHandler(PushHandler handler) { m_pushHandler = std::move(handler); }

        // Serializes `request`, sends it as (serviceHash, methodId), and calls `callback` once
        // with the matching response, or success=false if the connection fails before one
        // arrives. No timeout here -- ctx.Bnet(...)'s timeout (bot-phase-1.md) is applied above
        // this class, same as BotWorldConnection owning no await/timeout logic of its own.
        void Call(uint32 serviceHash, uint32 methodId, google::protobuf::Message const& request, CallCallback callback);

        void Close();
        bool IsClosing() const { return m_closed || m_stream->IsClosing(); }

    private:
        explicit BotBnetConnection(IO::Networking::AsyncSocket socket);

        void ReadNextFrame();
        void Dispatch(bgs::protocol::Header const& header, std::string payload);
        void OnError(IO::NetworkError const& error);

        std::shared_ptr<TlsStream> m_stream;
        PushHandler m_pushHandler;

        uint32 m_nextToken = 0;
        std::map<uint32, CallCallback> m_pendingCalls;

        bool m_closed = false;
};

#endif
