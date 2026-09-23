#ifndef REALBOTS_TESTROUTINE_H
#define REALBOTS_TESTROUTINE_H

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// Posts coroutine handles from any thread to the single test thread that drives
// them (bot-phase-1.md, "TestRoutine: tests, on the test thread").
class TestExecutor
{
    public:
        void Post(std::coroutine_handle<> handle)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push_back(handle);
            }
            m_cv.notify_one();
        }

        std::coroutine_handle<> WaitAndPop()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]() { return !m_queue.empty(); });
            std::coroutine_handle<> handle = m_queue.front();
            m_queue.pop_front();
            return handle;
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::deque<std::coroutine_handle<>> m_queue;
};

// A test scenario, run to completion on the test thread by RunTest. Every
// REQUIRE/CHECK inside it runs there too, so Catch2 assertions work as usual.
struct TestRoutine
{
    struct promise_type
    {
        std::exception_ptr Exception;
        bool Finished = false;
        TestExecutor* Executor = nullptr;

        TestRoutine get_return_object();
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() { Finished = true; }
        void unhandled_exception()
        {
            Exception = std::current_exception();
            Finished = true;
        }
    };

    using HandleType = std::coroutine_handle<promise_type>;

    explicit TestRoutine(HandleType handle) : m_handle(handle) {}
    TestRoutine(TestRoutine const&) = delete;
    TestRoutine& operator=(TestRoutine const&) = delete;
    TestRoutine(TestRoutine&& other) noexcept : m_handle(std::exchange(other.m_handle, {})) {}
    ~TestRoutine() { if (m_handle) m_handle.destroy(); }

    HandleType GetHandle() const { return m_handle; }

    private:
        HandleType m_handle;
};

inline TestRoutine TestRoutine::promise_type::get_return_object()
{
    return TestRoutine{ HandleType::from_promise(*this) };
}

// Drives factory() (a lambda returning a TestRoutine) to completion and rethrows
// whatever it threw. If factory is itself the coroutine (the usual case: a lambda
// marked -> TestRoutine with co_await inside), calling it only creates the frame;
// the frame keeps an implicit pointer back to the closure, so the closure has to
// outlive every resume. Storing it here works because RunTest itself doesn't
// return until the routine is Finished.
template<typename Factory>
void RunTest(Factory&& factory)
{
    TestExecutor executor;
    Factory factoryStorage = std::forward<Factory>(factory);
    TestRoutine routine = factoryStorage();
    routine.GetHandle().promise().Executor = &executor;
    routine.GetHandle().resume();

    while (!routine.GetHandle().promise().Finished)
    {
        std::coroutine_handle<> next = executor.WaitAndPop();
        next.resume();
    }

    if (std::exception_ptr ex = routine.GetHandle().promise().Exception)
        std::rethrow_exception(ex);
}

// A free timer usable from a TestRoutine, and the primitive every WhenAll/WhenAny
// awaiter subscribes to: Subscribe() arranges for onReady() to run, from any
// thread, exactly once, when the wait is over.
class TestDelay
{
    public:
        explicit TestDelay(uint32_t ms) : m_ms(ms) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            Subscribe([executor, handle]() { executor->Post(handle); });
        }

        void await_resume() const noexcept {}

        void Subscribe(std::function<void()> onReady)
        {
            std::thread([ms = m_ms, onReady = std::move(onReady)]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                onReady();
            }).detach();
        }

    private:
        uint32_t m_ms;
};

// co_await WhenAll(std::move(awaiters)) -> std::vector<Result>, one per awaiter,
// in the same order. Waits for every one of them.
template<typename Awaiter>
class WhenAllAwaiter
{
    public:
        using Result = decltype(std::declval<Awaiter&>().await_resume());

        explicit WhenAllAwaiter(std::vector<Awaiter> awaiters) : m_awaiters(std::move(awaiters)) {}

        bool await_ready() const noexcept { return m_awaiters.empty(); }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            auto remaining = std::make_shared<std::atomic<size_t>>(m_awaiters.size());

            for (Awaiter& awaiter : m_awaiters)
            {
                awaiter.Subscribe([remaining, executor, handle]()
                {
                    if (remaining->fetch_sub(1, std::memory_order_acq_rel) == 1)
                        executor->Post(handle);
                });
            }
        }

        auto await_resume()
        {
            if constexpr (std::is_void_v<Result>)
            {
                for (Awaiter& awaiter : m_awaiters)
                    awaiter.await_resume();
            }
            else
            {
                std::vector<Result> results;
                results.reserve(m_awaiters.size());
                for (Awaiter& awaiter : m_awaiters)
                    results.push_back(awaiter.await_resume());
                return results;
            }
        }

    private:
        std::vector<Awaiter> m_awaiters;
};

// co_await WhenAny(std::move(awaiters)) -> the index of whichever finished first.
// The rest are left to finish on their own; their results are not collected.
template<typename Awaiter>
class WhenAnyAwaiter
{
    public:
        explicit WhenAnyAwaiter(std::vector<Awaiter> awaiters) : m_awaiters(std::move(awaiters)) {}

        bool await_ready() const noexcept { return m_awaiters.empty(); }

        void await_suspend(TestRoutine::HandleType handle)
        {
            TestExecutor* executor = handle.promise().Executor;
            auto fired = std::make_shared<std::atomic<bool>>(false);
            auto firstIndex = m_firstIndex;

            for (size_t i = 0; i < m_awaiters.size(); ++i)
            {
                m_awaiters[i].Subscribe([fired, firstIndex, executor, handle, i]()
                {
                    bool expected = false;
                    if (fired->compare_exchange_strong(expected, true))
                    {
                        *firstIndex = i;
                        executor->Post(handle);
                    }
                });
            }
        }

        size_t await_resume() const { return *m_firstIndex; }

    private:
        std::vector<Awaiter> m_awaiters;
        std::shared_ptr<size_t> m_firstIndex = std::make_shared<size_t>(0);
};

template<typename Awaiter>
WhenAllAwaiter<Awaiter> WhenAll(std::vector<Awaiter> awaiters)
{
    return WhenAllAwaiter<Awaiter>(std::move(awaiters));
}

template<typename Range, typename Func>
auto WhenAll(Range& range, Func&& func)
{
    using Awaiter = decltype(func(*std::begin(range)));
    std::vector<Awaiter> awaiters;
    for (auto& item : range)
        awaiters.push_back(func(item));
    return WhenAllAwaiter<Awaiter>(std::move(awaiters));
}

template<typename Awaiter>
WhenAnyAwaiter<Awaiter> WhenAny(std::vector<Awaiter> awaiters)
{
    return WhenAnyAwaiter<Awaiter>(std::move(awaiters));
}

#endif
