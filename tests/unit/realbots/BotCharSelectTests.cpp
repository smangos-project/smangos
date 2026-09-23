// End-to-end coverage for BotCharSelect: a fake WorldSocket-shaped server playing CharSelect and
// EnteringWorld by hand over a raw AsyncSocket, same style as BotWorldConnectionTests.cpp /
// BotWorldHandshakeTests.cpp. Runs on an unarmed (unencrypted) BotWorldConnection -- BotCharSelect
// doesn't care about the connection's crypt state, only what BotWorldHandshake handed it, so
// skipping the handshake dance and going straight to the V2 banner keeps this test focused.
//
// Every AsyncSocket::Read/Write completion is written inline, nested directly in the TEST_CASE
// body, never through an intermediate free function taking a std::function callback parameter.
// An earlier draft of this file used such helpers (ReadFramedPacket/WriteFramedPacket/
// PlayServerBanner) and hit an intermittent (~30-50% of runs) access violation inside
// libcrypto-3-x64.dll, strictly after every assertion in the test had already passed -- i.e.
// during teardown, not test logic. Extensive isolation this session (Debug and ASan builds never
// reproduced it; Release/RelWithDebInfo did, consistently, at a high rate) narrowed it to those
// three helpers specifically: replacing them with inline lambdas -- the pattern every other test
// file in this suite already uses -- made the crash disappear across 90+ stress runs, with no
// other change required. The exact mechanism was not confirmed (a real debugger/crash dump would
// be needed), but the fix is real and reproducible. Do not reintroduce a std::function-wrapped
// free-function helper around AsyncSocket::Read/Write in this test suite without re-validating
// under repeated Release-build runs (`for /L %i in (1,1,30) do bin\Release\tests-realbots.exe
// "<test name>"`, checking exit codes) -- a handful of clean runs is not enough signal, and
// Debug/ASan builds will not show it either way.
#include "BotCharSelect.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
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

#pragma pack(push, 1)
struct WireHeader
{
    uint32 size;
    uint8 tag[12];
};
#pragma pack(pop)

// Pure data transformation, no socket/callback involved -- safe to factor out (matches
// BotWorldHandshakeTests.cpp's own Frame() helper).
std::vector<uint8> FrameUnencrypted(uint32 opcode, ByteBuffer const& body)
{
    std::vector<uint8> plaintext(sizeof(uint32) + body.size());
    std::memcpy(plaintext.data(), &opcode, sizeof(opcode));
    if (!body.empty())
        std::memcpy(plaintext.data() + sizeof(uint32), body.contents(), body.size());

    std::vector<uint8> framed(sizeof(WireHeader) + plaintext.size());
    WireHeader header{};
    header.size = uint32(plaintext.size());
    std::memcpy(framed.data(), &header, sizeof(header));
    std::memcpy(framed.data() + sizeof(header), plaintext.data(), plaintext.size());
    return framed;
}

ByteBuffer BuildEnumCharactersResultBody(uint32 characterCount, ClassicGuid const& firstGuid)
{
    ByteBuffer data;
    for (int i = 0; i < 9; ++i)
        data.WriteBit(i == 0);
    data.FlushBits();

    data << characterCount;
    for (int i = 0; i < 7; ++i)
        data << uint32(0);

    if (characterCount > 0)
        firstGuid.Write(data);
    return data;
}

// Decodes a CMSG_CREATE_CHARACTER body the way CharCreate::ReadFromWorldPacket does (see
// ClassicCreateCharacter::Write's own citation).
struct DecodedCreateCharacter
{
    std::string Name;
    uint8 Race = 0;
    uint8 Class = 0;
    uint8 Gender = 0;
};

DecodedCreateCharacter DecodeCreateCharacter(ByteBuffer& data)
{
    DecodedCreateCharacter out;
    uint32 nameLength = data.ReadBits(6);
    data.ReadBit(); // hasTemplateSet
    data.ReadBit(); // IsTrialBoost
    data.ReadBit(); // UseNPE
    data.ReadBit(); // HardcoreSelfFound
    data.ResetBitPos();

    uint32 customizationCount;
    int32 timerunningSeasonId;
    data >> out.Race >> out.Class >> out.Gender >> customizationCount >> timerunningSeasonId;
    out.Name = data.ReadString(nameLength);
    return out;
}

constexpr char const* TEST_CHARACTER_NAME = "Botto";
}

TEST_CASE("BotCharSelect: an existing character logs straight in", "[botcharselect]")
{
    constexpr uint16_t TestPort = 28540;

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    ClassicGuid const existingGuid{ 0x1234ull, 0 };
    WaitFlag serverDone;
    WaitFlag bannerDone;

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
                bannerDone.Set();

                // Read CMSG_ENUM_CHARACTERS (header + empty body).
                auto enumHeader = std::make_shared<WireHeader>();
                serverSocket->Read(reinterpret_cast<char*>(enumHeader.get()), sizeof(WireHeader), [&, serverSocket, enumHeader](IO::NetworkError const& err, size_t) -> void
                {
                    REQUIRE_FALSE(err);
                    auto enumBody = std::make_shared<std::vector<uint8>>(enumHeader->size);
                    serverSocket->Read(reinterpret_cast<char*>(enumBody->data()), enumBody->size(), [&, serverSocket, enumBody](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);
                        REQUIRE(enumBody->size() >= sizeof(uint32));
                        uint32 opcode;
                        std::memcpy(&opcode, enumBody->data(), sizeof(opcode));
                        CHECK(opcode == CLASSIC_CMSG_ENUM_CHARACTERS);

                        ByteBuffer resultBody = BuildEnumCharactersResultBody(1, existingGuid);
                        auto framedResult = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_ENUM_CHARACTERS_RESULT, resultBody));
                        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedResult) }, [&, serverSocket](IO::NetworkError const& err)
                        {
                            REQUIRE_FALSE(err);

                            // Read CMSG_PLAYER_LOGIN.
                            auto loginHeader = std::make_shared<WireHeader>();
                            serverSocket->Read(reinterpret_cast<char*>(loginHeader.get()), sizeof(WireHeader), [&, serverSocket, loginHeader](IO::NetworkError const& err, size_t) -> void
                            {
                                REQUIRE_FALSE(err);
                                auto loginBody = std::make_shared<std::vector<uint8>>(loginHeader->size);
                                serverSocket->Read(reinterpret_cast<char*>(loginBody->data()), loginBody->size(), [&, serverSocket, loginBody](IO::NetworkError const& err, size_t) -> void
                                {
                                    REQUIRE_FALSE(err);

                                    ByteBuffer data;
                                    data.append(loginBody->data(), loginBody->size());
                                    data.rpos(0);
                                    uint32 opcode;
                                    data >> opcode;
                                    CHECK(opcode == CLASSIC_CMSG_PLAYER_LOGIN);

                                    ClassicGuid loginGuid;
                                    REQUIRE(loginGuid.Read(data));
                                    CHECK(loginGuid.Low == existingGuid.Low);

                                    ByteBuffer verifyBody;
                                    verifyBody << uint32(1) << 0.0f << 0.0f << 0.0f << 0.0f << uint32(0);
                                    auto framedVerify = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_LOGIN_VERIFY_WORLD, verifyBody));
                                    serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedVerify) }, [&, serverSocket](IO::NetworkError const& err)
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

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort);
    auto descriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(descriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(descriptor.value()));
    std::shared_ptr<BotWorldConnection> connection = BotWorldConnection::Create(std::move(clientSocket), [](BotPacket) {}, []() {});
    connection->StartHandshake();

    REQUIRE(bannerDone.WaitFor(std::chrono::seconds(5)));

    std::shared_ptr<BotCharSelect> charSelect = BotCharSelect::Create(ctx.get(), connection);

    WaitFlag done;
    BotCharSelect::Result result;
    BotCharSelect::Options options;
    options.CreateIfNone = true;
    options.CharacterName = TEST_CHARACTER_NAME;
    charSelect->EnterWorld(options, [&](BotCharSelect::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    CHECK(result.CharacterGuid.Low == existingGuid.Low);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));

    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
    connection->Close();
}

TEST_CASE("BotCharSelect: creates a character when none exist, then logs in", "[botcharselect]")
{
    constexpr uint16_t TestPort = 28541;

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    ClassicGuid const newGuid{ 0xABCDull, 0 };
    WaitFlag serverDone;
    WaitFlag bannerDone;

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
                bannerDone.Set();

                // Read CMSG_ENUM_CHARACTERS.
                auto enumHeader = std::make_shared<WireHeader>();
                serverSocket->Read(reinterpret_cast<char*>(enumHeader.get()), sizeof(WireHeader), [&, serverSocket, enumHeader](IO::NetworkError const& err, size_t) -> void
                {
                    REQUIRE_FALSE(err);
                    auto enumBody = std::make_shared<std::vector<uint8>>(enumHeader->size);
                    serverSocket->Read(reinterpret_cast<char*>(enumBody->data()), enumBody->size(), [&, serverSocket, enumBody](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);
                        REQUIRE(enumBody->size() >= sizeof(uint32));
                        uint32 opcode;
                        std::memcpy(&opcode, enumBody->data(), sizeof(opcode));
                        CHECK(opcode == CLASSIC_CMSG_ENUM_CHARACTERS);

                        ByteBuffer emptyResult = BuildEnumCharactersResultBody(0, ClassicGuid{});
                        auto framedEmpty = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_ENUM_CHARACTERS_RESULT, emptyResult));
                        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedEmpty) }, [&, serverSocket](IO::NetworkError const& err)
                        {
                            REQUIRE_FALSE(err);

                            // Read CMSG_CREATE_CHARACTER.
                            auto createHeader = std::make_shared<WireHeader>();
                            serverSocket->Read(reinterpret_cast<char*>(createHeader.get()), sizeof(WireHeader), [&, serverSocket, createHeader](IO::NetworkError const& err, size_t) -> void
                            {
                                REQUIRE_FALSE(err);
                                auto createBody = std::make_shared<std::vector<uint8>>(createHeader->size);
                                serverSocket->Read(reinterpret_cast<char*>(createBody->data()), createBody->size(), [&, serverSocket, createBody](IO::NetworkError const& err, size_t) -> void
                                {
                                    REQUIRE_FALSE(err);

                                    ByteBuffer data;
                                    data.append(createBody->data(), createBody->size());
                                    data.rpos(0);
                                    uint32 opcode;
                                    data >> opcode;
                                    CHECK(opcode == CLASSIC_CMSG_CREATE_CHARACTER);

                                    DecodedCreateCharacter created = DecodeCreateCharacter(data);
                                    CHECK(created.Name == TEST_CHARACTER_NAME);
                                    CHECK(created.Race == 1);
                                    CHECK(created.Class == 4);

                                    ByteBuffer createResponse;
                                    createResponse << CLASSIC_CHAR_CREATE_SUCCESS;
                                    newGuid.Write(createResponse);
                                    auto framedCreateResponse = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_CREATE_CHAR, createResponse));
                                    serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedCreateResponse) }, [&, serverSocket](IO::NetworkError const& err)
                                    {
                                        REQUIRE_FALSE(err);

                                        // Read CMSG_PLAYER_LOGIN.
                                        auto loginHeader = std::make_shared<WireHeader>();
                                        serverSocket->Read(reinterpret_cast<char*>(loginHeader.get()), sizeof(WireHeader), [&, serverSocket, loginHeader](IO::NetworkError const& err, size_t) -> void
                                        {
                                            REQUIRE_FALSE(err);
                                            auto loginBody = std::make_shared<std::vector<uint8>>(loginHeader->size);
                                            serverSocket->Read(reinterpret_cast<char*>(loginBody->data()), loginBody->size(), [&, serverSocket, loginBody](IO::NetworkError const& err, size_t) -> void
                                            {
                                                REQUIRE_FALSE(err);

                                                ByteBuffer loginData;
                                                loginData.append(loginBody->data(), loginBody->size());
                                                loginData.rpos(0);
                                                uint32 loginOpcode;
                                                loginData >> loginOpcode;
                                                CHECK(loginOpcode == CLASSIC_CMSG_PLAYER_LOGIN);

                                                ClassicGuid loginGuid;
                                                REQUIRE(loginGuid.Read(loginData));
                                                CHECK(loginGuid.Low == newGuid.Low);

                                                ByteBuffer verifyBody;
                                                verifyBody << uint32(1) << 0.0f << 0.0f << 0.0f << 0.0f << uint32(0);
                                                auto framedVerify = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_LOGIN_VERIFY_WORLD, verifyBody));
                                                serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedVerify) }, [&, serverSocket](IO::NetworkError const& err)
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
        });
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort);
    auto descriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(descriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(descriptor.value()));
    std::shared_ptr<BotWorldConnection> connection = BotWorldConnection::Create(std::move(clientSocket), [](BotPacket) {}, []() {});
    connection->StartHandshake();

    REQUIRE(bannerDone.WaitFor(std::chrono::seconds(5)));

    std::shared_ptr<BotCharSelect> charSelect = BotCharSelect::Create(ctx.get(), connection);

    WaitFlag done;
    BotCharSelect::Result result;
    BotCharSelect::Options options;
    options.CreateIfNone = true;
    options.CharacterName = TEST_CHARACTER_NAME;
    options.Race = 1;
    options.Class = 4;
    charSelect->EnterWorld(options, [&](BotCharSelect::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    INFO("error: " << result.Error);
    CHECK(result.Success);
    CHECK(result.CharacterGuid.Low == newGuid.Low);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));

    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
    connection->Close();
}

TEST_CASE("BotCharSelect: fails cleanly with no characters and CreateIfNone false", "[botcharselect]")
{
    constexpr uint16_t TestPort = 28542;

    auto ctx = IO::IoContext::CreateIoContext();
    REQUIRE(ctx != nullptr);
    std::thread ioThread([&ctx]() { ctx->RunUntilShutdown(); });

    auto acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx.get(), "127.0.0.1", TestPort);
    REQUIRE(acceptor != nullptr);

    WaitFlag serverDone;
    WaitFlag bannerDone;

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
                bannerDone.Set();

                auto enumHeader = std::make_shared<WireHeader>();
                serverSocket->Read(reinterpret_cast<char*>(enumHeader.get()), sizeof(WireHeader), [&, serverSocket, enumHeader](IO::NetworkError const& err, size_t) -> void
                {
                    REQUIRE_FALSE(err);
                    auto enumBody = std::make_shared<std::vector<uint8>>(enumHeader->size);
                    serverSocket->Read(reinterpret_cast<char*>(enumBody->data()), enumBody->size(), [&, serverSocket, enumBody](IO::NetworkError const& err, size_t) -> void
                    {
                        REQUIRE_FALSE(err);
                        REQUIRE(enumBody->size() >= sizeof(uint32));
                        uint32 opcode;
                        std::memcpy(&opcode, enumBody->data(), sizeof(opcode));
                        CHECK(opcode == CLASSIC_CMSG_ENUM_CHARACTERS);

                        ByteBuffer emptyResult = BuildEnumCharactersResultBody(0, ClassicGuid{});
                        auto framedEmpty = std::make_shared<std::vector<uint8>>(FrameUnencrypted(CLASSIC_SMSG_ENUM_CHARACTERS_RESULT, emptyResult));
                        serverSocket->Write({ std::shared_ptr<std::vector<uint8> const>(framedEmpty) }, [&, serverSocket](IO::NetworkError const& err)
                        {
                            REQUIRE_FALSE(err);
                            serverDone.Set();
                        });
                    });
                });
            });
        });
    });

    auto ip = IO::Networking::IpAddress::TryParseFromString("127.0.0.1");
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, TestPort);
    auto descriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::seconds(5));
    REQUIRE(descriptor.has_value());

    IO::Networking::AsyncSocket clientSocket(ctx.get(), std::move(descriptor.value()));
    std::shared_ptr<BotWorldConnection> connection = BotWorldConnection::Create(std::move(clientSocket), [](BotPacket) {}, []() {});
    connection->StartHandshake();

    REQUIRE(bannerDone.WaitFor(std::chrono::seconds(5)));

    std::shared_ptr<BotCharSelect> charSelect = BotCharSelect::Create(ctx.get(), connection);

    WaitFlag done;
    BotCharSelect::Result result;
    BotCharSelect::Options options;
    options.CreateIfNone = false;
    charSelect->EnterWorld(options, [&](BotCharSelect::Result r)
    {
        result = std::move(r);
        done.Set();
    });

    REQUIRE(done.WaitFor(std::chrono::seconds(5)));
    CHECK_FALSE(result.Success);
    INFO("error: " << result.Error);

    REQUIRE(serverDone.WaitFor(std::chrono::seconds(5)));

    acceptor->ClosePortAndStopAcceptingNewConnections();
    ctx->Shutdown();
    ioThread.join();
    connection->Close();
}
