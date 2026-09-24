#include "RealBotConfig.h"
#include "Config/Config.h"

bool RealBotConfig::Enabled()
{
    return sConfig.GetBoolDefault("RealBots.Enable", false);
}

uint32 RealBotConfig::IoThreads()
{
    return uint32(sConfig.GetIntDefault("RealBots.IoThreads", 1));
}

uint32 RealBotConfig::WorkerThreads()
{
    return uint32(sConfig.GetIntDefault("RealBots.WorkerThreads", 1));
}

uint32 RealBotConfig::TickIntervalMs()
{
    return uint32(sConfig.GetIntDefault("RealBots.TickIntervalMs", 100));
}

uint32 RealBotConfig::PingIntervalMs()
{
    return uint32(sConfig.GetIntDefault("RealBots.PingIntervalMs", 5000));
}

uint32 RealBotConfig::LogoutTimeoutMs()
{
    return uint32(sConfig.GetIntDefault("RealBots.LogoutTimeoutMs", 5000));
}

std::string RealBotConfig::DefaultTarget()
{
    return sConfig.GetStringDefault("RealBots.DefaultTarget", "");
}

std::string RealBotConfig::DefaultTasks()
{
    return sConfig.GetStringDefault("RealBots.DefaultTasks", "");
}

bool RealBotConfig::PacketLogEnabled()
{
    return sConfig.GetBoolDefault("RealBots.PacketLog", false);
}

std::string RealBotConfig::PacketLogDir()
{
    return sConfig.GetStringDefault("RealBots.PacketLogDir", "realbots-packets");
}

uint32 RealBotConfig::ReapAfterMs()
{
    return uint32(sConfig.GetIntDefault("RealBots.ReapAfterMs", 0));
}
