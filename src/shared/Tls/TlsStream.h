#ifndef SHARED_TLSSTREAM_H
#define SHARED_TLSSTREAM_H

#include "IO/Networking/AsyncSocket.h"
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct bio_st BIO;

// A TLS stream over IO::AsyncSocket, either role, driven by OpenSSL's memory BIOs. SSL_do_handshake
// never touches the socket itself; instead SSL reads/writes ciphertext to/from a pair of memory
// BIOs, and this class is the pump moving bytes between those BIOs and the real AsyncSocket -- the
// same "callbacks might not context switch" contract as AsyncSocket itself (IO/README.md), since
// every step here is either a callback invoked from one or scheduled onto one.
class TlsStream : public std::enable_shared_from_this<TlsStream>
{
    public:
        using ConnectCallback = std::function<void(bool success)>;
        using ReadCallback = std::function<void(IO::NetworkError const&, size_t)>;
        using WriteCallback = std::function<void(IO::NetworkError const&)>;

        // Client role: no certificate verification is performed (the login portal serves a
        // self-signed certificate, same as the real Battle.net login flow does for a private
        // server). Builds and owns its own SSL_CTX.
        static std::shared_ptr<TlsStream> CreateClient(IO::Networking::AsyncSocket socket);

        // Server role: `sslCtx` must already have a certificate and private key loaded
        // (SSL_CTX_use_certificate/SSL_CTX_use_PrivateKey) and must outlive every stream created
        // from it -- one context serves every accepted connection, the way a real listener loads
        // its certificate once, not per connection.
        static std::shared_ptr<TlsStream> CreateServer(IO::Networking::AsyncSocket socket, SSL_CTX* sslCtx);

        ~TlsStream();
        TlsStream(TlsStream const&) = delete;
        TlsStream& operator=(TlsStream const&) = delete;

        // Runs the TLS handshake as whichever role this stream was created with (SSL_connect for
        // a client, SSL_accept for a server -- both just SSL_do_handshake once the role is set).
        void Handshake(ConnectCallback callback);

        // Fills `target` with exactly `size` plaintext bytes, like AsyncSocket::Read.
        void Read(char* target, size_t size, ReadCallback callback);

        // Completes as soon as at least one plaintext byte is available, with up to `maxSize`
        // bytes in `target` and the count in the callback.
        void ReadSome(char* target, size_t maxSize, ReadCallback callback);

        // Encrypts and sends `data`. Callbacks fire in the order Write() was called.
        void Write(std::vector<uint8> data, WriteCallback callback);

        void Close();
        bool IsClosing() const { return m_closed || m_socket.IsClosing(); }

    private:
        enum class Role { Client, Server };

        TlsStream(IO::Networking::AsyncSocket socket, Role role, SSL_CTX* sslCtx, bool ownsSslCtx);

        void PumpHandshake();
        void FlushOutgoing(std::function<void()> then);
        void PullIncoming(std::function<void()> then);
        void ContinueRead();
        void OnError(IO::NetworkError const& error);

        IO::Networking::AsyncSocket m_socket;
        SSL_CTX* m_sslCtx = nullptr;
        bool m_ownsSslCtx = false; // client role builds+frees its own; server role's outlives every stream
        SSL* m_ssl = nullptr;
        BIO* m_readBio = nullptr;  // ciphertext received from the socket, fed to SSL_read
        BIO* m_writeBio = nullptr; // ciphertext SSL produced, drained to the socket

        ConnectCallback m_onConnect;

        char* m_readTarget = nullptr;
        size_t m_readTargetSize = 0;
        size_t m_readFilled = 0;
        bool m_readSome = false;
        ReadCallback m_readCallback;

        std::mutex m_writeQueueLock;
        std::deque<WriteCallback> m_pendingWriteCallbacks;
        bool m_writeFlushRunning = false;

        bool m_closed = false;
};

#endif
