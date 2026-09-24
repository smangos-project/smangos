#ifndef REALBOTS_REALBOTCONFIG_H
#define REALBOTS_REALBOTCONFIG_H

#include "Common.h"
#include <string>

// Global RealBots.* config keys (bot-phase-1.md, "Targets" -> "Global keys"). Each getter reads
// sConfig directly, never cached -- safe to call from any thread, and picks up sConfig.Reload().
// Per-target keys (RealBotTarget) come later, in bot-phase-1.md's step 6.
namespace RealBotConfig
{
    // mangosd only -- gates whether World::SetInitialWorldSettings starts sRealBotMgr at all.
    // contrib/realbotrunner ignores this and always starts (bot-phase-1.md, "Global keys").
    bool Enabled();
    uint32 IoThreads();
    uint32 WorkerThreads();
    uint32 TickIntervalMs();
    uint32 PingIntervalMs();
    uint32 LogoutTimeoutMs();
    std::string DefaultTarget();
    std::string DefaultTasks();
    bool PacketLogEnabled();
    std::string PacketLogDir();
    // How long a bot must sit in Disconnected/Failed before RealBotMgr::Update auto-removes it
    // and frees its leased account. 0 (the default) disables auto-reaping entirely, so a Failed
    // bot stays inspectable (info/list) until an operator explicitly removes it.
    uint32 ReapAfterMs();
}

#endif
