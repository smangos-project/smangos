#include "ArgparserForServer.h"
#include "SystemConfig.h"
#include "Config/Config.h"
#include "Log.h"
#include "Util.h"

#include <filesystem>

// Print out the usage string for this program on the console.
void printUsage(char const* thisExecutableName)
{
#if defined(WIN32)
    printf(R"END(Usage: %s [<options>]
    -v, --version               print version and exist
    -c, --config <config_file>  use config_file as configuration file
    --server-dir <dir>          run from dir (default: <source root>/.server)
    --dump-stores <dir>         write the loaded world stores to dir and exit
  Running as service functions:
    -s run                      run as service
    -s install                  install service
    -s uninstall                uninstall service
)END", thisExecutableName);
#else
    printf(R"END(Usage: %s [<options>]
    -v, --version               print version and exist
    -c, --config <config_file>  use config_file as configuration file
    --server-dir <dir>          run from dir (default: <source root>/.server)
    --dump-stores <dir>         write the loaded world stores to dir and exit
  Running as daemon functions:
    -s run                      run as daemon
    -s stop                     stop daemon
)END", thisExecutableName);
#endif
}

nonstd::expected<ServerStartupArguments, int /* exit value */> ParseServerStartupArguments(int /*MUTABLE*/ argc, char** /*MUTABLE*/ argv) // we are changing the meaning of argc and argv
{
    char const* thisExecutableName = argv[0];
    argv++; // skip the first argument, since it's just the executable name
    argc--;

    ServerStartupArguments args
    {
        ServiceDaemonAction::NotSet,
        "",
        "",
        ""
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
            if (argc == 0)
            {
                printf("Error: Config filename required\n");
                printUsage(thisExecutableName);
                return nonstd::make_unexpected(EXIT_FAILURE);
            }

            std::string nextPart = std::string(*argv);
            argv++;
            argc--;

            args.configFilePath = nextPart;
        }
        else if (part == "--server-dir")
        {
            if (argc == 0)
            {
                printf("Error: Server directory required\n");
                printUsage(thisExecutableName);
                return nonstd::make_unexpected(EXIT_FAILURE);
            }

            args.serverDir = std::string(*argv);
            argv++;
            argc--;
        }
        else if (part == "--dump-stores")
        {
            if (argc == 0)
            {
                printf("Error: Dump directory required\n");
                printUsage(thisExecutableName);
                return nonstd::make_unexpected(EXIT_FAILURE);
            }

            args.dumpStoresDir = std::string(*argv);
            argv++;
            argc--;
        }
        else if (part == "-s")
        {
            if (argc == 0)
            {
                printf("Error: Config filename required\n");
                printUsage(thisExecutableName);
                return nonstd::make_unexpected(EXIT_FAILURE);
            }

            std::string nextPart = std::string(*argv);
            argv++;
            argc--;

            if (nextPart == "run")
            {
                args.inputServiceMode = ServiceDaemonAction::Start;
            }
#ifdef WIN32
            else if (nextPart == "install")
            {
                args.inputServiceMode = ServiceDaemonAction::Install;
            }
            else if (nextPart == "uninstall")
            {
                args.inputServiceMode = ServiceDaemonAction::Uninstall;
            }
#else
            else if (nextPart == "stop")
            {
                args.inputServiceMode = ServiceDaemonAction::Stop;
            }
#endif
            else
            {
                printf("Error: Unknown service mode\n");
                printUsage(thisExecutableName);
                return nonstd::make_unexpected(EXIT_FAILURE);
            }
        }
        else
        {
            printf("Error: Unknown argument provided '%s'\n", part.c_str());
            printUsage(thisExecutableName);
            return nonstd::make_unexpected(EXIT_FAILURE);
        }
    }

    return args;
}

bool EnterServerDirectory(ServerStartupArguments& args, char const* defaultConfigName)
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
            args.configFilePath = (serverDir / "config" / defaultConfigName).string();
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

bool LoadServerConfig(ServerStartupArguments const& args)
{
    std::string const commonFile = (std::filesystem::path(args.serverDir) / "config" / "common.conf").string();
    std::string error;
    if (sConfig.LoadLayers({ commonFile, args.configFilePath }, error))
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
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "process priority class set to HIGH");
        else
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't set process priority class.");
    }
#endif
}
