#ifndef REALBOTS_REALBOT_H
#define REALBOTS_REALBOT_H

#include "BotBrain.h"
#include "BotIdentity.h"
#include "BotPacket.h"
#include "BotStage.h"
#include "LockedQueue.h"
#include <memory>
#include <mutex>
#include <vector>

class BotContext;

// One bot. Touched only from its own worker thread (port-plan.md /
// bot-plan.md: "Bots touch no server runtime state"; nothing here reaches into
// sWorld, a Map, a Player or a live WorldSession). A RealBot with no connection
// at all -- ticked by hand, packets injected -- is exactly what TestFakeBot uses
// to exercise the coroutine layer without a server.
class RealBot
{
    public:
        RealBot();
        virtual ~RealBot() = default;

        void Tick(uint32 diff);

        BotStage GetStage() const { return m_stage; }
        void SetStage(BotStage stage);

        BotIdentity const& GetIdentity() const { return m_identity; }
        BotIdentity& GetIdentity() { return m_identity; }

        uint32 GetTimeMs() const { return m_elapsedMs; }
        uint32 GetLatencyMs() const { return m_latencyMs; }

        BotBrain& GetBrain() { return m_brain; }
        BotContext& GetContext() { return *m_context; }

        // Queues an incoming packet for dispatch on the next Tick. TestFakeBot uses
        // this directly to inject packets; a real connection will use it too once
        // Net/BotWorldConnection exists.
        void QueueIncoming(BotPacket packet);

        // Runs a packet through the task stack's OnPacket hooks (Outgoing) and, unless
        // a task returned Handled or Drop, records it as sent. There's no real wire yet.
        template<typename T>
        void Send(T const& packet)
        {
            BotPacket pkt;
            pkt.Opcode = T::Opcode;
            packet.Write(pkt.Data);
            SendRaw(std::move(pkt));
        }

        void SendRaw(BotPacket packet);
        std::vector<BotPacket> const& GetSentPackets() const { return m_sentPackets; }

    private:
        void ProcessIncoming(uint32 diff);

        BotStage m_stage = BotStage::Disconnected;
        BotIdentity m_identity;
        BotBrain m_brain;
        std::unique_ptr<BotContext> m_context;
        LockedQueue<BotPacket, std::mutex> m_incoming;
        std::vector<BotPacket> m_sentPackets;
        uint32 m_elapsedMs = 0;
        uint32 m_latencyMs = 0;
};

#endif
