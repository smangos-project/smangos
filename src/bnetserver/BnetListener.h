#ifndef BNETSERVER_BNETLISTENER_H
#define BNETSERVER_BNETLISTENER_H

#include "Common.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/AsyncSocketAcceptor.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Called for each accepted connection that passed the IP ban check, with the peer address as seen
// through any trusted proxy.
using BnetSessionFactory = std::function<void(std::unique_ptr<IO::Networking::AsyncSocket> socket, std::string clientIp)>;

// Listens on `bindIp:port` on `ioContext`. Peers in `trustedProxies` send a Proxy Protocol v2 header
// carrying the real client address before anything else. Returns nullptr if the port cannot be bound.
std::unique_ptr<IO::Networking::AsyncSocketAcceptor> BnetStartListener(IO::IoContext* ioContext, std::string const& bindIp, uint16 port,
    std::vector<std::string> const& trustedProxies, BnetSessionFactory factory);

#endif
