#ifndef REALBOTS_BOTCONNECTOR_H
#define REALBOTS_BOTCONNECTOR_H

#include "IO/Networking/IpAddress.h"
#include "IO/Networking/NetworkError.h"
#include "IO/Networking/SocketDescriptor.h"
#include "nonstd/expected.hpp"
#include <chrono>
#include <functional>

// Runs IO::Networking::SocketConnector::ConnectBlocking (a real blocking DNS+connect call) on
// its own thread, so it never blocks a bot's worker thread or an IO thread (bot-phase-1.md,
// "Net/BotConnector.h/.cpp ConnectBlocking on its own thread"). The callback runs on that same
// connector thread, not the caller's -- same rule as the rest of the IO library ("callbacks
// might not context switch", IO/README.md): the caller must handle the cross-thread hop itself
// (e.g. posting into a bot's thread-safe inbox), this class does not.
class BotConnector
{
    public:
        using ConnectCallback = std::function<void(nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError>)>;

        static void ConnectAsync(IO::Networking::IpEndpoint const& target, std::chrono::milliseconds timeout, ConnectCallback callback);
};

#endif
