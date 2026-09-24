#include "BotContext.h"
#include "BotAwaitables.h"
#include "BotRoutine.h"
#include "BotTaskRegistry.h"
#include "RealBot.h"
#include "Log.h"
#include <cstdarg>
#include <cstdio>

BotStage BotContext::GetStage() const
{
    return m_bot.GetStage();
}

void BotContext::SetStage(BotStage stage)
{
    m_bot.SetStage(stage);
}

BotIdentity const& BotContext::GetIdentity() const
{
    return m_bot.GetIdentity();
}

uint32 BotContext::GetLatencyMs() const
{
    return m_bot.GetLatencyMs();
}

uint32 BotContext::GetTimeMs() const
{
    return m_bot.GetTimeMs();
}

void BotContext::Disconnect(bool clean)
{
    m_bot.Disconnect(clean);
}

namespace
{
// A free function, not a BotContext member: sLog expands to MaNGOS::Singleton<Log>,
// and unqualified `Log` there would otherwise resolve to BotContext::Log itself.
void WriteBotLog(char const* text)
{
    sLog.Out(LOG_BASIC, LOG_LVL_BASIC, "[RealBot] %s", text);
}
}

void BotContext::Log(char const* fmt, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    WriteBotLog(buffer);
}

void BotContext::PushTask(std::unique_ptr<BotTask> task)
{
    m_bot.GetBrain().PushTask(std::move(task), *this);
}

BotTask* BotContext::AddHook(std::unique_ptr<BotTask> hook, bool persistent)
{
    return m_bot.GetBrain().AddHook(std::move(hook), *this, persistent);
}

void BotContext::RemoveHook(BotTask* hook)
{
    m_bot.GetBrain().RemoveHook(hook);
}

RawPacketAwaiter BotContext::RawPacket(uint32 opcode, uint32 timeoutMs)
{
    return RawPacketAwaiter(m_bot, opcode, timeoutMs);
}

StageAwaiter BotContext::Stage(BotStage stage, uint32 timeoutMs)
{
    return StageAwaiter(m_bot, stage, timeoutMs);
}

DelayAwaiter BotContext::Delay(uint32 ms)
{
    return DelayAwaiter(ms);
}

RunAwaiter BotContext::Run(std::string taskName, BotTaskArgs args)
{
    return RunAwaiter(m_bot.GetBrain(), *this, sBotTaskRegistry.Create(taskName, *this, args));
}

RunAwaiter BotContext::Run(BotRoutine routine)
{
    return RunAwaiter(m_bot.GetBrain(), *this, std::make_unique<BotRoutineTask>("run", std::move(routine)));
}

HoldAwaiter BotContext::Hold()
{
    return HoldAwaiter();
}
