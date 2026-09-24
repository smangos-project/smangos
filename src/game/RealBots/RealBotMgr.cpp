#include "RealBotMgr.h"
#include "BotTaskRegistry.h"
#include "IO/Context/IoContext.h"
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "RealBotConfig.h"
#include "Tasks/BuiltinBotTasks.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <utility>

INSTANTIATE_SINGLETON_1(RealBotMgr);

namespace
{
// +/- 20% of baseMs (bot-plan.md, "Workers: ... jittered tick"), so several worker threads
// (or several RealBotMgr-owning processes) don't all wake up in lockstep.
uint32 JitteredTickMs(uint32 baseMs)
{
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<double> jitter(0.8, 1.2);
    return uint32(std::max(1.0, double(baseMs) * jitter(rng)));
}
}

RealBotMgr::~RealBotMgr() = default;

void RealBotMgr::Start()
{
    RegisterBuiltinBotTasks();

    m_targets.Load();
    for (RealBotTarget const& target : m_targets.All())
        // try_emplace constructs the BotAccountLease in place (its std::mutex member makes it
        // neither copyable nor movable, so emplace(name, BotAccountLease(...)) -- which would
        // move a temporary into the map -- doesn't compile).
        m_leases.try_emplace(target.Name, target.Accounts);

    m_ioContext = IO::IoContext::CreateIoContext();
    if (!m_ioContext)
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "RealBotMgr: failed to create an IoContext");
        return;
    }

    m_running = true;

    uint32 const ioThreads = std::max<uint32>(1, RealBotConfig::IoThreads());
    for (uint32 i = 0; i < ioThreads; ++i)
        m_ioThreads.emplace_back([this]() { m_ioContext->RunUntilShutdown(); });

    m_workerCount = std::max<uint32>(1, RealBotConfig::WorkerThreads());
    for (uint32 i = 0; i < m_workerCount; ++i)
        m_workerThreads.emplace_back([this, i]() { WorkerLoop(i); });

    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "RealBotMgr: started (%u IO thread(s), %u worker(s))", ioThreads, m_workerCount);
}

void RealBotMgr::Stop()
{
    if (!m_running)
        return;

    m_running = false;
    for (std::thread& t : m_workerThreads)
        if (t.joinable())
            t.join();
    m_workerThreads.clear();

    // Bots' connections close while the IoContext is still running, so their async Close()
    // actually completes instead of leaking a half-closed socket.
    RemoveAll();

    if (m_ioContext)
        m_ioContext->Shutdown();

    for (std::thread& t : m_ioThreads)
        if (t.joinable())
            t.join();
    m_ioThreads.clear();

    m_ioContext.reset();
}

void RealBotMgr::Update(uint32 diff)
{
    uint32 const reapAfterMs = RealBotConfig::ReapAfterMs();
    if (reapAfterMs == 0)
        return;

    std::vector<std::string> const names = List();
    std::vector<std::string> toRemove;

    for (std::string const& name : names)
    {
        std::optional<RealBotInfo> info = GetInfo(name);
        if (!info)
            continue;

        bool const terminal = info->Stage == BotStage::Disconnected || info->Stage == BotStage::Failed;
        if (!terminal)
        {
            m_terminalSinceMs.erase(name);
            continue;
        }

        uint32& elapsed = m_terminalSinceMs[name];
        elapsed += diff;
        if (elapsed >= reapAfterMs)
            toRemove.push_back(name);
    }

    // Bots removed elsewhere (an explicit "remove") drop out of the timer map too, so it doesn't
    // grow forever across many short-lived bots.
    for (auto it = m_terminalSinceMs.begin(); it != m_terminalSinceMs.end();)
        it = std::find(names.begin(), names.end(), it->first) == names.end() ? m_terminalSinceMs.erase(it) : std::next(it);

    for (std::string const& name : toRemove)
        Remove(name);
}

void RealBotMgr::WorkerLoop(uint32 workerIndex)
{
    uint32 const tickMs = std::max<uint32>(1, RealBotConfig::TickIntervalMs());
    auto last = std::chrono::steady_clock::now();

    while (m_running)
    {
        auto const now = std::chrono::steady_clock::now();
        uint32 const diff = uint32(std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count());
        last = now;

        {
            std::lock_guard<std::mutex> lock(m_botsLock);
            for (auto& [name, bot] : m_bots)
            {
                auto const it = m_botWorkerIndex.find(name);
                if (it != m_botWorkerIndex.end() && it->second == workerIndex)
                    bot->Tick(diff);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(JitteredTickMs(tickMs)));
    }
}

std::string RealBotMgr::Add(std::string const& targetName, std::string name, std::string* error)
{
    return AddWithOverrides(targetName, RealBotAddOverrides{}, std::move(name), error);
}

std::string RealBotMgr::AddWithOverrides(std::string const& targetName, RealBotAddOverrides const& overrides, std::string name, std::string* error)
{
    RealBotTarget const* target = m_targets.Find(targetName);
    if (!target)
    {
        if (error)
            *error = "no such target '" + targetName + "'";
        return "";
    }

    auto leaseIt = m_leases.find(targetName);
    std::optional<RealBotAccountCredential> account = leaseIt != m_leases.end() ? leaseIt->second.Lease() : std::nullopt;
    if (!account)
    {
        if (error)
            *error = "no free accounts left for target '" + targetName + "'";
        return "";
    }

    RealBotLoginOptions options;
    options.LoginAddress = target->LoginAddress;
    options.RestPort = target->RestPort;
    options.RpcPort = target->RpcPort;
    options.AccountName = account->Name;
    options.Password = overrides.Password.value_or(account->Password);
    options.RealmName = target->RealmName;
    options.RealmId = target->RealmId;
    options.ClientBuild = target->ClientBuild;
    options.VerifyServerSignature = target->VerifyServerSignature;
    if (overrides.CharacterName)
        options.CharacterName = *overrides.CharacterName;
    if (overrides.CreateIfNone)
        options.CreateIfNone = *overrides.CreateIfNone;
    if (overrides.Race)
        options.Race = *overrides.Race;
    if (overrides.Class)
        options.Class = *overrides.Class;
    options.StopAt = overrides.StopAt;

    std::string const botName = AddWithOptions(std::move(options), std::move(name));

    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        m_botLeases[botName] = BotLease{ targetName, account->Name };
    }

    return botName;
}

std::string RealBotMgr::AddWithOptions(RealBotLoginOptions options, std::string name)
{
    uint32 const id = m_nextId;

    if (name.empty())
        name = options.AccountName.empty() ? ("bot" + std::to_string(id)) : options.AccountName;

    std::shared_ptr<RealBot> bot = std::make_shared<RealBot>();
    bot->SetName(name);
    bot->SetIoContext(m_ioContext.get());
    bot->GetLoginOptions() = std::move(options);

    if (RealBotConfig::PacketLogEnabled())
        bot->EnablePacketLog(RealBotConfig::PacketLogDir());

    bot->GetBrain().PushTask(sBotTaskRegistry.Create("login", bot->GetContext(), BotTaskArgs()), bot->GetContext());

    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        m_bots[name] = bot;
        m_botWorkerIndex[name] = id % m_workerCount;
    }
    ++m_nextId;

    return name;
}

void RealBotMgr::Remove(std::string const& name)
{
    std::shared_ptr<RealBot> bot;
    std::optional<BotLease> lease;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        auto it = m_bots.find(name);
        if (it == m_bots.end())
            return;
        bot = std::move(it->second);
        m_bots.erase(it);
        m_botWorkerIndex.erase(name);

        auto leaseIt = m_botLeases.find(name);
        if (leaseIt != m_botLeases.end())
        {
            lease = leaseIt->second;
            m_botLeases.erase(leaseIt);
        }
    }
    // `bot` is the last shared_ptr once this scope ends (the worker loop only ever borrows a
    // reference under m_botsLock, above, never keeps its own) -- destroying it here runs
    // ~RealBot, which destroys the brain without resuming any suspended coroutine
    // (bot-phase-1.md's rule) and closes its connections.
    if (lease)
        ReleaseLease(*lease);
}

void RealBotMgr::RemoveAll()
{
    std::unordered_map<std::string, std::shared_ptr<RealBot>> bots;
    std::unordered_map<std::string, BotLease> leases;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        bots = std::move(m_bots);
        m_bots.clear();
        leases = std::move(m_botLeases);
        m_botLeases.clear();
        m_botWorkerIndex.clear();
    }

    for (auto const& [name, lease] : leases)
        ReleaseLease(lease);
}

void RealBotMgr::ReleaseLease(BotLease const& lease)
{
    auto it = m_leases.find(lease.TargetName);
    if (it != m_leases.end())
        it->second.Release(lease.AccountName);
}

std::optional<std::pair<size_t, size_t>> RealBotMgr::GetLeaseCounts(std::string const& targetName) const
{
    auto it = m_leases.find(targetName);
    if (it == m_leases.end())
        return std::nullopt;
    return std::make_pair(it->second.Free(), it->second.Total());
}

std::vector<std::string> RealBotMgr::List() const
{
    std::lock_guard<std::mutex> lock(m_botsLock);
    std::vector<std::string> names;
    names.reserve(m_bots.size());
    for (auto const& [name, bot] : m_bots)
        names.push_back(name);
    return names;
}

std::optional<RealBotInfo> RealBotMgr::GetInfo(std::string const& name) const
{
    std::shared_ptr<RealBot> bot;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        auto it = m_bots.find(name);
        if (it == m_bots.end())
            return std::nullopt;
        bot = it->second;
    }
    return bot->GetPublishedInfo();
}

bool RealBotMgr::RunTask(std::string const& botName, std::string const& taskName, BotTaskArgs args)
{
    if (!sBotTaskRegistry.Contains(taskName))
        return false;

    std::shared_ptr<RealBot> bot;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        auto it = m_bots.find(botName);
        if (it == m_bots.end())
            return false;
        bot = it->second;
    }

    // PushTask must only ever run on the bot's own thread (bot-plan.md: "each bot lives on one
    // worker thread and needs no locks"); Post marshals it there from whatever thread called us.
    bot->Post([bot, taskName, args]()
    {
        bot->GetBrain().PushTask(sBotTaskRegistry.Create(taskName, bot->GetContext(), args), bot->GetContext());
    });
    return true;
}
