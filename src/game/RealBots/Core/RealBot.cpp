#include "RealBot.h"
#include "BotContext.h"
#include "BotPacketLog.h"
#include "BotWorldConnection.h"
#include "ClassicOpcodes.h"
#include "Packets/Session.h"
#include "BotTaskRegistry.h"
#include "RealBotConfig.h"
#include <algorithm>

RealBot::RealBot()
    : m_context(std::make_unique<BotContext>(*this))
{
    PublishInfo();
}

RealBot::~RealBot()
{
    CloseConnections();
}

void RealBot::SetStage(BotStage stage)
{
    if (stage == m_stage)
        return;

    BotStage from = m_stage;
    m_stage = stage;
    m_held = false;
    m_stageEnteredMs = m_elapsedMs;

    m_brain.DispatchStageChanged(*m_context, from, stage);

    if (stage == BotStage::Failed)
    {
        // A failed bot is finished: its tasks and connections go, the way they do on Disconnected
        // (the brain's clear is deferred if this is called from one of its own tasks).
        m_failedStage = from;
        m_brain.Clear();
        CloseConnections();
    }

    if (stage == BotStage::Disconnected)
    {
        m_disconnectReason = m_disconnectRequested ? BotDisconnectReason::Requested : BotDisconnectReason::ConnectionLost;
        m_disconnectRequested = false;

        m_brain.Clear();
        CloseConnections();
    }
}

void RealBot::EnablePacketLog(std::string const& dir)
{
    m_packetLog = std::make_unique<BotPacketLog>(dir, m_name.empty() ? "bot" : m_name);
}

RealBot::WorldCallbacks RealBot::MakeWorldCallbacks(BotConnectionKind kind)
{
    std::weak_ptr<RealBot> weakSelf = weak_from_this();
    uint32 const generation = m_connectionGeneration[size_t(kind)];

    WorldCallbacks callbacks;
    callbacks.OnPacket = [weakSelf, kind, generation](BotPacket packet)
    {
        packet.Connection = kind;
        if (std::shared_ptr<RealBot> self = weakSelf.lock())
        {
            self->Post([self, kind, generation, packet = std::move(packet)]() mutable
            {
                if (generation == self->m_connectionGeneration[size_t(kind)])
                    self->QueueIncoming(std::move(packet));
            });
        }
    };
    callbacks.OnDisconnect = [weakSelf, kind, generation]()
    {
        if (std::shared_ptr<RealBot> self = weakSelf.lock())
        {
            self->Post([self, kind, generation]()
            {
                if (generation == self->m_connectionGeneration[size_t(kind)])
                    self->OnConnectionLost(kind);
            });
        }
    };
    return callbacks;
}

void RealBot::AttachConnection(BotConnectionKind kind, std::shared_ptr<BotWorldConnection> connection)
{
    (kind == BotConnectionKind::Realm ? m_realmConnection : m_instanceConnection) = std::move(connection);
}

std::shared_ptr<BotWorldConnection> RealBot::GetConnection(BotConnectionKind kind) const
{
    return kind == BotConnectionKind::Realm ? m_realmConnection : m_instanceConnection;
}

void RealBot::CloseConnection(BotConnectionKind kind)
{
    // The generation moves on even with nothing attached: a connection still being made for this
    // kind (see ConnectAwaiter) must not deliver into whatever comes next.
    ++m_connectionGeneration[size_t(kind)];

    // Detach the callbacks first: closing must not report a "connection lost" for a close we
    // asked for.
    std::shared_ptr<BotWorldConnection>& connection = kind == BotConnectionKind::Realm ? m_realmConnection : m_instanceConnection;
    if (connection)
    {
        connection->SetPacketCallback(nullptr);
        connection->SetDisconnectCallback(nullptr);
        connection->Close();
        connection.reset();
    }
}

void RealBot::CloseConnections()
{
    CloseConnection(BotConnectionKind::Instance);
    CloseConnection(BotConnectionKind::Realm);
}

bool RealBot::AllowConnect(BotConnectionKind kind, std::string const& address, uint16_t port)
{
    return m_brain.DispatchConnect(*m_context, kind, address, port) == BotPacketAction::Continue;
}

void RealBot::OnConnectionLost(BotConnectionKind kind)
{
    // Whoever is waiting on that connection hears about it first (hooks, and a routine's pending
    // wait, which resumes as failed).
    m_brain.DispatchConnectionLost(*m_context, kind);

    // The Instance connection being refused or dropped while entering the world is the login
    // routine's to handle: it can ask the server for another attempt (CMSG_CONNECT_TO_FAILED).
    if (kind == BotConnectionKind::Instance && m_stage == BotStage::EnteringWorld)
        return;

    // A clean logout in progress: the server answers on the Realm connection
    // (SMSG_LOGOUT_COMPLETE) and closes the Instance one right after, so its closing is expected and
    // must not end the wait -- handling it as a disconnect would tear the session down and discard the
    // very packet being waited for (still in flight from the Realm connection). The wait ends on that
    // packet, on the Realm connection closing, or on the deadline.
    if (kind == BotConnectionKind::Instance && m_awaitingLogoutComplete)
        return;

    // Losing a world connection while the login is still running is a failed login, not a
    // disconnect: the bot never got anywhere to be disconnected from.
    if (m_stage >= BotStage::WorldConnecting && m_stage <= BotStage::EnteringWorld)
    {
        m_lastError = std::string(kind == BotConnectionKind::Realm ? "realm" : "instance")
            + " connection closed during " + BotStageName(m_stage);
        SetStage(BotStage::Failed);
        return;
    }

    SetStage(BotStage::Disconnected);
}

void RealBot::Post(std::function<void()> fn)
{
    m_commands.add(std::move(fn));

    m_wakePending = true;
    if (m_wakeCallback)
        m_wakeCallback();
}

void RealBot::QueueIncoming(BotPacket packet)
{
    m_incoming.add(std::move(packet));
}

namespace
{
// Opcodes the client only ever sends on the Realm connection: the handshake and the character
// screen, everything before SMSG_CONNECT_TO. Anything else follows the world session to the
// Instance connection once there is one (CMSG_AUTH_CONTINUED_SESSION is the Instance's own
// handshake and is always sent with an explicit connection).
bool IsRealmOnlyOpcode(uint32 opcode)
{
    switch (opcode)
    {
        case CLASSIC_CMSG_AUTH_SESSION:
        case CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK:
        case CLASSIC_CMSG_ENUM_CHARACTERS:
        case CLASSIC_CMSG_CREATE_CHARACTER:
        case CLASSIC_CMSG_PLAYER_LOGIN:
        case CLASSIC_CMSG_CONNECT_TO_FAILED:
            return true;
        default:
            return false;
    }
}
}

void RealBot::SendRaw(BotPacket packet, std::optional<BotConnectionKind> connection, std::array<uint8, 32> const* armEncryption)
{
    if (connection)
        packet.Connection = *connection;
    else if (!IsRealmOnlyOpcode(packet.Opcode) && m_instanceConnection)
        packet.Connection = BotConnectionKind::Instance;
    else
        packet.Connection = BotConnectionKind::Realm;

    BotPacketAction action = m_brain.DispatchPacket(*m_context, packet, BotPacketDir::Outgoing);
    if (action == BotPacketAction::Drop)
        return;

    LogPacket(packet, BotPacketDir::Outgoing);

    if (std::shared_ptr<BotWorldConnection> target = GetConnection(packet.Connection))
    {
        if (armEncryption)
        {
            AES_GCM::Key key;
            std::copy(armEncryption->begin(), armEncryption->end(), key.begin());
            target->SendRaw(packet.Opcode, packet.Data, &key);
        }
        else
        {
            target->SendRaw(packet.Opcode, packet.Data);
        }
    }

    m_sentPackets.push_back(std::move(packet));
}

void RealBot::Disconnect(bool clean)
{
    m_disconnectRequested = true;

    if (clean && m_stage == BotStage::InWorld && HasConnection(BotConnectionKind::Instance))
    {
        SetStage(BotStage::LoggingOut);
        Send(ClassicLogoutRequest{});
        m_awaitingLogoutComplete = true;
        m_logoutDeadlineMs = m_elapsedMs + (m_loginOptions.LogoutTimeoutMs ? m_loginOptions.LogoutTimeoutMs : RealBotConfig::LogoutTimeoutMs());
    }
    else
    {
        SetStage(BotStage::Disconnected);
    }
}

void RealBot::Reconnect()
{
    Post([this]() { ReconnectNow(); });
}

void RealBot::ReconnectNow()
{
    // Down first, the way an abrupt Disconnect does it. SetStage(Disconnected) is a no-op when the
    // bot already is, so what it does on a change (tasks and hooks, connections) is done directly too.
    if (m_stage != BotStage::Disconnected)
    {
        m_disconnectRequested = true;
        SetStage(BotStage::Disconnected);
    }
    m_brain.Clear();
    CloseConnections();

    // A new session: nothing of the last one's outcome carries over.
    m_lastError.clear();
    m_failedStage = BotStage::Disconnected;
    m_disconnectReason = BotDisconnectReason::None;
    m_disconnectRequested = false;
    m_awaitingLogoutComplete = false;
    m_latencyMs = 0;
    m_pingTimerMs = 0;
    m_pingInFlight = false;
    m_stageEnteredMs = m_elapsedMs;
    m_held = false;

    // A bot without the built-in tasks registered (the coroutine tests' TestFakeBot) has no login to run.
    if (std::unique_ptr<BotTask> login = sBotTaskRegistry.Create("login", *m_context, BotTaskArgs()))
        m_brain.PushTask(std::move(login), *m_context);
}

void RealBot::ProcessCommands()
{
    std::function<void()> fn;
    while (m_commands.next(fn))
        fn();
}

void RealBot::ProcessIncoming(uint32 diff)
{
    // Between packets the brain gets a zero-time update, so a routine whose wait was just satisfied
    // (or whose ctx.Run child just finished) has resumed and registered its next await before the
    // next packet is dispatched. Without it a server sending two packets back to back -- an
    // SMSG_RESUME_COMMS then SMSG_LOGIN_VERIFY_WORLD -- would have the second dispatched to nobody,
    // the routine only reaching its next await at the end of the tick. Posted completions (a
    // connect finishing) get the same treatment from the update before the first packet.
    m_brain.Update(*m_context, 0);

    BotPacket packet;
    while (m_incoming.next(packet))
    {
        LogPacket(packet, BotPacketDir::Incoming);

        if (m_packetObserver)
            m_packetObserver(packet);

        BotPacketAction action = m_brain.DispatchPacket(*m_context, packet, BotPacketDir::Incoming);

        // Logout completion is framework bookkeeping, not something a task hook can swallow: a
        // task may still observe SMSG_LOGOUT_COMPLETE (dispatch above already ran), but Disconnect
        // always resolves once it arrives.
        if (packet.Opcode == CLASSIC_SMSG_LOGOUT_COMPLETE && m_awaitingLogoutComplete)
        {
            m_awaitingLogoutComplete = false;
            SetStage(BotStage::Disconnected);
        }
        else if (action == BotPacketAction::Continue)
        {
            ApplyDefaultHandling(packet, BotPacketDir::Incoming);
        }

        m_brain.Update(*m_context, 0);
    }
}

void RealBot::ApplyDefaultHandling(BotPacket const& packet, BotPacketDir dir)
{
    if (dir != BotPacketDir::Incoming)
        return;

    ByteBuffer body = packet.Data;
    body.rpos(0);

    switch (packet.Opcode)
    {
        case CLASSIC_SMSG_PONG:
        {
            Pong pong;
            if (pong.Read(body) && m_pingInFlight && pong.Serial == m_pingSerial)
            {
                // Measured on the wall clock, in whole milliseconds rounded up, so a localhost round
                // trip under 1ms reads as 1: 0 keeps meaning "never measured".
                auto const roundTrip = std::chrono::steady_clock::now() - m_pingSentAt;
                m_latencyMs = uint32(std::max<int64>(1, (std::chrono::duration_cast<std::chrono::microseconds>(roundTrip).count() + 999) / 1000));
                m_pingInFlight = false;
            }
            break;
        }
        case CLASSIC_SMSG_TIME_SYNC_REQUEST:
        {
            TimeSyncRequest request;
            if (request.Read(body))
            {
                TimeSyncResponse response;
                response.SequenceIndex = request.SequenceIndex;
                response.ClientTime = m_elapsedMs;
                Send(response);
            }
            break;
        }
        default:
            break;
    }
}

void RealBot::UpdateLogout(uint32 diff)
{
    if (!m_awaitingLogoutComplete)
        return;

    if (m_elapsedMs >= m_logoutDeadlineMs)
    {
        m_awaitingLogoutComplete = false;
        SetStage(BotStage::Disconnected);
    }
}

void RealBot::UpdateStageWatchdog()
{
    uint32 const timeoutMs = m_loginOptions.StageTimeoutMs;
    if (timeoutMs == 0)
        return;

    switch (m_stage)
    {
        case BotStage::Disconnected:
        case BotStage::InWorld:
        case BotStage::LoggingOut: // has its own timeout (UpdateLogout)
        case BotStage::Failed:
            return;
        default:
            break;
    }

    // A stage the login was told to stop at is held on purpose, for as long as its owner likes.
    if (m_loginOptions.StopAt != BotStage::InWorld && m_stage == m_loginOptions.StopAt)
        return;

    if (m_elapsedMs - m_stageEnteredMs < timeoutMs)
        return;

    m_lastError = std::string("stage ") + BotStageName(m_stage) + " timed out after " + std::to_string(timeoutMs) + "ms";
    SetStage(BotStage::Failed);
}

void RealBot::UpdateKeepalive(uint32 diff)
{
    // Only once the world session is up: mid-handshake the connections are not yet encrypted, and
    // a ping there is a protocol violation, not a keepalive.
    if (m_stage != BotStage::InWorld)
        return;

    uint32 const interval = RealBotConfig::PingIntervalMs();
    if (interval == 0)
        return;

    m_pingTimerMs += diff;
    if (m_pingTimerMs < interval)
        return;
    m_pingTimerMs = 0;

    Ping ping;
    ping.Serial = ++m_pingSerial;
    ping.Latency = m_latencyMs;
    m_pingSentAt = std::chrono::steady_clock::now();
    m_pingInFlight = true;
    Send(ping);

    Send(KeepAlive{});
}

void RealBot::PublishInfo()
{
    std::lock_guard<std::mutex> lock(m_infoLock);
    m_publishedInfo.Name = m_name;
    m_publishedInfo.Stage = m_stage;
    m_publishedInfo.LatencyMs = m_latencyMs;
    m_publishedInfo.CharacterName = m_identity.CharacterName;
    m_publishedInfo.LastError = m_lastError;
    m_publishedInfo.TaskStack = m_brain.TaskNames();
    m_publishedInfo.FailedStage = m_failedStage;
    m_publishedInfo.DisconnectReason = m_disconnectReason;
    m_publishedInfo.Session = m_identity.Session;
    m_publishedInfo.CharacterGuid = m_identity.CharacterGuid;
    m_publishedInfo.Held = m_held;
}

RealBotInfo RealBot::GetPublishedInfo() const
{
    std::lock_guard<std::mutex> lock(m_infoLock);
    return m_publishedInfo;
}

void RealBot::LogPacket(BotPacket const& packet, BotPacketDir dir)
{
    if (m_packetLog)
        m_packetLog->Write(m_elapsedMs, dir, packet.Connection, packet.Opcode, packet.Data, packet.Compressed);
}

void RealBot::TickNow()
{
    auto const now = std::chrono::steady_clock::now();
    uint32 const diff = uint32(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTickTime).count());
    m_lastTickTime = now;
    Tick(diff);
}

void RealBot::Tick(uint32 diff)
{
    ProcessCommands();
    m_elapsedMs += diff;
    ProcessIncoming(diff);
    UpdateLogout(diff);
    UpdateStageWatchdog();
    UpdateKeepalive(diff);
    m_brain.Update(*m_context, diff);
    PublishInfo();
}
