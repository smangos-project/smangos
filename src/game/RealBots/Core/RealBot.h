#ifndef REALBOTS_REALBOT_H
#define REALBOTS_REALBOT_H

#include "BotBrain.h"
#include "BotIdentity.h"
#include "BotPacket.h"
#include "BotStage.h"
#include "LockedQueue.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class BotContext;
class BotWorldConnection;
class BotPacketLog;
namespace IO { class IoContext; }

// Login options a bot's built-in "login" task drives once it starts (Tasks/BuiltinBotTasks.cpp),
// set once before the bot starts ticking (RealBotMgr::Add). Mirrors BotLogin::Options one field
// at a time rather than including it directly, so RealBot.h (used bare by TestFakeBot and the
// coroutine unit tests) never has to pull in the whole Net/ chain.
struct RealBotLoginOptions
{
    std::string LoginAddress;
    uint16_t RestPort = 0;
    uint16_t RpcPort = 0;
    std::string AccountName;
    std::string Password;
    std::string RealmName;
    uint32_t RealmId = 0;
    int32_t ClientBuild = 0;
    bool VerifyServerSignature = true;
    bool CreateIfNone = true;
    std::string CharacterName;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Gender = 0;
    float FarClip = 533.0f;
    // Holds the login routine at this stage instead of continuing past it -- InWorld (the
    // default) means "don't stop early" (bot-phase-1.md, "Login": "stop_at=<stage>").
    BotStage StopAt = BotStage::InWorld;
};

// A snapshot of a bot's state, safe to read from any thread -- published once per Tick, after
// the brain has run (bot-plan.md's "Thread-safe reads ... published info (copy under a mutex)").
// What RealBotMgr's list/info commands read; never touched by the bot's own logic.
struct RealBotInfo
{
    std::string Name;
    BotStage Stage = BotStage::Disconnected;
    uint32 LatencyMs = 0;
    std::string CharacterName;
    std::string LastError;
    // Task names, top of the stack first (BotBrain::TaskNames) -- what "tasks <name>" shows.
    std::vector<std::string> TaskStack;
    // The stage the bot was at right before it became BotStage::Failed (Disconnected otherwise);
    // "info"'s "stage Failed (was CharSelect)" and TestBot::GetFailedStage() (bot-examples.md #6/#7).
    BotStage FailedStage = BotStage::Disconnected;
    BotDisconnectReason DisconnectReason = BotDisconnectReason::None;
};

// One bot. Touched only from its own worker thread (port-plan.md /
// bot-plan.md: "Bots touch no server runtime state"; nothing here reaches into
// sWorld, a Map, a Player or a live WorldSession). A RealBot with no connection
// at all -- ticked by hand, packets injected -- is exactly what TestFakeBot uses
// to exercise the coroutine layer without a server; RealBotMgr's bots are the
// same class with a live BotWorldConnection pair attached.
class RealBot : public std::enable_shared_from_this<RealBot>
{
    public:
        RealBot();
        virtual ~RealBot();

        void SetName(std::string name) { m_name = std::move(name); }
        std::string const& GetName() const { return m_name; }

        // The IoContext BotLogin/BotWorldConnection run their networking on -- set once by
        // RealBotMgr::Add, null for TestFakeBot (which never logs in for real).
        void SetIoContext(IO::IoContext* ioContext) { m_ioContext = ioContext; }
        IO::IoContext* GetIoContext() const { return m_ioContext; }

        RealBotLoginOptions& GetLoginOptions() { return m_loginOptions; }
        RealBotLoginOptions const& GetLoginOptions() const { return m_loginOptions; }

        // Opens this bot's own file under dir (bot-phase-1.md, "Packet log"); every Send/
        // QueueIncoming from then on is written to it.
        void EnablePacketLog(std::string const& dir);

        void Tick(uint32 diff);

        BotStage GetStage() const { return m_stage; }
        void SetStage(BotStage stage);

        void SetLastError(std::string error) { m_lastError = std::move(error); }
        std::string const& GetLastError() const { return m_lastError; }

        BotStage GetFailedStage() const { return m_failedStage; }
        BotDisconnectReason GetDisconnectReason() const { return m_disconnectReason; }

        BotIdentity const& GetIdentity() const { return m_identity; }
        BotIdentity& GetIdentity() { return m_identity; }

        uint32 GetTimeMs() const { return m_elapsedMs; }
        uint32 GetLatencyMs() const { return m_latencyMs; }

        BotBrain& GetBrain() { return m_brain; }
        BotContext& GetContext() { return *m_context; }

        // Attaches a live connection pair -- typically BotLogin::Result's Connection (Instance,
        // used for Send/SendRaw from here on) and RealmConnection (kept open only for what still
        // routes through it: SMSG_PONG, SMSG_LOGOUT_COMPLETE). RealmConnection may be null.
        // Bot-thread only: called from the "login" task once BotLogin hands back its result via
        // Post (bot-plan.md: connections are handed off, never touched cross-thread directly).
        void AttachConnections(std::shared_ptr<BotWorldConnection> connection, std::shared_ptr<BotWorldConnection> realmConnection);
        void CloseConnections();

        // Posts a callback to run on this bot's own thread, on its next Tick -- how anything
        // running on another thread (an IO completion, a BotLogin result) safely reaches into a
        // bot without touching its state directly (bot-plan.md, "IO threads only move bytes into
        // a bot's inbox"). Safe to call from any thread, including this bot's own.
        void Post(std::function<void()> fn);

        // Queues an incoming packet for dispatch on the next Tick. TestFakeBot uses this directly
        // to inject packets; a real connection's packet callback reaches it through Post.
        void QueueIncoming(BotPacket packet);

        // Runs a packet through the task stack's OnPacket hooks (Outgoing) and, unless a task
        // returned Handled or Drop, writes it to the attached connection (if any) and records it.
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

        // clean: sends CLASSIC_CMSG_LOGOUT_REQUEST and waits up to RealBots.LogoutTimeoutMs for
        // CLASSIC_SMSG_LOGOUT_COMPLETE before moving to Disconnected; otherwise (or if there's no
        // connection to ask) moves to Disconnected immediately (bot-phase-1.md, "Login").
        void Disconnect(bool clean);

        RealBotInfo GetPublishedInfo() const;

    private:
        void ProcessCommands();
        void ProcessIncoming(uint32 diff);
        void UpdateLogout(uint32 diff);
        void UpdateKeepalive(uint32 diff);
        void ApplyDefaultHandling(BotPacket const& packet, BotPacketDir dir);
        void PublishInfo();
        void LogPacket(BotPacket const& packet, BotPacketDir dir);

        std::string m_name;
        IO::IoContext* m_ioContext = nullptr;
        RealBotLoginOptions m_loginOptions;

        BotStage m_stage = BotStage::Disconnected;
        BotStage m_failedStage = BotStage::Disconnected;
        BotDisconnectReason m_disconnectReason = BotDisconnectReason::None;
        // Set at the start of Disconnect() (clean or forced), consumed and reset by SetStage once
        // it actually reaches Disconnected -- distinguishes a requested disconnect from the
        // connection's own onDisconnect firing unprompted (bot-plan.md's BotDisconnectReason).
        bool m_disconnectRequested = false;
        BotIdentity m_identity;
        std::string m_lastError;
        BotBrain m_brain;
        std::unique_ptr<BotContext> m_context;

        LockedQueue<BotPacket, std::mutex> m_incoming;
        LockedQueue<std::function<void()>, std::mutex> m_commands;
        std::vector<BotPacket> m_sentPackets;

        std::shared_ptr<BotWorldConnection> m_connection;
        std::shared_ptr<BotWorldConnection> m_realmConnection;
        std::unique_ptr<BotPacketLog> m_packetLog;

        uint32 m_elapsedMs = 0;
        uint32 m_latencyMs = 0;

        uint32 m_pingTimerMs = 0;
        uint32 m_pingSerial = 0;
        uint32 m_pingSentAtMs = 0;
        bool m_pingInFlight = false;

        bool m_awaitingLogoutComplete = false;
        uint32 m_logoutDeadlineMs = 0;

        mutable std::mutex m_infoLock;
        RealBotInfo m_publishedInfo;
};

#endif
