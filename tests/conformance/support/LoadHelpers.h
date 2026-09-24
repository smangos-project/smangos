#ifndef REALBOTS_TESTS_CONFORMANCE_LOADHELPERS_H
#define REALBOTS_TESTS_CONFORMANCE_LOADHELPERS_H

#include "BotTask.h"
#include "ConformanceRun.h"
#include "Percentiles.h"
#include "RealBotMgr.h"
#include "RealBotTarget.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Notes when a bot enters each stage, for the load scenarios' per-stage login-time table (a hook, so
// it costs the bot nothing and sees every stage change).
class StageTimings
{
    public:
        struct Entered
        {
            BotStage Stage;
            std::chrono::steady_clock::time_point At;
        };

        void Note(BotStage stage)
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_entered.push_back(Entered{ stage, std::chrono::steady_clock::now() });
        }

        std::vector<Entered> Entries() const
        {
            std::lock_guard<std::mutex> lock(m_lock);
            return m_entered;
        }

    private:
        mutable std::mutex m_lock;
        std::vector<Entered> m_entered;
};

class StageTimingHook : public BotTask
{
    public:
        explicit StageTimingHook(std::shared_ptr<StageTimings> timings) : m_timings(std::move(timings)) {}

        char const* GetName() const override { return "stage_timing"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }
        void OnStageChanged(BotContext&, BotStage, BotStage to) override { m_timings->Note(to); }

    private:
        std::shared_ptr<StageTimings> m_timings;
};

// How many accounts the target's pool has free right now.
inline size_t FreeAccounts()
{
    std::optional<std::pair<size_t, size_t>> counts = sRealBotMgr.GetLeaseCounts(ConformanceRun::Target());
    return counts ? counts->first : 0;
}

// Skips the scenario (never fails it) when `bots` bots cannot run: the pool has fewer free accounts,
// or the target caps [load] scenarios lower (RealBots.Target.X.LoadBots). The message says how many
// it needed.
inline void RequireLoadCapacity(size_t bots)
{
    size_t const cap = ConformanceRun::TargetConfig().LoadBots;
    if (cap != 0 && bots > cap)
        SKIP("needs " << bots << " bots, the target's LoadBots caps [load] scenarios at " << cap);

    size_t const free = FreeAccounts();
    if (free < bots)
        SKIP("needs " << bots << " free accounts, the target has " << free << " (see contrib/realbotrunner/provision-accounts.ps1)");
}

// For each stage the bots entered, how long each spent getting from entering it to entering the next
// one (a bot's last stage has no successor and is left out). Printed by the load scenarios.
inline std::string DescribeStageTimes(std::vector<std::shared_ptr<StageTimings>> const& allTimings)
{
    std::map<int, std::vector<uint32_t>> byStage;
    for (std::shared_ptr<StageTimings> const& timings : allTimings)
    {
        std::vector<StageTimings::Entered> const entries = timings->Entries();
        for (size_t i = 0; i + 1 < entries.size(); ++i)
        {
            auto const spent = std::chrono::duration_cast<std::chrono::milliseconds>(entries[i + 1].At - entries[i].At);
            byStage[int(entries[i].Stage)].push_back(uint32_t(spent.count()));
        }
    }

    std::string out = "login time per stage:";
    for (auto const& [stage, samples] : byStage)
        out += std::string("\n  ") + BotStageName(BotStage(stage)) + ": " + Describe(Summarize(samples));
    return out;
}

// Time from `start` until `bots` entered InWorld, per bot, in ms (bots that never did are absent).
inline std::vector<uint32_t> TimesToInWorld(std::vector<std::shared_ptr<StageTimings>> const& allTimings, std::chrono::steady_clock::time_point start)
{
    std::vector<uint32_t> out;
    for (std::shared_ptr<StageTimings> const& timings : allTimings)
    {
        for (StageTimings::Entered const& entered : timings->Entries())
        {
            if (entered.Stage == BotStage::InWorld)
            {
                out.push_back(uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(entered.At - start).count()));
                break;
            }
        }
    }
    return out;
}

#endif
