#ifndef REALBOTS_TESTS_TESTBOTDRIVER_H
#define REALBOTS_TESTS_TESTBOTDRIVER_H

#include "BotAwaitables.h"
#include "BotContext.h"
#include "BotRoutine.h"
#include "IO/Context/IoContext.h"
#include "RealBot.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

// A real RealBot (its own IoContext-backed connections, hooks, the whole dispatch) ticked by a
// thread of its own, the way RealBotMgr's workers tick one, for unit tests that run the bot's
// routines against a fake server. The test thread never touches the bot directly: it Post()s onto
// the bot's thread (Query() waits for the result) and reads the published info, exactly like
// tests/conformance's TestBot.
class TestBotDriver
{
    public:
        // tickMs: how long the bot's thread sleeps between ticks (RealBotMgr's default is 100). A
        // short one keeps tests quick; a long one batches whatever the server sends between two
        // ticks into one, which is how the real host sees traffic.
        explicit TestBotDriver(IO::IoContext* ioContext, uint32 tickMs = 5)
            : m_bot(std::make_shared<RealBot>())
        {
            m_bot->SetName("testbot");
            m_bot->SetIoContext(ioContext);

            // REALBOTS_TEST_PACKET_LOG=<dir> writes this bot's traffic to <dir>/testbot.log, the same
            // file a real bot's packet log would produce.
            if (char const* dir = std::getenv("REALBOTS_TEST_PACKET_LOG"))
                m_bot->EnablePacketLog(dir);

            m_thread = std::thread([this, tickMs]()
            {
                auto last = std::chrono::steady_clock::now();
                while (m_running)
                {
                    auto const now = std::chrono::steady_clock::now();
                    uint32 const diff = uint32(std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count());
                    last = now;
                    m_bot->Tick(diff);
                    std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
                }
            });
        }

        ~TestBotDriver()
        {
            m_running = false;
            m_thread.join();
        }

        RealBot& Bot() { return *m_bot; }
        RealBotInfo Info() const { return m_bot->GetPublishedInfo(); }

        // Runs fn on the bot's thread and waits for it, returning what it returned.
        template<typename Fn>
        auto Query(Fn fn) -> decltype(fn(std::declval<RealBot&>()))
        {
            using Result = decltype(fn(std::declval<RealBot&>()));
            struct Slot
            {
                std::mutex Mutex;
                std::condition_variable Cv;
                bool Done = false;
                std::optional<Result> Value;
            };
            auto slot = std::make_shared<Slot>();

            m_bot->Post([this, slot, fn = std::move(fn)]() mutable
            {
                Result value = fn(*m_bot);
                std::lock_guard<std::mutex> lock(slot->Mutex);
                slot->Value = std::move(value);
                slot->Done = true;
                slot->Cv.notify_all();
            });

            std::unique_lock<std::mutex> lock(slot->Mutex);
            slot->Cv.wait(lock, [&]() { return slot->Done; });
            return std::move(*slot->Value);
        }

        // Pushes the routine `make(ctx)` returns, as a task named `name`. make must be a plain
        // function call that returns a BotRoutine (never a coroutine lambda itself).
        template<typename Make>
        void PushRoutine(char const* name, Make make)
        {
            Query([name, make = std::move(make)](RealBot& bot)
            {
                bot.GetBrain().PushTask(std::make_unique<BotRoutineTask>(name, make(bot.GetContext())), bot.GetContext());
                return true;
            });
        }

        bool WaitUntil(std::function<bool()> predicate, std::chrono::milliseconds timeout = std::chrono::milliseconds(8000)) const
        {
            auto const deadline = std::chrono::steady_clock::now() + timeout;
            while (!predicate())
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            return true;
        }

        bool WaitForStage(BotStage stage, std::chrono::milliseconds timeout = std::chrono::milliseconds(8000)) const
        {
            return WaitUntil([&]() { return Info().Stage == stage; }, timeout);
        }

        // True once every task on the bot's stack has finished. Asked on the bot's own thread, not
        // read from the published info, which lags a tick behind a push.
        bool WaitForIdleStack(std::chrono::milliseconds timeout = std::chrono::milliseconds(8000))
        {
            return WaitUntil([&]() { return Query([](RealBot& bot) { return bot.GetBrain().Empty(); }); }, timeout);
        }

    private:
        std::shared_ptr<RealBot> m_bot;
        std::atomic<bool> m_running{ true };
        std::thread m_thread;
};

#endif
