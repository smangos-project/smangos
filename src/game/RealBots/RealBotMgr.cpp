#include "RealBotMgr.h"
#include "BotTaskRegistry.h"
#include "IO/Context/IoContext.h"
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "RealBotConfig.h"
#include "Tasks/BuiltinBotTasks.h"
#include <algorithm>
#include <chrono>

INSTANTIATE_SINGLETON_1(RealBotMgr);

RealBotMgr::~RealBotMgr() = default;

void RealBotMgr::Start()
{
    RegisterBuiltinBotTasks();

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

    m_workerThread = std::thread([this]() { WorkerLoop(); });

    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "RealBotMgr: started (%u IO thread(s))", ioThreads);
}

void RealBotMgr::Stop()
{
    if (!m_running)
        return;

    m_running = false;
    if (m_workerThread.joinable())
        m_workerThread.join();

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

void RealBotMgr::WorkerLoop()
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
                bot->Tick(diff);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
    }
}

std::string RealBotMgr::Add(RealBotLoginOptions options, std::string name)
{
    if (name.empty())
        name = options.AccountName.empty() ? ("bot" + std::to_string(m_nextId)) : options.AccountName;

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
    }
    ++m_nextId;

    return name;
}

void RealBotMgr::Remove(std::string const& name)
{
    std::shared_ptr<RealBot> bot;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        auto it = m_bots.find(name);
        if (it == m_bots.end())
            return;
        bot = std::move(it->second);
        m_bots.erase(it);
    }
    // `bot` is the last shared_ptr once this scope ends (the worker loop only ever borrows a
    // reference under m_botsLock, above, never keeps its own) -- destroying it here runs
    // ~RealBot, which destroys the brain without resuming any suspended coroutine
    // (bot-phase-1.md's rule) and closes its connections.
}

void RealBotMgr::RemoveAll()
{
    std::unordered_map<std::string, std::shared_ptr<RealBot>> bots;
    {
        std::lock_guard<std::mutex> lock(m_botsLock);
        bots = std::move(m_bots);
        m_bots.clear();
    }
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
