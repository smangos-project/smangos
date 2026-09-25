#ifndef SHARED_BNETSERVICEHASH_H
#define SHARED_BNETSERVICEHASH_H

#include "Common.h"

// (serviceHash, methodId) constants for Battle.net RPC dispatch. Nothing on the wire carries
// service/method names, so both sides hardcode the numeric values from the .proto
// `option (service_options)`/`(method_options)` annotations. Only the subset the login flow calls
// or listens for.
enum class BnetOriginalHash : uint32
{
    AccountService         = 0x62DA0891u,
    AuthenticationListener = 0x71240E35u,
    AuthenticationService  = 0x0DECFC01u,
    ChallengeListener      = 0xBBDA171Fu,
    ConnectionService      = 0x65446991u,
    GameUtilitiesService   = 0x3FC1274Du,
    ResourcesService       = 0xECBE75BAu,
};

enum class BnetConnectionServiceMethods : uint32
{
    Connect           = 1,
    KeepAlive         = 5,
    RequestDisconnect = 7,
};

// ConnectionService is also client-exported: the server pushes this to announce a disconnect.
constexpr uint32 BNET_CONNECTION_SERVICE_FORCE_DISCONNECT = 4;

enum class BnetAuthenticationServiceMethods : uint32
{
    Logon                    = 1,
    VerifyWebCredentials     = 7,
    GenerateWebCredentials   = 8,
};

// AuthenticationListener is client-exported (the server calls us); the client never calls it.
enum class BnetAuthenticationListenerMethods : uint32
{
    OnLogonComplete = 5,
};

// ChallengeListener is client-exported as well.
enum class BnetChallengeListenerMethods : uint32
{
    OnExternalChallenge = 3,
};

enum class BnetAccountServiceMethods : uint32
{
    GetAccountState     = 30,
    GetGameAccountState = 31,
};

enum class BnetGameUtilitiesServiceMethods : uint32
{
    // Named GenericClientRequest in the RPC dispatch table, ProcessClientRequest in the .proto rpc
    // declaration -- same id, both names appear in the wild.
    ProcessClientRequest     = 1,
    GetAllValuesForAttribute = 10,
};

enum class BnetResourcesServiceMethods : uint32
{
    GetContentHandle = 1,
};

#endif
