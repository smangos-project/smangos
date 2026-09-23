#include "BotInstanceConnect.h"
#include "BotConnector.h"
#include "ClassicOpcodes.h"
#include "ClassicPackets.h"
#include "WorldHandshakeCrypto.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "Errors.h"
#include "IO/Networking/IpAddress.h"
#include "Log.h"

std::shared_ptr<BotInstanceConnect> BotInstanceConnect::Create(IO::IoContext* ioContext)
{
    return std::shared_ptr<BotInstanceConnect>(new BotInstanceConnect(ioContext));
}

BotInstanceConnect::BotInstanceConnect(IO::IoContext* ioContext) : m_ioContext(ioContext)
{
}

void BotInstanceConnect::Connect(Options options, ConnectCallback callback)
{
    m_options = std::move(options);
    m_callback = std::move(callback);
    OpenSocket();
}

void BotInstanceConnect::OpenSocket()
{
    auto ip = IO::Networking::IpAddress::TryParseFromString(m_options.WorldAddress);
    if (!ip.has_value())
    {
        Fail("bad WorldAddress '" + m_options.WorldAddress + "'");
        return;
    }
    IO::Networking::IpEndpoint endpoint(*ip, m_options.WorldPort);

    std::shared_ptr<BotInstanceConnect> self = shared_from_this();
    BotConnector::ConnectAsync(endpoint, std::chrono::seconds(10), [self](nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError> descriptor)
    {
        if (!descriptor.has_value())
        {
            self->Fail("instance connect failed: " + descriptor.error().ToString());
            return;
        }

        IO::Networking::AsyncSocket socket(self->m_ioContext, std::move(descriptor.value()));
        self->m_connection = BotWorldConnection::Create(std::move(socket),
            [self](BotPacket packet) { self->OnPacket(std::move(packet)); },
            [self]() { self->OnDisconnect(); });
        self->m_connection->StartHandshake();
    });
}

void BotInstanceConnect::OnPacket(BotPacket packet)
{
    switch (m_step)
    {
        case Step::WaitingAuthChallenge:
            if (packet.Opcode == CLASSIC_SMSG_AUTH_CHALLENGE)
                HandleAuthChallenge(packet);
            break;
        case Step::WaitingEnterEncryptedMode:
            if (packet.Opcode == CLASSIC_SMSG_ENTER_ENCRYPTED_MODE)
                HandleEnterEncryptedMode(packet);
            break;
        case Step::WaitingResumeComms:
            if (packet.Opcode == CLASSIC_SMSG_RESUME_COMMS)
                HandleResumeComms(packet);
            break;
        case Step::Done:
            break;
    }
}

void BotInstanceConnect::OnDisconnect()
{
    if (m_step != Step::Done)
        Fail("instance connection closed during handshake");
}

void BotInstanceConnect::HandleAuthChallenge(BotPacket& packet)
{
    ClassicAuthChallenge challenge;
    if (!challenge.Read(packet.Data))
    {
        Fail("malformed SMSG_AUTH_CHALLENGE");
        return;
    }
    m_serverChallenge = challenge.ServerChallenge;

    BigNumber localChallengeBn;
    localChallengeBn.SetRand(int(m_localChallenge.size() * 8));
    std::vector<uint8> localChallengeBytes = localChallengeBn.AsByteArray(int(m_localChallenge.size()), false);
    std::copy(localChallengeBytes.begin(), localChallengeBytes.end(), m_localChallenge.begin());

    m_encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(m_options.SessionKey, m_localChallenge, m_serverChallenge);

    ClassicAuthContinuedSession session;
    session.DosResponse = 0;
    session.Key = m_options.PackedKey;
    session.LocalChallenge = m_localChallenge;
    session.Digest = WorldHandshakeCrypto::ComputeContinuedSessionDigest(m_options.SessionKey, m_options.PackedKey, m_localChallenge, m_serverChallenge);

    m_step = Step::WaitingEnterEncryptedMode;
    m_connection->Send(session);
}

void BotInstanceConnect::HandleEnterEncryptedMode(BotPacket& packet)
{
    ClassicEnterEncryptedMode enterEncryptedMode;
    if (!enterEncryptedMode.Read(packet.Data))
    {
        Fail("malformed SMSG_ENTER_ENCRYPTED_MODE");
        return;
    }

    if (m_options.VerifyServerSignature)
    {
        std::array<uint8, 64> toSign = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(m_encryptKey, enterEncryptedMode.Enabled);

        Ed25519 ed25519;
        if (!ed25519.LoadFromByteArray(WorldHandshakeCrypto::EnterEncryptedModePrivateKey()))
        {
            Fail("failed to load Ed25519 key");
            return;
        }

        std::vector<uint8> context(WorldHandshakeCrypto::EnableEncryptionContext().begin(), WorldHandshakeCrypto::EnableEncryptionContext().end());
        if (!ed25519.VerifyWithContext(toSign, enterEncryptedMode.Signature, context))
        {
            Fail("SMSG_ENTER_ENCRYPTED_MODE signature verification failed");
            return;
        }
    }

    m_step = Step::WaitingResumeComms;
    m_connection->Send(ClassicEnterEncryptedModeAck{});

    if (enterEncryptedMode.Enabled)
    {
        AES_GCM::Key key;
        std::copy(m_encryptKey.begin(), m_encryptKey.end(), key.begin());
        m_connection->ArmEncryption(key);
    }
}

void BotInstanceConnect::HandleResumeComms(BotPacket& packet)
{
    ClassicResumeComms resumeComms;
    resumeComms.Read(packet.Data); // empty body; always true
    Succeed();
}

void BotInstanceConnect::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotInstanceConnect: %s", error.c_str());
    m_step = Step::Done;
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
        m_connection->Close();
    }
    if (m_callback)
    {
        ConnectCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = false;
        result.Error = error;
        callback(std::move(result));
    }
}

void BotInstanceConnect::Succeed()
{
    m_step = Step::Done;
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
    }
    if (m_callback)
    {
        ConnectCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = true;
        result.Connection = m_connection;
        callback(std::move(result));
    }
}
