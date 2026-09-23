#include "BotRestClient.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
#include "Errors.h"
#include "Util.h"
#include <catch_amalgamated.hpp>
#include <nlohmann/json.hpp>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

using json = nlohmann::json;

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
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("botrestclient-test"), -1, -1, 0);
    X509_set_issuer_name(out.cert, name);

    MANGOS_ASSERT(X509_sign(out.cert, out.key, EVP_sha256()) > 0);
    return out;
}

std::vector<uint8> HexDecode(std::string const& hex)
{
    if (hex.empty() || (hex.size() % 2) != 0)
        return {};
    std::vector<uint8> out(hex.size() / 2);
    HexStrToByteArray(hex, out.data());
    return out;
}

std::string HexEncode(std::vector<uint8> const& bytes)
{
    std::string hex;
    hexEncodeByteArray(const_cast<uint8*>(bytes.data()), uint32(bytes.size()), hex);
    return hex;
}

bool ReadLine(SSL* ssl, std::string& outLine)
{
    outLine.clear();
    char c;
    for (;;)
    {
        int n = SSL_read(ssl, &c, 1);
        if (n <= 0)
            return !outLine.empty();
        if (c == '\n')
        {
            if (!outLine.empty() && outLine.back() == '\r')
                outLine.pop_back();
            return true;
        }
        outLine.push_back(c);
    }
}

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string content;
};

bool ReadHttpRequest(SSL* ssl, HttpRequest& request)
{
    std::string requestLine;
    if (!ReadLine(ssl, requestLine) || requestLine.empty())
        return false;

    std::istringstream iss(requestLine);
    std::string version;
    if (!(iss >> request.method >> request.path >> version))
        return false;

    size_t contentLength = 0;
    std::string line;
    while (ReadLine(ssl, line) && !line.empty())
    {
        if (line.size() > 15 && _strnicmp(line.c_str(), "Content-Length:", 15) == 0)
        {
            size_t valueStart = line.find_first_not_of(" \t", 15);
            if (valueStart != std::string::npos)
                contentLength = size_t(strtoull(line.c_str() + valueStart, nullptr, 10));
        }
    }

    request.content.assign(contentLength, '\0');
    size_t received = 0;
    while (received < contentLength)
    {
        int n = SSL_read(ssl, &request.content[received], int(contentLength - received));
        if (n <= 0)
            return false;
        received += size_t(n);
    }
    return true;
}

void SendHttpResponse(SSL* ssl, int statusCode, std::string const& body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " Status\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Content-Type: application/json;charset=UTF-8\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n"
        << body;
    std::string out = response.str();
    SSL_write(ssl, out.data(), int(out.size()));
}

bool ParseFormInputs(std::string const& body, std::unordered_map<std::string, std::string>& out)
{
    try
    {
        json request = json::parse(body);
        for (auto const& input : request.at("inputs"))
            out[input.at("input_id").get<std::string>()] = input.at("value").get<std::string>();
        return true;
    }
    catch (json::exception const&)
    {
        return false;
    }
}

// Minimal stand-in for vibemangos's LoginRESTService (src/bnetserver/REST/LoginRESTService.cpp):
// enough of the SRP6 login/srp/ -> login/ exchange to drive BotRestClient end to end, run on a
// raw blocking socket like the reference implementation itself does.
void RunFakeLoginRestServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx,
    std::string const& accountName, std::string const& correctPassword, WaitFlag& serverDone)
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

    std::string srpUsername = BnetSRP6::ComputeSrpUsername(accountName);

    BnetSRP6 srp;
    srp.CalculateVerifier(srpUsername, correctPassword);
    srp.CalculateHostPublicEphemeral();

    for (;;)
    {
        HttpRequest request;
        if (!ReadHttpRequest(ssl, request))
            break;

        if (request.method == "POST" && request.path == "/bnetserver/login/srp/")
        {
            json challenge;
            challenge["version"] = 1;
            challenge["iterations"] = 1;
            challenge["modulus"] = HexEncode(BnetSRP6::GetModulus());
            challenge["generator"] = HexEncode(BnetSRP6::GetGenerator());
            challenge["hash_function"] = "SHA-256";
            challenge["username"] = srpUsername;
            challenge["salt"] = HexEncode(srp.GetSalt());
            challenge["public_B"] = HexEncode(srp.GetHostPublicEphemeral());
            SendHttpResponse(ssl, 200, challenge.dump());
        }
        else if (request.method == "POST" && request.path == "/bnetserver/login/")
        {
            std::unordered_map<std::string, std::string> inputs;
            ParseFormInputs(request.content, inputs);
            std::vector<uint8> A = HexDecode(inputs["public_A"]);
            std::vector<uint8> clientM1 = HexDecode(inputs["client_evidence_M1"]);

            if (A.empty() || clientM1.empty() || !srp.VerifyClientEvidence(A, clientM1))
            {
                json failure;
                failure["authentication_state"] = "LOGIN";
                failure["error_code"] = "UNABLE_TO_DECODE";
                failure["error_message"] = "Unable to validate account name and password";
                SendHttpResponse(ssl, 400, failure.dump());
            }
            else
            {
                json success;
                success["authentication_state"] = "DONE";
                success["login_ticket"] = "test-login-ticket";
                success["server_evidence_M2"] = HexEncode(srp.CalculateServerEvidence());
                SendHttpResponse(ssl, 200, success.dump());
            }
        }
        else
        {
            SendHttpResponse(ssl, 404, "{}");
        }
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
    }

    void Start(std::string const& accountName, std::string const& correctPassword)
    {
        acceptor->AutoAcceptSocketsUntilClose([this, accountName, correctPassword](IO::Networking::SocketDescriptor descriptor)
        {
            serverThread = std::thread([this, descriptor = std::move(descriptor), accountName, correctPassword]() mutable
            {
                RunFakeLoginRestServer(std::move(descriptor), serverCtx, accountName, correctPassword, serverDone);
            });
        });
    }

    std::shared_ptr<BotRestClient> ConnectClient()
    {
        auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
        REQUIRE(ip.has_value());
        IO::Networking::IpEndpoint endpoint(*ip, port);

        auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
        REQUIRE(maybeDescriptor.has_value());

        IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
        return BotRestClient::Create(std::move(clientSocket));
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

TEST_CASE("BotRestClient: logs in with the correct password", "[botrestclient]")
{
    TestFixture fixture(28519);
    fixture.Start("bot1@bots", "correct-password");

    std::shared_ptr<BotRestClient> client = fixture.ConnectClient();

    WaitFlag done;
    BotRestClient::LoginResult result;
    client->Login("bot1@bots", "correct-password", [&](BotRestClient::LoginResult r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK(result.Success);
    CHECK(result.LoginTicket == "test-login-ticket");

    client->Close();
}

TEST_CASE("BotRestClient: a wrong password fails without a login ticket", "[botrestclient]")
{
    TestFixture fixture(28520);
    fixture.Start("bot1@bots", "correct-password");

    std::shared_ptr<BotRestClient> client = fixture.ConnectClient();

    WaitFlag done;
    BotRestClient::LoginResult result;
    client->Login("bot1@bots", "definitely-wrong", [&](BotRestClient::LoginResult r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(result.Success);
    CHECK(result.LoginTicket.empty());
    INFO("server said: " << result.Error);

    client->Close();
}
