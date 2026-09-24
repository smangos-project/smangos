// Standalone console host for RealBotMgr (bot-phase-1.md step 5's "minimal RealBotMgr (one
// worker) and the runner"). Deliberately does not link `game`: RealBots touches no server
// runtime state (bot-plan.md), so this needs only shared/framework/bnetproto/zlib/openssl --
// exactly what tests-realbots already links, compiled directly from the same RealBots sources
// rather than through the `game` static library (sidesteps bot-phase-1.md's own "first risk to
// clear" note about linking `game`'s script loader/world/database symbols -- nothing here needs
// them).
//
// `RealBotCommands` now holds every verb (targets/add/remove/list/info/task/tasks/status/help),
// shared with a future `.realbot` chat command (step 7) -- this host just wires its console loop
// to it, printing through std::cout and handling its own "quit"/"exit" (not a RealBotCommands verb:
// a chat command has no equivalent). `RealBotTarget`/`BotAccountLease` mean "add" takes a target
// name (RealBots.Target.<name>.* in the -c config) and leases one of its configured accounts,
// rather than raw connection parameters on the command line.
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "Log.h"
#include "RealBotCommands.h"
#include "RealBotMgr.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

char const* g_mainLogFileName = "RealBotRunner.log";

namespace
{
void PrintLine(std::string const& line)
{
    std::cout << line << "\n";
}
}

int main(int argc, char** argv)
{
    std::string configPath = "realbotrunner.conf";
    for (int i = 1; i < argc; ++i)
    {
        std::string const arg = argv[i];
        if (arg == "-c" && i + 1 < argc)
            configPath = argv[++i];
    }

    if (sConfig.LoadFromFile(configPath))
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Loaded config %s", configPath.c_str());
    else
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "No config at %s -- using RealBots.* defaults", configPath.c_str());

    sLog.OpenWorldLogFiles();

    if (!Crypto::InitializeCryptoAndPrintVersion())
        return EXIT_FAILURE;

    sRealBotMgr.Start();

    // RealBotMgr::Update (lifecycle housekeeping -- reaping long-Disconnected/Failed bots, not
    // ticking, which the worker pool already does on its own threads) needs a periodic caller;
    // the console loop below blocks on std::cin, so that caller is this small timer thread rather
    // than the main thread (bot-phase-1.md, "Hosts": "ticks Update every 100 ms").
    std::atomic<bool> updating{ true };
    std::thread updateThread([&updating]()
    {
        auto last = std::chrono::steady_clock::now();
        while (updating)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto const now = std::chrono::steady_clock::now();
            uint32_t const diff = uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count());
            last = now;
            sRealBotMgr.Update(diff);
        }
    });

    std::cout << "RealBots runner -- type 'help' for commands, 'quit' to exit.\n";

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line == "quit" || line == "exit")
            break;

        try
        {
            RealBotCommands::Handle(line, PrintLine);
        }
        catch (std::exception const& e)
        {
            std::cout << "error: " << e.what() << "\n";
        }
    }

    updating = false;
    updateThread.join();

    sRealBotMgr.Stop();
    return EXIT_SUCCESS;
}
