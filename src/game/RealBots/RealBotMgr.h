#ifndef REALBOTS_REALBOTMGR_H
#define REALBOTS_REALBOTMGR_H

#include "BotAccountLease.h"
#include "BotStage.h"
#include "BotTaskArgs.h"
#include "IO/Context/IoContext.h"
#include "Policies/Singleton.h"
#include "RealBot.h"
#include "RealBotTarget.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Per-bot overrides on top of a target's own defaults -- what TestBot (tests/conformance) needs
// to drive scenarios the plain `Add(target)` can't: a wrong password to fail on purpose, a stable
// or unique character name, and StopAt for mid-login scenarios (bot-examples.md's
// `TestBotOptions().StopAt(...).Password(...)`).
struct RealBotAddOverrides
{
    std::optional<std::string> AccountName;    // logs in as this account name instead of the leased one (an unknown account)
    std::optional<std::string> Password;
    std::optional<std::string> CharacterName;
    std::optional<bool> CreateIfNone;
    std::optional<uint8_t> Race;
    std::optional<uint8_t> Class;
    BotStage StopAt = BotStage::InWorld;
    std::optional<std::string> RealmName;      // "" joins by RealmId instead
    std::optional<uint32_t> RealmId;
    std::optional<bool> VerifyServerSignature;
    std::string LoginTicket;                   // RealBotLoginOptions::LoginTicketOverride
    std::optional<uint32_t> PacketTimeoutMs;
    std::optional<uint32_t> StageTimeoutMs;    // 0 turns the stage watchdog off
    // Hooks added before the bot's first tick, so a scenario can be sure they see the very first
    // packet; one factory call per bot (the bot owns what it makes). Persistent hooks survive a
    // disconnect (and so a Reconnect).
    struct InitialHook
    {
        std::function<std::unique_ptr<BotTask>()> Make;
        bool Persistent = false;
    };
    std::vector<InitialHook> Hooks;
    // The bot's owner manages its lifetime: Update never reaps it (RealBots.ReapAfterMs), however
    // long it sits Disconnected or Failed -- what a test that reconnects it needs.
    bool Keep = false;
    // Name of a running bot whose account this one logs in with too, without a second lease (two
    // sessions on one account, for the duplicate-login scenarios). The account goes back to the
    // pool when the last bot using it is removed.
    std::string ShareAccountWith;
};

// IoContext.h is included in full (not forward-declared) even though only a pointer to it is
// stored: any translation unit that includes Config.h (almost all of them -- it drags in
// Policies/SingletonImp.h) implicitly re-instantiates MaNGOS::Singleton<RealBotMgr>::Instance()
// for itself, including the `new RealBotMgr` inside it, whose exception-unwinding path needs
// std::unique_ptr<IO::IoContext>'s destructor -- a complete type -- right there, not just in
// RealBotMgr.cpp.

// Owns every bot's IoContext, IO thread(s) and worker pool, so both hosts (mangosd, `.realbot`;
// contrib/realbotrunner) start it the same way (bot-phase-1.md, "Framework"). RealBots.WorkerThreads
// worker threads each tick a fixed subset of bots (id % worker count, assigned once at Add and
// never rebalanced -- bot-plan.md's "Workers: bot pinned to id % count"), on a jittered interval
// (RealBots.TickIntervalMs +/- 20%) so many workers don't all wake up in lockstep.
class RealBotMgr
{
    public:
        // Out-of-line so m_ioContext's unique_ptr<IO::IoContext> doesn't need a complete
        // IoContext type wherever sRealBotMgr's static instance destructor gets instantiated
        // (i.e. every translation unit that touches sRealBotMgr, not just RealBotMgr.cpp).
        ~RealBotMgr();

        void Start();
        void Stop();
        // Lifecycle housekeeping, not ticking (that's the worker pool's job on its own threads):
        // reaps bots that have sat in Disconnected/Failed for RealBots.ReapAfterMs, releasing
        // their leases. Both hosts call this periodically (bot-phase-1.md, "Hosts") -- mangosd
        // from World::Update, the runner from its own timer alongside the console loop. A no-op
        // when RealBots.ReapAfterMs is 0 (the default).
        void Update(uint32 diff);

        // Leases a free account from the named target, creates a bot with it, gives it a name
        // (or one derived from the account name if empty), and pushes the built-in "login" task --
        // it starts logging in on its very next Tick. Empty return (with *error* set, if given)
        // means no such target or no free account left in its lease pool.
        std::string Add(std::string const& targetName, std::string name = "", std::string* error = nullptr);
        // Same lease-and-login as Add, with overrides applied on top of the target's own defaults
        // (RealBotAddOverrides) -- the entry point TestBot uses; Add() itself is unchanged so the
        // runner/.realbot's plain "add <target>" keeps working exactly as before.
        std::string AddWithOverrides(std::string const& targetName, RealBotAddOverrides const& overrides, std::string name = "", std::string* error = nullptr);
        void Remove(std::string const& name);
        void RemoveAll();
        // Logs the named bot in again (RealBot::Reconnect); false if there is no such bot.
        bool Reconnect(std::string const& name);

        std::vector<std::string> List() const;
        std::optional<RealBotInfo> GetInfo(std::string const& name) const;
        // The bot itself, for callers that need more than GetInfo's published snapshot --
        // tests/conformance's TestBot, which drives Send/SendRaw/Disconnect/SetPacketObserver
        // through RealBot's own already-thread-safe (Post-marshalled or lock-guarded) API rather
        // than RealBotMgr growing a forwarding method for each one.
        std::shared_ptr<RealBot> GetBot(std::string const& name) const;

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
            std::string Password; // the leased credential's own, for a bot that shares the account
        };

        // What a worker sleeps on: it wakes at its regular tick or as soon as any bot pinned to it
        // has work (RealBot::SetWakeCallback), so a login step is answered in microseconds instead
        // of at the next 100ms tick -- vibemangos sends SMSG_CONNECT_TO in parallel with loading the
        // character, and a client slow to open the Instance connection loses that race.
        struct WorkerSignal
        {
            std::mutex Mutex;
            std::condition_variable Cv;
            bool Wake = false;
        };

        void WorkerLoop(uint32 workerIndex);
        void WakeWorker(uint32 workerIndex);
        std::string AddWithOptions(RealBotLoginOptions options, std::string name, std::vector<RealBotAddOverrides::InitialHook> const* hooks = nullptr);
        // Drops one bot's use of its account; the account itself is released when it was the last.
        void ReleaseLease(BotLease const& lease);
        static std::string AccountKey(BotLease const& lease) { return lease.TargetName + "\n" + lease.AccountName; }

        std::unique_ptr<IO::IoContext> m_ioContext;
        std::vector<std::thread> m_ioThreads;
        std::vector<std::thread> m_workerThreads;
        std::vector<std::unique_ptr<WorkerSignal>> m_workerSignals;
        uint32 m_workerCount = 1;
        std::atomic<bool> m_running{ false };

        // Populated once by Start() from RealBots.Targets, before any thread but this one runs --
        // never resized afterwards, so reading m_targets/m_leases via find() needs no lock of its
        // own; only each BotAccountLease's internal state is mutated concurrently.
        RealBotTargetList m_targets;
        std::unordered_map<std::string, BotAccountLease> m_leases;

        mutable std::mutex m_botsLock;
        std::unordered_map<std::string, std::shared_ptr<RealBot>> m_bots;
        std::unordered_map<std::string, BotLease> m_botLeases;
        // How many bots are logged in on each leased account (more than one only with
        // ShareAccountWith), keyed by AccountKey. Guarded by m_botsLock.
        std::unordered_map<std::string, uint32> m_accountUsers;
        // Bots Update never reaps (RealBotAddOverrides::Keep). Guarded by m_botsLock.
        std::unordered_set<std::string> m_keepBots;
        // Which worker thread ticks each bot -- id % m_workerCount, fixed at Add time.
        std::unordered_map<std::string, uint32> m_botWorkerIndex;
        uint32 m_nextId = 1;

        // How long each bot has continuously been Disconnected/Failed, accumulated by Update();
        // only Update() itself touches this, so it needs no lock beyond the contract that hosts
        // call Update() from one thread at a time (bot-phase-1.md never has two).
        std::unordered_map<std::string, uint32> m_terminalSinceMs;
};

#define sRealBotMgr MaNGOS::Singleton<RealBotMgr>::Instance()

#endif
