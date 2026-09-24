// Standalone console host for RealBotMgr (bot-phase-1.md step 5's "minimal RealBotMgr (one
// worker) and the runner"). Deliberately does not link `game`: RealBots touches no server
// runtime state (bot-plan.md), so this needs only shared/framework/bnetproto/zlib/openssl --
// exactly what tests-realbots already links, compiled directly from the same RealBots sources
// rather than through the `game` static library (sidesteps bot-phase-1.md's own "first risk to
// clear" note about linking `game`'s script loader/world/database symbols -- nothing here needs
// them).
//
// `RealBotCommands` (bot-phase-1.md step 6) doesn't exist yet, so there is no command
// implementation shared with a future `.realbot` chat command: the console loop below is this
// host's own, temporary until step 6 replaces it. `RealBotTarget`/`BotAccountLease` do exist, so
// "add" takes a target name (RealBots.Target.<name>.* in the -c config) and leases one of its
// configured accounts, rather than raw connection parameters on the command line.
#include "BotStage.h"
#include "BotTaskArgs.h"
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "Log.h"
#include "RealBotMgr.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

char const* g_mainLogFileName = "RealBotRunner.log";

namespace
{
char const* StageName(BotStage stage)
{
    switch (stage)
    {
        case BotStage::Disconnected: return "Disconnected";
        case BotStage::LoginPortal: return "LoginPortal";
        case BotStage::LoginRpc: return "LoginRpc";
        case BotStage::RealmSelect: return "RealmSelect";
        case BotStage::WorldConnecting: return "WorldConnecting";
        case BotStage::WorldHandshake: return "WorldHandshake";
        case BotStage::CharSelect: return "CharSelect";
        case BotStage::EnteringWorld: return "EnteringWorld";
        case BotStage::InWorld: return "InWorld";
        case BotStage::LoggingOut: return "LoggingOut";
        case BotStage::Failed: return "Failed";
        default: return "Unknown";
    }
}

std::string TrimLeadingSpace(std::string s)
{
    size_t const pos = s.find_first_not_of(' ');
    return pos == std::string::npos ? std::string() : s.substr(pos);
}

void PrintHelp()
{
    std::cout <<
        "  targets                    lists every configured target and its free account count\n"
        "  add <target> [count]       leases account(s) from <target> and adds that many bots\n"
        "  remove <name|all>          removes a bot\n"
        "  list                       lists every bot and its stage\n"
        "  info <name>                shows one bot's stage, latency, character, last error\n"
        "  task <name> <task> [args]  pushes a named task onto a running bot\n"
        "  help                       shows this text\n"
        "  quit                       stops every bot and exits\n";
}

void PrintTargets()
{
    for (RealBotTarget const& target : sRealBotMgr.GetTargets().All())
    {
        std::cout << target.Name << "\t" << target.LoginAddress << ":" << target.RestPort << "/" << target.RpcPort << "\ttags:";
        for (std::string const& tag : target.Tags)
            std::cout << " " << tag;

        if (std::optional<std::pair<size_t, size_t>> counts = sRealBotMgr.GetLeaseCounts(target.Name))
            std::cout << "\taccounts " << counts->first << "/" << counts->second << " free";

        std::cout << "\n";
    }
}

void HandleCommand(std::string const& line)
{
    std::istringstream iss(line);
    std::string verb;
    iss >> verb;
    if (verb.empty())
        return;

    if (verb == "targets")
    {
        PrintTargets();
    }
    else if (verb == "add")
    {
        std::string targetName;
        iss >> targetName;
        uint32_t count = 1;
        uint32_t parsedCount = 0;
        if (iss >> parsedCount) // absent (or unparsable) leaves count at its default of 1
            count = parsedCount;

        if (targetName.empty())
        {
            std::cout << "add needs a target name -- try 'targets'\n";
            return;
        }

        for (uint32_t i = 0; i < std::max<uint32_t>(1, count); ++i)
        {
            std::string error;
            std::string const name = sRealBotMgr.Add(targetName, "", &error);
            if (name.empty())
                std::cout << "add: " << error << "\n";
            else
                std::cout << "bot '" << name << "' is connecting\n";
        }
    }
    else if (verb == "remove")
    {
        std::string name;
        iss >> name;
        if (name == "all")
            sRealBotMgr.RemoveAll();
        else
            sRealBotMgr.Remove(name);
    }
    else if (verb == "list")
    {
        for (std::string const& name : sRealBotMgr.List())
            if (std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name))
                std::cout << info->Name << "\t" << StageName(info->Stage) << "\t" << info->LatencyMs << " ms\n";
    }
    else if (verb == "info")
    {
        std::string name;
        iss >> name;
        std::optional<RealBotInfo> info = sRealBotMgr.GetInfo(name);
        if (!info)
        {
            std::cout << "no such bot '" << name << "'\n";
            return;
        }
        std::cout << "stage       " << StageName(info->Stage) << "\n";
        std::cout << "latency     " << info->LatencyMs << " ms\n";
        std::cout << "character   " << (info->CharacterName.empty() ? "-" : info->CharacterName) << "\n";
        std::cout << "error       " << (info->LastError.empty() ? "-" : info->LastError) << "\n";
    }
    else if (verb == "task")
    {
        std::string name, taskName, rest;
        iss >> name >> taskName;
        std::getline(iss, rest);
        if (!sRealBotMgr.RunTask(name, taskName, BotTaskArgs::Parse(TrimLeadingSpace(rest))))
            std::cout << "no such bot or task\n";
    }
    else if (verb == "help")
    {
        PrintHelp();
    }
    else
    {
        std::cout << "unknown command '" << verb << "' -- try 'help'\n";
    }
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

    std::cout << "RealBots runner -- type 'help' for commands, 'quit' to exit.\n";

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line == "quit" || line == "exit")
            break;

        try
        {
            HandleCommand(line);
        }
        catch (std::exception const& e)
        {
            std::cout << "error: " << e.what() << "\n";
        }
    }

    sRealBotMgr.Stop();
    return EXIT_SUCCESS;
}
