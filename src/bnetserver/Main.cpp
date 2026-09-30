/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// The Battle.net login server: the REST login portal and the RPC port the modern client uses to log in
// and pick a realm.

#include "Common.h"
#include "Database/DatabaseEnv.h"

#include "BnetServer.h"
#include "Bnet/BnetRealmList.h"
#include "Config/Config.h"
#include "Crypto/InitializeCrypto.h"
#include "IO/Multithreading/CreateThread.h"
#include "IO/Timer/AsyncSystemTimer.h"
#include "Log.h"
#include "SystemConfig.h"
#include "Util.h"
#include "ArgparserForServer.h"
#include "migrations_list.h"
#include "revision.h"

#ifdef WIN32
#include "ServiceWin32.h"

char serviceName[] = "bnetserver";
char serviceLongName[] = "MaNGOS bnetserver service";
char serviceDescription[] = "Battle.net login server";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
volatile int m_ServiceStatus = -1;
#else
#include "PosixDaemon.h"
#endif

bool StartDB();
void UnhookSignals();
void HookSignals();

char const* g_mainLogFileName = "Bnetserver.log";
volatile bool stopEvent = false;
DatabaseType LoginDatabase;

extern int main(int argc, char** argv)
{
    ServerStartupArguments args;
    {
        auto parseResult = ParseServerStartupArguments(argc, argv);
        if (!parseResult)
            return parseResult.error();

        args = parseResult.value();
    }

    if (!EnterServerDirectory(args, "bnetserver.conf"))
        return EXIT_FAILURE;

    if (!LoadServerConfig(args)) // must be done before (linux) service init
        return EXIT_FAILURE;

    switch (args.inputServiceMode)
    {
    case ServiceDaemonAction::NotSet:
        break;
#ifdef WIN32
    case ServiceDaemonAction::Install:
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Installing service...");
        return WinServiceInstall() ? EXIT_SUCCESS : EXIT_FAILURE;
    case ServiceDaemonAction::Uninstall:
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Uninstalling service...");
        return WinServiceUninstall() ? EXIT_SUCCESS : EXIT_FAILURE;
    case ServiceDaemonAction::Start:
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Starting service...");
        return WinServiceRun() ? EXIT_SUCCESS : EXIT_FAILURE;
#else
    case ServiceDaemonAction::Start:
        startDaemon();
        break;
    case ServiceDaemonAction::Stop:
        stopDaemon();
        break;
#endif
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Core revision: %s [bnet-server]", _FULLVERSION);
    CheckCommonConfVersion();
    if (!Crypto::InitializeCryptoAndPrintVersion())
    {
        sLog.WaitBeforeContinueIfNeed();
        return EXIT_FAILURE;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "<Ctrl-C> to stop.");
    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Using configuration files %s.", sConfig.GetFilename().c_str());

    uint32 confVersion = sConfig.GetIntDefault("Bnet.ConfVersion", 0);
    if (confVersion < _BNETSERVERCONFVERSION)
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, " WARNING: Your bnetserver.conf version indicates your conf file is out of date!");
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          Please check for updates, as your current default values may cause ");
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "          strange behavior.                                                  ");
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "*****************************************************************************");
        Log::WaitBeforeContinueIfNeed();
    }

    std::string pidfile = sConfig.GetStringDefault("PidFile", "");
    if (!pidfile.empty())
    {
        uint32 pid = CreatePIDFile(pidfile);
        if (!pid)
        {
            sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Cannot create PID file %s.\n", pidfile.c_str());
            Log::WaitBeforeContinueIfNeed();
            return 1;
        }

        sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "Daemon PID: %u\n", pid);
    }

    if (!StartDB())
    {
        Log::WaitBeforeContinueIfNeed();
        return 1;
    }

    sBnetRealmList.Initialize(
        uint8(sConfig.GetIntDefault("Bnet.Region", 1)),
        uint8(sConfig.GetIntDefault("Bnet.Battlegroup", 1)),
        sConfig.GetIntDefault("RealmsStateUpdateDelay", 20));
    if (!BnetServerStart())
    {
        Log::WaitBeforeContinueIfNeed();
        LoginDatabase.HaltDelayThread();
        return 1;
    }

    HookSignals();

#ifdef WIN32
    {
        HANDLE hProcess = ::GetCurrentProcess();

        uint32 Aff = sConfig.GetIntDefault("UseProcessors", 0);
        if (Aff > 0)
        {
            ULONG_PTR appAff;
            ULONG_PTR sysAff;

            if (::GetProcessAffinityMask(hProcess, &appAff, &sysAff))
            {
                ULONG_PTR curAff = Aff & appAff;

                if (!curAff)
                {
                    sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Processors marked in UseProcessors bitmask (hex) %x not accessible for bnetserver. Accessible processors bitmask (hex): %x", Aff, appAff);
                }
                else
                {
                    if (::SetProcessAffinityMask(hProcess, curAff))
                        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Using processors (bitmask, hex): %x", curAff);
                    else
                        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't set used processors (hex): %x", curAff);
                }
            }
            sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "");
        }

        if (sConfig.GetBoolDefault("ProcessPriority", false))
        {
            if (::SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS))
                sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "bnetserver process priority class set to HIGH");
            else
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Can't set bnetserver process priority class.");
        }
    }
#endif

    LoginDatabase.AllowAsyncTransactions();

#ifndef WIN32
    detachDaemon();
#endif
    while (!stopEvent)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

#ifdef WIN32
        if (m_ServiceStatus == 0) stopEvent = true;
        while (m_ServiceStatus == 2) Sleep(1000);
#endif
    }

    sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "Waiting for IO thread to finish");

    BnetServerStop();
    sAsyncSystemTimer.RemoveAllTimersAndStopThread();
    LoginDatabase.HaltDelayThread();

    UnhookSignals();

    sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "Halting process...");
    return 0;
}

void OnSignal(int s)
{
    switch (s)
    {
        case SIGINT:
        case SIGTERM:
            stopEvent = true;
            break;
#ifdef _WIN32
        case SIGBREAK:
            stopEvent = true;
            break;
#endif
    }

    ::signal(s, OnSignal);
}

bool StartDB()
{
    std::string dbstring = sConfig.GetStringDefault("LoginDatabase.Info", "");
    if (dbstring.empty())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Database not specified");
        return false;
    }

    // Password is the fourth of the five ';'-separated tokens; keep it out of the log.
    std::string dbStringLog = dbstring;

    if (std::count(dbStringLog.begin(), dbStringLog.end(), ';') == 4)
    {
        std::string::iterator start = dbStringLog.end(), end = dbStringLog.end();

        int occurrence = 0;
        for (std::string::iterator itr = dbStringLog.begin(); itr != dbStringLog.end(); ++itr)
        {
            if (*itr == ';')
                ++occurrence;

            if (occurrence == 3 && start == dbStringLog.end())
                start = itr + 1;
            else if (occurrence == 4 && end == dbStringLog.end())
                end = itr;

            if (start != dbStringLog.end() && end != dbStringLog.end())
                break;
        }

        dbStringLog.replace(start, end, "*");
    }
    else
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Incorrectly formatted database connection string for logon database");
        return false;
    }

    sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "Database: %s", dbStringLog.c_str());
    if (!LoginDatabase.Initialize(dbstring.c_str(), sConfig.GetIntDefault("LoginDatabase.Connections", 1), sConfig.GetIntDefault("LoginDatabase.WorkerThreads", 1)))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "Cannot connect to database");
        return false;
    }

    if (!LoginDatabase.CheckRequiredMigrations(MIGRATIONS_LOGON))
    {
        LoginDatabase.HaltDelayThread();
        return false;
    }

    return true;
}

void HookSignals()
{
    ::signal(SIGINT, OnSignal);
    ::signal(SIGTERM, OnSignal);
#ifdef _WIN32
    ::signal(SIGBREAK, OnSignal);
#endif
}

void UnhookSignals()
{
    ::signal(SIGINT, nullptr);
    ::signal(SIGTERM, nullptr);
#ifdef _WIN32
    ::signal(SIGBREAK, nullptr);
#endif
}
