#include "BotWorldConnection.h"
#include "BotConnector.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
#include "Errors.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
// A tiny synchronous rendezvous point: tests below drive real async socket I/O, but a Catch2
// TEST_CASE body needs to block until that exchange has actually happened.
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

constexpr uint16_t TestPort = 28517;
constexpr uint32 TestOpcode = 0x424242;

struct TestPacketBody
{
    static constexpr uint32 Opcode = TestOpcode;
    std::string Text = "bye";
    void Write(ByteBuffer& data) const { data.WriteString(Text); }
};
}

// Plays the server half of the V2 banner handshake by hand (bot-phase-1.md: "BotWorldConnection
// ... over loopback against a raw byte peer"), then sends one unencrypted framed packet and
// checks BotWorldConnection decodes it correctly, and that a packet the bot sends back arrives
// with the frame this test expects. Encryption/compression are exercised elsewhere (PacketCrypt
// already has its own half-against-half test in tests-shared; arming it here would just be
// re-testing that).
TEST_CASE("BotWorldConnection: completes the V2 banner exchange and exchanges a framed packet", "[botworldconnection]")
{
    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);

    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    WaitFlag serverGotClientBanner;
    WaitFlag serverGotBotPacket;
    WaitFlag botGotPacket;
    BotPacket receivedByBot;
    std::vector<uint8> receivedByServer;

    acceptor->AutoAcceptSocketsUntilClose([&](IO::Networking::SocketDescriptor descriptor)
    {
        auto serverSocket = std::make_shared<IO::Networking::AsyncSocket>(ctx.get(), std::move(descriptor));
        MANGOS_ASSERT(!(serverSocket->InitializeAndFixateMemoryLocation()));

        std::string banner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
        auto bannerBuf = std::make_shared<std::vector<uint8>>(banner.begin(), banner.end());

        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(bannerBuf) }, [&, serverSocket](IO::NetworkError const& err)
        {
            REQUIRE_FALSE(err);

            std::string expectedClientBanner = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";
            auto clientBannerBuf = std::make_shared<std::vector<char>>(expectedClientBanner.size());

            serverSocket->Read(clientBannerBuf->data(), clientBannerBuf->size(), [&, serverSocket, clientBannerBuf, expectedClientBanner](IO::NetworkError const& err, size_t) -> void
            {
                REQUIRE_FALSE(err);
                CHECK(std::string(clientBannerBuf->begin(), clientBannerBuf->end()) == expectedClientBanner);
                serverGotClientBanner.Set();

                // Send one unencrypted framed test packet: [u32 size][12-byte zero tag][u32 opcode][body].
                std::vector<uint8> plaintext(sizeof(uint32) + 3);
                plaintext[0] = uint8(TestOpcode & 0xFF);
                plaintext[1] = uint8((TestOpcode >> 8) & 0xFF);
                plaintext[2] = uint8((TestOpcode >> 16) & 0xFF);
                plaintext[3] = uint8((TestOpcode >> 24) & 0xFF);
                plaintext[4] = 'b'; plaintext[5] = 'o'; plaintext[6] = 't';

                auto framed = std::make_shared<std::vector<uint8>>();
                uint32 size = uint32(plaintext.size());
                framed->insert(framed->end(), reinterpret_cast<uint8*>(&size), reinterpret_cast<uint8*>(&size) + sizeof(size));
                framed->insert(framed->end(), 12, 0); // zero tag: crypt is never armed in this test
                framed->insert(framed->end(), plaintext.begin(), plaintext.end());

                serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framed) }, [&, serverSocket](IO::NetworkError const& err)
                {
                    REQUIRE_FALSE(err);

                    // Now read whatever the bot sends back: header (16 bytes) then its body.
                    auto header = std::make_shared<std::vector<uint8>>(16);
                    serverSocket->Read(reinterpret_cast<char*>(header->data()), header->size(), [&, serverSocket, header](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);
                        uint32 bodySize;
                        std::memcpy(&bodySize, header->data(), sizeof(bodySize));

                        auto body = std::make_shared<std::vector<uint8>>(bodySize);
                        serverSocket->Read(reinterpret_cast<char*>(body->data()), body->size(), [&, serverSocket, body](IO::NetworkError const& err, size_t) -> void
                        {
                            REQUIRE_FALSE(err);
                            receivedByServer = *body;
                            serverGotBotPacket.Set();
                        });
                    });
                });
            });
        });
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort);

    auto maybeDescriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(maybeDescriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(maybeDescriptor.value()));
    std::shared_ptr<BotWorldConnection> connection = BotWorldConnection::Create(std::move(clientSocket),
        [&](BotPacket packet)
        {
            receivedByBot = std::move(packet);
            botGotPacket.Set();
        },
        [](){});

    connection->StartHandshake();

    REQUIRE(botGotPacket.WaitFor(std::chrono::seconds(5)));
    CHECK(receivedByBot.Opcode == TestOpcode);
    REQUIRE(receivedByBot.Data.size() == 3);

    connection->Send(TestPacketBody{});

    REQUIRE(serverGotClientBanner.WaitFor(std::chrono::seconds(5)));
    REQUIRE(serverGotBotPacket.WaitFor(std::chrono::seconds(5)));
    REQUIRE(receivedByServer.size() == sizeof(uint32) + 3);
    CHECK(std::string(receivedByServer.begin() + sizeof(uint32), receivedByServer.end()) == "bye");

    connection->Close();
    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
}
