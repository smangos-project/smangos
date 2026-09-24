#ifndef REALBOTS_REALBOTMGR_H
#define REALBOTS_REALBOTMGR_H

#include "BotAccountLease.h"
#include "BotTaskArgs.h"
#include "IO/Context/IoContext.h"
#include "Policies/Singleton.h"
#include "RealBot.h"
#include "RealBotTarget.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// IoContext.h is included in full (not forward-declared) even though only a pointer to it is
// stored: any translation unit that includes Config.h (almost all of them -- it drags in
// Policies/SingletonImp.h) implicitly re-instantiates MaNGOS::Singleton<RealBotMgr>::Instance()
// for itself, including the `new RealBotMgr` inside it, whose exception-unwinding path needs
// std::unique_ptr<IO::IoContext>'s destructor -- a complete type -- right there, not just in
// RealBotMgr.cpp.

// Owns every bot's IoContext, IO thread(s) and worker thread, so both hosts (mangosd, `.realbot`;
// contrib/realbotrunner) start it the same way (bot-phase-1.md, "Framework"). Phase 1's version:
// a single worker thread ticking every bot in turn (bot-phase-1.md step 5's "minimal RealBotMgr
// (one worker)") -- the pinned worker pool (RealBots.WorkerThreads > 1) is step 6.
class RealBotMgr
{
    public:
        // Out-of-line so m_ioContext's unique_ptr<IO::IoContext> doesn't need a complete
        // IoContext type wherever sRealBotMgr's static instance destructor gets instantiated
        // (i.e. every translation unit that touches sRealBotMgr, not just RealBotMgr.cpp).
        ~RealBotMgr();

        void Start();
        void Stop();

        // Leases a free account from the named target, creates a bot with it, gives it a name
        // (or one derived from the account name if empty), and pushes the built-in "login" task --
        // it starts logging in on its very next Tick. Empty return (with *error* set, if given)
        // means no such target or no free account left in its lease pool.
        std::string Add(std::string const& targetName, std::string name = "", std::string* error = nullptr);
        void Remove(std::string const& name);
        void RemoveAll();

        std::vector<std::string> List() const;
        std::optional<RealBotInfo> GetInfo(std::string const& name) const;

        RealBotTargetList const& GetTargets() const { return m_targets; }
        // Free/total account counts for a target's lease pool (bot-examples.md #6's "targets"
        // command); nullopt if the target has no lease pool (i.e. isn't in GetTargets().All()).
        std::optional<std::pair<size_t, size_t>> GetLeaseCounts(std::string const& targetName) const;

        // Pushes a named task onto a bot, marshalled onto its own thread via RealBot::Post --
        // how the runner/.realbot's "task <bot> <task> [args]" command reaches a running bot from
        // whatever thread the console lives on (bot-examples.md #6).
        bool RunTask(std::string const& botName, std::string const& taskName, BotTaskArgs args);

    private:
        // Which target and account a bot was leased from, so Remove/RemoveAll can hand the
        // account back to that target's lease pool.
        struct BotLease
        {
            std::string TargetName;
            std::string AccountName;
        };

        void WorkerLoop();
        std::string AddWithOptions(RealBotLoginOptions options, std::string name);
        void ReleaseLease(BotLease const& lease);

        std::unique_ptr<IO::IoContext> m_ioContext;
        std::vector<std::thread> m_ioThreads;
        std::thread m_workerThread;
        std::atomic<bool> m_running{ false };

        // Populated once by Start() from RealBots.Targets, before any thread but this one runs --
        // never resized afterwards, so reading m_targets/m_leases via find() needs no lock of its
        // own; only each BotAccountLease's internal state is mutated concurrently.
        RealBotTargetList m_targets;
        std::unordered_map<std::string, BotAccountLease> m_leases;

        mutable std::mutex m_botsLock;
        std::unordered_map<std::string, std::shared_ptr<RealBot>> m_bots;
        std::unordered_map<std::string, BotLease> m_botLeases;
        uint32 m_nextId = 1;
};

#define sRealBotMgr MaNGOS::Singleton<RealBotMgr>::Instance()

#endif
