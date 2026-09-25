// End-to-end coverage for BotLogin: the whole account-name-in, InWorld-out sequence, against a
// combined fake server -- a fake LoginRESTService (SRP6 portal) and a fake Battle.net RPC server
// (both blocking-thread TLS, same shape as BotBnetLoginTests.cpp's, extended here to also parse
// the client's join-secret out of the RealmListTicket request so the world fake server can
// reconstruct the same joinSessionKey), plus a fake WorldSocket (AsyncSocket-based, same shape as
// BotWorldHandshakeTests.cpp / BotCharSelectTests.cpp's, now carrying both the handshake and
// CharSelect/EnteringWorld traffic on one PacketCrypt-locked-step connection, encrypted from
// SMSG_AUTH_RESPONSE onward).
//
// Every AsyncSocket::Read/Write completion below is written inline, nested directly in the
// TEST_CASE body, and every nested lambda that calls a method on `serverSocket` captures it by
// value explicitly (never a bare [&]) -- see BotCharSelectTests.cpp's header comment for why:
// a bare [&] on an inner completion only captures a *reference* into whatever the *enclosing*
// (often already-freed-by-then) lambda's own closure holds, not an owned copy, which is exactly
// the bug that cost a full debugging session there.
#include "BotLogin.h"
#include "BotTaskRegistry.h"
#include "BuiltinBotTasks.h"
#include "FakeWorldServer.h"
#include "TestBotDriver.h"
#include "Bnet/BnetLoginForms.h"
#include "Bnet/BnetRealmListJson.h"
#include "Bnet/BnetServiceHash.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "Crypto/Ed25519.h"
#include "Crypto/Authentication/WorldHandshakeCrypto.h"
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
#include "Packets/Auth.h"
#include "Packets/Classic/Guid.h"
#include "Packets/CharScreen.h"
#include "Packets/Classic/Character.h"

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
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<unsigned char const*>("botlogin-test"), -1, -1, 0);
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

constexpr char const* TEST_ACCOUNT = "bot1@bots";
constexpr char const* TEST_PASSWORD = "correct-password";
constexpr char const* TEST_LOGIN_TICKET = "test-login-ticket";
constexpr char const* TEST_JOIN_TICKET = "12345";
constexpr char const* TEST_REALM_NAME = "Test Realm";
constexpr uint32 TEST_REALM_ADDRESS = 0x01010001;
constexpr char const* TEST_WORLD_IP = "127.0.0.1";
constexpr char const* TEST_CHARACTER_NAME = "Botto";

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
            if (nonstd::optional<std::vector<std::pair<std::string, std::string>>> parsed = BnetLoginForms::Parse(request.content))
                for (auto const& [id, value] : *parsed)
                    inputs[id] = value;
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

// Reverses BotBnetLogin::RequestRealmListTicket()'s Param_ClientInfo blob
// ("RealmListTicketClientInformation:" + {"info":{"secret":[...]}}), to recover the client's half
// of the join session key -- the real bnetserver stores this the same way (see
// GameUtilitiesService.cpp's RequestRealmListTicket), so the world fake server below can
// reconstruct the exact joinSessionKey WorldHandshakeCrypto needs.
std::vector<uint8> ExtractClientSecret(bgs::protocol::game_utilities::v1::ClientRequest const& req)
{
    for (auto const& attr : req.attribute())
    {
        if (attr.name() != "Param_ClientInfo" || !attr.value().has_blob_value())
            continue;

        nonstd::optional<json> parsed = BnetRealmListJson::DecodePlain(attr.value().blob_value());
        if (!parsed)
            continue;

        try
        {
            std::vector<uint8> secret;
            for (auto const& b : parsed->at("info").at("secret"))
                secret.push_back(uint8(b.get<int>()));
            return secret;
        }
        catch (json::exception const&)
        {
            return {};
        }
    }
    return {};
}

void RunFakeBnetRpcServer(IO::Networking::SocketDescriptor descriptor, SSL_CTX* serverCtx, uint16_t worldPort,
    std::vector<uint8>& outJoinSessionKey, WaitFlag& joinSessionKeyReady, WaitFlag& serverDone)
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

    std::string const serverSecret(32, 'S');

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
                std::vector<uint8> clientSecret = ExtractClientSecret(request);
                REQUIRE(clientSecret.size() == 32);

                outJoinSessionKey = clientSecret;
                outJoinSessionKey.insert(outJoinSessionKey.end(), serverSecret.begin(), serverSecret.end());
                joinSessionKeyReady.Set();

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
                nonstd::optional<std::string> blob = BnetRealmListJson::EncodeCompressed("JSONRealmListUpdates", { { "updates", updates } });
                MANGOS_ASSERT(blob);

                auto* attr = response.add_attribute();
                attr->set_name("Param_RealmList");
                attr->mutable_value()->set_blob_value(*blob);
            }
            else if (HasCommand(request, "Command_RealmJoinRequest_v1_c1"))
            {
                json addresses = {
                    { "families", json::array({
                        { { "family", 1 }, { "addresses", json::array({ { { "ip", TEST_WORLD_IP }, { "port", int32_t(worldPort) } } }) } }
                    }) }
                };
                nonstd::optional<std::string> addressesBlob = BnetRealmListJson::EncodeCompressed("JSONRealmListServerIPAddresses", addresses);
                MANGOS_ASSERT(addressesBlob);

                auto* ticketAttr = response.add_attribute();
                ticketAttr->set_name("Param_RealmJoinTicket");
                ticketAttr->mutable_value()->set_blob_value(TEST_JOIN_TICKET);

                auto* addressesAttr = response.add_attribute();
                addressesAttr->set_name("Param_ServerAddresses");
                addressesAttr->mutable_value()->set_blob_value(*addressesBlob);

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

}

namespace
{
// Notes every stage the bot enters, in order.
class StageRecorder : public BotTask
{
    public:
        char const* GetName() const override { return "stage_recorder"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }
        void OnStageChanged(BotContext&, BotStage, BotStage to) override { Seen.push_back(to); }

        std::vector<BotStage> Seen;
};
}

TEST_CASE("BotLogin: account name and password in, InWorld out", "[botlogin]")
{
    constexpr uint16_t RestPort = 28550;
    constexpr uint16_t RpcPort = 28551;
    constexpr uint16_t WorldPort = 28552;

    ClassicGuid const existingGuid{ 0x555555ull, 0 };

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    SelfSignedCert serverCert = GenerateSelfSignedCert();
    SSL_CTX* serverSslCtx = SSL_CTX_new(TLS_server_method());
    REQUIRE(serverSslCtx != nullptr);
    REQUIRE(SSL_CTX_use_certificate(serverSslCtx, serverCert.cert) == 1);
    REQUIRE(SSL_CTX_use_PrivateKey(serverSslCtx, serverCert.key) == 1);

    // ---- REST fake server ----
    auto restAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", RestPort);
    REQUIRE(restAcceptor != nullptr);
    WaitFlag restServerDone;
    std::thread restServerThread;
    restAcceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        restServerThread = std::thread([&restServerDone, &serverSslCtx, descriptor = std::move(descriptor)]() mutable
        {
            RunFakeLoginRestServer(std::move(descriptor), serverSslCtx, restServerDone);
        });
    });

    // ---- RPC fake server ----
    auto rpcAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", RpcPort);
    REQUIRE(rpcAcceptor != nullptr);
    WaitFlag rpcServerDone;
    WaitFlag joinSessionKeyReady;
    std::vector<uint8> joinSessionKey;
    std::thread rpcServerThread;
    rpcAcceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        rpcServerThread = std::thread([&rpcServerDone, &serverSslCtx, &joinSessionKey, &joinSessionKeyReady, descriptor = std::move(descriptor)]() mutable
        {
            RunFakeBnetRpcServer(std::move(descriptor), serverSslCtx, WorldPort, joinSessionKey, joinSessionKeyReady, rpcServerDone);
        });
    });

    // ---- World fake servers: the Realm, then the Instance its SMSG_CONNECT_TO points at ----
    constexpr uint16_t InstancePort = 28553;
    constexpr uint64 RedirectKey = 0x0123456789ABCDEFull;
    WaitFlag realmSessionReady;
    std::array<uint8, 40> realmSessionKey{};
    FakeWorldRealmResult realmResult;
    FakeWorldInstanceResult instanceResult;

    auto worldRealm = std::make_unique<FakeWorldServer>(ctx.get(), WorldPort, [&](FakeWorldPeer& peer)
    {
        if (!joinSessionKeyReady.WaitFor(std::chrono::seconds(5)))
            return;

        FakeWorldRealmOptions realmOptions;
        realmOptions.JoinSessionKey = joinSessionKey;
        realmOptions.ExpectedTicket = TEST_JOIN_TICKET;
        realmResult = FakeWorldRunRealmHandshake(peer, realmOptions);
        realmSessionKey = realmResult.SessionKey;
        realmSessionReady.Set();
        if (!realmResult.Completed)
            return;

        ClassicEnumCharacters enumRequest;
        if (!peer.Expect(enumRequest))
            return;

        ClassicEnumCharactersResult characters;
        characters.CharacterCount = 1;
        characters.FirstCharacterGuid = existingGuid;
        if (!peer.Send(characters))
            return;

        PlayerLogin playerLogin;
        if (!peer.Expect(playerLogin))
            return;

        ConnectTo connectTo;
        connectTo.AddressIPv4 = { 127, 0, 0, 1 };
        connectTo.Port = InstancePort;
        connectTo.Key = RedirectKey;
        connectTo.ConnectionType = 1;
        if (!peer.Send(connectTo))
            return;

        peer.WaitForClientClose();
    });

    auto worldInstance = std::make_unique<FakeWorldServer>(ctx.get(), InstancePort, [&](FakeWorldPeer& peer)
    {
        if (!realmSessionReady.WaitFor(std::chrono::seconds(5)))
            return;

        FakeWorldInstanceOptions instanceOptions;
        instanceOptions.SessionKey = realmSessionKey;
        instanceOptions.ExpectedPackedKey = RedirectKey;
        instanceResult = FakeWorldRunInstanceHandshake(peer, instanceOptions);
        if (!instanceResult.Completed)
            return;

        LoginVerifyWorld verify;
        verify.MapId = 1;
        if (!peer.Send(verify))
            return;

        peer.WaitForClientClose();
    });

    // ---- The bot itself: a real RealBot running the built-in "login" task ----
    RegisterBuiltinBotTasks();
    {
        TestBotDriver driver(ctx.get());
        StageRecorder* stages = nullptr;
        driver.Query([&](RealBot& bot)
        {
            auto recorder = std::make_unique<StageRecorder>();
            stages = recorder.get();
            bot.GetContext().AddHook(std::move(recorder));

            RealBotLoginOptions& options = bot.GetLoginOptions();
            options.LoginAddress = "127.0.0.1";
            options.RestPort = RestPort;
            options.RpcPort = RpcPort;
            options.AccountName = TEST_ACCOUNT;
            options.Password = TEST_PASSWORD;
            options.RealmName = TEST_REALM_NAME;
            options.ClientBuild = 58797;
            options.CreateIfNone = true;
            options.CharacterName = TEST_CHARACTER_NAME;

            bot.GetBrain().PushTask(sBotTaskRegistry.Create("login", bot.GetContext(), BotTaskArgs()), bot.GetContext());
            return true;
        });

        bool const inWorld = driver.WaitForStage(BotStage::InWorld, std::chrono::seconds(10));
        INFO("last error: " << driver.Info().LastError << ", stage " << BotStageName(driver.Info().Stage));
        REQUIRE(inWorld);

        // Every stage on the way, in order, each seen as it began (the Battle.net ones reported from
        // the IO threads, the world ones by the routines).
        CHECK(driver.Query([&](RealBot&) { return stages->Seen; }) == std::vector<BotStage>{
            BotStage::LoginPortal, BotStage::LoginRpc, BotStage::RealmSelect, BotStage::WorldConnecting,
            BotStage::WorldHandshake, BotStage::CharSelect, BotStage::EnteringWorld, BotStage::InWorld });

        CHECK(driver.Query([&](RealBot& bot)
        {
            return bot.GetIdentity().CharacterGuid.Low == existingGuid.Low && bot.GetIdentity().CharacterName == TEST_CHARACTER_NAME
                && bot.HasConnection(BotConnectionKind::Realm) && bot.HasConnection(BotConnectionKind::Instance);
        }));

        // Close the bot's sockets, which lets both fake world scripts (waiting on the client
        // closing) finish.
        driver.Query([](RealBot& bot) { bot.CloseConnections(); return true; });
        REQUIRE(worldRealm->WaitForFinished(1));
        REQUIRE(worldInstance->WaitForFinished(1));
    }

    CHECK(realmResult.Completed);
    CHECK(realmResult.DigestMatched);
    CHECK(realmResult.TicketMatched);
    INFO("instance error: " << instanceResult.Error);
    CHECK(instanceResult.Completed);
    CHECK(instanceResult.KeyMatched);
    CHECK(instanceResult.DigestMatched);

    worldInstance.reset();
    worldRealm.reset();
    restAcceptor->ClosePortAndStopAcceptingNewConnections();
    rpcAcceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();

    REQUIRE(restServerDone.WaitFor(std::chrono::seconds(5)));
    REQUIRE(rpcServerDone.WaitFor(std::chrono::seconds(5)));
    if (restServerThread.joinable())
        restServerThread.join();
    if (rpcServerThread.joinable())
        rpcServerThread.join();

    SSL_CTX_free(serverSslCtx);
}

TEST_CASE("BotLogin: StopAt LoginPortal holds immediately, without any network call", "[botlogin]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    std::shared_ptr<BotLogin> login = BotLogin::Create(ctx.get());

    BotLogin::Options options;
    // Deliberately unroutable -- StopAt(LoginPortal) must never try to reach it.
    options.LoginAddress = "127.0.0.1";
    options.RestPort = 1; // nothing listens here
    options.RpcPort = 1;
    options.AccountName = TEST_ACCOUNT;
    options.Password = TEST_PASSWORD;
    options.StopAt = BotStage::LoginPortal;

    WaitFlag done;
    BotLogin::Result result;
    login->Login(options, [&](BotLogin::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(1)));
    CHECK(result.Success);
    CHECK(result.Stopped);

    ctx->Shutdown();
    ioThread.join();
}

TEST_CASE("BotLogin: StopAt WorldConnecting holds after RealmSelect, before opening the world socket", "[botlogin]")
{
    constexpr uint16_t RestPort = 28556;
    constexpr uint16_t RpcPort = 28557;
    // Deliberately nothing listens here -- StopAt(WorldConnecting) must never try to reach it.
    constexpr uint16_t UnroutableWorldPort = 1;

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    SelfSignedCert serverCert = GenerateSelfSignedCert();
    SSL_CTX* serverSslCtx = SSL_CTX_new(TLS_server_method());
    REQUIRE(serverSslCtx != nullptr);
    REQUIRE(SSL_CTX_use_certificate(serverSslCtx, serverCert.cert) == 1);
    REQUIRE(SSL_CTX_use_PrivateKey(serverSslCtx, serverCert.key) == 1);

    auto restAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", RestPort);
    REQUIRE(restAcceptor != nullptr);
    WaitFlag restServerDone;
    std::thread restServerThread;
    restAcceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        restServerThread = std::thread([&restServerDone, &serverSslCtx, descriptor = std::move(descriptor)]() mutable
        {
            RunFakeLoginRestServer(std::move(descriptor), serverSslCtx, restServerDone);
        });
    });

    auto rpcAcceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", RpcPort);
    REQUIRE(rpcAcceptor != nullptr);
    WaitFlag rpcServerDone;
    WaitFlag joinSessionKeyReady;
    std::vector<uint8> joinSessionKey;
    std::thread rpcServerThread;
    rpcAcceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        rpcServerThread = std::thread([&rpcServerDone, &serverSslCtx, &joinSessionKey, &joinSessionKeyReady, descriptor = std::move(descriptor)]() mutable
        {
            RunFakeBnetRpcServer(std::move(descriptor), serverSslCtx, UnroutableWorldPort, joinSessionKey, joinSessionKeyReady, rpcServerDone);
        });
    });

    std::shared_ptr<BotLogin> login = BotLogin::Create(ctx.get());

    BotLogin::Options options;
    options.LoginAddress = "127.0.0.1";
    options.RestPort = RestPort;
    options.RpcPort = RpcPort;
    options.AccountName = TEST_ACCOUNT;
    options.Password = TEST_PASSWORD;
    options.RealmName = TEST_REALM_NAME;
    options.ClientBuild = 58797;
    options.StopAt = BotStage::WorldConnecting;

    WaitFlag done;
    BotLogin::Result result;
    login->Login(options, [&](BotLogin::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(10)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    CHECK(result.Stopped);

    restAcceptor->ClosePortAndStopAcceptingNewConnections();
    rpcAcceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();

    REQUIRE(restServerDone.WaitFor(std::chrono::seconds(5)));
    REQUIRE(rpcServerDone.WaitFor(std::chrono::seconds(5)));
    if (restServerThread.joinable())
        restServerThread.join();
    if (rpcServerThread.joinable())
        rpcServerThread.join();

    SSL_CTX_free(serverSslCtx);
}

