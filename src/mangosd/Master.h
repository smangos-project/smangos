/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <https://github.com/mangos/zero>
 * Copyright (C) 2011-2016 Nostalrius <https://nostalrius.org>
 * Copyright (C) 2016-2017 Elysium Project <https://github.com/elysium-project>
 *
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

// \addtogroup mangosd
// @{
// \file

#ifndef _MASTER_H
#define _MASTER_H

#include "Common.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "Policies/Singleton.h"

#include <memory>
#include <thread>
#include <vector>

class Database;

// Start the server
class Master
{
    public:
        Master();
        ~Master();
        // Starts the world databases (Login must already run), loads the world and starts the threads.
        // Returns false only while no thread runs yet. After that a failure stops the world and Run()
        // returns the error code. With a dump directory it writes the stores and exits the process.
        bool Start(std::string const& dumpStoresDir);
        // Blocks until the world stops, tears down and returns the exit code (0 shutdown, 1 error, 2 restart).
        int Run();
        static volatile uint32  m_masterLoopCounter;
        static volatile bool    m_handleSigvSignals;
        static void SigvSignalHandler();
        static void ArmAnticrash();

    private:
        bool _StartDB();

        void _HookSignals();
        void _UnhookSignals();
        static void _OnSignal(int s);

        void clearOnlineAccounts();

        std::thread m_worldThread;
        std::unique_ptr<std::thread> m_cliThread;
        std::unique_ptr<std::thread> m_soapThread;
        std::unique_ptr<std::thread> m_freezeThread;
        std::unique_ptr<IO::IoContext> m_ioCtx;
        std::vector<std::thread> m_ioRunners;
        std::unique_ptr<IO::Networking::AsyncSocketAcceptor> m_raServer;
};

extern uint32 realmID;
bool LoadRealmId();
bool EnsureRealmRow(uint32 id);
bool StartDB(std::string const& name, Database& database, char const** migrations);

#define sMaster MaNGOS::Singleton<Master>::Instance()
#endif
// @}
