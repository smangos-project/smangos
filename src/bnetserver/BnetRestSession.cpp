#include "BnetRestSession.h"
#include "Config/Config.h"
#include "IO/Timer/AsyncSystemTimer.h"
#include "Log.h"

namespace
{
char const* ReasonForStatus(int status)
{
    switch (status)
    {
        case 400: return "Bad Request";
        case 413: return "Payload Too Large";
        case 431: return "Request Header Fields Too Large";
        case 501: return "Not Implemented";
        default:  return "Error";
    }
}
}

BnetRestSession::BnetRestSession(std::shared_ptr<TlsStream> stream, std::string clientIp)
    : m_stream(std::move(stream)), m_clientIp(std::move(clientIp)), m_readBuffer(4096)
{
}

std::shared_ptr<BnetRestSession> BnetRestSession::Create(std::unique_ptr<IO::Networking::AsyncSocket> socket, SSL_CTX* sslCtx, std::string clientIp)
{
    std::shared_ptr<TlsStream> stream = TlsStream::CreateServer(std::move(socket), sslCtx);
    return std::shared_ptr<BnetRestSession>(new BnetRestSession(std::move(stream), std::move(clientIp)));
}

void BnetRestSession::Start()
{
    // A connection that does nothing for this long is dropped; the timer holds only a weak reference.
    if (int const seconds = sConfig.GetIntDefault("LoginREST.SessionDuration", 300))
    {
        sAsyncSystemTimer.ScheduleFunctionOnce(std::chrono::seconds(seconds), [weak = std::weak_ptr<BnetRestSession>(shared_from_this())]()
        {
            if (std::shared_ptr<BnetRestSession> self = weak.lock())
            {
                sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] REST connection reached LoginREST.SessionDuration, closing", self->m_clientIp.c_str());
                self->m_stream->Close();
            }
        });
    }

    std::shared_ptr<BnetRestSession> self = shared_from_this();
    m_stream->Handshake([self](bool success)
    {
        if (!success)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] REST TLS handshake failed", self->m_clientIp.c_str());
            return;
        }
        self->ReadMore();
    });
}

void BnetRestSession::ReadMore()
{
    std::shared_ptr<BnetRestSession> self = shared_from_this();
    m_stream->ReadSome(m_readBuffer.data(), m_readBuffer.size(), [self](IO::NetworkError const& error, size_t received)
    {
        if (error)
            return;
        self->OnData(received);
    });
}

void BnetRestSession::OnData(size_t received)
{
    HttpRequestParser::Result result = m_parser.Feed(m_readBuffer.data(), received);

    bool keepOpen = true;
    while (result == HttpRequestParser::Result::Complete)
    {
        bool const keepAlive = m_parser.KeepAlive();
        SendResponse(BnetLoginPortal::Handle(m_parser.Request(), m_clientIp), keepAlive);
        if (!keepAlive)
        {
            keepOpen = false;
            break;
        }
        result = m_parser.Next();
    }

    if (keepOpen && result == HttpRequestParser::Result::Malformed)
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] REST request rejected with status %d", m_clientIp.c_str(), m_parser.ErrorStatus());
        BnetHttpResponse response;
        response.status = m_parser.ErrorStatus();
        response.reason = ReasonForStatus(response.status);
        SendResponse(response, false);
        keepOpen = false;
    }

    if (keepOpen)
        ReadMore();
}

void BnetRestSession::SendResponse(BnetHttpResponse const& response, bool keepAlive)
{
    std::vector<HttpMessage::Header> headers = response.headers;
    headers.push_back({ "Content-Type", response.contentType });
    headers.push_back({ "Content-Length", std::to_string(response.body.size()) });
    headers.push_back({ "Connection", keepAlive ? "keep-alive" : "close" });

    std::string const message = HttpMessage::BuildResponse(response.status, response.reason, headers, response.body);

    std::shared_ptr<BnetRestSession> self = shared_from_this();
    m_stream->Write(std::vector<uint8>(message.begin(), message.end()), [self, keepAlive](IO::NetworkError const& error)
    {
        if (error || !keepAlive)
            self->m_stream->Close();
    });
}
