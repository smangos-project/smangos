#include "BnetListener.h"
#include "BnetAccountStore.h"
#include "Log.h"
#include "ProxyProtocol/ProxyV2Reader.h"

#include <algorithm>

namespace
{
void AdmitConnection(std::unique_ptr<IO::Networking::AsyncSocket> socket, std::string const& clientIp, BnetSessionFactory const& factory)
{
    if (BnetAccountStore::IsIpBanned(clientIp))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] Connection from a banned IP refused", clientIp.c_str());
        return;
    }

    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] Connection accepted", clientIp.c_str());
    factory(std::move(socket), clientIp);
}
}

std::unique_ptr<IO::Networking::AsyncSocketAcceptor> BnetStartListener(IO::IoContext* ioContext, std::string const& bindIp, uint16 port,
    std::vector<std::string> const& trustedProxies, BnetSessionFactory factory)
{
    std::unique_ptr<IO::Networking::AsyncSocketAcceptor> listener = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ioContext, bindIp, port);
    if (!listener)
        return nullptr;

    listener->AutoAcceptSocketsUntilClose([ioContext, trustedProxies, factory](IO::Networking::SocketDescriptor descriptor)
    {
        auto socket = std::make_unique<IO::Networking::AsyncSocket>(ioContext, std::move(descriptor));
        if (IO::NetworkError const initError = socket->InitializeAndFixateMemoryLocation())
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] Failed to initialize the socket: %s", socket->GetRemoteIpString().c_str(), initError.ToString().c_str());
            return;
        }

        std::string const peerIp = socket->GetRemoteIpString();
        if (trustedProxies.empty() || std::find(trustedProxies.begin(), trustedProxies.end(), peerIp) == trustedProxies.end())
        {
            AdmitConnection(std::move(socket), peerIp, factory);
            return;
        }

        // The callback must be copyable, so the socket travels in a shared holder and is taken out once.
        auto holder = std::make_shared<std::unique_ptr<IO::Networking::AsyncSocket>>(std::move(socket));
        IO::Networking::AsyncSocket* rawSocket = holder->get();
        ProxyProtocol::ReadProxyV2Handshake(rawSocket, [holder, peerIp, factory](nonstd::expected<IO::Networking::IpAddress, IO::NetworkError> const& maybeIp)
        {
            if (!maybeIp.has_value())
            {
                sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] Failed to parse the proxy header: %s", peerIp.c_str(), maybeIp.error().ToString().c_str());
                return;
            }
            AdmitConnection(std::move(*holder), maybeIp.value().ToString(), factory);
        });
    });

    return listener;
}
