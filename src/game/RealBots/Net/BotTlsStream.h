#ifndef REALBOTS_BOTTLSSTREAM_H
#define REALBOTS_BOTTLSSTREAM_H

#include "IO/Networking/AsyncSocket.h"
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct bio_st BIO;

// A TLS client stream over IO::AsyncSocket, driven by OpenSSL's memory BIOs (bot-phase-1.md,
// "TLS: OpenSSL ... via memory BIOs (BotTlsStream). No certificate verification."). SSL_connect
// never touches the socket itself; instead SSL reads/writes ciphertext to/from a pair of memory
// BIOs, and this class is the pump moving bytes between those BIOs and the real AsyncSocket --
// the same "callbacks might not context switch" contract as AsyncSocket itself (IO/README.md),
// since every step here is either a callback invoked from one or scheduled onto one.
//
// Used by BotRestClient (HTTPS to the login portal) and, later, BotBnetConnection (TLS+protobuf
// RPC) -- both need a byte-stream abstraction on top of a socket, not a packet-framed one like
// BotWorldConnection.
class BotTlsStream : public std::enable_shared_from_this<BotTlsStream>
{
    public:
        using ConnectCallback = std::function<void(bool success)>;
        using ReadCallback = std::function<void(IO::NetworkError const&, size_t)>;
        using WriteCallback = std::function<void(IO::NetworkError const&)>;

        static std::shared_ptr<BotTlsStream> Create(IO::Networking::AsyncSocket socket);

        ~BotTlsStream();
        BotTlsStream(BotTlsStream const&) = delete;
        BotTlsStream& operator=(BotTlsStream const&) = delete;

        // Runs the TLS client handshake. No certificate verification is performed (the
        // references use self-signed certificates; see bot-phase-1.md's shared-pieces list).
        void Connect(ConnectCallback callback);

        // Fills `target` with exactly `size` plaintext bytes, like AsyncSocket::Read.
        void Read(char* target, size_t size, ReadCallback callback);

        // Encrypts and sends `data`. Callbacks fire in the order Write() was called.
        void Write(std::vector<uint8> data, WriteCallback callback);

        void Close();
        bool IsClosing() const { return m_closed || m_socket.IsClosing(); }

    private:
        explicit BotTlsStream(IO::Networking::AsyncSocket socket);

        void PumpHandshake();
        void FlushOutgoing(std::function<void()> then);
        void PullIncoming(std::function<void()> then);
        void ContinueRead();
        void OnError(IO::NetworkError const& error);

        IO::Networking::AsyncSocket m_socket;
        SSL_CTX* m_sslCtx = nullptr;
        SSL* m_ssl = nullptr;
        BIO* m_readBio = nullptr;  // ciphertext received from the socket, fed to SSL_read
        BIO* m_writeBio = nullptr; // ciphertext SSL produced, drained to the socket

        ConnectCallback m_onConnect;

        char* m_readTarget = nullptr;
        size_t m_readTargetSize = 0;
        size_t m_readFilled = 0;
        ReadCallback m_readCallback;

        std::mutex m_writeQueueLock;
        std::deque<WriteCallback> m_pendingWriteCallbacks;
        bool m_writeFlushRunning = false;

        bool m_closed = false;
};

#endif
