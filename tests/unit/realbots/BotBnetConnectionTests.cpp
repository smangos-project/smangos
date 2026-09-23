#include "BotBnetConnection.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
#include "Errors.h"
#include <catch_amalgamated.hpp>

#include "rpc_types.pb.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
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

// Same move-only footgun avoidance as BotRestClientTests.cpp/BotTlsStreamTests.cpp's copy of
// this helper -- see those files for why the copy assignment must be explicitly deleted.
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
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("botbnetconnection-test"), -1, -1, 0);
    X509_set_issuer_name(out.cert, name);

    MANGOS_ASSERT(X509_sign(out.cert, out.key, EVP_sha256()) > 0);
    return out;
}

constexpr uint32 RESPONSE_SERVICE_ID = 0xFE;
constexpr uint32 PUSH_SERVICE_ID = 0;

bool ReadExact(SSL* ssl, void* buffer, size_t size)
{
    char* dst = static_cast<char*>(buffer);
    size_t got = 0;
    while (got < size)
    {
        int n = SSL_read(ssl, dst + got, int(size - got));
        if (n <= 0)
            return false;
        got += size_t(n);
    }
    return true;
}

bool WriteExact(SSL* ssl, void const* buffer, size_t size)
{
    char const* src = static_cast<char const*>(buffer);
    size_t sent = 0;
    while (sent < size)
    {
        int n = SSL_write(ssl, src + sent, int(size - sent));
        if (n <= 0)
            return false;
        sent += size_t(n);
    }
    return true;
}

// Server-side framing mirroring vibemangos's BnetSession::ReadFrame/WriteFrame (see
// src/bnetserver/Server/BnetSession.cpp) -- enough of it to drive BotBnetConnection end to end.
bool ReadFrame(SSL* ssl, uint32& outServiceId, uint32& outServiceHash, uint32& outMethodId, uint32& outToken, std::string& outPayload)
{
    uint8 lengthBytes[2];
    if (!ReadExact(ssl, lengthBytes, sizeof(lengthBytes)))
        return false;
    uint16 headerLen = uint16((uint16(lengthBytes[0]) << 8) | lengthBytes[1]);
    if (headerLen == 0 || headerLen > 4096)
        return false;

    std::string headerBytes(headerLen, '\0');
    if (!ReadExact(ssl, &headerBytes[0], headerLen))
        return false;

    bgs::protocol::Header header;
    if (!header.ParseFromString(headerBytes))
        return false;

    outPayload.assign(header.size(), '\0');
    if (header.size() && !ReadExact(ssl, &outPayload[0], header.size()))
        return false;

    outServiceId = header.service_id();
    outServiceHash = header.service_hash();
    outMethodId = header.method_id();
    outToken = header.token();
    return true;
}

void WriteFrame(SSL* ssl, uint32 serviceId, uint32 serviceHash, uint32 methodId, uint32 token, uint32 status, std::string const& payload)
{
    bgs::protocol::Header header;
    header.set_service_id(serviceId);
    header.set_service_hash(serviceHash);
    header.set_method_id(methodId);
    header.set_token(token);
    header.set_status(status);
    if (!payload.empty())
        header.set_size(uint32(payload.size()));

    std::string headerBytes;
    header.SerializeToString(&headerBytes);

    uint16 headerLen = uint16(headerBytes.size());
    uint8 lengthBytes[2] = { uint8(headerLen >> 8), uint8(headerLen) };
    WriteExact(ssl, lengthBytes, sizeof(lengthBytes));
    WriteExact(ssl, headerBytes.data(), headerBytes.size());
    if (!payload.empty())
        WriteExact(ssl, payload.data(), payload.size());
}

constexpr uint32 TEST_KNOWN_SERVICE_HASH = 0x11111111;
constexpr uint32 TEST_KNOWN_METHOD_ID = 1;
constexpr uint32 TEST_PUSH_SERVICE_HASH = 0x22222222;
constexpr uint32 TEST_PUSH_METHOD_ID = 5;
constexpr char const* TEST_PUSH_PAYLOAD = "pushed-payload";

// Answers one known (serviceHash, methodId) with Ok, pushes one unprompted message right after
// the TLS handshake (so the test doesn't need to wait on a second Call to observe it), and
// answers anything else with RpcNotImplemented (5), matching BnetSession's own "no handler"
// path -- enough surface to prove Call()/response matching and push delivery both work over a
// real TLS+framing round trip, not just in-process function calls.
void RunFakeBnetServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx, WaitFlag& serverDone)
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

    WriteFrame(ssl, PUSH_SERVICE_ID, TEST_PUSH_SERVICE_HASH, TEST_PUSH_METHOD_ID, 1, 0, TEST_PUSH_PAYLOAD);

    for (;;)
    {
        uint32 serviceId, serviceHash, methodId, token;
        std::string payload;
        if (!ReadFrame(ssl, serviceId, serviceHash, methodId, token, payload))
            break;

        if (serviceHash == TEST_KNOWN_SERVICE_HASH && methodId == TEST_KNOWN_METHOD_ID)
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 0, std::string());
        else
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 5, std::string());
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    descriptor.CloseSocket();
    serverDone.Set();
}

struct TestFixture
{
    std::unique_ptr<IO::IoContext> ctx;
    std::thread ioThread;
    std::unique_ptr<IO::Networking::AsyncSocketAcceptor> acceptor;
    SelfSignedCert serverCert;
    SSL_CTX* serverCtx = nullptr;
    std::thread serverThread;
    WaitFlag serverDone;
    uint16_t port;

    explicit TestFixture(uint16_t port_) : port(port_)
    {
        ctx = IO::IoContext::CreateIoContext();
        REQUIRE(ctx != nullptr);
        ioThread = std::thread([this]() { ctx->RunUntilShutdown(); });

        serverCert = GenerateSelfSignedCert();
        serverCtx = SSL_CTX_new(TLS_server_method());
        REQUIRE(serverCtx != nullptr);
        REQUIRE(SSL_CTX_use_certificate(serverCtx, serverCert.cert) == 1);
        REQUIRE(SSL_CTX_use_PrivateKey(serverCtx, serverCert.key) == 1);

        acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", port);
        REQUIRE(acceptor != nullptr);

        acceptor->AutoAcceptSocketsUntilClose([this](IO::Networking::SocketDescriptor descriptor)
        {
            serverThread = std::thread([this, descriptor = std::move(descriptor)]() mutable
            {
                RunFakeBnetServer(std::move(descriptor), serverCtx, serverDone);
            });
        });
    }

    std::shared_ptr<BotBnetConnection> ConnectClient()
    {
        auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
        REQUIRE(ip.has_value());
        IO::Networking::IpEndpoint endpoint(*ip, port);

        auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
        REQUIRE(maybeDescriptor.has_value());

        IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
        return BotBnetConnection::Create(std::move(clientSocket));
    }

    ~TestFixture()
    {
        REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));
        if (serverThread.joinable())
            serverThread.join();
        acceptor->ClosePortAndStopAcceptingNewConnections();
        ctx->Shutdown();
        ioThread.join();
        SSL_CTX_free(serverCtx);
    }
};
}

TEST_CASE("BotBnetConnection: a known call gets an Ok response", "[botbnetconnection]")
{
    TestFixture fixture(28521);
    std::shared_ptr<BotBnetConnection> client = fixture.ConnectClient();

    WaitFlag connected;
    client->Connect([&](bool success) { CHECK(success); connected.Set(); });
    REQUIRE(connected.WaitFor(std::chrono::seconds(5)));

    WaitFlag done;
    bool callSuccess = false;
    uint32 status = 0xFFFFFFFF;
    bgs::protocol::NoData request;
    client->Call(TEST_KNOWN_SERVICE_HASH, TEST_KNOWN_METHOD_ID, request, [&](bool success, uint32 st, std::string)
    {
        callSuccess = success;
        status = st;
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK(callSuccess);
    CHECK(status == 0);

    client->Close();
}

TEST_CASE("BotBnetConnection: an unknown call gets RpcNotImplemented", "[botbnetconnection]")
{
    TestFixture fixture(28522);
    std::shared_ptr<BotBnetConnection> client = fixture.ConnectClient();

    WaitFlag connected;
    client->Connect([&](bool success) { CHECK(success); connected.Set(); });
    REQUIRE(connected.WaitFor(std::chrono::seconds(5)));

    WaitFlag done;
    uint32 status = 0;
    bgs::protocol::NoData request;
    client->Call(0xDEADBEEF, 99, request, [&](bool, uint32 st, std::string)
    {
        status = st;
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK(status == 5); // RpcNotImplemented

    client->Close();
}

TEST_CASE("BotBnetConnection: a push frame reaches the push handler", "[botbnetconnection]")
{
    TestFixture fixture(28523);
    std::shared_ptr<BotBnetConnection> client = fixture.ConnectClient();

    WaitFlag pushed;
    uint32 pushedHash = 0, pushedMethod = 0;
    std::string pushedPayload;
    client->SetPushHandler([&](uint32 serviceHash, uint32 methodId, std::string const& payload)
    {
        pushedHash = serviceHash;
        pushedMethod = methodId;
        pushedPayload = payload;
        pushed.Set();
    });

    WaitFlag connected;
    client->Connect([&](bool success) { CHECK(success); connected.Set(); });
    REQUIRE(connected.WaitFor(std::chrono::seconds(5)));

    REQUIRE(pushed.WaitFor(std::chrono::seconds(5)));
    CHECK(pushedHash == TEST_PUSH_SERVICE_HASH);
    CHECK(pushedMethod == TEST_PUSH_METHOD_ID);
    CHECK(pushedPayload == TEST_PUSH_PAYLOAD);

    client->Close();
}
