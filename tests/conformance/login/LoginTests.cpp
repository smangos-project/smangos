// "login/" scenarios (port-plan.md step 2: the Battle.net login server replacing realmd), against
// ConformanceRun::Target(). One scenario per piece of that step, so vmangos.txt shrinks as each piece
// lands rather than all at once. A bot's stage is the step in progress and StopAt(X) holds as X
// begins (README, "Stages and stop_at"); bot.Held(X) is "held there, with everything the steps before
// it produced in place".
#include "BnetServiceHash.h"
#include "BotBnetConnection.h"
#include "BotRestClient.h"
#include "ClassicPackets.h"
#include "ConformanceRun.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/IpAddress.h"
#include "IO/Networking/SocketConnector.h"
#include "RealBotTarget.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include "connection_service.pb.h"
#include <catch_amalgamated.hpp>
#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>

namespace
{
// A name no account has, different on every run.
std::string UnknownAccountName()
{
    std::random_device device;
    return "nobody" + std::to_string(device()) + "@bots";
}

// A one-shot cross-thread signal, the same shape as the unit tests' own (BotBnetConnectionTests.cpp
// etc.) -- these two scenarios talk to the live target directly (BotBnetConnection/BotRestClient,
// not TestBot) so they need their own IoContext and callback synchronization instead of TestRoutine's.
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

// A live IoContext/thread pair, torn down on scope exit -- these two scenarios' own connection,
// independent of RealBotMgr's (TestBot's).
struct RawIoContext
{
    std::unique_ptr<IO::IoContext> ctx = IO::IoContext::CreateIoContext();
    std::thread ioThread{ [this]() { ctx->RunUntilShutdown(); } };

    ~RawIoContext()
    {
        ctx->Shutdown();
        ioThread.join();
    }
};

IO::Networking::AsyncSocket ConnectRaw(IO::IoContext* ctx, std::string const& address, uint16_t port, uint32_t timeoutMs)
{
    auto ip = IO::Networking::IpAddress::TryParseFromString(address);
    REQUIRE(ip.has_value());
    IO::Networking::IpEndpoint endpoint(*ip, port);

    auto descriptor = IO::Networking::SocketConnector::ConnectBlocking(endpoint, std::chrono::milliseconds(timeoutMs));
    REQUIRE(descriptor.has_value());
    return IO::Networking::AsyncSocket(ctx, std::move(descriptor.value()));
}
}

TEST_CASE("login: a bot reaches the world", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        bot.Disconnect(true);
        REQUIRE(co_await bot.Stage(BotStage::Disconnected, 10000));
        CHECK(bot.GetDisconnectReason() == BotDisconnectReason::Requested);
    });
}

TEST_CASE("login: the portal issues a login ticket", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Held as LoginRpc begins: the portal login is done, the RPC port not yet touched.
        TestBot bot(TestBotOptions().StopAt(BotStage::LoginRpc));
        REQUIRE(co_await bot.Held(BotStage::LoginRpc, 15000));
        CHECK_FALSE(bot.Info().Session.LoginTicket.empty());
    });
}

TEST_CASE("login: wrong password fails at the login portal", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().Password("definitely-wrong"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 10000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
        INFO("server said: " << bot.GetLastError());
    });
}

TEST_CASE("login: an unknown account fails at the login portal", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().AccountName(UnknownAccountName()));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 10000));
        CHECK(bot.GetFailedStage() == BotStage::LoginPortal);
        INFO("server said: " << bot.GetLastError());
    });
}

TEST_CASE("login: the RPC logon completes", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Held as RealmSelect begins: ConnectionService.Connect, Logon and VerifyWebCredentials
        // answered, and the AuthenticationListener.OnLogonComplete push received.
        TestBot bot(TestBotOptions().StopAt(BotStage::RealmSelect));
        REQUIRE(co_await bot.Held(BotStage::RealmSelect, 15000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("login: a forged login ticket is rejected at the RPC step", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().LoginTicket("forged-ticket-0123456789abcdef0123456789abcdef"));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 15000));
        CHECK(bot.GetFailedStage() == BotStage::LoginRpc);
        INFO("server said: " << bot.GetLastError());
    });
}

// Not here, though bot-phase-2.md lists it: "a reused login ticket is rejected". Neither reference
// server rejects one: TrinityCore's bnetserver and vibemangos's LoginTicketStore both only *expire*
// tickets (a TTL, refreshable via refreshLoginTicket), never consume them, so a used ticket that has
// not expired logs in again. Asserting otherwise would encode a rule no server has; an expired
// ticket is the thing a server must reject, and that is not testable inside a run.

TEST_CASE("login: the realm list contains the configured realm and parses", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // The bot picks its realm out of the parsed realm list by the target's configured name or
        // id; held as WorldConnecting begins means the list was received, parsed and contained it.
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldConnecting));
        REQUIRE(co_await bot.Held(BotStage::WorldConnecting, 20000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("login: realm join returns an address, a ticket and a server secret", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldConnecting));
        REQUIRE(co_await bot.Held(BotStage::WorldConnecting, 20000));

        BotSessionInfo const session = bot.Info().Session;
        CHECK_FALSE(session.WorldAddress.empty());
        CHECK(session.WorldPort != 0);
        CHECK_FALSE(session.RealmJoinTicket.empty());
        // client secret (32) || server secret (32): the world session's join key.
        CHECK(session.JoinSessionKey.size() == 64);
    });
}

TEST_CASE("login: joining a realm that is not listed fails at realm select", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().RealmId(0x7FFFFF01));
        REQUIRE(co_await bot.Stage(BotStage::Failed, 20000));
        CHECK(bot.GetFailedStage() == BotStage::RealmSelect);
        INFO("bot said: " << bot.GetLastError());
    });
}

// Not a TestBot/TestRoutine scenario: it drives its own BotBnetConnection directly at the live
// target's RPC port, so it can call a method nothing implements -- something a normal login never
// does. Plain (non-coroutine) TEST_CASE, blocking on WaitFlag, the same style the unit tests use
// for this class.
TEST_CASE("login: an RPC call to an unknown method does not drop the connection", "[protocol]")
{
    RealBotTarget const& target = ConformanceRun::TargetConfig();
    RawIoContext io;

    std::shared_ptr<BotBnetConnection> rpc = BotBnetConnection::Create(ConnectRaw(io.ctx.get(), target.LoginAddress, target.RpcPort, target.ConnectTimeoutMs));

    WaitFlag connected;
    bool connectOk = false;
    rpc->Connect([&](bool success) { connectOk = success; connected.Set(); });
    REQUIRE(connected.WaitFor(std::chrono::milliseconds(target.ConnectTimeoutMs)));
    REQUIRE(connectOk);

    // ConnectionService exists; no method is ever numbered this high.
    bgs::protocol::connection::v1::ConnectRequest bogus;
    WaitFlag unknownDone;
    bool unknownSuccess = false;
    uint32_t unknownStatus = 0;
    rpc->Call(uint32_t(BnetOriginalHash::ConnectionService), 0xFFFFFFu, bogus,
        [&](bool success, uint32_t status, std::string) { unknownSuccess = success; unknownStatus = status; unknownDone.Set(); });
    REQUIRE(unknownDone.WaitFor(std::chrono::seconds(10)));
    CHECK(unknownSuccess); // the RPC round trip itself completed
    CHECK(unknownStatus != 0); // ...with an error status, not Ok
    CHECK_FALSE(rpc->IsClosing());

    // The connection survives: a real call right after it still gets answered normally.
    bgs::protocol::connection::v1::ConnectRequest real;
    WaitFlag realDone;
    bool realSuccess = false;
    uint32_t realStatus = 0xFFFFFFFFu;
    rpc->Call(uint32_t(BnetOriginalHash::ConnectionService), uint32_t(BnetConnectionServiceMethods::Connect), real,
        [&](bool success, uint32_t status, std::string) { realSuccess = success; realStatus = status; realDone.Set(); });
    REQUIRE(realDone.WaitFor(std::chrono::seconds(10)));
    CHECK(realSuccess);
    CHECK(realStatus == 0);

    rpc->Close();
}

// Same shape as above: BotRestClient::PostFormInputs is a raw send (design principle: "any task or
// test can send any packet, typed or raw, malformed on purpose"), no SRP login involved.
TEST_CASE("login: a malformed REST body gets a 4xx response, not a hang", "[protocol]")
{
    RealBotTarget const& target = ConformanceRun::TargetConfig();
    RawIoContext io;

    std::shared_ptr<BotRestClient> rest = BotRestClient::Create(ConnectRaw(io.ctx.get(), target.LoginAddress, target.RestPort, target.ConnectTimeoutMs));

    WaitFlag connected;
    bool connectOk = false;
    rest->Connect([&](bool success) { connectOk = success; connected.Set(); });
    REQUIRE(connected.WaitFor(std::chrono::milliseconds(target.ConnectTimeoutMs)));
    REQUIRE(connectOk);

    WaitFlag done;
    bool ok = false;
    BotRestClient::HttpResponse response;
    rest->PostFormInputs("/bnetserver/login/srp/", "{not valid json at all",
        [&](bool posted, BotRestClient::HttpResponse resp) { ok = posted; response = std::move(resp); done.Set(); });

    // The bound this scenario exists to hold the server to: never a hang.
    REQUIRE(done.WaitFor(std::chrono::seconds(10)));
    REQUIRE(ok);
    CHECK(response.Status >= 400);
    CHECK(response.Status < 500);

    rest->Close();
}
