#include "ArgparserForServer.h"
#include "SystemConfig.h"
#include "Config/Config.h"
#include "Log.h"
#include "Util.h"

#include <filesystem>

// Print out the usage string for this program on the console.
static void printUsage(char const* thisExecutableName)
{
    printf(R"END(Usage: %s [<options>]
    -v, --version               print version and exit
    -c, --config <config_file>  use config_file as common.conf
    --server-dir <dir>          run from dir (default: <source root>/.server)
    --services <list>           comma-separated: mangosd, bnetserver (default: Server.Services of common.conf)
    --dump-stores <dir>         write the loaded world stores to dir and exit (runs mangosd only)
)END", thisExecutableName);
}

// Fills the flags from a comma-separated list; false with a message on an unknown or empty list.
static bool ParseServiceList(std::string const& list, ServerServices& services, std::string& error)
{
    services = ServerServices();
    for (std::string const& name : SplitStringByDelimiter(list, ','))
    {
        if (name == "mangosd")
            services.mangosd = true;
        else if (name == "bnetserver")
            services.bnetserver = true;
        else
        {
            error = "Unknown service '" + name + "' (use mangosd, bnetserver)";
            return false;
        }
    }

    if (!services.mangosd && !services.bnetserver)
    {
        error = "No service selected (use mangosd, bnetserver)";
        return false;
    }
    return true;
}

nonstd::expected<ServerStartupArguments, int /* exit value */> ParseServerStartupArguments(int /*MUTABLE*/ argc, char** /*MUTABLE*/ argv) // we are changing the meaning of argc and argv
{
    char const* thisExecutableName = argv[0];
    argv++; // skip the first argument, since it's just the executable name
    argc--;

    ServerStartupArguments args;

    auto nextValue = [&](char const* missingMessage, std::string& value) -> bool
    {
        if (argc == 0)
        {
            printf("Error: %s\n", missingMessage);
            printUsage(thisExecutableName);
            return false;
        }

        value = std::string(*argv);
        argv++;
        argc--;
        return true;
    };

    while (argc > 0)
    {
        std::string part = std::string(*argv);
        argv++;
        argc--;

        if (part == "-h" || part == "--help")
        {
            printUsage(thisExecutableName);
            return nonstd::make_unexpected(EXIT_SUCCESS); // explicitly printing help is okay
        }
        else if (part == "-v" || part == "--version")
        {
            printf("Core revision: %s\n", _FULLVERSION);
            return nonstd::make_unexpected(EXIT_SUCCESS); // explicitly printing version is okay
        }
        else if (part == "-c" || part == "--config")
        {
            if (!nextValue("Config filename required", args.configFilePath))
                return nonstd::make_unexpected(EXIT_FAILURE);
        }
        else if (part == "--server-dir")
        {
            if (!nextValue("Server directory required", args.serverDir))
                return nonstd::make_unexpected(EXIT_FAILURE);
        }
        else if (part == "--services")
        {
            if (!nextValue("Service list required", args.services))
                return nonstd::make_unexpected(EXIT_FAILURE);
        }
        else if (part == "--dump-stores")
        {
            if (!nextValue("Dump directory required", args.dumpStoresDir))
                return nonstd::make_unexpected(EXIT_FAILURE);
        }
        else
        {
            printf("Error: Unknown argument provided '%s'\n", part.c_str());
            printUsage(thisExecutableName);
            return nonstd::make_unexpected(EXIT_FAILURE);
        }
    }

    if (!args.services.empty())
    {
        ServerServices services;
        std::string error;
        if (!ParseServiceList(args.services, services, error))
        {
            printf("Error: %s\n", error.c_str());
            return nonstd::make_unexpected(EXIT_FAILURE);
        }

        if (!args.dumpStoresDir.empty() && services.bnetserver)
        {
            printf("Error: --dump-stores runs mangosd only\n");
            return nonstd::make_unexpected(EXIT_FAILURE);
        }
    }

    return args;
}

bool EnterServerDirectory(ServerStartupArguments& args)
{
    try
    {
        std::filesystem::path serverDir = std::filesystem::absolute(args.serverDir.empty() ? std::filesystem::path(_SERVER_DIR_DEFAULT) : std::filesystem::path(args.serverDir)).lexically_normal();
        if (!std::filesystem::is_directory(serverDir))
        {
            fprintf(stderr, "server: %s does not exist; build the server_setup target first\n", serverDir.string().c_str());
            return false;
        }

        if (args.configFilePath.empty())
            args.configFilePath = (serverDir / "config" / "common.conf").string();
        else
            args.configFilePath = std::filesystem::absolute(args.configFilePath).string();

        if (!args.dumpStoresDir.empty())
            args.dumpStoresDir = std::filesystem::absolute(args.dumpStoresDir).string();

        std::filesystem::create_directories(serverDir / "logs");
        std::filesystem::current_path(serverDir);
        args.serverDir = serverDir.string();
        return true;
    }
    catch (std::filesystem::filesystem_error const& e)
    {
        fprintf(stderr, "server: %s\n", e.what());
        return false;
    }
}

bool ResolveServices(ServerStartupArguments const& args, ServerServices& services)
{
    std::string list = args.services;
    if (list.empty())
        list = args.dumpStoresDir.empty() ? "" : "mangosd";

    if (list.empty())
    {
        std::string error;
        if (!sConfig.LoadLayers({ args.configFilePath }, error))
        {
            fprintf(stderr, "server: %s\n", error.c_str());
            return false;
        }
        list = sConfig.GetStringDefault("Server.Services", "mangosd,bnetserver");
    }

    std::string error;
    if (!ParseServiceList(list, services, error))
    {
        fprintf(stderr, "server: %s\n", error.c_str());
        return false;
    }
    return true;
}

bool LoadServerConfig(ServerStartupArguments const& args, ServerServices const& services)
{
    std::filesystem::path const configDir = std::filesystem::path(args.serverDir) / "config";
    std::vector<std::string> files = { args.configFilePath };
    if (services.mangosd)
        files.push_back((configDir / "mangosd.conf").string());
    if (services.bnetserver)
        files.push_back((configDir / "bnetserver.conf").string());

    std::string error;
    if (sConfig.LoadLayers(files, error))
        return true;

    fprintf(stderr, "server: %s\n", error.c_str());
    return false;
}

void CheckCommonConfVersion()
{
    if (sConfig.GetIntDefault("Common.ConfVersion", 0) >= _COMMONCONFVERSION)
        return;

    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, " WARNING: Your common.conf version indicates your conf file is out of date!");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          Please check for updates, as your current default values may cause");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          unexpected behavior.");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
    Log::WaitBeforeContinueIfNeed();
}

bool CreateServerPidFile()
{
    std::string const pidFile = sConfig.GetStringDefault("PidFile", "");
    if (pidFile.empty())
        return true;

    uint32 const pid = CreatePIDFile(pidFile);
    if (!pid)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Cannot create PID file %s.\n", pidFile.c_str());
        Log::WaitBeforeContinueIfNeed();
        return false;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "Daemon PID: %u\n", pid);
    return true;
}

void ApplyProcessSettings()
{
#ifdef WIN32
    HANDLE hProcess = ::GetCurrentProcess();

    uint32 const affinity = sConfig.GetIntDefault("UseProcessors", 0);
    if (affinity > 0)
    {
        ULONG_PTR appAffinity;
        ULONG_PTR sysAffinity;

        if (::GetProcessAffinityMask(hProcess, &appAffinity, &sysAffinity))
        {
            ULONG_PTR const current = affinity & appAffinity; // remove non accessible processors

            if (!current)
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Processors marked in UseProcessors bitmask (hex) %x not accessible. Accessible processors bitmask (hex): %x", affinity, uint32(appAffinity));
            else if (::SetProcessAffinityMask(hProcess, current))
                sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Using processors (bitmask, hex): %x", uint32(current));
            else
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't set used processors (hex): %x", uint32(current));
        }
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
    }

    if (sConfig.GetBoolDefault("ProcessPriority", false))
    {
        if (::SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "server process priority class set to HIGH");
        else
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't set server process priority class.");
    }
#endif
}
