#include "RealBot.h"
#include "BotContext.h"
#include "BotPacketLog.h"
#include "BotWorldConnection.h"
#include "ClassicOpcodes.h"
#include "ClassicPackets.h"
#include "RealBotConfig.h"

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

    if (BotTask* top = m_brain.Top())
        top->OnStageChanged(*m_context, from, stage);

    if (stage == BotStage::Disconnected)
    {
        m_brain.Clear();
        CloseConnections();
    }
}

void RealBot::EnablePacketLog(std::string const& dir)
{
    m_packetLog = std::make_unique<BotPacketLog>(dir, m_name.empty() ? "bot" : m_name);
}

void RealBot::AttachConnections(std::shared_ptr<BotWorldConnection> connection, std::shared_ptr<BotWorldConnection> realmConnection)
{
    m_connection = std::move(connection);
    m_realmConnection = std::move(realmConnection);

    std::weak_ptr<RealBot> weakSelf = weak_from_this();
    auto onPacket = [weakSelf](BotPacket packet)
    {
        if (std::shared_ptr<RealBot> self = weakSelf.lock())
            self->Post([self, packet = std::move(packet)]() mutable { self->QueueIncoming(std::move(packet)); });
    };
    auto onDisconnect = [weakSelf]()
    {
        if (std::shared_ptr<RealBot> self = weakSelf.lock())
            self->Post([self]() { self->SetStage(BotStage::Disconnected); });
    };

    if (m_connection)
    {
        m_connection->SetPacketCallback(onPacket);
        m_connection->SetDisconnectCallback(onDisconnect);
    }
    if (m_realmConnection && m_realmConnection != m_connection)
    {
        m_realmConnection->SetPacketCallback(onPacket);
        m_realmConnection->SetDisconnectCallback(onDisconnect);
    }
}

void RealBot::CloseConnections()
{
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
        m_connection->Close();
        m_connection.reset();
    }
    if (m_realmConnection)
    {
        m_realmConnection->SetPacketCallback(nullptr);
        m_realmConnection->SetDisconnectCallback(nullptr);
        m_realmConnection->Close();
        m_realmConnection.reset();
    }
}

void RealBot::Post(std::function<void()> fn)
{
    m_commands.add(std::move(fn));
}

void RealBot::QueueIncoming(BotPacket packet)
{
    m_incoming.add(std::move(packet));
}

void RealBot::SendRaw(BotPacket packet)
{
    BotPacketAction action = m_brain.DispatchPacket(*m_context, packet, BotPacketDir::Outgoing);
    if (action == BotPacketAction::Drop)
        return;

    LogPacket(packet, BotPacketDir::Outgoing);

    if (m_connection)
        m_connection->SendRaw(packet.Opcode, packet.Data);

    m_sentPackets.push_back(std::move(packet));
}

void RealBot::Disconnect(bool clean)
{
    if (clean && m_connection)
    {
        SetStage(BotStage::LoggingOut);
        Send(ClassicLogoutRequest{});
        m_awaitingLogoutComplete = true;
        m_logoutDeadlineMs = m_elapsedMs + RealBotConfig::LogoutTimeoutMs();
    }
    else
    {
        SetStage(BotStage::Disconnected);
    }
}

void RealBot::ProcessCommands()
{
    std::function<void()> fn;
    while (m_commands.next(fn))
        fn();
}

void RealBot::ProcessIncoming(uint32 diff)
{
    BotPacket packet;
    while (m_incoming.next(packet))
    {
        LogPacket(packet, BotPacketDir::Incoming);

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
            ClassicPong pong;
            if (pong.Read(body) && m_pingInFlight && pong.Serial == m_pingSerial)
            {
                m_latencyMs = m_elapsedMs - m_pingSentAtMs;
                m_pingInFlight = false;
            }
            break;
        }
        case CLASSIC_SMSG_TIME_SYNC_REQUEST:
        {
            ClassicTimeSyncRequest request;
            if (request.Read(body))
            {
                ClassicTimeSyncResponse response;
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

void RealBot::UpdateKeepalive(uint32 diff)
{
    if (!m_connection)
        return;

    uint32 const interval = RealBotConfig::PingIntervalMs();
    if (interval == 0)
        return;

    m_pingTimerMs += diff;
    if (m_pingTimerMs < interval)
        return;
    m_pingTimerMs = 0;

    ClassicPing ping;
    ping.Serial = ++m_pingSerial;
    ping.Latency = m_latencyMs;
    m_pingSentAtMs = m_elapsedMs;
    m_pingInFlight = true;
    Send(ping);

    Send(ClassicKeepAlive{});
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
}

RealBotInfo RealBot::GetPublishedInfo() const
{
    std::lock_guard<std::mutex> lock(m_infoLock);
    return m_publishedInfo;
}

void RealBot::LogPacket(BotPacket const& packet, BotPacketDir dir)
{
    if (m_packetLog)
        m_packetLog->Write(m_elapsedMs, dir, packet.Opcode, packet.Data);
}

void RealBot::Tick(uint32 diff)
{
    ProcessCommands();
    m_elapsedMs += diff;
    ProcessIncoming(diff);
    UpdateLogout(diff);
    UpdateKeepalive(diff);
    m_brain.Update(*m_context, diff);
    PublishInfo();
}
