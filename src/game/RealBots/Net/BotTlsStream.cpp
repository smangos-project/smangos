#include "BotTlsStream.h"
#include "Errors.h"
#include "Log.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

std::shared_ptr<BotTlsStream> BotTlsStream::Create(IO::Networking::AsyncSocket socket)
{
    auto self = std::shared_ptr<BotTlsStream>(new BotTlsStream(std::move(socket)));
    IO::NetworkError err = self->m_socket.InitializeAndFixateMemoryLocation();
    MANGOS_ASSERT(!err);
    return self;
}

BotTlsStream::BotTlsStream(IO::Networking::AsyncSocket socket) : m_socket(std::move(socket))
{
    m_sslCtx = SSL_CTX_new(TLS_client_method());
    MANGOS_ASSERT(m_sslCtx);
    // No certificate verification: the references (vibemangos's bnetserver, TrinityCore) serve
    // self-signed certificates, same as the real Battle.net login flow does for a private server.
    SSL_CTX_set_verify(m_sslCtx, SSL_VERIFY_NONE, nullptr);

    m_ssl = SSL_new(m_sslCtx);
    MANGOS_ASSERT(m_ssl);

    m_readBio = BIO_new(BIO_s_mem());
    m_writeBio = BIO_new(BIO_s_mem());
    MANGOS_ASSERT(m_readBio && m_writeBio);
    // A memory BIO normally reports EOF (0) once drained; -1 makes it report "would block"
    // instead, which is what SSL_read/SSL_write expect from a non-blocking BIO that just has
    // nothing available right now.
    BIO_set_mem_eof_return(m_readBio, -1);
    BIO_set_mem_eof_return(m_writeBio, -1);

    // SSL takes ownership of both BIOs from this point on; they're freed by SSL_free.
    SSL_set_bio(m_ssl, m_readBio, m_writeBio);
    SSL_set_connect_state(m_ssl);
}

BotTlsStream::~BotTlsStream()
{
    if (m_ssl)
        SSL_free(m_ssl);
    if (m_sslCtx)
        SSL_CTX_free(m_sslCtx);
}

void BotTlsStream::Connect(ConnectCallback callback)
{
    m_onConnect = std::move(callback);
    PumpHandshake();
}

void BotTlsStream::PumpHandshake()
{
    std::shared_ptr<BotTlsStream> self = shared_from_this();

    int ret = SSL_do_handshake(m_ssl);
    if (ret == 1)
    {
        FlushOutgoing([self]()
        {
            if (self->m_onConnect)
            {
                ConnectCallback callback = std::move(self->m_onConnect);
                self->m_onConnect = nullptr;
                callback(true);
            }
        });
        return;
    }

    int sslError = SSL_get_error(m_ssl, ret);
    if (sslError == SSL_ERROR_WANT_READ)
    {
        FlushOutgoing([self]()
        {
            self->PullIncoming([self]() { self->PumpHandshake(); });
        });
    }
    else if (sslError == SSL_ERROR_WANT_WRITE)
    {
        FlushOutgoing([self]() { self->PumpHandshake(); });
    }
    else
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotTlsStream: handshake failed (SSL error %d)", sslError);
        if (m_onConnect)
        {
            ConnectCallback callback = std::move(m_onConnect);
            m_onConnect = nullptr;
            callback(false);
        }
    }
}

void BotTlsStream::FlushOutgoing(std::function<void()> then)
{
    int pending = BIO_ctrl_pending(m_writeBio);
    if (pending <= 0)
    {
        then();
        return;
    }

    auto buffer = std::make_shared<std::vector<uint8>>(size_t(pending));
    int n = BIO_read(m_writeBio, buffer->data(), pending);
    if (n <= 0)
    {
        then();
        return;
    }
    buffer->resize(size_t(n));

    std::shared_ptr<BotTlsStream> self = shared_from_this();
    m_socket.Write({ std::shared_ptr<std::vector<uint8> const>(buffer) }, [self, then = std::move(then)](IO::NetworkError const& error)
    {
        if (error)
        {
            self->OnError(error);
            return;
        }
        then();
    });
}

void BotTlsStream::PullIncoming(std::function<void()> then)
{
    auto buffer = std::make_shared<std::vector<char>>(16384);
    std::shared_ptr<BotTlsStream> self = shared_from_this();
    m_socket.ReadSome(buffer->data(), buffer->size(), [self, buffer, then = std::move(then)](IO::NetworkError const& error, size_t received)
    {
        if (error)
        {
            self->OnError(error);
            return;
        }
        if (received == 0)
        {
            self->OnError(IO::NetworkError(IO::NetworkError::ErrorType::SocketClosed));
            return;
        }
        BIO_write(self->m_readBio, buffer->data(), int(received));
        then();
    });
}

void BotTlsStream::Read(char* target, size_t size, ReadCallback callback)
{
    m_readTarget = target;
    m_readTargetSize = size;
    m_readFilled = 0;
    m_readCallback = std::move(callback);
    ContinueRead();
}

void BotTlsStream::ContinueRead()
{
    std::shared_ptr<BotTlsStream> self = shared_from_this();

    while (m_readFilled < m_readTargetSize)
    {
        int n = SSL_read(m_ssl, m_readTarget + m_readFilled, int(m_readTargetSize - m_readFilled));
        if (n > 0)
        {
            m_readFilled += size_t(n);
            continue;
        }

        int sslError = SSL_get_error(m_ssl, n);
        if (sslError == SSL_ERROR_WANT_READ)
        {
            PullIncoming([self]() { self->ContinueRead(); });
            return;
        }
        if (sslError == SSL_ERROR_ZERO_RETURN)
        {
            OnError(IO::NetworkError(IO::NetworkError::ErrorType::SocketClosed));
            return;
        }

        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotTlsStream: SSL_read failed (SSL error %d)", sslError);
        OnError(IO::NetworkError(IO::NetworkError::ErrorType::InternalError));
        return;
    }

    ReadCallback callback = std::move(m_readCallback);
    m_readCallback = nullptr;
    callback(IO::NetworkError(IO::NetworkError::ErrorType::NoError), m_readFilled);
}

void BotTlsStream::Write(std::vector<uint8> data, WriteCallback callback)
{
    if (IsClosing())
        return;

    int n = SSL_write(m_ssl, data.data(), int(data.size()));
    if (n <= 0)
    {
        int sslError = SSL_get_error(m_ssl, n);
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotTlsStream: SSL_write failed (SSL error %d)", sslError);
        callback(IO::NetworkError(IO::NetworkError::ErrorType::InternalError));
        return;
    }

    bool startFlush;
    {
        std::lock_guard<std::mutex> lock(m_writeQueueLock);
        m_pendingWriteCallbacks.push_back(std::move(callback));
        startFlush = !m_writeFlushRunning;
        if (startFlush)
            m_writeFlushRunning = true;
    }

    if (!startFlush)
        return;

    std::shared_ptr<BotTlsStream> self = shared_from_this();
    FlushOutgoing([self]()
    {
        std::deque<WriteCallback> callbacks;
        {
            std::lock_guard<std::mutex> lock(self->m_writeQueueLock);
            callbacks = std::move(self->m_pendingWriteCallbacks);
            self->m_pendingWriteCallbacks.clear();
            self->m_writeFlushRunning = false;
        }
        for (WriteCallback& callback : callbacks)
            callback(IO::NetworkError(IO::NetworkError::ErrorType::NoError));
    });
}

void BotTlsStream::OnError(IO::NetworkError const& error)
{
    if (error.GetErrorType() != IO::NetworkError::ErrorType::SocketClosed || !IsClosing())
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotTlsStream: %s", error.ToString().c_str());

    if (m_readCallback)
    {
        ReadCallback callback = std::move(m_readCallback);
        m_readCallback = nullptr;
        callback(error, m_readFilled);
    }

    if (m_onConnect)
    {
        ConnectCallback callback = std::move(m_onConnect);
        m_onConnect = nullptr;
        callback(false);
    }

    std::deque<WriteCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_writeQueueLock);
        callbacks = std::move(m_pendingWriteCallbacks);
        m_pendingWriteCallbacks.clear();
        m_writeFlushRunning = false;
    }
    for (WriteCallback& callback : callbacks)
        callback(error);

    Close();
}

void BotTlsStream::Close()
{
    if (m_closed)
        return;
    m_closed = true;
    m_socket.CloseSocket();
}
