#include "BotConnector.h"
#include "IO/Networking/SocketConnector.h"
#include "IO/Multithreading/CreateThread.h"

void BotConnector::ConnectAsync(IO::Networking::IpEndpoint const& target, std::chrono::milliseconds timeout, ConnectCallback callback)
{
    IO::Multithreading::CreateThread("BotConnector", [target, timeout, callback = std::move(callback)]()
    {
        callback(IO::Networking::SocketConnector::ConnectBlocking(target, timeout));
    }).detach();
}
