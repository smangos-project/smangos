#include "BnetRpcSession.h"
#include "BnetServices.h"
#include "Bnet/BnetRpcErrors.h"
#include "Bnet/BnetRpcFraming.h"
#include "Config/Config.h"
#include "IO/Timer/AsyncSystemTimer.h"
#include "Log.h"

#include "rpc_types.pb.h"
#include <google/protobuf/message.h>

namespace
{
int64 NowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

constexpr int64 IDLE_CHECK_SECONDS = 15;
}

BnetRpcSession::BnetRpcSession(std::shared_ptr<TlsStream> stream, std::string clientIp)
    : m_stream(std::move(stream)), m_clientIp(std::move(clientIp))
{
    m_connectedAtMs = NowMs();
    m_lastActivityMs = m_connectedAtMs;
}

std::shared_ptr<BnetRpcSession> BnetRpcSession::Create(std::unique_ptr<IO::Networking::AsyncSocket> socket, SSL_CTX* sslCtx, std::string clientIp)
{
    std::shared_ptr<TlsStream> stream = TlsStream::CreateServer(std::move(socket), sslCtx);
    return std::shared_ptr<BnetRpcSession>(new BnetRpcSession(std::move(stream), std::move(clientIp)));
}

void BnetRpcSession::Start()
{
    ScheduleIdleCheck();

    std::shared_ptr<BnetRpcSession> self = shared_from_this();
    m_stream->Handshake([self](bool success)
    {
        if (!success)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] RPC TLS handshake failed", self->m_clientIp.c_str());
            return;
        }
        self->ReadFrame();
    });
}

void BnetRpcSession::Touch()
{
    m_lastActivityMs = NowMs();
}

// Drops a connection that has not logged in by the deadline, or that has said nothing for the idle
// limit. The timer holds only a weak reference, so it never keeps a dead connection alive.
void BnetRpcSession::ScheduleIdleCheck()
{
    sAsyncSystemTimer.ScheduleFunctionOnce(std::chrono::seconds(IDLE_CHECK_SECONDS), [weak = std::weak_ptr<BnetRpcSession>(shared_from_this())]()
    {
        std::shared_ptr<BnetRpcSession> self = weak.lock();
        if (!self || self->m_stream->IsClosing())
            return;

        int64 const now = NowMs();
        int64 const loginDeadlineMs = int64(sConfig.GetIntDefault("Battlenet.LoginTimeout", 60)) * 1000;
        int64 const idleLimitMs = int64(sConfig.GetIntDefault("Battlenet.IdleTimeout", 600)) * 1000;

        if (!self->m_authenticated && loginDeadlineMs > 0 && now - self->m_connectedAtMs > loginDeadlineMs)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] RPC connection did not log in in time, closing", self->m_clientIp.c_str());
            self->m_stream->Close();
            return;
        }
        if (idleLimitMs > 0 && now - self->m_lastActivityMs > idleLimitMs)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] RPC connection idle, closing", self->m_clientIp.c_str());
            self->m_stream->Close();
            return;
        }
        self->ScheduleIdleCheck();
    });
}

void BnetRpcSession::ReadFrame()
{
    auto lengthBytes = std::make_shared<std::array<uint8, 2>>();
    std::shared_ptr<BnetRpcSession> self = shared_from_this();
    m_stream->Read(reinterpret_cast<char*>(lengthBytes->data()), lengthBytes->size(), [self, lengthBytes](IO::NetworkError const& error, size_t)
    {
        if (error)
            return;

        uint16 const headerLength = BnetRpcFraming::DecodeHeaderLength(lengthBytes->data());
        if (headerLength == 0 || headerLength > BnetRpcFraming::MaxHeaderLength)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] RPC frame with header length %u, closing", self->m_clientIp.c_str(), uint32(headerLength));
            self->m_stream->Close();
            return;
        }

        auto headerBytes = std::make_shared<std::vector<char>>(headerLength);
        self->m_stream->Read(headerBytes->data(), headerBytes->size(), [self, headerBytes](IO::NetworkError const& error, size_t)
        {
            if (error)
                return;
            self->OnHeader(headerBytes);
        });
    });
}

void BnetRpcSession::OnHeader(std::shared_ptr<std::vector<char>> const& headerBytes)
{
    auto header = std::make_shared<bgs::protocol::Header>();
    if (!BnetRpcFraming::DecodeHeader(headerBytes->data(), headerBytes->size(), *header))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] RPC frame with an unreadable header, closing", m_clientIp.c_str());
        m_stream->Close();
        return;
    }

    uint32 const payloadSize = header->size();
    if (payloadSize > BnetRpcFraming::MaxPayloadSize)
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "[%s] RPC payload of %u bytes, closing", m_clientIp.c_str(), payloadSize);
        m_stream->Close();
        return;
    }

    std::shared_ptr<BnetRpcSession> self = shared_from_this();
    if (payloadSize == 0)
    {
        OnPayload(*header, std::string());
        ReadFrame();
        return;
    }

    auto payload = std::make_shared<std::string>(payloadSize, '\0');
    m_stream->Read(payload->data(), payload->size(), [self, header, payload](IO::NetworkError const& error, size_t)
    {
        if (error)
            return;
        self->OnPayload(*header, std::move(*payload));
        self->ReadFrame();
    });
}

void BnetRpcSession::OnPayload(bgs::protocol::Header const& header, std::string payload)
{
    Touch();

    // A response to something this side sent; nothing here waits on one.
    if (header.service_id() == BnetRpcFraming::ResponseServiceId)
        return;

    m_afterResponse.clear();

    std::string response;
    uint32 const status = BnetServices::Dispatch(*this, header.service_hash(), header.method_id(), payload, response);
    SendResponse(header.token(), status, response);

    std::vector<std::function<void()>> actions = std::move(m_afterResponse);
    m_afterResponse.clear();
    for (std::function<void()> const& action : actions)
        action();
}

void BnetRpcSession::SendResponse(uint32 token, uint32 status, std::string const& payload)
{
    bgs::protocol::Header header;
    header.set_service_id(BnetRpcFraming::ResponseServiceId);
    header.set_token(token);
    if (status != BNET_ERROR_OK)
        header.set_status(status);
    if (!payload.empty())
        header.set_size(uint32(payload.size()));

    Write(BnetRpcFraming::EncodeFrame(header, payload), false);
}

void BnetRpcSession::SendPush(uint32 serviceHash, uint32 methodId, google::protobuf::Message const& message)
{
    std::string payload;
    message.SerializeToString(&payload);

    bgs::protocol::Header header;
    header.set_service_id(BnetRpcFraming::PushServiceId);
    header.set_service_hash(serviceHash);
    header.set_method_id(methodId);
    header.set_token(m_pushToken++);
    if (!payload.empty())
        header.set_size(uint32(payload.size()));

    Write(BnetRpcFraming::EncodeFrame(header, payload), false);
}

void BnetRpcSession::CloseAfterWrites()
{
    Write(std::vector<uint8>(), true);
}

void BnetRpcSession::Write(std::vector<uint8> frame, bool closeAfter)
{
    std::shared_ptr<BnetRpcSession> self = shared_from_this();
    if (frame.empty())
    {
        if (closeAfter)
            m_stream->Close();
        return;
    }

    m_stream->Write(std::move(frame), [self, closeAfter](IO::NetworkError const& error)
    {
        if (error || closeAfter)
            self->m_stream->Close();
    });
}
