#include "BnetServices.h"
#include "BnetLoginPortal.h"
#include "BnetLoginTickets.h"
#include "BnetRealmList.h"
#include "BnetRpcSession.h"
#include "Bnet/BnetClientBuild.h"
#include "Bnet/BnetRealmListJson.h"
#include "Bnet/BnetRpcErrors.h"
#include "Bnet/BnetServiceHash.h"
#include "Crypto/BigNumber.h"
#include "Log.h"
#include "Util.h"

#include "account_service.pb.h"
#include "authentication_service.pb.h"
#include "challenge_service.pb.h"
#include "connection_service.pb.h"
#include "game_utilities_service.pb.h"

#include <chrono>
#include <cstring>
#include <functional>
#include <set>
#include <unordered_map>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

namespace
{
using ServiceHandler = std::function<uint32(BnetRpcSession&, std::string const&, std::string&)>;

// "WoW" as the fourcc the client identifies its game by.
constexpr uint32 PROGRAM_WOW = 0x576F57;
constexpr uint64 ACCOUNT_ID_HIGH = 0x100000000000000ull;
constexpr uint64 GAME_ACCOUNT_ID_HIGH = 0x200000200576F57ull;

// Opaque cache tags the client stores next to account state; it never checks their value.
constexpr uint32 TAG_PRIVACY_INFO = 0xD7CA834D;
constexpr uint32 TAG_GAME_LEVEL_INFO = 0x5C46D483;
constexpr uint32 TAG_GAME_STATUS = 0x98B75F99;

constexpr char const* COMMAND_PREFIX = "Command_";

template <typename T>
bool Parse(T& message, std::string const& payload)
{
    return message.ParseFromString(payload);
}

bool IsValidPlatform(std::string const& platform)
{
    return platform == "Win" || platform == "Wn64" || platform == "Mc64" || platform == "MacA";
}

bool IsValidLocale(std::string const& locale)
{
    static std::set<std::string> const locales = {
        "enUS", "enGB", "koKR", "frFR", "deDE", "zhCN", "zhTW", "esES", "esMX", "ruRU", "ptBR", "itIT",
    };
    return locales.count(locale) != 0;
}

uint8 LocaleIndex(std::string const& locale)
{
    static std::unordered_map<std::string, uint8> const indexes = {
        { "enUS", LOCALE_enUS }, { "enGB", LOCALE_enUS }, { "koKR", LOCALE_koKR }, { "frFR", LOCALE_frFR },
        { "deDE", LOCALE_deDE }, { "zhCN", LOCALE_zhCN }, { "zhTW", LOCALE_zhTW }, { "esES", LOCALE_esES },
        { "esMX", LOCALE_esMX },
    };
    auto itr = indexes.find(locale);
    return itr != indexes.end() ? itr->second : uint8(LOCALE_enUS);
}

// "Command_RealmListRequest_v1_c1" and "..._v1" both name the same command; the trailing part is the
// client's service-expansion suffix.
std::string ParamName(std::string name)
{
    if (name.compare(0, std::strlen(COMMAND_PREFIX), COMMAND_PREFIX) == 0)
    {
        size_t const pos = name.rfind('_');
        if (pos != std::string::npos)
            name.erase(pos);
    }
    return name;
}

// The attributes of a ClientRequest, by parameter name.
struct RequestParams
{
    std::string command;
    std::unordered_map<std::string, bgs::protocol::Variant const*> values;

    bgs::protocol::Variant const* Find(std::string const& name) const
    {
        auto itr = values.find(name);
        return itr != values.end() ? itr->second : nullptr;
    }
};

void AddBlob(bgs::protocol::game_utilities::v1::ClientResponse& response, char const* name, std::string const& blob)
{
    bgs::protocol::Attribute* attribute = response.add_attribute();
    attribute->set_name(name);
    attribute->mutable_value()->set_blob_value(blob);
}

// -- ConnectionService

uint32 HandleConnect(BnetRpcSession&, std::string const& request, std::string& response)
{
    bgs::protocol::connection::v1::ConnectRequest connectRequest;
    if (!Parse(connectRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    static int64 const startMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int64 const nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    bgs::protocol::connection::v1::ConnectResponse connectResponse;
    if (connectRequest.has_client_id())
        connectResponse.mutable_client_id()->CopyFrom(connectRequest.client_id());
    connectResponse.mutable_server_id()->set_label(uint32(getpid()));
    connectResponse.mutable_server_id()->set_epoch(uint32(startMs / 1000));
    connectResponse.set_server_time(uint64(nowMs));
    connectResponse.set_use_bindless_rpc(connectRequest.use_bindless_rpc());

    connectResponse.SerializeToString(&response);
    return BNET_ERROR_OK;
}

uint32 HandleKeepAlive(BnetRpcSession&, std::string const&, std::string&)
{
    return BNET_ERROR_OK;
}

uint32 HandleRequestDisconnect(BnetRpcSession& session, std::string const& request, std::string&)
{
    bgs::protocol::connection::v1::DisconnectRequest disconnectRequest;
    if (!Parse(disconnectRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    session.AfterResponse([&session, errorCode = disconnectRequest.error_code()]()
    {
        bgs::protocol::connection::v1::DisconnectNotification notification;
        notification.set_error_code(errorCode);
        session.SendPush(uint32(BnetOriginalHash::ConnectionService), BNET_CONNECTION_SERVICE_FORCE_DISCONNECT, notification);
        session.CloseAfterWrites();
    });
    return BNET_ERROR_OK;
}

// -- AuthenticationService

uint32 VerifyLoginTicket(BnetRpcSession& session, std::string const& ticket)
{
    nonstd::optional<uint32> bnetAccountId = sBnetLoginTickets.Verify(ticket);
    if (!bnetAccountId)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: login ticket rejected", session.GetClientIp().c_str());
        return BNET_ERROR_DENIED;
    }

    if (BnetAccountStore::IsIpBanned(session.GetClientIp()))
        return BNET_ERROR_GAME_ACCOUNT_BANNED;

    nonstd::optional<BnetAccount> account = BnetAccountStore::LoadAccount(*bnetAccountId);
    if (!account)
        return BNET_ERROR_DENIED;

    if (account->banned)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: banned account '%s' tried to log in", session.GetClientIp().c_str(), account->login.c_str());
        return account->permanentlyBanned ? BNET_ERROR_GAME_ACCOUNT_BANNED : BNET_ERROR_GAME_ACCOUNT_SUSPENDED;
    }

    bgs::protocol::authentication::v1::LogonResult logonResult;
    logonResult.set_error_code(0);
    logonResult.mutable_account_id()->set_low(account->id);
    logonResult.mutable_account_id()->set_high(ACCOUNT_ID_HIGH);
    for (BnetGameAccount const& gameAccount : account->gameAccounts)
    {
        bgs::protocol::EntityId* gameAccountId = logonResult.add_game_account_id();
        gameAccountId->set_low(gameAccount.id);
        gameAccountId->set_high(GAME_ACCOUNT_ID_HIGH);
    }

    BigNumber sessionKey;
    sessionKey.SetRand(64 * 8);
    std::vector<uint8> const keyBytes = sessionKey.AsByteArray(64, false);
    logonResult.set_session_key(keyBytes.data(), keyBytes.size());

    BnetRpcClientState& state = session.GetState();
    state.gameAccount = nullptr;
    state.hasClientSecret = false;
    state.account = std::move(account);
    session.MarkAuthenticated();

    session.AfterResponse([&session, logonResult]()
    {
        session.SendPush(uint32(BnetOriginalHash::AuthenticationListener), uint32(BnetAuthenticationListenerMethods::OnLogonComplete), logonResult);
    });
    return BNET_ERROR_OK;
}

uint32 HandleLogon(BnetRpcSession& session, std::string const& request, std::string&)
{
    bgs::protocol::authentication::v1::LogonRequest logonRequest;
    if (!Parse(logonRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    std::string const& ip = session.GetClientIp();
    if (logonRequest.program() != "WoW")
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: logon for program '%s' refused", ip.c_str(), logonRequest.program().c_str());
        return BNET_ERROR_BAD_PROGRAM;
    }
    if (!IsValidPlatform(logonRequest.platform()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: logon from platform '%s' refused", ip.c_str(), logonRequest.platform().c_str());
        return BNET_ERROR_BAD_PLATFORM;
    }
    if (!IsValidLocale(logonRequest.locale()))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: logon with locale '%s' refused", ip.c_str(), logonRequest.locale().c_str());
        return BNET_ERROR_BAD_LOCALE;
    }
    if (logonRequest.application_version() != int32(BNET_CLIENT_BUILD))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[%s] RPC: logon with client build %d refused (this server speaks %u)", ip.c_str(),
            logonRequest.application_version(), BNET_CLIENT_BUILD);
        return BNET_ERROR_BAD_VERSION;
    }

    BnetRpcClientState& state = session.GetState();
    state.logonReceived = true;
    state.platform = logonRequest.platform();
    state.locale = logonRequest.locale();
    state.build = logonRequest.application_version();

    if (logonRequest.has_cached_web_credentials())
        return VerifyLoginTicket(session, logonRequest.cached_web_credentials());

    // No stored login: point the client at the web login form.
    session.AfterResponse([&session]()
    {
        bgs::protocol::challenge::v1::ChallengeExternalRequest challenge;
        challenge.set_payload_type("web_auth_url");
        challenge.set_payload(BnetLoginPortal::GetLoginFormUrl(session.GetClientIp()));
        session.SendPush(uint32(BnetOriginalHash::ChallengeListener), uint32(BnetChallengeListenerMethods::OnExternalChallenge), challenge);
    });
    return BNET_ERROR_OK;
}

uint32 HandleVerifyWebCredentials(BnetRpcSession& session, std::string const& request, std::string&)
{
    bgs::protocol::authentication::v1::VerifyWebCredentialsRequest verifyRequest;
    if (!Parse(verifyRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;
    if (!verifyRequest.has_web_credentials())
        return BNET_ERROR_DENIED;
    if (!session.GetState().logonReceived)
        return BNET_ERROR_DENIED;

    return VerifyLoginTicket(session, verifyRequest.web_credentials());
}

uint32 HandleGenerateWebCredentials(BnetRpcSession& session, std::string const&, std::string& response)
{
    BnetRpcClientState& state = session.GetState();
    if (!state.IsAuthed())
        return BNET_ERROR_DENIED;

    bgs::protocol::authentication::v1::GenerateWebCredentialsResponse credentials;
    credentials.set_web_credentials(sBnetLoginTickets.Issue(state.account->id));
    credentials.SerializeToString(&response);
    return BNET_ERROR_OK;
}

// -- AccountService

uint32 HandleGetAccountState(BnetRpcSession& session, std::string const& request, std::string& response)
{
    if (!session.GetState().IsAuthed())
        return BNET_ERROR_DENIED;

    bgs::protocol::account::v1::GetAccountStateRequest stateRequest;
    if (!Parse(stateRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    bgs::protocol::account::v1::GetAccountStateResponse stateResponse;
    if (stateRequest.options().field_privacy_info())
    {
        auto* privacy = stateResponse.mutable_state()->mutable_privacy_info();
        privacy->set_is_using_rid(false);
        privacy->set_is_visible_for_view_friends(false);
        privacy->set_is_hidden_from_friend_finder(true);
        stateResponse.mutable_tags()->set_privacy_info_tag(TAG_PRIVACY_INFO);
    }

    stateResponse.SerializeToString(&response);
    return BNET_ERROR_OK;
}

uint32 HandleGetGameAccountState(BnetRpcSession& session, std::string const& request, std::string& response)
{
    BnetRpcClientState& state = session.GetState();
    if (!state.IsAuthed())
        return BNET_ERROR_DENIED;

    bgs::protocol::account::v1::GetGameAccountStateRequest stateRequest;
    if (!Parse(stateRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    BnetGameAccount const* gameAccount = state.account->FindGameAccount(uint32(stateRequest.game_account_id().low()));

    bgs::protocol::account::v1::GetGameAccountStateResponse stateResponse;
    if (stateRequest.options().field_game_level_info())
    {
        if (gameAccount)
        {
            auto* levelInfo = stateResponse.mutable_state()->mutable_game_level_info();
            levelInfo->set_name(gameAccount->name);
            levelInfo->set_program(PROGRAM_WOW);
        }
        stateResponse.mutable_tags()->set_game_level_info_tag(TAG_GAME_LEVEL_INFO);
    }

    if (stateRequest.options().field_game_status())
    {
        auto* status = stateResponse.mutable_state()->mutable_game_status();
        if (gameAccount)
        {
            status->set_is_suspended(gameAccount->banned);
            status->set_is_banned(gameAccount->permanentlyBanned);
            status->set_suspension_expires(uint64(gameAccount->unbanDate) * 1000000);
        }
        status->set_program(PROGRAM_WOW);
        stateResponse.mutable_tags()->set_game_status_tag(TAG_GAME_STATUS);
    }

    stateResponse.SerializeToString(&response);
    return BNET_ERROR_OK;
}

// -- ResourcesService

// The client asks for a content handle early in the character screen; there is nothing to serve, but
// an error answer stops the character list from showing.
uint32 HandleGetContentHandle(BnetRpcSession&, std::string const&, std::string&)
{
    return BNET_ERROR_OK;
}

// -- GameUtilitiesService

uint32 GetRealmListTicket(BnetRpcSession& session, RequestParams const& params, bgs::protocol::game_utilities::v1::ClientResponse& response)
{
    BnetRpcClientState& state = session.GetState();

    BnetGameAccount const* gameAccount = nullptr;
    if (bgs::protocol::Variant const* identity = params.Find("Param_Identity"); identity && identity->has_blob_value())
    {
        if (nonstd::optional<nlohmann::json> json = BnetRealmListJson::DecodePlain(identity->blob_value()))
        {
            try
            {
                gameAccount = state.account->FindGameAccount(json->at("gameAccountID").get<uint32>());
            }
            catch (nlohmann::json::exception const&)
            {
            }
        }
    }
    if (!gameAccount)
        return BNET_ERROR_UTIL_SERVER_INVALID_IDENTITY_ARGS;

    if (gameAccount->permanentlyBanned)
        return BNET_ERROR_GAME_ACCOUNT_BANNED;
    if (gameAccount->banned)
        return BNET_ERROR_GAME_ACCOUNT_SUSPENDED;

    bool haveSecret = false;
    std::array<uint8, 32> secret{};
    uint32 platform = 0, arch = 0, type = 0;
    if (bgs::protocol::Variant const* clientInfo = params.Find("Param_ClientInfo"); clientInfo && clientInfo->has_blob_value())
    {
        if (nonstd::optional<nlohmann::json> json = BnetRealmListJson::DecodePlain(clientInfo->blob_value()))
        {
            try
            {
                nlohmann::json const& info = json->at("info");
                nlohmann::json const& secretJson = info.at("secret");
                if (secretJson.is_array() && secretJson.size() == secret.size())
                {
                    for (size_t i = 0; i < secret.size(); ++i)
                        secret[i] = uint8(secretJson[i].get<int>());
                    haveSecret = true;
                }
                platform = info.value("platformType", 0u);
                arch = info.value("clientArch", 0u);
                type = info.value("type", 0u);
            }
            catch (nlohmann::json::exception const&)
            {
                haveSecret = false;
            }
        }
    }
    if (!haveSecret)
        return BNET_ERROR_WOW_SERVICES_DENIED_REALM_LIST_TICKET;

    state.gameAccount = gameAccount;
    state.clientSecret = secret;
    state.hasClientSecret = true;
    state.clientPlatform = platform;
    state.clientArch = arch;
    state.clientType = type;

    AddBlob(response, "Param_RealmListTicket", std::string("AuthRealmListTicket\0", 20));
    return BNET_ERROR_OK;
}

uint32 GetLastCharPlayed(BnetRpcSession& session, RequestParams const& params, bgs::protocol::game_utilities::v1::ClientResponse& response)
{
    BnetRpcClientState& state = session.GetState();
    if (!state.gameAccount)
        return BNET_ERROR_DENIED;

    bgs::protocol::Variant const* subRegion = params.Find("Command_LastCharPlayedRequest_v1");
    if (!subRegion || !subRegion->has_string_value())
        return BNET_ERROR_UTIL_SERVER_UNKNOWN_REALM;
    if (subRegion->string_value() != sBnetRealmList.GetSubRegionAddress())
        return BNET_ERROR_OK;

    nonstd::optional<BnetAccountStore::LastPlayedCharacter> character = BnetAccountStore::LoadLastPlayedCharacter(
        state.gameAccount->id, sBnetRealmList.GetRegion(), sBnetRealmList.GetBattlegroup());
    if (!character)
        return BNET_ERROR_OK;

    // A realm that is gone or offline gives the client nothing to preselect, so it shows the realm list.
    nonstd::optional<BnetRealm> realm = sBnetRealmList.FindByAddress(sBnetRealmList.GetRealmAddress(character->realmId));
    if (!realm || realm->IsOffline())
        return BNET_ERROR_OK;

    nonstd::optional<std::string> entry = BnetRealmListJson::EncodeCompressed("JamJSONRealmEntry",
        sBnetRealmList.BuildRealmEntry(*realm, state.gameAccount->GetSecurityOn(realm->id)));
    if (!entry)
        return BNET_ERROR_UTIL_SERVER_FAILED_TO_SERIALIZE;

    AddBlob(response, "Param_RealmEntry", *entry);

    bgs::protocol::Attribute* name = response.add_attribute();
    name->set_name("Param_CharacterName");
    name->mutable_value()->set_string_value(character->name);

    uint64 const guid = character->guid;
    AddBlob(response, "Param_CharacterGUID", std::string(reinterpret_cast<char const*>(&guid), sizeof(guid)));

    bgs::protocol::Attribute* lastPlayed = response.add_attribute();
    lastPlayed->set_name("Param_LastPlayedTime");
    lastPlayed->mutable_value()->set_int_value(int64(character->lastPlayedTime));
    return BNET_ERROR_OK;
}

uint32 GetRealmList(BnetRpcSession& session, RequestParams const& params, bgs::protocol::game_utilities::v1::ClientResponse& response)
{
    BnetRpcClientState& state = session.GetState();
    if (!state.gameAccount)
        return BNET_ERROR_DENIED;

    // An empty sub-region asks for every realm.
    std::string subRegion;
    if (bgs::protocol::Variant const* value = params.Find("Command_RealmListRequest_v1"); value && value->has_string_value())
        subRegion = value->string_value();
    bool const allRealms = subRegion.empty() || subRegion == sBnetRealmList.GetSubRegionAddress();

    nlohmann::json updates = nlohmann::json::array();
    nlohmann::json counts = nlohmann::json::array();
    if (allRealms)
    {
        std::map<uint32, uint32> const characterCounts = BnetAccountStore::LoadCharacterCounts(state.gameAccount->id);
        for (BnetRealm const& realm : sBnetRealmList.GetRealms())
        {
            updates.push_back({
                { "update", sBnetRealmList.BuildRealmEntry(realm, state.gameAccount->GetSecurityOn(realm.id)) },
                { "deleting", false },
            });

            auto count = characterCounts.find(realm.id);
            counts.push_back({
                { "wowRealmAddress", sBnetRealmList.GetRealmAddress(realm.id) },
                { "count", count != characterCounts.end() ? count->second : 0u },
            });
        }
    }

    nonstd::optional<std::string> realmList = BnetRealmListJson::EncodeCompressed("JSONRealmListUpdates", { { "updates", updates } });
    nonstd::optional<std::string> characterCounts = BnetRealmListJson::EncodeCompressed("JSONRealmCharacterCountList", { { "counts", counts } });
    if (!realmList || !characterCounts)
        return BNET_ERROR_UTIL_SERVER_FAILED_TO_SERIALIZE;

    AddBlob(response, "Param_RealmList", *realmList);
    AddBlob(response, "Param_CharacterCountList", *characterCounts);
    return BNET_ERROR_OK;
}

uint32 JoinRealm(BnetRpcSession& session, RequestParams const& params, bgs::protocol::game_utilities::v1::ClientResponse& response)
{
    BnetRpcClientState& state = session.GetState();
    if (!state.gameAccount || !state.hasClientSecret)
        return BNET_ERROR_DENIED;

    bgs::protocol::Variant const* address = params.Find("Param_RealmAddress");
    if (!address || !address->has_uint_value())
        return BNET_ERROR_UTIL_SERVER_UNKNOWN_REALM;

    nonstd::optional<BnetRealm> realm = sBnetRealmList.FindByAddress(uint32(address->uint_value()));
    if (!realm)
        return BNET_ERROR_UTIL_SERVER_UNKNOWN_REALM;
    if (realm->IsOffline() || state.gameAccount->GetSecurityOn(realm->id) < realm->allowedSecurityLevel)
        return BNET_ERROR_USER_SERVER_NOT_PERMITTED_ON_REALM;

    nonstd::optional<IO::Networking::IpAddress> clientIp = IO::Networking::IpAddress::TryParseFromString(session.GetClientIp());
    IO::Networking::IpAddress const& realmAddress = clientIp ? realm->GetAddressForClient(*clientIp) : realm->externalAddress;

    nlohmann::json serverAddresses = {
        { "families", nlohmann::json::array({
            {
                { "family", realmAddress.GetType() == IO::Networking::IpAddress::Type::IPv6 ? 2 : 1 },
                { "addresses", nlohmann::json::array({ { { "ip", realmAddress.ToString() }, { "port", realm->port } } }) },
            },
        }) },
    };
    nonstd::optional<std::string> serverAddressesBlob = BnetRealmListJson::EncodeCompressed("JSONRealmListServerIPAddresses", serverAddresses);
    if (!serverAddressesBlob)
        return BNET_ERROR_UTIL_SERVER_FAILED_TO_SERIALIZE;

    BigNumber serverSecretBn;
    serverSecretBn.SetRand(32 * 8);
    std::vector<uint8> const serverSecret = serverSecretBn.AsByteArray(32, false);

    std::vector<uint8> sessionKey(state.clientSecret.begin(), state.clientSecret.end());
    sessionKey.insert(sessionKey.end(), serverSecret.begin(), serverSecret.end());

    if (!BnetAccountStore::StoreJoinInfo(state.gameAccount->id, ByteArrayToHexStr(sessionKey.data(), uint32(sessionKey.size())),
        session.GetClientIp(), LocaleIndex(state.locale), state.platform))
    {
        return BNET_ERROR_INTERNAL;
    }

    nlohmann::json joinTicket = {
        { "gameAccount", state.gameAccount->name },
        { "platform", state.clientPlatform },
        { "clientArch", state.clientArch },
        { "type", state.clientType },
    };

    AddBlob(response, "Param_RealmJoinTicket", joinTicket.dump());
    AddBlob(response, "Param_ServerAddresses", *serverAddressesBlob);
    AddBlob(response, "Param_JoinSecret", std::string(reinterpret_cast<char const*>(serverSecret.data()), serverSecret.size()));
    return BNET_ERROR_OK;
}

uint32 HandleProcessClientRequest(BnetRpcSession& session, std::string const& request, std::string& response)
{
    if (!session.GetState().IsAuthed())
        return BNET_ERROR_DENIED;

    bgs::protocol::game_utilities::v1::ClientRequest clientRequest;
    if (!Parse(clientRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    RequestParams params;
    for (bgs::protocol::Attribute const& attribute : clientRequest.attribute())
    {
        std::string const name = ParamName(attribute.name());
        if (name.compare(0, std::strlen(COMMAND_PREFIX), COMMAND_PREFIX) == 0 && params.command.empty())
            params.command = name;
        params.values[name] = &attribute.value();
    }

    if (params.command.empty())
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] RPC: client request without a command", session.GetClientIp().c_str());
        return BNET_ERROR_RPC_MALFORMED_REQUEST;
    }

    bgs::protocol::game_utilities::v1::ClientResponse clientResponse;
    uint32 status;
    if (params.command == "Command_RealmListTicketRequest_v1")
        status = GetRealmListTicket(session, params, clientResponse);
    else if (params.command == "Command_LastCharPlayedRequest_v1")
        status = GetLastCharPlayed(session, params, clientResponse);
    else if (params.command == "Command_RealmListRequest_v1")
        status = GetRealmList(session, params, clientResponse);
    else if (params.command == "Command_RealmJoinRequest_v1")
        status = JoinRealm(session, params, clientResponse);
    else
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "[%s] RPC: unknown client request command '%s'", session.GetClientIp().c_str(), params.command.c_str());
        return BNET_ERROR_RPC_NOT_IMPLEMENTED;
    }

    if (status == BNET_ERROR_OK)
        clientResponse.SerializeToString(&response);
    return status;
}

uint32 HandleGetAllValuesForAttribute(BnetRpcSession& session, std::string const& request, std::string& response)
{
    if (!session.GetState().IsAuthed())
        return BNET_ERROR_DENIED;

    bgs::protocol::game_utilities::v1::GetAllValuesForAttributeRequest valuesRequest;
    if (!Parse(valuesRequest, request))
        return BNET_ERROR_RPC_MALFORMED_REQUEST;

    if (ParamName(valuesRequest.attribute_key()) != "Command_RealmListRequest_v1")
        return BNET_ERROR_RPC_NOT_IMPLEMENTED;

    bgs::protocol::game_utilities::v1::GetAllValuesForAttributeResponse valuesResponse;
    valuesResponse.add_attribute_value()->set_string_value(sBnetRealmList.GetSubRegionAddress());
    valuesResponse.SerializeToString(&response);
    return BNET_ERROR_OK;
}

uint64 Key(BnetOriginalHash service, uint32 method)
{
    return (uint64(service) << 32) | method;
}

std::unordered_map<uint64, ServiceHandler> const& Handlers()
{
    static std::unordered_map<uint64, ServiceHandler> const handlers = {
        { Key(BnetOriginalHash::ConnectionService, uint32(BnetConnectionServiceMethods::Connect)), HandleConnect },
        { Key(BnetOriginalHash::ConnectionService, uint32(BnetConnectionServiceMethods::KeepAlive)), HandleKeepAlive },
        { Key(BnetOriginalHash::ConnectionService, uint32(BnetConnectionServiceMethods::RequestDisconnect)), HandleRequestDisconnect },
        { Key(BnetOriginalHash::AuthenticationService, uint32(BnetAuthenticationServiceMethods::Logon)), HandleLogon },
        { Key(BnetOriginalHash::AuthenticationService, uint32(BnetAuthenticationServiceMethods::VerifyWebCredentials)), HandleVerifyWebCredentials },
        { Key(BnetOriginalHash::AuthenticationService, uint32(BnetAuthenticationServiceMethods::GenerateWebCredentials)), HandleGenerateWebCredentials },
        { Key(BnetOriginalHash::AccountService, uint32(BnetAccountServiceMethods::GetAccountState)), HandleGetAccountState },
        { Key(BnetOriginalHash::AccountService, uint32(BnetAccountServiceMethods::GetGameAccountState)), HandleGetGameAccountState },
        { Key(BnetOriginalHash::GameUtilitiesService, uint32(BnetGameUtilitiesServiceMethods::ProcessClientRequest)), HandleProcessClientRequest },
        { Key(BnetOriginalHash::GameUtilitiesService, uint32(BnetGameUtilitiesServiceMethods::GetAllValuesForAttribute)), HandleGetAllValuesForAttribute },
        { Key(BnetOriginalHash::ResourcesService, uint32(BnetResourcesServiceMethods::GetContentHandle)), HandleGetContentHandle },
    };
    return handlers;
}
}

uint32 BnetServices::Dispatch(BnetRpcSession& session, uint32 serviceHash, uint32 methodId, std::string const& request, std::string& response)
{
    auto handlers = Handlers().find((uint64(serviceHash) << 32) | methodId);
    if (handlers == Handlers().end())
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_DEBUG, "[%s] RPC: no handler for service 0x%08X method %u", session.GetClientIp().c_str(), serviceHash, methodId);
        return BNET_ERROR_RPC_NOT_IMPLEMENTED;
    }
    return handlers->second(session, request, response);
}
