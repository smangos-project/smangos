#include "Percentiles.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Summarize: nearest-rank percentiles of a known set", "[percentiles]")
{
    // 1..100 ms: the p-th percentile is p itself under nearest rank.
    std::vector<uint32_t> samples;
    for (uint32_t i = 100; i >= 1; --i)
        samples.push_back(i); // deliberately unsorted

    PercentileSummary const summary = Summarize(samples);
    CHECK(summary.Count == 100);
    CHECK(summary.Min == 1);
    CHECK(summary.P50 == 50);
    CHECK(summary.P90 == 90);
    CHECK(summary.P99 == 99);
    CHECK(summary.Max == 100);
}

TEST_CASE("Summarize: small and degenerate sets", "[percentiles]")
{
    PercentileSummary empty = Summarize({});
    CHECK(empty.Count == 0);
    CHECK(empty.Max == 0);

    PercentileSummary one = Summarize({ 42 });
    CHECK(one.P50 == 42);
    CHECK(one.P99 == 42);
    CHECK(one.Min == 42);

    // Ten samples: p90 is the 9th smallest, p99 the 10th (ceil(9.9) = 10).
    PercentileSummary ten = Summarize({ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 });
    CHECK(ten.P50 == 50);
    CHECK(ten.P90 == 90);
    CHECK(ten.P99 == 100);
}

TEST_CASE("Describe formats every field", "[percentiles]")
{
    PercentileSummary const summary = Summarize({ 5, 5, 5 });
    CHECK(Describe(summary) == "n=3 min=5ms p50=5ms p90=5ms p99=5ms max=5ms");
}
