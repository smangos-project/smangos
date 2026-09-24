#include "ConformanceRun.h"

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
