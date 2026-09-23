// Standalone console host for RealBotMgr (bot-phase-1.md step 5's "minimal RealBotMgr (one
// worker) and the runner"). Deliberately does not link `game`: RealBots touches no server
// runtime state (bot-plan.md), so this needs only shared/framework/bnetproto/zlib/openssl --
// exactly what tests-realbots already links, compiled directly from the same RealBots sources
// rather than through the `game` static library (sidesteps bot-phase-1.md's own "first risk to
// clear" note about linking `game`'s script loader/world/database symbols -- nothing here needs
// them).
//
// `RealBotTarget`/`BotAccountLease`/`RealBotCommands` (bot-phase-1.md step 6) don't exist yet, so
// there is no multi-target config and no shared command implementation with `.realbot` chat
// commands: a bot's login parameters are given inline on the "add" command line, and the console
// loop below is this host's own, temporary until step 6 replaces it.
#include "BotStage.h"
#include "BotTaskArgs.h"
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "Log.h"
#include "RealBotMgr.h"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

char const* g_mainLogFileName = "RealBotRunner.log";

namespace
{
RealBotLoginOptions ParseLoginOptions(BotTaskArgs const& args)
{
    RealBotLoginOptions options;
    options.LoginAddress = args.GetString("login", "127.0.0.1");
    options.RestPort = uint16_t(args.GetUInt32("restport", 8081));
    options.RpcPort = uint16_t(args.GetUInt32("rpcport", 1119));
    options.AccountName = args.GetString("account");
    options.Password = args.GetString("password");
    options.RealmName = args.GetString("realm");
    options.RealmId = args.GetUInt32("realmid", 0);
    options.ClientBuild = int32_t(args.GetUInt32("build", 58797));
    options.VerifyServerSignature = args.GetBool("verifysig", true);
    options.CreateIfNone = args.GetBool("create", true);
    options.CharacterName = args.GetString("character");
    options.Race = uint8(args.GetUInt32("race", 0));
    options.Class = uint8(args.GetUInt32("class", 0));
    options.Gender = uint8(args.GetUInt32("gender", 0));
    return options;
}

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
        "  add <key=value,...>        adds a bot; keys: account, password, login, restport,\n"
        "                             rpcport, realm, realmid, build, character, race, class,\n"
        "                             gender, create (0/1)\n"
        "  remove <name|all>          removes a bot\n"
        "  list                       lists every bot and its stage\n"
        "  info <name>                shows one bot's stage, latency, character, last error\n"
        "  task <name> <task> [args]  pushes a named task onto a running bot\n"
        "  help                       shows this text\n"
        "  quit                       stops every bot and exits\n";
}

void HandleCommand(std::string const& line)
{
    std::istringstream iss(line);
    std::string verb;
    iss >> verb;
    if (verb.empty())
        return;

    if (verb == "add")
    {
        std::string rest;
        std::getline(iss, rest);
        BotTaskArgs const args = BotTaskArgs::Parse(TrimLeadingSpace(rest));
        RealBotLoginOptions const options = ParseLoginOptions(args);
        if (options.AccountName.empty() || options.Password.empty())
        {
            std::cout << "add needs at least account=...,password=...\n";
            return;
        }

        std::string const name = sRealBotMgr.Add(options);
        std::cout << "bot '" << name << "' is connecting\n";
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
