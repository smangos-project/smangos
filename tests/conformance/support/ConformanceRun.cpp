#include "ConformanceRun.h"
#include "RealBotMgr.h"
#include "RealBotTarget.h"

namespace
{
std::string g_target;
}

void ConformanceRun::SetTarget(std::string target)
{
    g_target = std::move(target);
}

std::string const& ConformanceRun::Target()
{
    return g_target;
}

std::string ConformanceRun::GmTarget()
{
    std::string const name = g_target + "-gm";
    return sRealBotMgr.GetTargets().Find(name) ? name : std::string();
}

RealBotTarget const& ConformanceRun::TargetConfig()
{
    return *sRealBotMgr.GetTargets().Find(g_target);
}
