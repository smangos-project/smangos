#include "BotBnetLogin.h"
#include "BotConnector.h"
#include "BnetServiceHash.h"
#include "Compression/ZLib.h"
#include "Crypto/BigNumber.h"
#include "Errors.h"
#include "IO/Networking/IpAddress.h"
#include "Log.h"

#include "authentication_service.pb.h"
#include "connection_service.pb.h"
#include "game_utilities_service.pb.h"

#include <nlohmann/json.hpp>
#include <cstring>

namespace
{
constexpr char const* COMMAND_VERSION_SUFFIX = "c1"; // Classic Era (WowServiceExpansion = 1)

void AddStringAttribute(bgs::protocol::game_utilities::v1::ClientRequest& req, std::string const& name, std::string const& value)
{
    auto* attr = req.add_attribute();
    attr->set_name(name);
    attr->mutable_value()->set_string_value(value);
}

void AddBlobAttribute(bgs::protocol::game_utilities::v1::ClientRequest& req, std::string const& name, std::string const& value)
{
    auto* attr = req.add_attribute();
    attr->set_name(name);
    attr->mutable_value()->set_blob_value(value);
}

void AddBoolAttribute(bgs::protocol::game_utilities::v1::ClientRequest& req, std::string const& name, bool value)
{
    auto* attr = req.add_attribute();
    attr->set_name(name);
    attr->mutable_value()->set_bool_value(value);
}

void AddUintAttribute(bgs::protocol::game_utilities::v1::ClientRequest& req, std::string const& name, uint64 value)
{
    auto* attr = req.add_attribute();
    attr->set_name(name);
    attr->mutable_value()->set_uint_value(value);
}

std::string const* FindBlobAttribute(bgs::protocol::game_utilities::v1::ClientResponse const& resp, std::string const& name)
{
    for (auto const& attr : resp.attribute())
        if (attr.name() == name && attr.value().has_blob_value())
            return &attr.value().blob_value();
    return nullptr;
}

// Reverses GameUtilitiesService.cpp's DeflateJson: [u32 LE uncompressedLength]
// [zlib_compress(name + ":" + json.dump() + '\0')]. The name prefix itself isn't validated here
// (the server doesn't validate it on the way in either, see GetRealmListTicket's own comment).
nonstd::optional<nlohmann::json> InflateNamedJson(std::string const& blob)
{
    if (blob.size() < 4)
        return nonstd::nullopt;

    uint32 uncompressedLen;
    std::memcpy(&uncompressedLen, blob.data(), sizeof(uncompressedLen));

    std::vector<uint8> compressed(blob.begin() + 4, blob.end());
    auto decompressed = Compression::ZLib::Decompress(compressed, uncompressedLen);
    if (!decompressed)
        return nonstd::nullopt;

    std::string plain(reinterpret_cast<char const*>(decompressed->data()), decompressed->size());
    size_t colon = plain.find(':');
    if (colon == std::string::npos)
        return nonstd::nullopt;

    std::string jsonPart = plain.substr(colon + 1);
    if (!jsonPart.empty() && jsonPart.back() == '\0')
        jsonPart.pop_back();

    try
    {
        return nlohmann::json::parse(jsonPart);
    }
    catch (nlohmann::json::exception const&)
    {
        return nonstd::nullopt;
    }
}
}

std::shared_ptr<BotBnetLogin> BotBnetLogin::Create(IO::IoContext* ioContext)
{
    return std::shared_ptr<BotBnetLogin>(new BotBnetLogin(ioContext));
}

BotBnetLogin::BotBnetLogin(IO::IoContext* ioContext) : m_ioContext(ioContext)
{
}

void BotBnetLogin::Login(Options options, LoginCallback callback)
{
    m_options = std::move(options);
    m_callback = std::move(callback);
    ConnectRest();
}

void BotBnetLogin::ConnectRest()
{
    auto ip = IO::Networking::IpAddress::TryParseFromString(m_options.LoginAddress);
    if (!ip.has_value())
    {
        Fail("bad LoginAddress '" + m_options.LoginAddress + "'");
        return;
    }
    IO::Networking::IpEndpoint endpoint(*ip, m_options.RestPort);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    BotConnector::ConnectAsync(endpoint, std::chrono::milliseconds(m_options.ConnectTimeoutMs), [self](nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError> descriptor)
    {
        if (!descriptor.has_value())
        {
            self->Fail("REST connect failed: " + descriptor.error().ToString());
            return;
        }

        IO::Networking::AsyncSocket socket(self->m_ioContext, std::move(descriptor.value()));
        self->m_rest = BotRestClient::Create(std::move(socket));
        self->m_rest->Login(self->m_options.AccountName, self->m_options.Password, [self](BotRestClient::LoginResult result)
        {
            self->OnRestLogin(std::move(result));
        });
    });
}

void BotBnetLogin::OnRestLogin(BotRestClient::LoginResult result)
{
    // Nothing after LoginPortal reuses this connection (LoginRpc opens its own to RpcPort), so
    // it's closed here rather than left open until Fail()/Succeed() -- mirrors every test caller
    // of BotRestClient itself closing it once Login()'s callback fires.
    if (m_rest)
        m_rest->Close();

    if (!result.Success)
    {
        Fail("login portal: " + result.Error);
        return;
    }
    m_loginTicket = result.LoginTicket;
    if (m_options.OnStage)
        m_options.OnStage(BotStage::LoginRpc);
    if (m_options.StopAt == BotStage::LoginRpc)
    {
        StopEarly();
        return;
    }
    ConnectRpc();
}

void BotBnetLogin::ConnectRpc()
{
    auto ip = IO::Networking::IpAddress::TryParseFromString(m_options.LoginAddress);
    if (!ip.has_value())
    {
        Fail("bad LoginAddress '" + m_options.LoginAddress + "'");
        return;
    }
    IO::Networking::IpEndpoint endpoint(*ip, m_options.RpcPort);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    BotConnector::ConnectAsync(endpoint, std::chrono::milliseconds(m_options.ConnectTimeoutMs), [self](nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError> descriptor)
    {
        if (!descriptor.has_value())
        {
            self->Fail("RPC connect failed: " + descriptor.error().ToString());
            return;
        }

        IO::Networking::AsyncSocket socket(self->m_ioContext, std::move(descriptor.value()));
        self->m_rpc = BotBnetConnection::Create(std::move(socket));
        self->m_rpc->SetPushHandler([self](uint32 serviceHash, uint32 methodId, std::string const& payload)
        {
            self->OnPush(serviceHash, methodId, payload);
        });
        self->m_rpc->Connect([self](bool success) { self->OnRpcConnected(success); });
    });
}

void BotBnetLogin::OnRpcConnected(bool success)
{
    if (!success)
    {
        Fail("RPC TLS handshake failed");
        return;
    }

    bgs::protocol::connection::v1::ConnectRequest request;
    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::ConnectionService), uint32(BnetConnectionServiceMethods::Connect), request,
        [self](bool success, uint32 status, std::string payload) { self->OnConnectServiceResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnConnectServiceResponse(bool success, uint32 status, std::string)
{
    if (!success || status != 0)
    {
        Fail("ConnectionService.Connect failed (status " + std::to_string(status) + ")");
        return;
    }

    bgs::protocol::authentication::v1::LogonRequest request;
    request.set_program("WoW");
    request.set_platform("Win");
    request.set_locale("enUS");
    request.set_version("1.15.6");
    request.set_application_version(m_options.ClientBuild);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::AuthenticationService), uint32(BnetAuthenticationServiceMethods::Logon), request,
        [self](bool success, uint32 status, std::string payload) { self->OnLogonResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnLogonResponse(bool success, uint32 status, std::string)
{
    if (!success || status != 0)
    {
        Fail("AuthenticationService.Logon failed (status " + std::to_string(status) + ")");
        return;
    }

    bgs::protocol::authentication::v1::VerifyWebCredentialsRequest request;
    request.set_web_credentials(m_loginTicket);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::AuthenticationService), uint32(BnetAuthenticationServiceMethods::VerifyWebCredentials), request,
        [self](bool success, uint32 status, std::string payload) { self->OnVerifyWebCredentialsResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnVerifyWebCredentialsResponse(bool success, uint32 status, std::string)
{
    if (!success || status != 0)
    {
        Fail("AuthenticationService.VerifyWebCredentials failed (status " + std::to_string(status) + ")");
        return;
    }
    m_verifyOk = true;
    MaybeStartRealmSelect();
}

void BotBnetLogin::OnPush(uint32 serviceHash, uint32 methodId, std::string const& payload)
{
    // ChallengeListener.OnExternalChallenge (the "here's the web login URL" push) and anything
    // else land here too and are ignored -- LoginPortal already ran the SRP exchange directly.
    if (serviceHash != uint32(BnetOriginalHash::AuthenticationListener) || methodId != uint32(BnetAuthenticationListenerMethods::OnLogonComplete))
        return;

    bgs::protocol::authentication::v1::LogonResult result;
    if (!result.ParseFromString(payload))
    {
        Fail("malformed LogonResult push");
        return;
    }

    m_logonCompletePushed = true;
    m_logonErrorCode = result.error_code();
    MaybeStartRealmSelect();
}

void BotBnetLogin::MaybeStartRealmSelect()
{
    if (!m_verifyOk || !m_logonCompletePushed)
        return;

    if (m_logonErrorCode != 0)
    {
        Fail("logon failed, error_code " + std::to_string(m_logonErrorCode));
        return;
    }

    if (m_options.OnStage)
        m_options.OnStage(BotStage::RealmSelect);
    if (m_options.StopAt == BotStage::RealmSelect)
    {
        StopEarly();
        return;
    }

    RequestRealmListTicket();
}

void BotBnetLogin::RequestRealmListTicket()
{
    BigNumber secretBn;
    secretBn.SetRand(32 * 8);
    m_clientSecret = secretBn.AsByteArray(32, false);

    nlohmann::json clientInfo;
    clientInfo["info"]["secret"] = nlohmann::json::array();
    for (uint8 b : m_clientSecret)
        clientInfo["info"]["secret"].push_back(int(b));

    bgs::protocol::game_utilities::v1::ClientRequest request;
    AddBlobAttribute(request, "Param_ClientInfo", "RealmListTicketClientInformation:" + clientInfo.dump());
    AddBoolAttribute(request, std::string("Command_RealmListTicketRequest_v1_") + COMMAND_VERSION_SUFFIX, true);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::GameUtilitiesService), uint32(BnetGameUtilitiesServiceMethods::ProcessClientRequest), request,
        [self](bool success, uint32 status, std::string payload) { self->OnRealmListTicketResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnRealmListTicketResponse(bool success, uint32 status, std::string)
{
    if (!success || status != 0)
    {
        Fail("RealmListTicket request failed (status " + std::to_string(status) + ")");
        return;
    }
    RequestRealmList();
}

void BotBnetLogin::RequestRealmList()
{
    bgs::protocol::game_utilities::v1::ClientRequest request;
    AddStringAttribute(request, std::string("Command_RealmListRequest_v1_") + COMMAND_VERSION_SUFFIX, std::string());

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::GameUtilitiesService), uint32(BnetGameUtilitiesServiceMethods::ProcessClientRequest), request,
        [self](bool success, uint32 status, std::string payload) { self->OnRealmListResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnRealmListResponse(bool success, uint32 status, std::string payload)
{
    if (!success || status != 0)
    {
        Fail("RealmList request failed (status " + std::to_string(status) + ")");
        return;
    }

    bgs::protocol::game_utilities::v1::ClientResponse response;
    if (!response.ParseFromString(payload))
    {
        Fail("malformed RealmList response");
        return;
    }

    std::string const* listBlob = FindBlobAttribute(response, "Param_RealmList");
    if (!listBlob)
    {
        Fail("RealmList response missing Param_RealmList");
        return;
    }

    nonstd::optional<nlohmann::json> listJson = InflateNamedJson(*listBlob);
    if (!listJson)
    {
        Fail("RealmList response had an unreadable Param_RealmList blob");
        return;
    }

    uint32 chosenAddress = 0;
    bool found = false;
    try
    {
        for (auto const& update : listJson->at("updates"))
        {
            if (!update.contains("update"))
                continue;
            auto const& realm = update.at("update");

            bool matches;
            if (!m_options.RealmName.empty())
                matches = realm.value("name", std::string()) == m_options.RealmName;
            else if (m_options.RealmId != 0)
                matches = realm.value("cfgRealmsID", 0u) == m_options.RealmId;
            else
                matches = true;

            if (matches)
            {
                chosenAddress = realm.value("wowRealmAddress", 0u);
                found = true;
                break;
            }
        }
    }
    catch (nlohmann::json::exception const&)
    {
        Fail("RealmList response's Param_RealmList had an unexpected shape");
        return;
    }

    if (!found)
    {
        Fail("no matching realm in the realm list");
        return;
    }

    RequestRealmJoin(chosenAddress);
}

void BotBnetLogin::RequestRealmJoin(uint32 realmAddress)
{
    bgs::protocol::game_utilities::v1::ClientRequest request;
    AddUintAttribute(request, "Param_RealmAddress", realmAddress);
    AddBoolAttribute(request, std::string("Command_RealmJoinRequest_v1_") + COMMAND_VERSION_SUFFIX, true);

    std::shared_ptr<BotBnetLogin> self = shared_from_this();
    m_rpc->Call(uint32(BnetOriginalHash::GameUtilitiesService), uint32(BnetGameUtilitiesServiceMethods::ProcessClientRequest), request,
        [self](bool success, uint32 status, std::string payload) { self->OnRealmJoinResponse(success, status, std::move(payload)); });
}

void BotBnetLogin::OnRealmJoinResponse(bool success, uint32 status, std::string payload)
{
    if (!success || status != 0)
    {
        Fail("RealmJoin request failed (status " + std::to_string(status) + ")");
        return;
    }

    bgs::protocol::game_utilities::v1::ClientResponse response;
    if (!response.ParseFromString(payload))
    {
        Fail("malformed RealmJoin response");
        return;
    }

    std::string const* ticketBlob = FindBlobAttribute(response, "Param_RealmJoinTicket");
    std::string const* addressesBlob = FindBlobAttribute(response, "Param_ServerAddresses");
    std::string const* secretBlob = FindBlobAttribute(response, "Param_JoinSecret");
    if (!ticketBlob || !addressesBlob || !secretBlob)
    {
        Fail("RealmJoin response missing an expected attribute");
        return;
    }

    nonstd::optional<nlohmann::json> addressesJson = InflateNamedJson(*addressesBlob);
    if (!addressesJson)
    {
        Fail("RealmJoin response had an unreadable Param_ServerAddresses blob");
        return;
    }

    std::string worldAddress;
    uint16_t worldPort = 0;
    try
    {
        auto const& firstFamily = addressesJson->at("families").at(0);
        auto const& firstAddress = firstFamily.at("addresses").at(0);
        worldAddress = firstAddress.at("ip").get<std::string>();
        worldPort = uint16_t(firstAddress.at("port").get<int>());
    }
    catch (nlohmann::json::exception const&)
    {
        Fail("RealmJoin response's Param_ServerAddresses had an unexpected shape");
        return;
    }

    Result result;
    result.Success = true;
    result.WorldAddress = worldAddress;
    result.WorldPort = worldPort;
    result.RealmJoinTicket = *ticketBlob;
    result.SessionKey = m_clientSecret;
    result.SessionKey.insert(result.SessionKey.end(), secretBlob->begin(), secretBlob->end());

    Succeed(std::move(result));
}

void BotBnetLogin::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotBnetLogin: %s", error.c_str());
    if (m_rpc)
        m_rpc->Close();
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = false;
        result.Error = error;
        callback(std::move(result));
    }
}

void BotBnetLogin::Succeed(Result result)
{
    if (m_rpc)
        m_rpc->Close();
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        callback(std::move(result));
    }
}

void BotBnetLogin::StopEarly()
{
    // Nothing left open on this side worth keeping past LoginPortal/LoginRpc -- REST already
    // closed itself in OnRestLogin, and RPC (if connected at all) has no further use once the
    // caller isn't going to drive RealmSelect.
    if (m_rpc)
        m_rpc->Close();
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.Stopped = true;
        callback(std::move(result));
    }
}
