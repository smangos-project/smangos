// End-to-end coverage for BotBnetLogin: a fake LoginRESTService (SRP6 portal, same shape as
// BotRestClientTests.cpp's) plus a fake BnetSession-like RPC server (same framing as
// BotBnetConnectionTests.cpp's) answering just enough of ConnectionService/AuthenticationService/
// GameUtilitiesService to drive LoginPortal -> LoginRpc -> RealmSelect through to a world server
// address + session key, the way vibemangos's bnetserver would (see GameUtilitiesService.cpp for
// the exact attribute/JSON shapes this mirrors).
#include "BotBnetLogin.h"
#include "BnetServiceHash.h"
#include "Compression/ZLib.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "Errors.h"
#include "Util.h"
#include <catch_amalgamated.hpp>
#include <nlohmann/json.hpp>

#include "authentication_service.pb.h"
#include "game_utilities_service.pb.h"
#include "rpc_types.pb.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <algorithm>
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

// Mirrors BotRestClient.cpp's own NormalizeAccountString: real account provisioning uppercases
// both the account name and password before computing the verifier, so this fake has to as well
// now that the client normalizes what it sends (see BotRestClientTests.cpp's copy of this).
std::string NormalizeAccountString(std::string const& utf8str)
{
    std::wstring wstr;
    if (!Utf8toWStr(utf8str, wstr))
        return utf8str;
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), wcharToUpperOnlyLatin);
    std::string out;
    if (!WStrToUtf8(wstr, out))
        return utf8str;
    return out;
}

// Same move-only footgun avoidance as the other RealBots TLS test files' copy of this helper.
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
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("botbnetlogin-test"), -1, -1, 0);
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

// [u32 LE uncompressedLength][zlib_compress(name + ":" + json.dump() + '\0')] -- matches
// GameUtilitiesService.cpp's DeflateJson exactly, the counterpart to BotBnetLogin's own
// InflateNamedJson.
std::string DeflateJson(std::string const& name, json const& j)
{
    std::string plain = name + ":" + j.dump() + '\0';
    auto compressed = Compression::ZLib::Compress(reinterpret_cast<uint8 const*>(plain.data()), plain.size());
    MANGOS_ASSERT(compressed);

    uint32 uncompressedLen = uint32(plain.size());
    std::string out(reinterpret_cast<char const*>(&uncompressedLen), sizeof(uncompressedLen));
    out.append(reinterpret_cast<char const*>(compressed->data()), compressed->size());
    return out;
}

// ---- fake LoginRESTService (SRP6 portal), same shape as BotRestClientTests.cpp's ----

bool ReadLineSsl(SSL* ssl, std::string& outLine)
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
    if (!ReadLineSsl(ssl, requestLine) || requestLine.empty())
        return false;

    std::istringstream iss(requestLine);
    std::string version;
    if (!(iss >> request.method >> request.path >> version))
        return false;

    size_t contentLength = 0;
    std::string line;
    while (ReadLineSsl(ssl, line) && !line.empty())
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

constexpr char const* TEST_ACCOUNT = "bot1@bots";
constexpr char const* TEST_PASSWORD = "correct-password";
constexpr char const* TEST_LOGIN_TICKET = "test-login-ticket";
constexpr char const* TEST_JOIN_TICKET = "12345";
constexpr char const* TEST_REALM_NAME = "Test Realm";
constexpr uint32 TEST_REALM_ADDRESS = 0x01010001;
constexpr char const* TEST_WORLD_IP = "127.0.0.1";
constexpr uint16_t TEST_WORLD_PORT = 8129;

void RunFakeLoginRestServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx, WaitFlag& serverDone)
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

    std::string srpUsername = BnetSRP6::ComputeSrpUsername(NormalizeAccountString(TEST_ACCOUNT));

    BnetSRP6 srp;
    srp.CalculateVerifier(srpUsername, NormalizeAccountString(TEST_PASSWORD));
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
                success["login_ticket"] = TEST_LOGIN_TICKET;
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

// ---- fake Battle.net RPC server, same framing as BotBnetConnectionTests.cpp's ----

constexpr uint32 RESPONSE_SERVICE_ID = 0xFE;
constexpr uint32 PUSH_SERVICE_ID = 0;

// See BotBnetConnectionTests.cpp's copy of this helper for why WANT_READ/WANT_WRITE must be
// retried rather than treated as a fatal error, even on a blocking socket.
bool ReadExact(SSL* ssl, void* buffer, size_t size)
{
    char* dst = static_cast<char*>(buffer);
    size_t got = 0;
    while (got < size)
    {
        int n = SSL_read(ssl, dst + got, int(size - got));
        if (n > 0)
        {
            got += size_t(n);
            continue;
        }
        int err = SSL_get_error(ssl, n);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
            continue;
        return false;
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
        if (n > 0)
        {
            sent += size_t(n);
            continue;
        }
        int err = SSL_get_error(ssl, n);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
            continue;
        return false;
    }
    return true;
}

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

bool HasCommand(bgs::protocol::game_utilities::v1::ClientRequest const& req, std::string const& commandName)
{
    for (auto const& attr : req.attribute())
        if (attr.name() == commandName)
            return true;
    return false;
}

void RunFakeBnetRpcServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx, WaitFlag& serverDone)
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

    for (;;)
    {
        uint32 serviceId, serviceHash, methodId, token;
        std::string payload;
        if (!ReadFrame(ssl, serviceId, serviceHash, methodId, token, payload))
            break;

        if (serviceHash == uint32(BnetOriginalHash::ConnectionService) && methodId == uint32(BnetConnectionServiceMethods::Connect))
        {
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 0, std::string());
        }
        else if (serviceHash == uint32(BnetOriginalHash::AuthenticationService) && methodId == uint32(BnetAuthenticationServiceMethods::Logon))
        {
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 0, std::string());
        }
        else if (serviceHash == uint32(BnetOriginalHash::AuthenticationService) && methodId == uint32(BnetAuthenticationServiceMethods::VerifyWebCredentials))
        {
            bgs::protocol::authentication::v1::VerifyWebCredentialsRequest request;
            request.ParseFromString(payload);

            bgs::protocol::authentication::v1::LogonResult logonResult;
            logonResult.set_error_code(request.web_credentials() == TEST_LOGIN_TICKET ? 0 : 1);
            std::string pushPayload;
            logonResult.SerializeToString(&pushPayload);
            WriteFrame(ssl, PUSH_SERVICE_ID, uint32(BnetOriginalHash::AuthenticationListener), uint32(BnetAuthenticationListenerMethods::OnLogonComplete), 1, 0, pushPayload);

            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 0, std::string());
        }
        else if (serviceHash == uint32(BnetOriginalHash::GameUtilitiesService) && methodId == uint32(BnetGameUtilitiesServiceMethods::ProcessClientRequest))
        {
            bgs::protocol::game_utilities::v1::ClientRequest request;
            request.ParseFromString(payload);

            bgs::protocol::game_utilities::v1::ClientResponse response;
            if (HasCommand(request, "Command_RealmListTicketRequest_v1_c1"))
            {
                auto* attr = response.add_attribute();
                attr->set_name("Param_RealmListTicket");
                attr->mutable_value()->set_blob_value("AuthRealmListTicket");
            }
            else if (HasCommand(request, "Command_RealmListRequest_v1_c1"))
            {
                json realm = {
                    { "wowRealmAddress", int32_t(TEST_REALM_ADDRESS) },
                    { "cfgRealmsID", 1 },
                    { "name", TEST_REALM_NAME },
                    { "flags", 0 }
                };
                json updates = json::array({ { { "update", realm }, { "deleting", false } } });
                std::string blob = DeflateJson("JSONRealmListUpdates", { { "updates", updates } });

                auto* attr = response.add_attribute();
                attr->set_name("Param_RealmList");
                attr->mutable_value()->set_blob_value(blob);
            }
            else if (HasCommand(request, "Command_RealmJoinRequest_v1_c1"))
            {
                json addresses = {
                    { "families", json::array({
                        { { "family", 1 }, { "addresses", json::array({ { { "ip", TEST_WORLD_IP }, { "port", int32_t(TEST_WORLD_PORT) } } }) } }
                    }) }
                };
                std::string addressesBlob = DeflateJson("JSONRealmListServerIPAddresses", addresses);

                auto* ticketAttr = response.add_attribute();
                ticketAttr->set_name("Param_RealmJoinTicket");
                ticketAttr->mutable_value()->set_blob_value(TEST_JOIN_TICKET);

                auto* addressesAttr = response.add_attribute();
                addressesAttr->set_name("Param_ServerAddresses");
                addressesAttr->mutable_value()->set_blob_value(addressesBlob);

                std::string serverSecret(32, 'S');
                auto* secretAttr = response.add_attribute();
                secretAttr->set_name("Param_JoinSecret");
                secretAttr->mutable_value()->set_blob_value(serverSecret);
            }

            std::string responsePayload;
            response.SerializeToString(&responsePayload);
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 0, responsePayload);
        }
        else
        {
            WriteFrame(ssl, RESPONSE_SERVICE_ID, serviceHash, methodId, token, 5, std::string()); // RpcNotImplemented
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
    std::unique_ptr<IO::Networking::AsyncSocketAcceptor> restAcceptor;
    std::unique_ptr<IO::Networking::AsyncSocketAcceptor> rpcAcceptor;
    SelfSignedCert serverCert;
    SSL_CTX* serverCtx = nullptr;
    std::thread restServerThread;
    std::thread rpcServerThread;
    WaitFlag restServerDone;
    WaitFlag rpcServerDone;
    uint16_t restPort;
    uint16_t rpcPort;

    TestFixture(uint16_t restPort_, uint16_t rpcPort_) : restPort(restPort_), rpcPort(rpcPort_)
    {
        ctx = IO::IoContext::CreateIoContext();
        REQUIRE(ctx != nullptr);
        ioThread = std::thread([this]() { ctx->RunUntilShutdown(); });

        serverCert = GenerateSelfSignedCert();
        serverCtx = SSL_CTX_new(TLS_server_method());
        REQUIRE(serverCtx != nullptr);
        REQUIRE(SSL_CTX_use_certificate(serverCtx, serverCert.cert) == 1);
        REQUIRE(SSL_CTX_use_PrivateKey(serverCtx, serverCert.key) == 1);

        restAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", restPort);
        REQUIRE(restAcceptor != nullptr);
        rpcAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", rpcPort);
        REQUIRE(rpcAcceptor != nullptr);

        restAcceptor->AutoAcceptSocketsUntilClose([this](IO::Networking::SocketDescriptor descriptor)
        {
            restServerThread = std::thread([this, descriptor = std::move(descriptor)]() mutable
            {
                RunFakeLoginRestServer(std::move(descriptor), serverCtx, restServerDone);
            });
        });
        rpcAcceptor->AutoAcceptSocketsUntilClose([this](IO::Networking::SocketDescriptor descriptor)
        {
            rpcServerThread = std::thread([this, descriptor = std::move(descriptor)]() mutable
            {
                RunFakeBnetRpcServer(std::move(descriptor), serverCtx, rpcServerDone);
            });
        });
    }

    ~TestFixture()
    {
        REQUIRE(restServerDone.WaitFor(std::chrono::seconds(5)));
        REQUIRE(rpcServerDone.WaitFor(std::chrono::seconds(5)));
        if (restServerThread.joinable())
            restServerThread.join();
        if (rpcServerThread.joinable())
            rpcServerThread.join();
        restAcceptor->ClosePortAndStopAcceptingNewConnections();
        rpcAcceptor->ClosePortAndStopAcceptingNewConnections();
        ctx->Shutdown();
        ioThread.join();
        SSL_CTX_free(serverCtx);
    }
};
}

TEST_CASE("BotBnetLogin: a correct login reaches a world address and session key", "[botbnetlogin]")
{
    TestFixture fixture(28524, 28525);

    BotBnetLogin::Options options;
    options.LoginAddress = "127.0.0.1";
    options.RestPort = fixture.restPort;
    options.RpcPort = fixture.rpcPort;
    options.AccountName = TEST_ACCOUNT;
    options.Password = TEST_PASSWORD;
    options.RealmName = TEST_REALM_NAME;
    options.ClientBuild = 58797;

    std::shared_ptr<BotBnetLogin> login = BotBnetLogin::Create(fixture.ctx.get());

    WaitFlag done;
    BotBnetLogin::Result result;
    login->Login(options, [&](BotBnetLogin::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    CHECK(result.WorldAddress == TEST_WORLD_IP);
    CHECK(result.WorldPort == TEST_WORLD_PORT);
    CHECK(result.RealmJoinTicket == TEST_JOIN_TICKET);
    CHECK(result.SessionKey.size() == 64);
}

TEST_CASE("BotBnetLogin: a wrong password fails at the login portal", "[botbnetlogin]")
{
    TestFixture fixture(28526, 28527);

    BotBnetLogin::Options options;
    options.LoginAddress = "127.0.0.1";
    options.RestPort = fixture.restPort;
    options.RpcPort = fixture.rpcPort;
    options.AccountName = TEST_ACCOUNT;
    options.Password = "definitely-wrong";
    options.ClientBuild = 58797;

    std::shared_ptr<BotBnetLogin> login = BotBnetLogin::Create(fixture.ctx.get());

    WaitFlag done;
    BotBnetLogin::Result result;
    login->Login(options, [&](BotBnetLogin::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(result.Success);
    INFO("error: " << result.Error);

    // The RPC server never gets a connection in this path -- close its acceptor's listener
    // manually so the fixture's destructor doesn't hang waiting for a connection that never
    // arrives (RunFakeBnetRpcServer only runs once AutoAcceptSocketsUntilClose sees one).
    fixture.rpcServerDone.Set();
}
