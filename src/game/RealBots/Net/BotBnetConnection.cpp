#include "BotBnetConnection.h"
#include "Log.h"

#include "rpc_types.pb.h"
#include <google/protobuf/message.h>

namespace
{
// Mirrors BnetSession.cpp's RESPONSE_SERVICE_ID/PUSH_SERVICE_ID exactly (jimsproxy
// BnetTcpSession.cs:408): 0xFE marks a reply to one of our own requests, 0 marks an unprompted
// message the server pushed.
constexpr uint32 RESPONSE_SERVICE_ID = 0xFE;
constexpr uint32 PUSH_SERVICE_ID = 0;
}

std::shared_ptr<BotBnetConnection> BotBnetConnection::Create(IO::Networking::AsyncSocket socket)
{
    return std::shared_ptr<BotBnetConnection>(new BotBnetConnection(std::move(socket)));
}

BotBnetConnection::BotBnetConnection(IO::Networking::AsyncSocket socket)
{
    m_stream = BotTlsStream::Create(std::move(socket));
}

void BotBnetConnection::Connect(ConnectCallback callback)
{
    std::shared_ptr<BotBnetConnection> self = shared_from_this();
    m_stream->Connect([self, callback](bool success)
    {
        if (!success)
        {
            callback(false);
            return;
        }
        self->ReadNextFrame();
        callback(true);
    });
}

void BotBnetConnection::Call(uint32 serviceHash, uint32 methodId, google::protobuf::Message const& request, CallCallback callback)
{
    uint32 const token = ++m_nextToken;

    bgs::protocol::Header header;
    header.set_service_id(0);
    header.set_service_hash(serviceHash);
    header.set_method_id(methodId);
    header.set_token(token);

    std::string payload;
    request.SerializeToString(&payload);
    if (!payload.empty())
        header.set_size(uint32(payload.size()));

    std::string headerBytes;
    header.SerializeToString(&headerBytes);

    std::vector<uint8> frame;
    frame.reserve(2 + headerBytes.size() + payload.size());
    uint16 const headerLen = uint16(headerBytes.size());
    frame.push_back(uint8(headerLen >> 8));
    frame.push_back(uint8(headerLen));
    frame.insert(frame.end(), headerBytes.begin(), headerBytes.end());
    frame.insert(frame.end(), payload.begin(), payload.end());

    m_pendingCalls[token] = std::move(callback);

    std::shared_ptr<BotBnetConnection> self = shared_from_this();
    m_stream->Write(std::move(frame), [self](IO::NetworkError const& error)
    {
        if (error)
            self->OnError(error);
    });
}

void BotBnetConnection::ReadNextFrame()
{
    auto lengthBuf = std::make_shared<std::array<uint8, 2>>();
    std::shared_ptr<BotBnetConnection> self = shared_from_this();
    m_stream->Read(reinterpret_cast<char*>(lengthBuf->data()), lengthBuf->size(), [self, lengthBuf](IO::NetworkError const& error, size_t)
    {
        if (error)
        {
            self->OnError(error);
            return;
        }

        uint16 const headerLen = uint16((uint16((*lengthBuf)[0]) << 8) | (*lengthBuf)[1]);
        if (headerLen == 0 || headerLen > 4096)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotBnetConnection: bogus header length %u", headerLen);
            self->OnError(IO::NetworkError(IO::NetworkError::ErrorType::InternalError));
            return;
        }

        auto headerBuf = std::make_shared<std::vector<char>>(headerLen);
        self->m_stream->Read(headerBuf->data(), headerBuf->size(), [self, headerBuf](IO::NetworkError const& error, size_t)
        {
            if (error)
            {
                self->OnError(error);
                return;
            }

            auto header = std::make_shared<bgs::protocol::Header>();
            if (!header->ParseFromArray(headerBuf->data(), int(headerBuf->size())))
            {
                sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotBnetConnection: failed to parse Header");
                self->OnError(IO::NetworkError(IO::NetworkError::ErrorType::InternalError));
                return;
            }

            uint32 const payloadSize = header->size();
            if (payloadSize > 0x100000) // 1 MiB sanity bound, matching BnetSession::ReadFrame
            {
                sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotBnetConnection: payload size %u exceeds sanity bound", payloadSize);
                self->OnError(IO::NetworkError(IO::NetworkError::ErrorType::InternalError));
                return;
            }

            if (payloadSize == 0)
            {
                self->Dispatch(*header, std::string());
                self->ReadNextFrame();
                return;
            }

            auto payloadBuf = std::make_shared<std::string>(payloadSize, '\0');
            self->m_stream->Read(payloadBuf->data(), payloadBuf->size(), [self, header, payloadBuf](IO::NetworkError const& error, size_t)
            {
                if (error)
                {
                    self->OnError(error);
                    return;
                }
                self->Dispatch(*header, std::move(*payloadBuf));
                self->ReadNextFrame();
            });
        });
    });
}

void BotBnetConnection::Dispatch(bgs::protocol::Header const& header, std::string payload)
{
    if (header.service_id() == RESPONSE_SERVICE_ID)
    {
        auto it = m_pendingCalls.find(header.token());
        if (it == m_pendingCalls.end())
            return;
        CallCallback callback = std::move(it->second);
        m_pendingCalls.erase(it);
        callback(true, header.status(), std::move(payload));
    }
    else if (header.service_id() == PUSH_SERVICE_ID)
    {
        if (m_pushHandler)
            m_pushHandler(header.service_hash(), header.method_id(), payload);
    }
    else
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotBnetConnection: ignoring frame with unexpected service_id %u", header.service_id());
    }
}

void BotBnetConnection::OnError(IO::NetworkError const& error)
{
    if (m_closed)
        return;

    if (error.GetErrorType() != IO::NetworkError::ErrorType::SocketClosed || !IsClosing())
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotBnetConnection: %s", error.ToString().c_str());

    std::map<uint32, CallCallback> pending = std::move(m_pendingCalls);
    m_pendingCalls.clear();
    for (auto& [token, callback] : pending)
    {
        (void)token;
        callback(false, 0, std::string());
    }

    Close();
}

void BotBnetConnection::Close()
{
    if (m_closed)
        return;
    m_closed = true;
    m_stream->Close();
}
