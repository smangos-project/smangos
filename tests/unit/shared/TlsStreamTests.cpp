#include "Tls/TlsStream.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
#include "Errors.h"
#include <catch_amalgamated.hpp>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

namespace
{
// Mirrors BotWorldConnectionTests.cpp's WaitFlag: a Catch2 TEST_CASE body needs to block on
// real async I/O and a blocking server thread until they've both actually run.
class WaitFlag
{
    public:
        void Set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_set = true;
            m_cv.notify_all();
        }

        bool WaitFor(std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cv.wait_for(lock, timeout, [this]() { return m_set; });
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_set = false;
};

constexpr uint16_t TestPort = 28518;

// An ephemeral, self-signed certificate for the server half of a test -- a client-role TlsStream
// never verifies its peer, so nothing about this cert needs to be trustworthy, only present.
// Move-only: a user-declared destructor suppresses the implicit move ctor/assignment but NOT
// the implicit (shallow, pointer-copying) copy assignment -- `member = GenerateSelfSignedCert()`
// would compile, shallow-copy the raw pointers, then immediately double-free them when the
// temporary's destructor runs, unless copy is explicitly removed and move is provided instead.
struct SelfSignedCert
{
    EVP_PKEY* key = nullptr;
    X509* cert = nullptr;

    SelfSignedCert() = default;
    SelfSignedCert(SelfSignedCert const&) = delete;
    SelfSignedCert& operator=(SelfSignedCert const&) = delete;

    SelfSignedCert(SelfSignedCert&& other) noexcept : key(other.key), cert(other.cert)
    {
        other.key = nullptr;
        other.cert = nullptr;
    }

    SelfSignedCert& operator=(SelfSignedCert&& other) noexcept
    {
        if (this != &other)
        {
            if (cert)
                X509_free(cert);
            if (key)
                EVP_PKEY_free(key);
            key = other.key;
            cert = other.cert;
            other.key = nullptr;
            other.cert = nullptr;
        }
        return *this;
    }

    ~SelfSignedCert()
    {
        if (cert)
            X509_free(cert);
        if (key)
            EVP_PKEY_free(key);
    }
};

SelfSignedCert GenerateSelfSignedCert()
{
    SelfSignedCert out;
    out.key = EVP_PKEY_Q_keygen(nullptr, nullptr, "RSA", size_t(2048));
    MANGOS_ASSERT(out.key);

    out.cert = X509_new();
    MANGOS_ASSERT(out.cert);
    X509_set_version(out.cert, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(out.cert), 1);
    X509_gmtime_adj(X509_getm_notBefore(out.cert), 0);
    X509_gmtime_adj(X509_getm_notAfter(out.cert), 3600);
    X509_set_pubkey(out.cert, out.key);

    X509_NAME* name = X509_get_subject_name(out.cert);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("tlsstream-test"), -1, -1, 0);
    X509_set_issuer_name(out.cert, name);

    MANGOS_ASSERT(X509_sign(out.cert, out.key, EVP_sha256()) > 0);
    return out;
}

// Plays the server half of a TLS connection by hand, with blocking OpenSSL calls on a raw
// socket fd -- proves the client role against an independent TLS implementation, simpler than
// making this stand-in non-blocking too, and the client role is what's under test here (the
// server role gets its own test below, TlsStream on both ends).
// Takes ownership of `descriptor` for the connection's whole lifetime: SSL_set_fd's BIO doesn't
// close the fd on SSL_free (matching LoginRESTService, which closes it itself afterwards), so
// this closes it explicitly at the end -- required before SocketDescriptor's destructor runs.
void RunBlockingTlsServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx, std::string const& expectedRequest,
    std::string const& response, WaitFlag& serverDone)
{
    SSL* ssl = SSL_new(serverCtx);
    SSL_set_fd(ssl, int(descriptor.GetNativeSocket()));

    if (SSL_accept(ssl) <= 0)
    {
        SSL_free(ssl);
        descriptor.CloseSocket();
        serverDone.Set();
        return;
    }

    std::string received(expectedRequest.size(), '\0');
    size_t got = 0;
    while (got < received.size())
    {
        int n = SSL_read(ssl, received.data() + got, int(received.size() - got));
        if (n <= 0)
            break;
        got += size_t(n);
    }
    CHECK(received == expectedRequest);

    SSL_write(ssl, response.data(), int(response.size()));

    SSL_shutdown(ssl);
    SSL_free(ssl);
    descriptor.CloseSocket();
    serverDone.Set();
}
}

TEST_CASE("TlsStream: completes a TLS handshake and exchanges plaintext both ways", "[tlsstream]")
{
    SelfSignedCert serverCert = GenerateSelfSignedCert();

    SSL_CTX* serverCtx = SSL_CTX_new(TLS_server_method());
    REQUIRE(serverCtx != nullptr);
    REQUIRE(SSL_CTX_use_certificate(serverCtx, serverCert.cert) == 1);
    REQUIRE(SSL_CTX_use_PrivateKey(serverCtx, serverCert.key) == 1);

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    std::string const request = "hello from the bot";
    std::string const response = "hello from the server";

    WaitFlag serverDone;
    std::thread serverThread;

    acceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        serverThread = std::thread([&, descriptor = std::move(descriptor)]() mutable
        {
            RunBlockingTlsServer(std::move(descriptor), serverCtx, request, response, serverDone);
        });
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort);

    auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(maybeDescriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
    std::shared_ptr<TlsStream> stream = TlsStream::CreateClient(std::move(clientSocket));

    WaitFlag connected;
    bool connectSucceeded = false;
    stream->Handshake([&](bool success)
    {
        connectSucceeded = success;
        connected.Set();
    });
    REQUIRE(connected.WaitFor(std::chrono::seconds(5)));
    REQUIRE(connectSucceeded);

    WaitFlag wrote;
    IO::NetworkError writeError(IO::NetworkError::ErrorType::NoError);
    std::vector<uint8> requestBytes(request.begin(), request.end());
    stream->Write(std::move(requestBytes), [&](IO::NetworkError const& error)
    {
        writeError = error;
        wrote.Set();
    });
    REQUIRE(wrote.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(writeError);

    std::vector<char> responseBuffer(response.size());
    WaitFlag readDone;
    IO::NetworkError readError(IO::NetworkError::ErrorType::NoError);
    stream->Read(responseBuffer.data(), responseBuffer.size(), [&](IO::NetworkError const& error, size_t)
    {
        readError = error;
        readDone.Set();
    });
    REQUIRE(readDone.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(readError);
    CHECK(std::string(responseBuffer.begin(), responseBuffer.end()) == response);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));
    serverThread.join();

    stream->Close();
    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();

    SSL_CTX_free(serverCtx);
}

TEST_CASE("TlsStream: connecting to a non-TLS peer fails the handshake instead of hanging", "[tlsstream]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort + 1);
    REQUIRE(acceptor != nullptr);

    // Accept the connection and immediately send garbage instead of a TLS ServerHello.
    acceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        auto serverSocket = std::make_shared<IO::Networking::AsyncSocket>(ctx.get(), std::move(descriptor));
        MANGOS_ASSERT(!(serverSocket->InitializeAndFixateMemoryLocation()));

        auto garbage = std::make_shared<std::vector<uint8>>(64, 0xFF);
        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(garbage) }, [serverSocket](IO::NetworkError const&) {});
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort + 1);

    auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(maybeDescriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
    std::shared_ptr<TlsStream> stream = TlsStream::CreateClient(std::move(clientSocket));

    WaitFlag connected;
    bool connectSucceeded = true;
    stream->Handshake([&](bool success)
    {
        connectSucceeded = success;
        connected.Set();
    });
    REQUIRE(connected.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(connectSucceeded);

    stream->Close();
    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
}

// The server role's own half-against-half: a TlsStream on both ends of the same loopback
// connection, instead of the blocking-OpenSSL stand-in the client-role test above uses.
TEST_CASE("TlsStream: a client and a server TlsStream complete a handshake and exchange plaintext both ways", "[tlsstream]")
{
    SelfSignedCert serverCert = GenerateSelfSignedCert();

    SSL_CTX* serverCtx = SSL_CTX_new(TLS_server_method());
    REQUIRE(serverCtx != nullptr);
    REQUIRE(SSL_CTX_use_certificate(serverCtx, serverCert.cert) == 1);
    REQUIRE(SSL_CTX_use_PrivateKey(serverCtx, serverCert.key) == 1);

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort + 2);
    REQUIRE(acceptor != nullptr);

    std::string const clientMessage = "hello from the client";
    std::string const serverMessage = "hello from the server";

    std::shared_ptr<TlsStream> serverStream;
    WaitFlag serverHandshakeDone;
    bool serverHandshakeOk = false;
    std::string serverReceived(clientMessage.size(), '\0');
    WaitFlag serverExchangeDone;

    acceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        IO::Networking::AsyncSocket serverSocket(ctx.get(), std::move(descriptor));
        serverStream = TlsStream::CreateServer(std::move(serverSocket), serverCtx);
        serverStream->Handshake([&](bool success)
        {
            serverHandshakeOk = success;
            serverHandshakeDone.Set();
            if (!success)
                return;

            serverStream->Read(serverReceived.data(), serverReceived.size(), [&](IO::NetworkError const& error, size_t)
            {
                CHECK_FALSE(error);
                std::vector<uint8> responseBytes(serverMessage.begin(), serverMessage.end());
                serverStream->Write(std::move(responseBytes), [&](IO::NetworkError const& writeError)
                {
                    CHECK_FALSE(writeError);
                    serverExchangeDone.Set();
                });
            });
        });
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort + 2);

    auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(maybeDescriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
    std::shared_ptr<TlsStream> clientStream = TlsStream::CreateClient(std::move(clientSocket));

    WaitFlag clientHandshakeDone;
    bool clientHandshakeOk = false;
    clientStream->Handshake([&](bool success)
    {
        clientHandshakeOk = success;
        clientHandshakeDone.Set();
    });
    REQUIRE(clientHandshakeDone.WaitFor(std::chrono::seconds(5)));
    CHECK(clientHandshakeOk);
    REQUIRE(serverHandshakeDone.WaitFor(std::chrono::seconds(5)));
    CHECK(serverHandshakeOk);

    WaitFlag clientWrote;
    IO::NetworkError clientWriteError(IO::NetworkError::ErrorType::NoError);
    std::vector<uint8> clientMessageBytes(clientMessage.begin(), clientMessage.end());
    clientStream->Write(std::move(clientMessageBytes), [&](IO::NetworkError const& error)
    {
        clientWriteError = error;
        clientWrote.Set();
    });
    REQUIRE(clientWrote.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(clientWriteError);

    REQUIRE(serverExchangeDone.WaitFor(std::chrono::seconds(5)));
    CHECK(serverReceived == clientMessage);

    std::vector<char> clientResponseBuffer(serverMessage.size());
    WaitFlag clientReadDone;
    IO::NetworkError clientReadError(IO::NetworkError::ErrorType::NoError);
    clientStream->Read(clientResponseBuffer.data(), clientResponseBuffer.size(), [&](IO::NetworkError const& error, size_t)
    {
        clientReadError = error;
        clientReadDone.Set();
    });
    REQUIRE(clientReadDone.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(clientReadError);
    CHECK(std::string(clientResponseBuffer.begin(), clientResponseBuffer.end()) == serverMessage);

    clientStream->Close();
    if (serverStream)
        serverStream->Close();
    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();

    SSL_CTX_free(serverCtx);
}
