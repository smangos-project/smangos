#include "BnetLoginPortal.h"
#include "Auth/LoginThrottle.h"
#include "BnetAccountStore.h"
#include "BnetLoginTickets.h"
#include "Bnet/BnetLoginForms.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Encoding/Base64.h"
#include "Crypto/Hash/HMACSHA256.h"
#include "Log.h"
#include "Util.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace
{
BnetPortalSettings g_settings;

constexpr char const* SESSION_COOKIE = "JSESSIONID";
constexpr time_t SESSION_IDLE_SECONDS = 10 * 60;
constexpr size_t MAX_SESSIONS = 10000;

// What the portal remembers between an SRP challenge and the proof that answers it.
struct PortalSession
{
    std::unique_ptr<BnetSRP6> srp;
    std::string login;
    uint32 bnetAccountId = 0; // 0 when the challenge was for an account that does not exist
    time_t lastUsed = 0;
};

std::mutex g_sessionsLock;
std::unordered_map<std::string, std::shared_ptr<PortalSession>> g_sessions;

// What a handler sees of the request that reached it.
struct PortalContext
{
    std::string clientIp;
    std::string sessionId;                        // cookie the request carried, if it matched a session
    std::shared_ptr<PortalSession> session;       // that session
    std::string newSessionId;                     // set when the handler started a session
};

std::string CookieValue(HttpRequest const& request, std::string const& name)
{
    nonstd::optional<std::string> header = HttpMessage::FindHeader(request.Headers, "cookie");
    if (!header)
        return std::string();

    size_t pos = 0;
    while (pos < header->size())
    {
        size_t const end = std::min(header->find(';', pos), header->size());
        size_t const start = header->find_first_not_of(' ', pos);
        if (start != std::string::npos && start < end)
        {
            size_t const equals = header->find('=', start);
            if (equals != std::string::npos && equals < end && header->compare(start, equals - start, name) == 0)
                return header->substr(equals + 1, end - equals - 1);
        }
        pos = end + 1;
    }
    return std::string();
}

std::shared_ptr<PortalSession> FindSession(std::string const& id)
{
    if (id.empty())
        return nullptr;

    std::lock_guard<std::mutex> lock(g_sessionsLock);
    auto itr = g_sessions.find(id);
    if (itr == g_sessions.end() || itr->second->lastUsed + SESSION_IDLE_SECONDS < time(nullptr))
        return nullptr;
    itr->second->lastUsed = time(nullptr);
    return itr->second;
}

std::string CreateSession(std::shared_ptr<PortalSession> const& session)
{
    BigNumber random;
    random.SetRand(16 * 8);
    std::string id = ByteArrayToHexStr(random.AsByteArray(16, false).data(), 16);

    session->lastUsed = time(nullptr);

    std::lock_guard<std::mutex> lock(g_sessionsLock);
    if (g_sessions.size() >= MAX_SESSIONS)
        g_sessions.erase(g_sessions.begin());
    g_sessions[id] = session;
    return id;
}

// Answers for accounts that do not exist look like answers for accounts that do: a stable salt
// derived from the login and a verifier nobody has the password for.
std::vector<uint8> g_fakeSaltKey;
std::vector<uint8> g_fakeVerifier;

std::string HexEncode(std::vector<uint8> const& bytes)
{
    return ByteArrayToHexStr(bytes.data(), uint32(bytes.size()));
}

std::vector<uint8> HexDecode(std::string const& hex)
{
    if (hex.empty() || (hex.size() % 2) != 0 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
        return {};
    std::vector<uint8> out(hex.size() / 2);
    HexStrToByteArray(hex, out.data());
    return out;
}

BnetHttpResponse JsonResponse(nlohmann::json const& body, int status = 200, char const* reason = "OK")
{
    BnetHttpResponse response;
    response.status = status;
    response.reason = reason;
    response.body = body.dump();
    return response;
}

BnetHttpResponse LoginResultDone()
{
    return JsonResponse({ { "authentication_state", "DONE" } });
}

BnetHttpResponse UnableToDecode()
{
    return JsonResponse({
        { "authentication_state", "LOGIN" },
        { "error_code", "UNABLE_TO_DECODE" },
        { "error_message", "There was an internal error while connecting to Battle.net. Please try again later." },
    }, 400, "Bad Request");
}

BnetHttpResponse Unauthorized()
{
    BnetHttpResponse response;
    response.status = 401;
    response.reason = "Unauthorized";
    response.headers.push_back({ "WWW-Authenticate", "Basic realm=\"Battle.net\"" });
    return response;
}

std::string FindInput(std::vector<std::pair<std::string, std::string>> const& inputs, std::string const& id)
{
    for (auto const& [inputId, value] : inputs)
        if (inputId == id)
            return value;
    return std::string();
}

// The login ticket carried as HTTP Basic credentials ("ticket:"), for the endpoints that act on an
// already-logged-in client.
std::string ExtractTicket(HttpRequest const& request)
{
    nonstd::optional<std::string> authorization = HttpMessage::FindHeader(request.Headers, "authorization");
    if (!authorization)
        return std::string();

    std::string encoded = *authorization;
    if (encoded.compare(0, 6, "Basic ") == 0)
        encoded.erase(0, 6);

    nonstd::optional<std::vector<uint8>> decoded = Base64::Decode(encoded);
    if (!decoded)
        return std::string();

    std::string credentials(decoded->begin(), decoded->end());
    return credentials.substr(0, credentials.find(':'));
}

bool IsLoopback(std::string const& ip)
{
    return ip.compare(0, 4, "127.") == 0 || ip == "[::1]" || ip == "::1";
}

std::vector<uint8> FakeSalt(std::string const& login)
{
    HMACSHA256::Generator hmac(g_fakeSaltKey);
    hmac.UpdateData(login);
    HMACSHA256::Digest const digest = hmac.GetDigest();
    return std::vector<uint8>(digest.begin(), digest.end());
}

BnetHttpResponse HandleGetForm(HttpRequest const&, PortalContext& context)
{
    nlohmann::json form = {
        { "type", "LOGIN_FORM" },
        { "inputs", nlohmann::json::array({
            { { "input_id", "account_name" }, { "type", "text" }, { "label", "Account name" }, { "max_length", 320 } },
            { { "input_id", "password" }, { "type", "password" }, { "label", "Password" }, { "max_length", 128 } },
            { { "input_id", "log_in_submit" }, { "type", "submit" }, { "label", "Log In" } },
        }) },
        { "srp_url", "https://" + BnetLoginPortal::GetHostnameForClient(context.clientIp) + ":" + std::to_string(g_settings.restPort) + "/bnetserver/login/srp/" },
    };
    return JsonResponse(form);
}

BnetHttpResponse HandleGetPortal(HttpRequest const&, PortalContext& context)
{
    BnetHttpResponse response;
    response.contentType = "text/plain";
    response.body = BnetLoginPortal::GetHostnameForClient(context.clientIp) + ":" + std::to_string(g_settings.rpcPort);
    return response;
}

BnetHttpResponse HandleGetGameAccounts(HttpRequest const& request, PortalContext&)
{
    std::string const ticket = ExtractTicket(request);
    if (ticket.empty())
        return Unauthorized();

    nlohmann::json list = { { "game_accounts", nlohmann::json::array() } };
    if (nonstd::optional<uint32> bnetAccountId = sBnetLoginTickets.Verify(ticket))
    {
        if (nonstd::optional<BnetAccount> account = BnetAccountStore::LoadAccount(*bnetAccountId))
        {
            for (BnetGameAccount const& gameAccount : account->gameAccounts)
            {
                nlohmann::json entry = {
                    { "display_name", gameAccount.name },
                    { "expansion", gameAccount.expansion },
                };
                if (gameAccount.banned)
                {
                    entry["is_suspended"] = !gameAccount.permanentlyBanned;
                    entry["is_banned"] = gameAccount.permanentlyBanned;
                    entry["suspension_reason"] = gameAccount.banReason;
                    entry["suspension_expires"] = gameAccount.unbanDate;
                }
                list["game_accounts"].push_back(std::move(entry));
            }
        }
    }
    return JsonResponse(list);
}

BnetHttpResponse HandleRefreshLoginTicket(HttpRequest const& request, PortalContext&)
{
    std::string const ticket = ExtractTicket(request);
    if (ticket.empty())
        return Unauthorized();

    nlohmann::json result;
    if (nonstd::optional<time_t> expiry = sBnetLoginTickets.Refresh(ticket))
        result["login_ticket_expiry"] = uint64(*expiry);
    else
    {
        result["login_ticket_expiry"] = 0;
        result["is_expired"] = true;
    }
    return JsonResponse(result);
}

BnetHttpResponse HandleSrpChallenge(HttpRequest const& request, PortalContext& context)
{
    auto inputs = BnetLoginForms::Parse(request.Body);
    if (!inputs)
        return UnableToDecode();

    std::string login = FindInput(*inputs, "account_name");
    if (login.empty() || login.size() > 320 || !BnetAccountStore::NormalizeLogin(login))
        return UnableToDecode();

    // A session already carrying this request's cookie is reused; otherwise this starts one.
    std::shared_ptr<PortalSession> session = context.session ? context.session : std::make_shared<PortalSession>();
    session->srp.reset();
    session->login = login;
    session->bnetAccountId = 0;

    std::vector<uint8> salt;
    std::vector<uint8> verifier;
    if (nonstd::optional<BnetSrpCredentials> credentials = BnetAccountStore::LoadSrpCredentials(login))
    {
        if (credentials->srpVersion == 1)
        {
            session->bnetAccountId = credentials->bnetAccountId;
            salt = std::move(credentials->salt);
            verifier = std::move(credentials->verifier);
        }
    }
    if (!session->bnetAccountId)
    {
        salt = FakeSalt(login);
        verifier = g_fakeVerifier;
    }

    session->srp = std::make_unique<BnetSRP6>();
    session->srp->Initialize(salt, verifier);
    session->srp->CalculateHostPublicEphemeral();

    if (!context.session)
        context.newSessionId = CreateSession(session);

    return JsonResponse({
        { "version", 1 },
        { "iterations", 1 },
        { "modulus", HexEncode(BnetSRP6::GetModulus()) },
        { "generator", HexEncode(BnetSRP6::GetGenerator()) },
        { "hash_function", "SHA-256" },
        { "username", BnetSRP6::ComputeSrpUsername(login) },
        { "salt", HexEncode(session->srp->GetSalt()) },
        { "public_B", HexEncode(session->srp->GetHostPublicEphemeral()) },
    });
}

BnetHttpResponse HandleLogin(HttpRequest const& request, PortalContext& context)
{
    auto inputs = BnetLoginForms::Parse(request.Body);
    if (!inputs)
        return UnableToDecode();

    std::string const& ip = context.clientIp;
    if (GetWrongPasswordAttemptsForIp(ip).IsBeingThrottled())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] Login portal: temporarily locked after repeated failures", ip.c_str());
        return LoginResultDone();
    }

    bool passwordCorrect = false;
    uint32 bnetAccountId = 0;
    std::string serverEvidence;

    std::string loginForLog;
    if (context.session && context.session->srp)
    {
        // Proof for the challenge this session was issued; it answers one challenge only.
        std::unique_ptr<BnetSRP6> srp = std::move(context.session->srp);
        loginForLog = context.session->login;
        std::vector<uint8> const publicA = HexDecode(FindInput(*inputs, "public_A"));
        std::vector<uint8> const clientEvidence = HexDecode(FindInput(*inputs, "client_evidence_M1"));

        if (context.session->bnetAccountId && !publicA.empty() && !clientEvidence.empty() && srp->VerifyClientEvidence(publicA, clientEvidence))
        {
            passwordCorrect = true;
            bnetAccountId = context.session->bnetAccountId;
            serverEvidence = HexEncode(srp->CalculateServerEvidence());
        }
    }
    else if (std::string upperPassword = FindInput(*inputs, "password"); !upperPassword.empty())
    {
        // Plain credentials, for a client that sends the password instead of doing SRP.
        std::string login = FindInput(*inputs, "account_name");
        if (login.empty() || login.size() > 320 || !BnetAccountStore::NormalizeLogin(login) || !BnetAccountStore::NormalizeLogin(upperPassword))
            return UnableToDecode();
        loginForLog = login;

        if (nonstd::optional<BnetSrpCredentials> credentials = BnetAccountStore::LoadSrpCredentials(login))
        {
            BnetSRP6 srp;
            srp.Initialize(credentials->salt, credentials->verifier);
            if (credentials->srpVersion == 1 && srp.CheckCredentials(BnetSRP6::ComputeSrpUsername(login), upperPassword))
            {
                passwordCorrect = true;
                bnetAccountId = credentials->bnetAccountId;
            }
        }
    }

    if (!passwordCorrect)
    {
        RecordWrongPasswordAttempt(ip);
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] Login portal: failed login for '%s'", ip.c_str(), loginForLog.c_str());
        return LoginResultDone();
    }

    ClearWrongPasswordCount(ip);
    BnetAccountStore::StoreLogin(bnetAccountId, ip);

    nlohmann::json result = {
        { "authentication_state", "DONE" },
        { "login_ticket", sBnetLoginTickets.Issue(bnetAccountId) },
    };
    if (!serverEvidence.empty())
        result["server_evidence_M2"] = serverEvidence;
    return JsonResponse(result);
}
}

void BnetLoginPortal::Configure(BnetPortalSettings const& settings)
{
    g_settings = settings;

    BigNumber key;
    key.SetRand(32 * 8);
    g_fakeSaltKey = key.AsByteArray(32, false);

    BnetSRP6 fake;
    fake.CalculateVerifier(BnetSRP6::ComputeSrpUsername("NOBODY"), HexEncode(g_fakeSaltKey));
    g_fakeVerifier = fake.GetVerifier();
}

std::string BnetLoginPortal::GetHostnameForClient(std::string const& clientIp)
{
    return IsLoopback(clientIp) ? g_settings.localHostname : g_settings.externalHostname;
}

std::string BnetLoginPortal::GetLoginFormUrl(std::string const& clientIp)
{
    return "https://" + GetHostnameForClient(clientIp) + ":" + std::to_string(g_settings.restPort) + "/bnetserver/login/";
}

BnetHttpResponse BnetLoginPortal::Handle(HttpRequest const& request, std::string const& clientIp)
{
    std::string const path = request.Line.Path.substr(0, request.Line.Path.find('?'));
    std::string const& method = request.Line.Method;

    PortalContext context;
    context.clientIp = clientIp;
    context.sessionId = CookieValue(request, SESSION_COOKIE);
    context.session = FindSession(context.sessionId);

    BnetHttpResponse response;
    if (method == "GET" && path == "/bnetserver/login/")
        response = HandleGetForm(request, context);
    else if (method == "GET" && path == "/bnetserver/gameAccounts/")
        response = HandleGetGameAccounts(request, context);
    else if (method == "GET" && path == "/bnetserver/portal/")
        response = HandleGetPortal(request, context);
    else if (method == "POST" && path == "/bnetserver/login/")
        response = HandleLogin(request, context);
    else if (method == "POST" && path == "/bnetserver/login/srp/")
        response = HandleSrpChallenge(request, context);
    else if (method == "POST" && path == "/bnetserver/refreshLoginTicket/")
        response = HandleRefreshLoginTicket(request, context);
    else
    {
        response.status = 404;
        response.reason = "Not Found";
    }

    if (!context.newSessionId.empty())
        response.headers.push_back({ "Set-Cookie", std::string(SESSION_COOKIE) + "=" + context.newSessionId + "; Path=/; Secure; HttpOnly" });
    return response;
}

void BnetLoginPortal::CleanupSessions()
{
    time_t const now = time(nullptr);

    std::lock_guard<std::mutex> lock(g_sessionsLock);
    for (auto itr = g_sessions.begin(); itr != g_sessions.end();)
    {
        if (itr->second->lastUsed + SESSION_IDLE_SECONDS < now)
            itr = g_sessions.erase(itr);
        else
            ++itr;
    }
}
