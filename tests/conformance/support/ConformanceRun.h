#ifndef REALBOTS_CONFORMANCERUN_H
#define REALBOTS_CONFORMANCERUN_H

#include <string>

// The name of the --target scenarios run against, set once by TestMain's custom Catch2 Session
// before it runs anything (bot-phase-1.md, "Conformance ... a target chosen on the command
// line"). Every scenario in login/, world_entry/, framework/ reads this via TestBot's default
// constructor rather than taking a target name itself, so `TestBot bot;` is enough
// (bot-examples.md #7's own style).
namespace ConformanceRun
{
    void SetTarget(std::string target);
    std::string const& Target();
}

struct RealBotTarget;

namespace ConformanceRun
{
    // The companion target holding accounts with game master rights, "<target>-gm" (a scenario that
    // needs one skips when it is not configured), or empty.
    std::string GmTarget();

    // The target's own config block (timeouts, tags, ServerCloseTimeoutMs, ...); the target must
    // exist, which TestMain checks before anything runs.
    RealBotTarget const& TargetConfig();
}

#endif
