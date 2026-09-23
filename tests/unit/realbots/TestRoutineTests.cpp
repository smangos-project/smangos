#include "TestRoutine.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <stdexcept>

TEST_CASE("TestRoutine: RunTest drives a routine to completion", "[test-routine]")
{
    bool reached = false;

    RunTest([&]() -> TestRoutine
    {
        reached = true;
        co_return;
    });

    CHECK(reached);
}

TEST_CASE("TestRoutine: a delay resumes the routine from another thread", "[test-routine]")
{
    bool reached = false;

    RunTest([&]() -> TestRoutine
    {
        co_await TestDelay(20);
        reached = true;
        co_return;
    });

    CHECK(reached);
}

TEST_CASE("TestRoutine: an exception thrown inside is rethrown by RunTest", "[test-routine]")
{
    auto run = [&]()
    {
        RunTest([]() -> TestRoutine
        {
            co_await TestDelay(1);
            throw std::runtime_error("scenario failed");
            co_return;
        });
    };

    REQUIRE_THROWS_AS(run(), std::runtime_error);
}

TEST_CASE("TestRoutine: WhenAll waits for every awaiter", "[test-routine]")
{
    auto start = std::chrono::steady_clock::now();
    size_t completedCount = 0;

    RunTest([&]() -> TestRoutine
    {
        std::vector<TestDelay> delays;
        delays.emplace_back(10);
        delays.emplace_back(40);
        delays.emplace_back(20);

        co_await WhenAll(std::move(delays));
        completedCount = 3;
        co_return;
    });

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    CHECK(completedCount == 3);
    CHECK(elapsed.count() >= 40); // must have waited for the slowest one
}

TEST_CASE("TestRoutine: WhenAny resolves on the first completion", "[test-routine]")
{
    size_t firstIndex = 999;

    RunTest([&]() -> TestRoutine
    {
        std::vector<TestDelay> delays;
        delays.emplace_back(200);
        delays.emplace_back(5);
        delays.emplace_back(200);

        firstIndex = co_await WhenAny(std::move(delays));
        co_return;
    });

    CHECK(firstIndex == 1);
}
