// Coverage the Phase 1 suite never had: the SMSG_CONNECT_TO redirect to a second ("Instance") world
// connection and everything after it (bot-phase-2.md, "Handshake through the normal dispatch",
// "Risk"). Written against the FakeWorldServer scripts, which only know the wire protocol, so the
// same scenarios keep testing the bot's handshake through its rewrite.
#include "BotCharSelect.h"
#include "BotWorldHandshake.h"
#include "FakeWorldServer.h"
#include "IO/Context/IoContext.h"
#include <catch_amalgamated.hpp>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace
{
constexpr uint16_t RealmPort = 28540;
constexpr uint16_t InstancePort = 28541;
constexpr char const* TicketString = "4242";
constexpr uint64 RedirectKey = 0x1122334455667788ull;

class Latch
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

std::vector<uint8> JoinSessionKey()
{
    std::vector<uint8> key(64);
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = uint8(i * 5 + 3);
    return key;
}

ClassicGuid const CharacterGuid{ 0x0000000000ABCDEFull, 0x0000000000004000ull };

// Everything the fake Realm and Instance scripts saw, written by their threads and read by the test
// only after the scripts are known to have finished.
struct WorldState
{
    FakeWorldRealmResult Realm;
    FakeWorldInstanceResult Instance;
    ClassicPlayerLogin PlayerLogin;
    bool SawPlayerLogin = false;
    bool InstanceConnected = false;
};

// The Realm side of a normal login: handshake, ENUM_CHARACTERS answered with one character,
// PLAYER_LOGIN answered with SMSG_CONNECT_TO pointing at InstancePort.
void RunRealmScript(FakeWorldPeer& peer, WorldState& state)
{
    FakeWorldRealmOptions options;
    options.JoinSessionKey = JoinSessionKey();
    options.ExpectedTicket = TicketString;
    state.Realm = FakeWorldRunRealmHandshake(peer, options);
    if (!state.Realm.Completed)
        return;

    ClassicEnumCharacters enumRequest;
    if (!peer.Expect(enumRequest))
        return;

    ClassicEnumCharactersResult characters;
    characters.CharacterCount = 1;
    characters.FirstCharacterGuid = CharacterGuid;
    if (!peer.Send(characters))
        return;

    if (!peer.Expect(state.PlayerLogin))
        return;
    state.SawPlayerLogin = true;

    ClassicConnectTo connectTo;
    connectTo.AddressIPv4 = { 127, 0, 0, 1 };
    connectTo.Port = InstancePort;
    connectTo.Key = RedirectKey;
    connectTo.ConnectionType = 1;
    if (!peer.Send(connectTo))
        return;

    // Keep the Realm connection open until the client is done with it.
    peer.WaitForClientClose();
}

void RunInstanceScript(FakeWorldPeer& peer, WorldState& state)
{
    state.InstanceConnected = true;

    FakeWorldInstanceOptions options;
    options.SessionKey = state.Realm.SessionKey;
    options.ExpectedPackedKey = RedirectKey;
    state.Instance = FakeWorldRunInstanceHandshake(peer, options);
    if (!state.Instance.Completed)
        return;

    ClassicLoginVerifyWorld verify;
    verify.MapId = 1;
    verify.X = 10.0f;
    verify.Y = 20.0f;
    verify.Z = 30.0f;
    if (!peer.Send(verify))
        return;

    peer.WaitForClientClose();
}

// Runs BotWorldHandshake against the fake Realm and hands its result to `then`.
struct Harness
{
    std::unique_ptr<IO::IoContext> Ctx = IO::IoContext::CreateIoContext();
    std::thread IoThread;
    WorldState State;

    Harness()
    {
        REQUIRE(Ctx != nullptr);
        IoThread = std::thread([this]() { Ctx->RunUntilShutdown(); });
    }

    ~Harness()
    {
        Ctx->Shutdown();
        IoThread.join();
    }

    BotWorldHandshake::Result Handshake()
    {
        BotWorldHandshake::Options options;
        options.WorldAddress = "127.0.0.1";
        options.WorldPort = RealmPort;
        options.SessionKey = JoinSessionKey();
        options.RealmJoinTicket = TicketString;
        options.RealmId = 1;

        Latch done;
        BotWorldHandshake::Result result;
        BotWorldHandshake::Create(Ctx.get())->Handshake(options, [&](BotWorldHandshake::Result r)
        {
            result = std::move(r);
            done.Set();
        });
        REQUIRE(done.WaitFor(std::chrono::seconds(5)));
        return result;
    }

    BotCharSelect::Result CharSelect(BotWorldHandshake::Result const& handshake, BotStage stopAt = BotStage::InWorld)
    {
        BotCharSelect::Options options;
        options.CreateIfNone = false;
        options.SessionKey = handshake.SessionKey;
        options.StopAt = stopAt;

        Latch done;
        BotCharSelect::Result result;
        BotCharSelect::Create(Ctx.get(), handshake.Connection)->EnterWorld(options, [&](BotCharSelect::Result r)
        {
            result = std::move(r);
            done.Set();
        });
        REQUIRE(done.WaitFor(std::chrono::seconds(5)));
        return result;
    }
};
}

TEST_CASE("BotCharSelect: SMSG_CONNECT_TO opens an Instance connection that resumes, then SMSG_LOGIN_VERIFY_WORLD arrives on it", "[botredirect]")
{
    Harness harness;
    FakeWorldServer realm(harness.Ctx.get(), RealmPort, [&](FakeWorldPeer& peer) { RunRealmScript(peer, harness.State); });
    FakeWorldServer instance(harness.Ctx.get(), InstancePort, [&](FakeWorldPeer& peer) { RunInstanceScript(peer, harness.State); });

    BotWorldHandshake::Result handshake = harness.Handshake();
    REQUIRE(handshake.Success);

    BotCharSelect::Result result = harness.CharSelect(handshake);
    INFO("error: " << result.Error);
    REQUIRE(result.Success);
    CHECK_FALSE(result.Stopped);
    CHECK(result.CharacterGuid.Low == CharacterGuid.Low);
    CHECK(result.CharacterGuid.High == CharacterGuid.High);
    REQUIRE(result.Connection != nullptr);
    REQUIRE(result.RealmConnection != nullptr);
    CHECK(result.Connection != result.RealmConnection);

    result.Connection->Close();
    result.RealmConnection->Close();
    REQUIRE(realm.WaitForFinished(1));
    REQUIRE(instance.WaitForFinished(1));

    CHECK(harness.State.Realm.DigestMatched);
    CHECK(harness.State.Realm.TicketMatched);
    CHECK(harness.State.SawPlayerLogin);
    CHECK(harness.State.PlayerLogin.Guid.Low == CharacterGuid.Low);
    INFO("instance error: " << harness.State.Instance.Error);
    CHECK(harness.State.Instance.Completed);
    CHECK(harness.State.Instance.KeyMatched);
    CHECK(harness.State.Instance.DigestMatched);
}

TEST_CASE("BotCharSelect: StopAt EnteringWorld holds at SMSG_CONNECT_TO without following the redirect", "[botredirect]")
{
    Harness harness;
    FakeWorldServer realm(harness.Ctx.get(), RealmPort, [&](FakeWorldPeer& peer) { RunRealmScript(peer, harness.State); });
    FakeWorldServer instance(harness.Ctx.get(), InstancePort, [&](FakeWorldPeer& peer) { RunInstanceScript(peer, harness.State); });

    BotWorldHandshake::Result handshake = harness.Handshake();
    REQUIRE(handshake.Success);

    BotCharSelect::Result result = harness.CharSelect(handshake, BotStage::EnteringWorld);
    INFO("error: " << result.Error);
    REQUIRE(result.Success);
    CHECK(result.Stopped);
    CHECK(result.RealmConnection == nullptr); // no redirect happened

    // Give a (wrong) follow-up connect time to show up, then check none did.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(instance.Accepted() == 0);
    CHECK_FALSE(harness.State.InstanceConnected);

    result.Connection->Close();
    REQUIRE(realm.WaitForFinished(1));
}

TEST_CASE("BotCharSelect: a failed Instance connection fails the whole char select", "[botredirect]")
{
    Harness harness;
    FakeWorldServer realm(harness.Ctx.get(), RealmPort, [&](FakeWorldPeer& peer) { RunRealmScript(peer, harness.State); });
    // The Instance server accepts, then drops the connection before the banner exchange completes.
    FakeWorldServer instance(harness.Ctx.get(), InstancePort, [&](FakeWorldPeer& peer)
    {
        harness.State.InstanceConnected = true;
        peer.Close();
    });

    BotWorldHandshake::Result handshake = harness.Handshake();
    REQUIRE(handshake.Success);

    BotCharSelect::Result result = harness.CharSelect(handshake);
    CHECK_FALSE(result.Success);
    CHECK(result.Error.find("instance") != std::string::npos);
    CHECK(harness.State.InstanceConnected);

    if (result.Connection)
        result.Connection->Close();
    if (result.RealmConnection)
        result.RealmConnection->Close();
    REQUIRE(realm.WaitForFinished(1));
}

TEST_CASE("BotWorldHandshake: the fake Realm sees a matching digest and ticket, and the bot ends with the same session key", "[botredirect]")
{
    Harness harness;
    FakeWorldServer realm(harness.Ctx.get(), RealmPort, [&](FakeWorldPeer& peer) { RunRealmScript(peer, harness.State); });

    BotWorldHandshake::Result handshake = harness.Handshake();
    REQUIRE(handshake.Success);
    CHECK(handshake.AuthResult == 0);

    handshake.Connection->Close();
    REQUIRE(realm.WaitForFinished(1));
    CHECK(harness.State.Realm.DigestMatched);
    CHECK(harness.State.Realm.TicketMatched);
    CHECK(handshake.SessionKey == harness.State.Realm.SessionKey);
}
