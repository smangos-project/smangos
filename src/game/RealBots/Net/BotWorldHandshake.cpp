#include "BotWorldHandshake.h"
#include "BotConnector.h"
#include "ClassicOpcodes.h"
#include "ClassicPackets.h"
#include "WorldHandshakeCrypto.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "Errors.h"
#include "IO/Networking/IpAddress.h"
#include "Log.h"

std::shared_ptr<BotWorldHandshake> BotWorldHandshake::Create(IO::IoContext* ioContext)
{
    return std::shared_ptr<BotWorldHandshake>(new BotWorldHandshake(ioContext));
}

BotWorldHandshake::BotWorldHandshake(IO::IoContext* ioContext) : m_ioContext(ioContext)
{
}

void BotWorldHandshake::Handshake(Options options, HandshakeCallback callback)
{
    m_options = std::move(options);
    m_callback = std::move(callback);
    Connect();
}

void BotWorldHandshake::Connect()
{
    auto ip = IO::Networking::IpAddress::TryParseFromString(m_options.WorldAddress);
    if (!ip.has_value())
    {
        Fail("bad WorldAddress '" + m_options.WorldAddress + "'");
        return;
    }
    IO::Networking::IpEndpoint endpoint(*ip, m_options.WorldPort);

    std::shared_ptr<BotWorldHandshake> self = shared_from_this();
    BotConnector::ConnectAsync(endpoint, std::chrono::seconds(10), [self](nonstd::expected<IO::Networking::SocketDescriptor, IO::NetworkError> descriptor)
    {
        if (!descriptor.has_value())
        {
            self->Fail("world connect failed: " + descriptor.error().ToString());
            return;
        }

        IO::Networking::AsyncSocket socket(self->m_ioContext, std::move(descriptor.value()));
        self->m_connection = BotWorldConnection::Create(std::move(socket),
            [self](BotPacket packet) { self->OnPacket(std::move(packet)); },
            [self]() { self->OnDisconnect(); });
        self->m_connection->StartHandshake();
    });
}

void BotWorldHandshake::OnPacket(BotPacket packet)
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
        case Step::WaitingAuthResponse:
            if (packet.Opcode == CLASSIC_SMSG_AUTH_RESPONSE)
                HandleAuthResponse(packet);
            break;
        case Step::Done:
            break;
    }
    // Anything else (an opcode we're not waiting on yet) is ignored here, not fatal --
    // bot-phase-1.md's "unknown opcodes are delivered, never fatal" applies to expected-but-
    // early opcodes too; this class only reacts to the one it's currently blocked on.
}

void BotWorldHandshake::OnDisconnect()
{
    if (m_step != Step::Done)
        Fail("world connection closed during handshake");
}

void BotWorldHandshake::HandleAuthChallenge(BotPacket& packet)
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

    m_sessionKey = WorldHandshakeCrypto::ComputeSessionKey(m_options.SessionKey, m_localChallenge, m_serverChallenge);
    m_encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(m_sessionKey, m_localChallenge, m_serverChallenge);

    ClassicAuthSession session;
    session.RegionId = m_options.RegionId;
    session.BattlegroupId = m_options.BattlegroupId;
    session.RealmId = m_options.RealmId;
    session.LocalChallenge = m_localChallenge;
    session.Digest = WorldHandshakeCrypto::ComputeAuthCheckDigest(m_options.SessionKey, m_localChallenge, m_serverChallenge);
    session.RealmJoinTicket = m_options.RealmJoinTicket;

    m_step = Step::WaitingEnterEncryptedMode;
    m_connection->Send(session);
}

void BotWorldHandshake::HandleEnterEncryptedMode(BotPacket& packet)
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

    m_step = Step::WaitingAuthResponse;
    m_connection->Send(ClassicEnterEncryptedModeAck{});

    if (enterEncryptedMode.Enabled)
    {
        AES_GCM::Key key;
        std::copy(m_encryptKey.begin(), m_encryptKey.end(), key.begin());
        m_connection->ArmEncryption(key);
    }
}

void BotWorldHandshake::HandleAuthResponse(BotPacket& packet)
{
    ClassicAuthResponse response;
    if (!response.Read(packet.Data))
    {
        Fail("malformed SMSG_AUTH_RESPONSE");
        return;
    }

    Succeed(response.Result);
}

void BotWorldHandshake::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotWorldHandshake: %s", error.c_str());
    m_step = Step::Done;
    if (m_connection)
    {
        // Breaks the m_connection <-> callback(self) cycle Connect() created when it passed
        // [self](...) to BotWorldConnection::Create -- without this, self and the connection
        // keep each other alive forever. Safe from inside a dispatch: DispatchPacket/Close
        // already copy the callback out before invoking it.
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
        m_connection->Close();
    }
    if (m_callback)
    {
        HandshakeCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = false;
        result.Error = error;
        callback(std::move(result));
    }
}

void BotWorldHandshake::Succeed(uint32 authResult)
{
    m_step = Step::Done;
    if (m_connection)
    {
        m_connection->SetPacketCallback(nullptr);
        m_connection->SetDisconnectCallback(nullptr);
    }
    if (m_callback)
    {
        HandshakeCallback callback = std::move(m_callback);
        m_callback = nullptr;
        Result result;
        result.Success = (authResult == 0);
        result.Connection = m_connection;
        result.AuthResult = authResult;
        if (!result.Success)
            result.Error = "SMSG_AUTH_RESPONSE result " + std::to_string(authResult);
        callback(std::move(result));
    }
}
