#ifndef BNETSERVER_BNETLOGINPORTAL_H
#define BNETSERVER_BNETLOGINPORTAL_H

#include "Common.h"
#include "Http/HttpRequestParser.h"

#include <string>
#include <vector>

struct BnetHttpResponse
{
    int status = 200;
    std::string reason = "OK";
    std::string contentType = "application/json;charset=utf-8";
    std::vector<HttpMessage::Header> headers;
    std::string body;
};

struct BnetPortalSettings
{
    std::string externalHostname = "127.0.0.1";
    std::string localHostname = "127.0.0.1";
    uint16 restPort = 8081;
    uint16 rpcPort = 1119;
};

// The login portal's endpoints, kept apart from the socket that carries them.
class BnetLoginPortal
{
    public:
        static void Configure(BnetPortalSettings const& settings);

        // The address a client should be told to use to reach this server.
        static std::string GetHostnameForClient(std::string const& clientIp);

        // URL of the login form, as announced over RPC.
        static std::string GetLoginFormUrl(std::string const& clientIp);

        // `clientIp` is the peer's address. Login state that spans requests (the SRP challenge a
        // proof answers) lives in a session tracked by cookie, since the client does not promise to
        // keep one connection between them.
        static BnetHttpResponse Handle(HttpRequest const& request, std::string const& clientIp);

        static void CleanupSessions();
};

#endif
