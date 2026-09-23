#ifndef REALBOTS_BNETSERVICEHASH_H
#define REALBOTS_BNETSERVICEHASH_H

#include "Common.h"

// Hardcoded (serviceHash, methodId) constants for Battle.net RPC dispatch -- the bot's side of
// vibemangos's src/bnetserver/Services/ServiceHash.h (see that file's own comment: no server or
// real client ever reads the .proto `option (service_options)`/`(method_options)` annotations at
// runtime, everyone just hardcodes the numeric values, matching jimsproxy's
// Framework/Constants/ServiceHash.cs verbatim). Only the subset bot-phase-1.md's LoginRpc/
// RealmSelect steps actually call or listen for.
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
    // Named GenericClientRequest in ServiceHash.h/ProcessClientRequest in the .proto rpc
    // declaration -- same id, both names appear in the references.
    ProcessClientRequest = 1,
};

#endif
