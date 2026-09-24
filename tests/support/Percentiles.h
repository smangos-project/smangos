#ifndef REALBOTS_TESTS_PERCENTILES_H
#define REALBOTS_TESTS_PERCENTILES_H

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

// Nearest-rank percentiles of a set of millisecond samples: what the load scenarios print for each
// login stage (informational; only the overall bound is asserted).
struct PercentileSummary
{
    size_t Count = 0;
    uint32_t Min = 0;
    uint32_t P50 = 0;
    uint32_t P90 = 0;
    uint32_t P99 = 0;
    uint32_t Max = 0;
};

// Nearest rank: the p-th percentile of n samples is the ceil(p/100 * n)-th smallest.
inline uint32_t NearestRankPercentile(std::vector<uint32_t> const& sortedSamples, uint32_t percent)
{
    if (sortedSamples.empty())
        return 0;
    size_t const rank = (size_t(percent) * sortedSamples.size() + 99) / 100; // ceil
    return sortedSamples[std::min(sortedSamples.size(), std::max<size_t>(1, rank)) - 1];
}

inline PercentileSummary Summarize(std::vector<uint32_t> samples)
{
    PercentileSummary out;
    out.Count = samples.size();
    if (samples.empty())
        return out;

    std::sort(samples.begin(), samples.end());
    out.Min = samples.front();
    out.P50 = NearestRankPercentile(samples, 50);
    out.P90 = NearestRankPercentile(samples, 90);
    out.P99 = NearestRankPercentile(samples, 99);
    out.Max = samples.back();
    return out;
}

inline std::string Describe(PercentileSummary const& summary)
{
    return "n=" + std::to_string(summary.Count) + " min=" + std::to_string(summary.Min) + "ms p50=" + std::to_string(summary.P50)
        + "ms p90=" + std::to_string(summary.P90) + "ms p99=" + std::to_string(summary.P99) + "ms max=" + std::to_string(summary.Max) + "ms";
}

#endif
