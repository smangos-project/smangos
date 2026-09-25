#ifndef BNETSERVER_BNETRESTSESSION_H
#define BNETSERVER_BNETRESTSESSION_H

#include "BnetLoginPortal.h"
#include "Http/HttpRequestParser.h"
#include "Tls/TlsStream.h"

#include <memory>
#include <vector>

// One connection to the login portal: TLS, then HTTP/1.1 requests answered by BnetLoginPortal
// until the client closes, asks to, or reaches LoginREST.SessionDuration.
class BnetRestSession : public std::enable_shared_from_this<BnetRestSession>
{
    public:
        // `clientIp` is the peer as the portal should see it (after any proxy header).
        static std::shared_ptr<BnetRestSession> Create(std::unique_ptr<IO::Networking::AsyncSocket> socket, SSL_CTX* sslCtx, std::string clientIp);

        void Start();

    private:
        BnetRestSession(std::shared_ptr<TlsStream> stream, std::string clientIp);

        void ReadMore();
        void OnData(size_t received);
        void SendResponse(BnetHttpResponse const& response, bool keepAlive);

        std::shared_ptr<TlsStream> m_stream;
        std::string m_clientIp;
        HttpRequestParser m_parser;
        std::vector<char> m_readBuffer;
};

#endif
