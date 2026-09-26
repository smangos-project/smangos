#include "TestBot.h"
#include "BotBrain.h"
#include "BotContext.h"
#include "BotTask.h"
#include "ConformanceRun.h"
#include "BotTaskRegistry.h"
#include "RealBot.h"
#include <stdexcept>

void TestStageAwaiter::Subscribe(std::function<void()> onReady)
{
    std::thread([this, bot = m_bot, target = m_target, timeoutMs = m_timeoutMs, requireHeld = m_requireHeld, onReady = std::move(onReady)]()
    {
        auto const deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        for (;;)
        {
            RealBotInfo const info = bot->GetPublishedInfo();
            if (info.Stage == target && (!requireHeld || info.Held))
            {
                m_reached = true;
                onReady();
                return;
            }
            // A failed bot never reaches any other stage: report that now instead of waiting out the
            // timeout (an expected-to-fail scenario would otherwise take its full bound to say so).
            if (info.Stage == BotStage::Failed && target != BotStage::Failed)
            {
                m_reached = false;
                onReady();
                return;
            }
            if (std::chrono::steady_clock::now() >= deadline)
            {
                m_reached = false;
                onReady();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }).detach();
}

void TestRawPacketAwaiter::Subscribe(std::function<void()> onReady)
{
    auto fired = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<TestPacketRecorder> recorder = m_recorder;

    uint64_t const watchId = recorder->Watch(
        [opcode = m_opcode](BotPacket const& packet) { return packet.Opcode == opcode; },
        [this, fired, onReady](BotPacket const& packet)
        {
            bool expected = false;
            if (!fired->compare_exchange_strong(expected, true))
                return;
            m_result = packet;
            onReady();
        });

    std::thread([fired, recorder, watchId, timeoutMs = m_timeoutMs, onReady]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        bool expected = false;
        if (fired->compare_exchange_strong(expected, true))
        {
            recorder->Unwatch(watchId);
            onReady();
        }
    }).detach();
}

void TestNoPacketAwaiter::Subscribe(std::function<void()> onReady)
{
    auto fired = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<TestPacketRecorder> recorder = m_recorder;

    uint64_t const watchId = recorder->Watch(
        [opcode = m_opcode](BotPacket const& packet) { return packet.Opcode == opcode; },
        [this, fired, onReady](BotPacket const&)
        {
            bool expected = false;
            if (!fired->compare_exchange_strong(expected, true))
                return;
            m_neverArrived = false; // it arrived -- that's the failure case for NoPacket
            onReady();
        });

    std::thread([this, fired, recorder, watchId, timeoutMs = m_timeoutMs, onReady]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        bool expected = false;
        if (fired->compare_exchange_strong(expected, true))
        {
            recorder->Unwatch(watchId);
            m_neverArrived = true;
            onReady();
        }
    }).detach();
}

TestBot::TestBot(TestBotOptions options)
{
    std::string const targetName = options.TargetOverride().empty() ? ConformanceRun::Target() : options.TargetOverride();

    RealBotAddOverrides overrides = options.Overrides();
    overrides.Keep = true; // the scenario, not the reaper, decides when this bot goes

    std::string error;
    m_name = sRealBotMgr.AddWithOverrides(targetName, overrides, "", &error);
    if (m_name.empty())
        throw std::runtime_error("TestBot: " + error);

    m_bot = sRealBotMgr.GetBot(m_name);
    m_recorder = std::make_shared<TestPacketRecorder>();

    std::shared_ptr<RealBot> bot = m_bot;
    std::shared_ptr<TestPacketRecorder> recorder = m_recorder;
    bot->Post([bot, recorder]()
    {
        bot->SetPacketObserver([recorder](BotPacket const& packet) { recorder->OnPacket(packet); });
    });
}

TestBot::~TestBot()
{
    if (!m_name.empty())
        sRealBotMgr.Remove(m_name);
}

void TestBot::SendRawPacket(BotPacket packet, std::optional<BotConnectionKind> connection) const
{
    std::shared_ptr<RealBot> bot = m_bot;
    bot->Post([bot, connection, packet = std::move(packet)]() mutable { bot->SendRaw(std::move(packet), connection); });
}

void TestBot::Disconnect(bool clean) const
{
    std::shared_ptr<RealBot> bot = m_bot;
    bot->Post([bot, clean]() { bot->Disconnect(clean); });
}

void TestBot::Reconnect() const
{
    m_bot->Reconnect();
}

void TestBot::PushTask(std::unique_ptr<BotTask> task) const
{
    std::shared_ptr<RealBot> bot = m_bot;
    auto shared = std::make_shared<std::unique_ptr<BotTask>>(std::move(task));
    bot->Post([bot, shared]() { bot->GetBrain().PushTask(std::move(*shared), bot->GetContext()); });
}

void TestBot::RunTask(std::string const& taskName, BotTaskArgs const& args) const
{
    std::shared_ptr<RealBot> bot = m_bot;
    bot->Post([bot, taskName, args]()
    {
        bot->GetBrain().PushTask(sBotTaskRegistry.Create(taskName, bot->GetContext(), args), bot->GetContext());
    });
}

BotTask* TestBot::AddHook(std::unique_ptr<BotTask> hook, bool persistent) const
{
    BotTask* handle = hook.get(); // the caller's thread owns it until the post below runs
    std::shared_ptr<RealBot> bot = m_bot;
    auto shared = std::make_shared<std::unique_ptr<BotTask>>(std::move(hook));
    bot->Post([bot, shared, persistent]() { bot->GetBrain().AddHook(std::move(*shared), bot->GetContext(), persistent); });
    return handle;
}

void TestBot::RemoveHook(BotTask* hook) const
{
    std::shared_ptr<RealBot> bot = m_bot;
    bot->Post([bot, hook]() { bot->GetBrain().RemoveHook(hook); });
}

std::string TestBot::GetLastError() const
{
    return m_bot->GetPublishedInfo().LastError;
}

BotStage TestBot::GetFailedStage() const
{
    return m_bot->GetPublishedInfo().FailedStage;
}

BotDisconnectReason TestBot::GetDisconnectReason() const
{
    return m_bot->GetPublishedInfo().DisconnectReason;
}

std::string TestBot::GetCharacterName() const
{
    return m_bot->GetPublishedInfo().CharacterName;
}

ClassicGuid TestBot::GetCharacterGuid() const
{
    return m_bot->GetPublishedInfo().CharacterGuid;
}

BotStage TestBot::GetStage() const
{
    return m_bot->GetPublishedInfo().Stage;
}

uint32_t TestBot::GetLatencyMs() const
{
    return m_bot->GetPublishedInfo().LatencyMs;
}
