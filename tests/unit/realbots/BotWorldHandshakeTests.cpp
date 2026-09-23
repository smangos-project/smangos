// End-to-end coverage for BotWorldHandshake: a fake WorldSocket-shaped server (plain V2 banner,
// then AuthChallenge/AuthSession/EnterEncryptedMode/EnterEncryptedModeAck/AuthResponse) played by
// hand over a raw AsyncSocket, the same style BotWorldConnectionTests.cpp uses. The fake server
// computes its side of the handshake with WorldHandshakeCrypto directly -- the same functions the
// bot uses -- since the digest/session-key/encrypt-key math is symmetric by construction (see
// WorldHandshakeCrypto.h's own comment on that).
#include "BotWorldHandshake.h"
#include "WorldHandshakeCrypto.h"
#include "ClassicPackets.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "Errors.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>

namespace
{
class WaitFlag
{
    public:
        void Set()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_set = true;
            m_cv.notify_all();
        }

        bool WaitFor(std::chrono::milliseconds timeout)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cv.wait_for(lock, timeout, [this]() { return m_set; });
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_set = false;
};

constexpr uint16_t TestPort = 28533;

#pragma pack(push, 1)
struct WireHeader
{
    uint32 size;
    uint8 tag[12];
};
#pragma pack(pop)

// PacketCrypt's counters advance on every send/recv in that direction from packet #0, armed or
// not (see PacketCrypt.h's own comment) -- so the fake server must run every packet it sends and
// receives through the *same* PacketCrypt instance, in order, exactly like BotWorldConnection
// does, or its counter falls out of lock-step with the bot's the moment encryption is armed.
// Encrypting here always (even before Init()) is what keeps that lock-step: unarmed, EncryptSend
// just zero-tags and still increments the counter.
std::vector<uint8> Frame(uint32 opcode, ByteBuffer const& body, PacketCrypt& serverCrypt)
{
    std::vector<uint8> plaintext(sizeof(uint32) + body.size());
    std::memcpy(plaintext.data(), &opcode, sizeof(opcode));
    if (!body.empty())
        std::memcpy(plaintext.data() + sizeof(uint32), body.contents(), body.size());

    AES_GCM::Tag tag;
    REQUIRE(serverCrypt.EncryptSend(plaintext.data(), plaintext.size(), tag));

    std::vector<uint8> framed(sizeof(WireHeader) + plaintext.size());
    WireHeader header{};
    header.size = uint32(plaintext.size());
    std::memcpy(header.tag, tag, sizeof(header.tag));
    std::memcpy(framed.data(), &header, sizeof(header));
    std::memcpy(framed.data() + sizeof(header), plaintext.data(), plaintext.size());
    return framed;
}

// Counterpart to Frame() for the two packets the fake server reads: keeps serverCrypt's recv
// counter in the same lock-step, unarmed reads included.
void Unframe(std::vector<uint8>& body, std::array<uint8, 12> const& tag, PacketCrypt& serverCrypt)
{
    AES_GCM::Tag rawTag;
    std::memcpy(rawTag, tag.data(), sizeof(rawTag));
    REQUIRE(serverCrypt.DecryptRecv(body.data(), body.size(), rawTag));
}

std::vector<uint8> TestSessionKey()
{
    std::vector<uint8> key(64);
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = uint8(i * 3 + 1);
    return key;
}

constexpr char const* TEST_REALM_JOIN_TICKET = "12345";
}

TEST_CASE("BotWorldHandshake: reaches a successful SMSG_AUTH_RESPONSE", "[botworldhandshake]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    std::vector<uint8> const joinSessionKey = TestSessionKey();
    WaitFlag serverDone;
    auto serverCrypt = std::make_shared<PacketCrypt>(PacketCryptRole::Server);

    acceptor->AutoAcceptSocketsUntilClose([&, joinSessionKey, serverCrypt](IO::Networking::SocketDescriptor descriptor)
    {
        auto serverSocket = std::make_shared<IO::Networking::AsyncSocket>(ctx.get(), std::move(descriptor));
        MANGOS_ASSERT(!(serverSocket->InitializeAndFixateMemoryLocation()));

        std::string banner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
        auto bannerBuf = std::make_shared<std::vector<uint8>>(banner.begin(), banner.end());

        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(bannerBuf) }, [&, serverSocket, joinSessionKey, serverCrypt](IO::NetworkError const& err)
        {
            REQUIRE_FALSE(err);

            std::string expectedClientBanner = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";
            auto clientBannerBuf = std::make_shared<std::vector<char>>(expectedClientBanner.size());

            serverSocket->Read(clientBannerBuf->data(), clientBannerBuf->size(), [&, serverSocket, joinSessionKey, serverCrypt](IO::NetworkError const& err, size_t) -> void
            {
                REQUIRE_FALSE(err);

                BigNumber serverChallengeBn;
                serverChallengeBn.SetRand(32 * 8);
                std::vector<uint8> serverChallengeBytes = serverChallengeBn.AsByteArray(32, false);
                auto serverChallenge = std::make_shared<std::array<uint8, 32>>();
                std::copy(serverChallengeBytes.begin(), serverChallengeBytes.end(), serverChallenge->begin());

                ByteBuffer challengeBody;
                challengeBody.append(std::array<uint8, 32>{}); // DosChallenge, all zero
                challengeBody.append(*serverChallenge);
                challengeBody << uint8(0); // DosZeroBits

                auto framedChallenge = std::make_shared<std::vector<uint8>>(Frame(CLASSIC_SMSG_AUTH_CHALLENGE, challengeBody, *serverCrypt));
                serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedChallenge) }, [&, serverSocket, serverChallenge, joinSessionKey, serverCrypt](IO::NetworkError const& err)
                {
                    REQUIRE_FALSE(err);

                    // Read the CMSG_AUTH_SESSION header, then its (variable-length, due to the
                    // trailing RealmJoinTicket) body.
                    auto header = std::make_shared<WireHeader>();
                    serverSocket->Read(reinterpret_cast<char*>(header.get()), sizeof(WireHeader), [&, serverSocket, header, serverChallenge, joinSessionKey, serverCrypt](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);

                        auto body = std::make_shared<std::vector<uint8>>(header->size);
                        serverSocket->Read(reinterpret_cast<char*>(body->data()), body->size(), [&, serverSocket, body, header, serverChallenge, joinSessionKey, serverCrypt](IO::NetworkError const& err, size_t) -> void
                        {
                            REQUIRE_FALSE(err);

                            std::array<uint8, 12> tag;
                            std::memcpy(tag.data(), header->tag, tag.size());
                            Unframe(*body, tag, *serverCrypt);

                            ByteBuffer data;
                            data.append(body->data(), body->size());
                            data.rpos(0);

                            uint32 opcode;
                            data >> opcode;
                            REQUIRE(opcode == CLASSIC_CMSG_AUTH_SESSION);

                            uint64 dosResponse; uint32 regionId, battlegroupId, realmId;
                            std::array<uint8, 32> localChallenge{};
                            std::array<uint8, 24> receivedDigest{};
                            data >> dosResponse >> regionId >> battlegroupId >> realmId;
                            data.read(localChallenge.data(), localChallenge.size());
                            data.read(receivedDigest.data(), receivedDigest.size());
                            uint8 useIPv6Byte; data >> useIPv6Byte;
                            uint32 ticketSize; data >> ticketSize;
                            std::string ticket = data.ReadString(ticketSize);
                            CHECK(ticket == TEST_REALM_JOIN_TICKET);

                            auto expectedDigest = WorldHandshakeCrypto::ComputeAuthCheckDigest(joinSessionKey, localChallenge, *serverChallenge);
                            CHECK(receivedDigest == expectedDigest);

                            auto sessionKey = WorldHandshakeCrypto::ComputeSessionKey(joinSessionKey, localChallenge, *serverChallenge);
                            auto encryptKey = std::make_shared<std::array<uint8, 32>>(WorldHandshakeCrypto::ComputeEncryptKey(sessionKey, localChallenge, *serverChallenge));

                            auto toSign = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(*encryptKey, true);
                            Ed25519 ed25519;
                            REQUIRE(ed25519.LoadFromByteArray(WorldHandshakeCrypto::EnterEncryptedModePrivateKey()));
                            std::vector<uint8> signContext(WorldHandshakeCrypto::EnableEncryptionContext().begin(), WorldHandshakeCrypto::EnableEncryptionContext().end());
                            std::vector<uint8> signature = ed25519.SignWithContext(toSign, signContext);
                            REQUIRE(signature.size() == 64);

                            ByteBuffer enterEncryptedModeBody;
                            enterEncryptedModeBody.append(signature.data(), signature.size());
                            enterEncryptedModeBody << uint8(0x80); // Enabled

                            auto framedEnterEncryptedMode = std::make_shared<std::vector<uint8>>(Frame(CLASSIC_SMSG_ENTER_ENCRYPTED_MODE, enterEncryptedModeBody, *serverCrypt));
                            serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedEnterEncryptedMode) }, [&, serverSocket, encryptKey, serverCrypt](IO::NetworkError const& err)
                            {
                                REQUIRE_FALSE(err);

                                // CMSG_ENTER_ENCRYPTED_MODE_ACK is still unencrypted on the wire
                                // (armed only after this is received, on both sides) -- but still
                                // has to go through serverCrypt to keep its recv counter in step.
                                auto ackHeader = std::make_shared<WireHeader>();
                                serverSocket->Read(reinterpret_cast<char*>(ackHeader.get()), sizeof(WireHeader), [&, serverSocket, ackHeader, encryptKey, serverCrypt](IO::NetworkError const& err, size_t) -> void
                                {
                                    REQUIRE_FALSE(err);
                                    REQUIRE(ackHeader->size == sizeof(uint32)); // opcode only, empty body

                                    auto ackBody = std::make_shared<std::vector<uint8>>(ackHeader->size);
                                    serverSocket->Read(reinterpret_cast<char*>(ackBody->data()), ackBody->size(), [&, serverSocket, ackBody, ackHeader, encryptKey, serverCrypt](IO::NetworkError const& err, size_t) -> void
                                    {
                                        REQUIRE_FALSE(err);

                                        std::array<uint8, 12> ackTag;
                                        std::memcpy(ackTag.data(), ackHeader->tag, ackTag.size());
                                        Unframe(*ackBody, ackTag, *serverCrypt);

                                        uint32 ackOpcode;
                                        std::memcpy(&ackOpcode, ackBody->data(), sizeof(ackOpcode));
                                        REQUIRE(ackOpcode == CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK);

                                        AES_GCM::Key key;
                                        std::copy(encryptKey->begin(), encryptKey->end(), key.begin());
                                        serverCrypt->Init(key);

                                        ByteBuffer authResponseBody;
                                        authResponseBody << uint32(0); // BattlenetRpcErrorCode::Ok
                                        authResponseBody.WriteBit(false); // SuccessInfo present -- not parsed by ClassicAuthResponse::Read either way
                                        authResponseBody.WriteBit(false); // WaitInfo present
                                        authResponseBody.FlushBits();

                                        auto framedAuthResponse = std::make_shared<std::vector<uint8>>(Frame(CLASSIC_SMSG_AUTH_RESPONSE, authResponseBody, *serverCrypt));
                                        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedAuthResponse) }, [&, serverSocket](IO::NetworkError const& err)
                                        {
                                            REQUIRE_FALSE(err);
                                            serverDone.Set();
                                        });
                                    });
                                });
                            });
                        });
                    });
                });
            });
        });
    });

    BotWorldHandshake::Options options;
    options.WorldAddress = "127.0.0.1";
    options.WorldPort = TestPort;
    options.SessionKey = joinSessionKey;
    options.RealmJoinTicket = TEST_REALM_JOIN_TICKET;
    options.RealmId = 1;

    std::shared_ptr<BotWorldHandshake> handshake = BotWorldHandshake::Create(ctx.get());

    WaitFlag handshakeDone;
    BotWorldHandshake::Result result;
    handshake->Handshake(options, [&](BotWorldHandshake::Result r)
    {
        result = std::move(r);
        handshakeDone.Set();
    });

    REQUIRE(handshakeDone.WaitFor(std::chrono::seconds(5)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    CHECK(result.AuthResult == 0);
    REQUIRE(result.Connection != nullptr);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));

    result.Connection->Close();
    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
}

TEST_CASE("BotWorldHandshake: a tampered Ed25519 signature is rejected", "[botworldhandshake]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort + 1);
    REQUIRE(acceptor != nullptr);

    std::vector<uint8> const joinSessionKey = TestSessionKey();
    WaitFlag serverDone;
    auto serverCrypt = std::make_shared<PacketCrypt>(PacketCryptRole::Server);

    acceptor->AutoAcceptSocketsUntilClose([&, serverCrypt](IO::Networking::SocketDescriptor descriptor)
    {
        auto serverSocket = std::make_shared<IO::Networking::AsyncSocket>(ctx.get(), std::move(descriptor));
        MANGOS_ASSERT(!(serverSocket->InitializeAndFixateMemoryLocation()));

        std::string banner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
        auto bannerBuf = std::make_shared<std::vector<uint8>>(banner.begin(), banner.end());

        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(bannerBuf) }, [&, serverSocket, serverCrypt](IO::NetworkError const& err)
        {
            REQUIRE_FALSE(err);

            std::string expectedClientBanner = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";
            auto clientBannerBuf = std::make_shared<std::vector<char>>(expectedClientBanner.size());

            serverSocket->Read(clientBannerBuf->data(), clientBannerBuf->size(), [&, serverSocket, serverCrypt](IO::NetworkError const& err, size_t) -> void
            {
                REQUIRE_FALSE(err);

                std::array<uint8, 32> serverChallenge{};
                ByteBuffer challengeBody;
                challengeBody.append(std::array<uint8, 32>{});
                challengeBody.append(serverChallenge);
                challengeBody << uint8(0);

                auto framedChallenge = std::make_shared<std::vector<uint8>>(Frame(CLASSIC_SMSG_AUTH_CHALLENGE, challengeBody, *serverCrypt));
                serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedChallenge) }, [&, serverSocket, serverCrypt](IO::NetworkError const& err)
                {
                    REQUIRE_FALSE(err);

                    auto header = std::make_shared<WireHeader>();
                    serverSocket->Read(reinterpret_cast<char*>(header.get()), sizeof(WireHeader), [&, serverSocket, header, serverCrypt](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);
                        auto body = std::make_shared<std::vector<uint8>>(header->size);
                        serverSocket->Read(reinterpret_cast<char*>(body->data()), body->size(), [&, serverSocket, body, header, serverCrypt](IO::NetworkError const& err, size_t) -> void
                        {
                            REQUIRE_FALSE(err);

                            std::array<uint8, 12> tag;
                            std::memcpy(tag.data(), header->tag, tag.size());
                            Unframe(*body, tag, *serverCrypt);

                            // A garbage (all-zero) signature -- Ed25519 verification must reject
                            // this, not treat it as a malformed-but-parseable packet.
                            std::array<uint8, 64> badSignature{};
                            ByteBuffer enterEncryptedModeBody;
                            enterEncryptedModeBody.append(badSignature);
                            enterEncryptedModeBody << uint8(0x80);

                            auto framedEnterEncryptedMode = std::make_shared<std::vector<uint8>>(Frame(CLASSIC_SMSG_ENTER_ENCRYPTED_MODE, enterEncryptedModeBody, *serverCrypt));
                            serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedEnterEncryptedMode) }, [&, serverSocket](IO::NetworkError const&)
                            {
                                serverDone.Set();
                            });
                        });
                    });
                });
            });
        });
    });

    BotWorldHandshake::Options options;
    options.WorldAddress = "127.0.0.1";
    options.WorldPort = TestPort + 1;
    options.SessionKey = joinSessionKey;
    options.RealmJoinTicket = TEST_REALM_JOIN_TICKET;
    options.RealmId = 1;

    std::shared_ptr<BotWorldHandshake> handshake = BotWorldHandshake::Create(ctx.get());

    WaitFlag handshakeDone;
    BotWorldHandshake::Result result;
    handshake->Handshake(options, [&](BotWorldHandshake::Result r)
    {
        result = std::move(r);
        handshakeDone.Set();
    });

    REQUIRE(handshakeDone.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(result.Success);
    INFO("error: " << result.Error);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));

    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
}
