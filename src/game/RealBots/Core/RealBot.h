#ifndef REALBOTS_REALBOT_H
#define REALBOTS_REALBOT_H

#include "BotBrain.h"
#include "BotIdentity.h"
#include "BotPacket.h"
#include "BotStage.h"
#include "BotWorldView.h"
#include "LockedQueue.h"
#include "Packets/Classic/Guid.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
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
    uint8 Race = 1;  // human
    uint8 Class = 1; // warrior
    uint8 Gender = 0;
    float FarClip = 533.0f;
    // Holds the login routine at this stage instead of continuing past it -- InWorld (the
    // default) means "don't stop early" (bot-phase-1.md, "Login": "stop_at=<stage>").
    BotStage StopAt = BotStage::InWorld;
    // Replaces the portal's login ticket at the RPC step (LoginRpc), to exercise a forged or reused
    // one; empty (the default) uses the ticket the portal issued.
    std::string LoginTicketOverride;
    // How long the world connections wait to connect, and for each packet a login step expects.
    uint32 ConnectTimeoutMs = 10000;
    uint32 PacketTimeoutMs = 10000;
    // A login stage held longer than this fails the bot (RealBot::UpdateStageWatchdog); 0 turns it
    // off. Not applied to InWorld, to a stage held on purpose by StopAt, or to Disconnected.
    uint32 StageTimeoutMs = 0;
    // How long a clean Disconnect waits for SMSG_LOGOUT_COMPLETE before giving up and disconnecting
    // anyway; 0 uses RealBots.LogoutTimeoutMs. A server's own logout countdown (about 20 s in the
    // open world) is longer than the default, which suits a bot that only wants to leave.
    uint32 LogoutTimeoutMs = 0;
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
    // BotIdentity::Session, once the Battle.net half of the login has produced it.
    BotSessionInfo Session;
    ClassicGuid CharacterGuid;
    // The login routine has stopped at RealBotLoginOptions::StopAt (the stage is that one, and
    // what it has produced is in place): set after the stage is entered, cleared on any stage change.
    bool Held = false;
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

        // Tick(elapsed wall time since this bot's last TickNow): how a host with real time to spare
        // (RealBotMgr's workers) ticks a bot, on its regular interval or early, as soon as it has
        // something to do (see SetWakeCallback).
        void TickNow();

        // Called from whichever thread Posts work to this bot (an IO completion, a packet, another
        // thread's command) right after the work is queued, so the host can tick the bot promptly
        // instead of at its next regular tick. Set once, before the bot is ticked; must be cheap and
        // never touch the bot. ConsumeWake() is the host's side: true if there has been work since
        // it last asked.
        void SetWakeCallback(std::function<void()> callback) { m_wakeCallback = std::move(callback); }
        bool ConsumeWake() { return m_wakePending.exchange(false); }

        BotStage GetStage() const { return m_stage; }
        void SetStage(BotStage stage);

        void SetLastError(std::string error) { m_lastError = std::move(error); }
        std::string const& GetLastError() const { return m_lastError; }

        // Marks the bot as held at its current stage (see RealBotInfo::Held); the built-in login task
        // calls it when it stops at StopAt. Cleared by the next stage change.
        void SetHeld() { m_held = true; }

        BotStage GetFailedStage() const { return m_failedStage; }
        BotDisconnectReason GetDisconnectReason() const { return m_disconnectReason; }

        BotIdentity const& GetIdentity() const { return m_identity; }
        BotIdentity& GetIdentity() { return m_identity; }

        uint32 GetTimeMs() const { return m_elapsedMs; }
        uint32 GetLatencyMs() const { return m_latencyMs; }

        BotBrain& GetBrain() { return m_brain; }
        BotContext& GetContext() { return *m_context; }

        // What a world connection delivers into: every packet is tagged with the connection it
        // arrived on and queued into this bot's inbox (never handed to whoever is driving the
        // handshake), and a close is posted as a connection loss (bot-phase-2.md, "Handshake
        // through the normal dispatch"). Safe to call from any thread, but only ever attach the
        // result to the connection it was made for, and only within the connection generation it
        // was made in: callbacks from a session that has since been closed are ignored.
        struct WorldCallbacks
        {
            std::function<void(BotPacket)> OnPacket;
            std::function<void()> OnDisconnect;
        };
        WorldCallbacks MakeWorldCallbacks(BotConnectionKind kind);

        // Takes ownership of a connection made with MakeWorldCallbacks(kind) (bot thread only).
        // The Instance connection, once attached, is where the world session lives; the Realm
        // connection stays open beside it for what still routes through it (SMSG_LOGOUT_COMPLETE, and
        // the answer to anything sent on it: a ping's pong comes back on the socket it went out on).
        void AttachConnection(BotConnectionKind kind, std::shared_ptr<BotWorldConnection> connection);
        std::shared_ptr<BotWorldConnection> GetConnection(BotConnectionKind kind) const;
        bool HasConnection(BotConnectionKind kind) const { return GetConnection(kind) != nullptr; }
        // Closes one connection (a failed Instance attempt, before the next) without touching the
        // other, and without reporting it as a connection loss.
        void CloseConnection(BotConnectionKind kind);
        void CloseConnections();

        // Asks the hooks and tasks whether a world connection may be opened (BotTask::OnConnect).
        bool AllowConnect(BotConnectionKind kind, std::string const& address, uint16_t port);

        // Posts a callback to run on this bot's own thread, on its next Tick -- how anything
        // running on another thread (an IO completion, a BotLogin result) safely reaches into a
        // bot without touching its state directly (bot-plan.md, "IO threads only move bytes into
        // a bot's inbox"). Safe to call from any thread, including this bot's own.
        void Post(std::function<void()> fn);

        // Queues an incoming packet for dispatch on the next Tick. TestFakeBot uses this directly
        // to inject packets; a real connection's packet callback reaches it through Post.
        void QueueIncoming(BotPacket packet);

        // Called for every Incoming packet, independent of the brain/task stack -- unlike a
        // BotTask, it keeps running across BotBrain::Clear() (bot-plan.md's Disconnected ->
        // brain wipe), which a task pushed on the stack wouldn't survive. Bot-thread-only to set
        // (like everything else here); tests/conformance's TestBot is the one caller, watching
        // for packets a TestRoutine on another thread is awaiting.
        void SetPacketObserver(std::function<void(BotPacket const&)> observer) { m_packetObserver = std::move(observer); }

        // Runs a packet through the hooks and task stack (Outgoing) and, unless one returned Drop,
        // writes it to a connection and records it. The connection is `connection` if given, else
        // the opcode's default route: the handshake and character-screen opcodes go to Realm,
        // everything else to Instance once one is attached, Realm before that. With no such
        // connection attached the packet is still shown to hooks and recorded, just not written.
        template<typename T>
        void Send(T const& packet, std::optional<BotConnectionKind> connection = std::nullopt)
        {
            BotPacket pkt;
            pkt.Opcode = T::Opcode;
            packet.Write(pkt.Data);
            SendRaw(std::move(pkt), connection);
        }

        // armEncryption: arm the chosen connection's PacketCrypt with this key as this packet is
        // sent, but only if it really is sent (a hook that drops CMSG_ENTER_ENCRYPTED_MODE_ACK
        // leaves the connection unarmed, which is what a "never ack" scenario needs).
        void SendRaw(BotPacket packet, std::optional<BotConnectionKind> connection = std::nullopt,
            std::array<uint8, 32> const* armEncryption = nullptr);
        std::vector<BotPacket> const& GetSentPackets() const { return m_sentPackets; }

        // clean: sends CLASSIC_CMSG_LOGOUT_REQUEST and waits up to RealBots.LogoutTimeoutMs for
        // CLASSIC_SMSG_LOGOUT_COMPLETE before moving to Disconnected; otherwise (or if there's no
        // connection to ask) moves to Disconnected immediately (bot-phase-1.md, "Login"). Not clean
        // is the abrupt one, "the client vanished": the sockets are closed with no logout request,
        // and BotDisconnectReason::Requested is still what gets recorded.
        void Disconnect(bool clean);

        // Logs the bot in again with the identity and login options it already has (bot-phase-2.md,
        // "Reconnect"): whatever it was doing is dropped abruptly (connections closed, tasks and
        // non-persistent hooks gone, no logout request), and a fresh built-in "login" task starts.
        // Works from any stage, Disconnected and Failed included. Posted like everything else, so it
        // is safe to call from any thread; there is no automatic policy for when (that is behavior).
        void Reconnect();

        RealBotInfo GetPublishedInfo() const;

        // What this bot has learned from SMSG_UPDATE_OBJECT so far (BotWorldView), fed from
        // ApplyDefaultHandling on the bot thread; safe to call from any thread.
        std::optional<BotKnownObject> GetKnownObject(ClassicGuid const& guid) const;
        std::vector<BotKnownObject> GetKnownObjects() const;
        // Where this bot itself now stands, after it sent a movement message.
        void NoteOwnMovement(ClassicMovementInfo const& status);

    private:
        void ProcessCommands();
        void ProcessIncoming(uint32 diff);
        void UpdateLogout(uint32 diff);
        void UpdateStageWatchdog();
        void UpdateKeepalive(uint32 diff);
        void ApplyDefaultHandling(BotPacket const& packet, BotPacketDir dir);
        void PublishInfo();
        void LogPacket(BotPacket const& packet, BotPacketDir dir);
        void OnConnectionLost(BotConnectionKind kind);
        void ReconnectNow();

        std::string m_name;
        IO::IoContext* m_ioContext = nullptr;
        RealBotLoginOptions m_loginOptions;

        BotStage m_stage = BotStage::Disconnected;
        bool m_held = false;
        uint32 m_stageEnteredMs = 0; // m_elapsedMs when m_stage was entered
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

        std::shared_ptr<BotWorldConnection> m_realmConnection;
        std::shared_ptr<BotWorldConnection> m_instanceConnection;
        // Bumped whenever a connection is closed (per kind), so a callback made for an earlier one
        // (see MakeWorldCallbacks) can tell it is stale. Bot-thread only.
        std::array<uint32, 2> m_connectionGeneration{};
        std::unique_ptr<BotPacketLog> m_packetLog;
        std::function<void(BotPacket const&)> m_packetObserver;

        uint32 m_elapsedMs = 0;
        uint32 m_latencyMs = 0;

        std::function<void()> m_wakeCallback;
        std::atomic<bool> m_wakePending{ false };
        std::chrono::steady_clock::time_point m_lastTickTime = std::chrono::steady_clock::now();

        uint32 m_pingTimerMs = 0;
        uint32 m_pingSerial = 0;
        std::chrono::steady_clock::time_point m_pingSentAt;
        bool m_pingInFlight = false;

        bool m_awaitingLogoutComplete = false;
        uint32 m_logoutDeadlineMs = 0;

        mutable std::mutex m_infoLock;
        RealBotInfo m_publishedInfo;

        mutable std::mutex m_worldViewLock;
        BotWorldView m_worldView;
};

#endif
