#ifndef REALBOTS_TESTPACKETRECORDER_H
#define REALBOTS_TESTPACKETRECORDER_H

#include "BotPacket.h"
#include <functional>
#include <mutex>
#include <vector>

// The bot-side half of TestBot's packet awaitables (bot-phase-1.md's "TestRecorderTask (bot
// thread) completes a pending test awaitable by posting to the executor's queue"). Owned by
// TestBot itself (via shared_ptr, so it stays alive independent of the RealBot/BotBrain it
// watches -- RealBot::SetPacketObserver's whole point is surviving BotBrain::Clear()), and fed
// every Incoming packet through OnPacket, called on the bot's own worker thread. Watch/Unwatch
// are the only calls made from another thread (a TestBot awaiter's await_suspend/destructor),
// safe because they only ever touch this object's own m_lock, never RealBot/BotBrain state.
class TestPacketRecorder
{
    public:
        using Predicate = std::function<bool(BotPacket const&)>;
        using OnReady = std::function<void(BotPacket const&)>;

        void OnPacket(BotPacket const& packet)
        {
            std::vector<OnReady> toFire;
            {
                std::lock_guard<std::mutex> lock(m_lock);
                for (auto it = m_pending.begin(); it != m_pending.end();)
                {
                    if (it->Predicate(packet))
                    {
                        toFire.push_back(std::move(it->Ready));
                        it = m_pending.erase(it);
                    }
                    else
                        ++it;
                }
            }
            for (OnReady& ready : toFire)
                ready(packet);
        }

        // Only ever matches packets from here on -- there's no backlog, exactly like
        // BotContext::Packet<T> (bot-phase-1.md: "Awaiting a packet only observes it").
        uint64_t Watch(Predicate predicate, OnReady ready)
        {
            std::lock_guard<std::mutex> lock(m_lock);
            uint64_t const id = m_nextId++;
            m_pending.push_back(Pending{ id, std::move(predicate), std::move(ready) });
            return id;
        }

        void Unwatch(uint64_t id)
        {
            std::lock_guard<std::mutex> lock(m_lock);
            for (auto it = m_pending.begin(); it != m_pending.end(); ++it)
            {
                if (it->Id == id)
                {
                    m_pending.erase(it);
                    return;
                }
            }
        }

    private:
        struct Pending
        {
            uint64_t Id;
            Predicate Predicate;
            OnReady Ready;
        };

        mutable std::mutex m_lock;
        std::vector<Pending> m_pending;
        uint64_t m_nextId = 1;
};

#endif
