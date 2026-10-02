#ifndef REALBOTS_TESTPACKETRECORDER_H
#define REALBOTS_TESTPACKETRECORDER_H

#include "BotPacket.h"
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
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
                m_history.push_back(packet);
                if (m_history.size() > MaxHistory)
                    m_history.pop_front();
                for (auto it = m_pending.begin(); it != m_pending.end();)
                {
                    if (it->Match(packet))
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

        // Every incoming packet the bot has seen (the last MaxHistory of them), for scenarios that
        // look back rather than wait ("what did the server send while the bot held at CharSelect?",
        // "did any packet arrive compressed?"). The awaiters below never see the backlog.
        std::vector<BotPacket> History() const
        {
            std::lock_guard<std::mutex> lock(m_lock);
            return std::vector<BotPacket>(m_history.begin(), m_history.end());
        }

        // The most recent packet with this opcode, or nothing if none has arrived.
        std::optional<BotPacket> Last(uint32_t opcode) const
        {
            std::lock_guard<std::mutex> lock(m_lock);
            for (auto it = m_history.rbegin(); it != m_history.rend(); ++it)
                if (it->Opcode == opcode)
                    return *it;
            return std::nullopt;
        }

        size_t Count(uint32_t opcode) const
        {
            std::lock_guard<std::mutex> lock(m_lock);
            size_t count = 0;
            for (BotPacket const& packet : m_history)
                if (packet.Opcode == opcode)
                    ++count;
            return count;
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
            Predicate Match;
            OnReady Ready;
        };

        static constexpr size_t MaxHistory = 4096;

        mutable std::mutex m_lock;
        std::deque<BotPacket> m_history;
        std::vector<Pending> m_pending;
        uint64_t m_nextId = 1;
};

#endif
