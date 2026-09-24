#ifndef SHARED_BNETSERVICEHASH_H
#define SHARED_BNETSERVICEHASH_H

#include "Common.h"

// (serviceHash, methodId) constants for Battle.net RPC dispatch. Nothing on the wire carries
// service/method names, so both sides hardcode the numeric values from the .proto
// `option (service_options)`/`(method_options)` annotations. Only the subset the bot's login flow
// calls or listens for.
enum class BnetOriginalHash : uint32
{
    AuthenticationListener = 0x71240E35u,
    AuthenticationService  = 0x0DECFC01u,
    ChallengeListener      = 0xBBDA171Fu,
    ConnectionService      = 0x65446991u,
    GameUtilitiesService   = 0x3FC1274Du,
};

enum class BnetConnectionServiceMethods : uint32
{
    Connect = 1,
};

enum class BnetAuthenticationServiceMethods : uint32
{
    Logon                = 1,
    VerifyWebCredentials = 7,
};

// AuthenticationListener is client-exported (the server calls us); the client never calls it.
enum class BnetAuthenticationListenerMethods : uint32
{
    OnLogonComplete = 5,
};

enum class BnetGameUtilitiesServiceMethods : uint32
{
    // Named GenericClientRequest in the RPC dispatch table, ProcessClientRequest in the .proto rpc
    // declaration -- same id, both names appear in the wild.
    ProcessClientRequest = 1,
};

#endif
