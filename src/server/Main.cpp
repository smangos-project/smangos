// The server process: runs mangosd, bnetserver or both, sharing one config, log and login database.

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "ArgparserForServer.h"
#include "BnetServer.h"
#include "Bnet/BnetRealmList.h"
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "IO/Multithreading/CreateThread.h"
#include "IO/Timer/AsyncSystemTimer.h"
#include "Log.h"
#include "Master.h"
#include "ProgressBar.h"
#include "SystemConfig.h"
#include "migrations_list.h"
#include "revision.h"

#include <atomic>
#include <csignal>

DatabaseType LoginDatabase;                                 // Accessor to the realm/login database

char const* g_mainLogFileName = "Server.log";

static std::atomic<bool> s_stopRequested{ false };

static void OnSignal(int s)
{
    s_stopRequested = true;
    ::signal(s, OnSignal);
}

static void HookSignals()
{
    ::signal(SIGINT, OnSignal);
    ::signal(SIGTERM, OnSignal);
#ifdef _WIN32
    ::signal(SIGBREAK, OnSignal);
#endif
}

static void UnhookSignals()
{
    ::signal(SIGINT, nullptr);
    ::signal(SIGTERM, nullptr);
#ifdef _WIN32
    ::signal(SIGBREAK, nullptr);
#endif
}

static void CheckBnetConfVersion()
{
    if (sConfig.GetIntDefault("Bnet.ConfVersion", 0) >= _BNETSERVERCONFVERSION)
        return;

    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, " WARNING: Your bnetserver.conf version indicates your conf file is out of date!");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          Please check for updates, as your current default values may cause");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          unexpected behavior.");
    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
    Log::WaitBeforeContinueIfNeed();
}

extern int main(int argc, char** argv)
{
    ServerStartupArguments args;
    {
        // parseResult is std::expected, where the error is the return code, that might be present when invalid args or "--help" is given
        auto parseResult = ParseServerStartupArguments(argc, argv);
        if (!parseResult)
            return parseResult.error();

        args = parseResult.value();
    }

    // Nothing may touch sLog before the config layers are loaded.
    ServerServices services;
    if (!EnterServerDirectory(args) || !ResolveServices(args, services) || !LoadServerConfig(args, services))
        return EXIT_FAILURE;

    std::string const serviceNames = std::string(services.mangosd ? "mangosd" : "") + (services.mangosd && services.bnetserver ? "," : "") + (services.bnetserver ? "bnetserver" : "");

    if (services.mangosd)
    {
        sLog.OpenWorldLogFiles();
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "\n\n"
            "MM     MM MM   MM         MM   MM  MMMMM   MMMMM   MMMMM\n"
            "MM     MM MM   MM         MM   MM MMM MMM MM   MM MMM MMM\n"
            " MM   MM  MMM MMM         MMM  MM MMM MMM MM   MM MMM\n"
            " MM   MM  MM M MM  MMMMM  MMMM MM MMM     MM   MM  MMM\n"
            "  MM MM   MM M MM M   MMM MM MMMM MMM     MM   MM   MMM\n"
            "  MM MM   MM M MM     MMM MM  MMM MMMMMMM MM   MM    MMM\n"
            "   MMM    MM   MM MMMMMMM MM   MM MM  MMM MM   MM     MMM\n"
            "   MMM    MM   MM MM  MMM MM   MM MMM MMM MM   MM MMM MMM\n"
            "    M     MM   MM MM  MMM MM   MM  MMMMMM  MMMMM   MMMMM\n"
            "                  MMMMMM\n"
            "                          https://github.com/vmangos\n\n");
    }
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Core revision: %s [server: %s]", _FULLVERSION, serviceNames.c_str());
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "<Ctrl-C> to stop.");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Using configuration files %s", sConfig.GetFilename().c_str());

    CheckCommonConfVersion();
    if (services.bnetserver)
        CheckBnetConfVersion();

    if (!Crypto::InitializeCryptoAndPrintVersion())
    {
        sLog.WaitBeforeContinueIfNeed();
        return EXIT_FAILURE;
    }

    BarGoLink::SetOutputState(sConfig.GetBoolDefault("ShowProgressBars", true));

    if (!CreateServerPidFile())
        return EXIT_FAILURE;

    ApplyProcessSettings();
    IO::Multithreading::RenameCurrentThread("server-main");
    (void)sAsyncSystemTimer; // Pre-initialize the system timer

    if (!StartDB("Login", LoginDatabase, MIGRATIONS_LOGON) || (services.mangosd && !(LoadRealmId() && EnsureRealmRow(realmID))))
    {
        Log::WaitBeforeContinueIfNeed();
        LoginDatabase.HaltDelayThread();
        return EXIT_FAILURE;
    }

    sBnetRealmList.Initialize(
        uint8(sConfig.GetIntDefault("Bnet.Region", 1)),
        uint8(sConfig.GetIntDefault("Bnet.Battlegroup", 1)),
        sConfig.GetIntDefault("RealmsStateUpdateDelay", 20));

    bool bnetStarted = false;
    if (services.bnetserver)
    {
        if (!BnetServerStart())
        {
            Log::WaitBeforeContinueIfNeed();
            sAsyncSystemTimer.RemoveAllTimersAndStopThread();
            LoginDatabase.HaltDelayThread();
            return EXIT_FAILURE;
        }
        bnetStarted = true;
    }

    if (services.mangosd && !sMaster.Start(args.dumpStoresDir))
    {
        if (bnetStarted)
            BnetServerStop();
        sAsyncSystemTimer.RemoveAllTimersAndStopThread();
        LoginDatabase.HaltDelayThread();
        return EXIT_FAILURE;
    }

    LoginDatabase.AllowAsyncTransactions();

    int exitCode = 0;
    if (services.mangosd)
    {
        exitCode = sMaster.Run(); // hooks its own signals
    }
    else
    {
        HookSignals();
        while (!s_stopRequested)
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        UnhookSignals();
    }

    if (bnetStarted)
        BnetServerStop();

    sAsyncSystemTimer.RemoveAllTimersAndStopThread();
    LoginDatabase.StopServer();

    sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "Halting process...");
    return exitCode;
}
