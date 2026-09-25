#ifndef BNETSERVER_BNETSERVICES_H
#define BNETSERVER_BNETSERVICES_H

#include "Common.h"

#include <string>

class BnetRpcSession;

// The RPC services the login flow uses, dispatched by (service hash, method id). Every call gets an
// answer: methods and services nobody implements are refused with an error status, never dropped.
class BnetServices
{
    public:
        // Runs the call and fills `response` with the serialized reply body (empty for none). Returns
        // the status for the response header.
        static uint32 Dispatch(BnetRpcSession& session, uint32 serviceHash, uint32 methodId,
            std::string const& request, std::string& response);
};

#endif
