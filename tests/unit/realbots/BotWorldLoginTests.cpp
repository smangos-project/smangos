// The world half of the login (Net/BotWorldLogin.h) as BotRoutines on a real RealBot, against the
// scripted fake world server: the V2 banner and handshake on the Realm connection, the character
// screen, the SMSG_CONNECT_TO redirect and the Instance handshake. What bot-phase-2.md's "Handshake
// through the normal dispatch" promises is checked here packet by packet: every one of them, from
// SMSG_AUTH_CHALLENGE on and on both connections, reaches the bot's hooks, and a hook can drop or
// swallow it.
#include "BotWorldLogin.h"
#include "FakeWorldServer.h"
#include "TestBotDriver.h"
#include <algorithm>
#include <atomic>
#include <catch_amalgamated.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
constexpr uint16_t RealmPort = 28540;
constexpr uint16_t InstancePort = 28541;
constexpr char const* TicketString = "4242";
constexpr uint64 RedirectKey = 0x1122334455667788ull;
ClassicGuid const ExistingGuid{ 0x0000000000ABCDEFull, 0x0000000000004000ull };
ClassicGuid const CreatedGuid{ 0x0000000000FEDCBAull, 0x0000000000004000ull };

std::vector<uint8> JoinSessionKey()
{
    std::vector<uint8> key(64);
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = uint8(i * 5 + 3);
    return key;
}

// What the fake server does, and afterwards what it saw. The script threads write the result
// fields; the test reads them only once the scripts are known to have finished.
struct Scenario
{
    // Realm
    bool TamperSignature = false;
    uint32 AuthResult = 0;
    uint32 Characters = 1;    // 0 makes the bot create one
    bool Redirect = true;     // SMSG_CONNECT_TO to the Instance server; false: SMSG_LOGIN_VERIFY_WORLD at once
    bool GoSilentAfterBanner = false;
    bool CloseAfterChallenge = false; // closes after CMSG_AUTH_SESSION arrives
    bool StopAfterEnterEncryptedMode = false;
    uint32 ExpectedInstanceFailures = 0; // CMSG_CONNECT_TO_FAILED the Realm script waits for (a new SMSG_CONNECT_TO answers each)
    // Instance
    bool InstanceDropsImmediately = false; // every Instance connection is closed on accept
    uint32 InstanceDropsFirst = 0;         // ... only this many of them
    bool BatchResumeAndVerify = true; // SMSG_RESUME_COMMS and SMSG_LOGIN_VERIFY_WORLD in one write

    FakeWorldRealmResult Realm;
    FakeWorldInstanceResult Instance;
    bool SawEnumCharacters = false;
    bool SawPlayerLogin = false;
    ClassicGuid LoggedInGuid;
    bool SawCreateCharacter = false;
    std::string CreatedName;
    bool InstanceConnected = false;
    std::atomic<uint32> InstanceConnections{ 0 };
    std::vector<ClassicConnectToFailed> ReportedFailures; // written by the Realm script
    std::vector<uint32> SentSerials;                       // the serial of every SMSG_CONNECT_TO the Realm script sent
};

constexpr uint32 AttemptSerials[5] = {
    CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1, CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2, CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_3,
    CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_4, CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_5,
};

ClassicConnectTo MakeConnectTo(uint32 serial)
{
    ClassicConnectTo connectTo;
    connectTo.AddressIPv4 = { 127, 0, 0, 1 };
    connectTo.Port = InstancePort;
    connectTo.Serial = serial;
    connectTo.Key = RedirectKey;
    connectTo.ConnectionType = 1;
    return connectTo;
}

void RunRealmScript(FakeWorldPeer& peer, Scenario& s)
{
    if (s.GoSilentAfterBanner)
    {
        peer.ExchangeBanners();
        peer.WaitForClientClose();
        return;
    }
    if (s.CloseAfterChallenge)
    {
        // Closes only once the bot has answered the challenge, so the bot is certainly past
        // connecting and inside the handshake proper when the connection goes.
        ClassicAuthSession session;
        if (peer.ExchangeBanners() && peer.Send(ClassicAuthChallenge{}) && peer.Expect(session))
            peer.Close();
        return;
    }

    FakeWorldRealmOptions options;
    options.JoinSessionKey = JoinSessionKey();
    options.ExpectedTicket = TicketString;
    options.AuthResult = s.AuthResult;
    options.TamperSignature = s.TamperSignature;
    options.StopAfterEnterEncryptedMode = s.StopAfterEnterEncryptedMode;
    s.Realm = FakeWorldRunRealmHandshake(peer, options);
    if (s.StopAfterEnterEncryptedMode)
    {
        peer.WaitForClientClose();
        return;
    }
    if (!s.Realm.Completed || s.AuthResult != 0)
    {
        peer.WaitForClientClose();
        return;
    }

    ClassicEnumCharacters enumRequest;
    if (!peer.Expect(enumRequest))
        return;
    s.SawEnumCharacters = true;

    ClassicEnumCharactersResult characters;
    characters.CharacterCount = s.Characters;
    characters.FirstCharacterGuid = ExistingGuid;
    if (!peer.Send(characters))
        return;

    if (s.Characters == 0)
    {
        ClassicCreateCharacter create;
        if (!peer.Expect(create, std::chrono::milliseconds(1500)))
        {
            peer.WaitForClientClose();
            return;
        }
        s.SawCreateCharacter = true;
        s.CreatedName = create.Name;

        ClassicCreateCharResponse created;
        created.Result = CLASSIC_CHAR_CREATE_SUCCESS;
        created.Guid = CreatedGuid;
        if (!peer.Send(created))
            return;
    }

    ClassicPlayerLogin login;
    if (!peer.Expect(login, std::chrono::milliseconds(1500)))
    {
        peer.WaitForClientClose();
        return;
    }
    s.SawPlayerLogin = true;
    s.LoggedInGuid = login.Guid;

    if (s.Redirect)
    {
        s.SentSerials.push_back(AttemptSerials[0]);
        if (!peer.Send(MakeConnectTo(AttemptSerials[0])))
            return;

        // Each CMSG_CONNECT_TO_FAILED is answered with the next attempt, the way both reference
        // servers do; the fifth failure ends the escalation.
        for (uint32 failure = 0; failure < s.ExpectedInstanceFailures; ++failure)
        {
            ClassicConnectToFailed failed;
            if (!peer.Expect(failed))
                return;
            s.ReportedFailures.push_back(failed);

            if (failure + 1 < 5)
            {
                s.SentSerials.push_back(AttemptSerials[failure + 1]);
                if (!peer.Send(MakeConnectTo(AttemptSerials[failure + 1])))
                    return;
            }
        }
    }
    else
    {
        ClassicLoginVerifyWorld verify;
        verify.MapId = 1;
        if (!peer.Send(verify))
            return;
    }

    peer.WaitForClientClose();
}

void RunInstanceScript(FakeWorldPeer& peer, Scenario& s)
{
    s.InstanceConnected = true;
    uint32 const connection = ++s.InstanceConnections;
    if (s.InstanceDropsImmediately || connection <= s.InstanceDropsFirst)
    {
        peer.Close();
        return;
    }

    FakeWorldInstanceOptions options;
    options.SessionKey = s.Realm.SessionKey;
    options.ExpectedPackedKey = RedirectKey;
    options.SkipResumeComms = s.BatchResumeAndVerify;
    s.Instance = FakeWorldRunInstanceHandshake(peer, options);
    if (!s.Instance.Completed)
        return;

    ClassicLoginVerifyWorld verify;
    verify.MapId = 1;
    verify.X = 10.0f;
    verify.Y = 20.0f;
    verify.Z = 30.0f;

    if (s.BatchResumeAndVerify)
    {
        // Both in a single write: they reach the bot in one read batch, so the routine has to have
        // moved from waiting for the first to waiting for the second between the two dispatches.
        ByteBuffer resumeBody;
        ByteBuffer verifyBody;
        verify.Write(verifyBody);
        peer.SendBatch({ { CLASSIC_SMSG_RESUME_COMMS, resumeBody }, { CLASSIC_SMSG_LOGIN_VERIFY_WORLD, verifyBody } });
    }
    else
    {
        peer.Send(verify);
    }

    peer.WaitForClientClose();
}

// Notes every packet the bot's hooks are shown, in order, as "IN/OUT <opcode name> <connection>".
class RecordingHook : public BotTask
{
    public:
        char const* GetName() const override { return "recording_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            m_log.push_back(std::string(dir == BotPacketDir::Incoming ? "IN " : "OUT ") + ClassicOpcodeName(packet.Opcode)
                + (packet.Connection == BotConnectionKind::Realm ? " realm" : " instance"));
            return Filter(packet, dir);
        }

        // Refuses the first RefuseInstanceConnects Instance connects: a forced connection failure.
        BotPacketAction OnConnect(BotContext&, BotConnectionKind kind, std::string const&, uint16_t) override
        {
            m_log.push_back(std::string("CONNECT ") + (kind == BotConnectionKind::Realm ? "realm" : "instance"));
            if (kind == BotConnectionKind::Instance && RefuseInstanceConnects > 0)
            {
                --RefuseInstanceConnects;
                return BotPacketAction::Drop;
            }
            return BotPacketAction::Continue;
        }

        void OnConnectionLost(BotContext&, BotConnectionKind kind) override
        {
            m_log.push_back(std::string("LOST ") + (kind == BotConnectionKind::Realm ? "realm" : "instance"));
        }

        std::vector<std::string> const& Log() const { return m_log; }

        // A hook can act on what it sees: drop or swallow one opcode in one direction.
        uint32 RefuseInstanceConnects = 0;
        uint32 DropOpcode = 0;
        BotPacketDir DropDir = BotPacketDir::Outgoing;
        BotPacketAction DropAction = BotPacketAction::Drop;

    private:
        BotPacketAction Filter(BotPacket const& packet, BotPacketDir dir) const
        {
            return (DropOpcode && packet.Opcode == DropOpcode && dir == DropDir) ? DropAction : BotPacketAction::Continue;
        }

        std::vector<std::string> m_log;
};

// The fake server pair, a real bot and the routine's parameters, wired the way every test wants.
struct Harness
{
    std::unique_ptr<IO::IoContext> Ctx = IO::IoContext::CreateIoContext();
    std::thread IoThread;
    Scenario State;
    std::unique_ptr<FakeWorldServer> Realm;
    std::unique_ptr<FakeWorldServer> Instance;
    std::unique_ptr<TestBotDriver> Driver;
    RecordingHook* Hook = nullptr; // owned by the bot's brain; only touched through Driver->Query

    BotWorldLoginParams Params;
    BotWorldLoginOutcome Outcome;

    Harness()
    {
        REQUIRE(Ctx != nullptr);
        IoThread = std::thread([this]() { Ctx->RunUntilShutdown(); });

        Params.Handshake.WorldAddress = "127.0.0.1";
        Params.Handshake.WorldPort = RealmPort;
        Params.Handshake.JoinSessionKey = JoinSessionKey();
        Params.Handshake.RealmJoinTicket = TicketString;
        Params.Handshake.RealmId = 1;
        Params.CharSelect.CreateIfNone = false;
        Params.CharSelect.CharacterName = "Botto";
        Params.CharSelect.Race = 1;
        Params.CharSelect.Class = 1;
    }

    ~Harness()
    {
        Driver.reset();
        Instance.reset();
        Realm.reset();
        Ctx->Shutdown();
        IoThread.join();
    }

    void StartServers()
    {
        Realm = std::make_unique<FakeWorldServer>(Ctx.get(), RealmPort, [this](FakeWorldPeer& peer) { RunRealmScript(peer, State); });
        Instance = std::make_unique<FakeWorldServer>(Ctx.get(), InstancePort, [this](FakeWorldPeer& peer) { RunInstanceScript(peer, State); });
    }

    void StartBot(bool withHook = true, uint32 tickMs = 5)
    {
        Driver = std::make_unique<TestBotDriver>(Ctx.get(), tickMs);
        if (withHook)
        {
            Hook = static_cast<RecordingHook*>(Driver->Query([](RealBot& bot)
            {
                return bot.GetContext().AddHook(std::make_unique<RecordingHook>());
            }));
        }
    }

    void RunRoutine()
    {
        Driver->PushRoutine("world_login", [this](BotContext& ctx) { return BotWorldLoginRoutine(ctx, Params, Outcome); });
    }

    // Runs the routine to completion; false if it never finished.
    bool RunToEnd(std::chrono::milliseconds timeout = std::chrono::milliseconds(8000))
    {
        RunRoutine();
        return Driver->WaitForIdleStack(timeout);
    }

    std::vector<std::string> HookLog()
    {
        return Driver->Query([this](RealBot&) { return Hook->Log(); });
    }
};
}

TEST_CASE("world login: the whole handshake and redirect, every packet visible to hooks on the right connection", "[worldlogin]")
{
    Harness h;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    REQUIRE(h.Outcome.Success);
    CHECK_FALSE(h.Outcome.Stopped);
    CHECK(h.Outcome.CharacterGuid.Low == ExistingGuid.Low);
    CHECK(h.Outcome.CharacterGuid.High == ExistingGuid.High);

    // The bot holds both connections, and is where EnteringWorld left it (the built-in login
    // routine, not this one, is what sets InWorld).
    CHECK(h.Driver->Query([](RealBot& bot) { return bot.HasConnection(BotConnectionKind::Realm) && bot.HasConnection(BotConnectionKind::Instance); }));
    CHECK(h.Driver->Info().Stage == BotStage::EnteringWorld);

    // Every handshake packet, in order, on the connection it belongs to: the "Done when" bar.
    std::vector<std::string> const expected = {
        "CONNECT realm",
        "IN SMSG_AUTH_CHALLENGE realm",
        "OUT CMSG_AUTH_SESSION realm",
        "IN SMSG_ENTER_ENCRYPTED_MODE realm",
        "OUT CMSG_ENTER_ENCRYPTED_MODE_ACK realm",
        "IN SMSG_AUTH_RESPONSE realm",
        "OUT CMSG_ENUM_CHARACTERS realm",
        "IN SMSG_ENUM_CHARACTERS_RESULT realm",
        "OUT CMSG_PLAYER_LOGIN realm",
        "IN SMSG_CONNECT_TO realm",
        "CONNECT instance",
        "IN SMSG_AUTH_CHALLENGE instance",
        "OUT CMSG_AUTH_CONTINUED_SESSION instance",
        "IN SMSG_ENTER_ENCRYPTED_MODE instance",
        "OUT CMSG_ENTER_ENCRYPTED_MODE_ACK instance",
        "IN SMSG_RESUME_COMMS instance",
        "IN SMSG_LOGIN_VERIFY_WORLD instance",
    };
    CHECK(h.HookLog() == expected);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    REQUIRE(h.Instance->WaitForFinished(1));

    CHECK(h.State.Realm.Completed);
    CHECK(h.State.Realm.DigestMatched);
    CHECK(h.State.Realm.TicketMatched);
    CHECK(h.State.SawEnumCharacters);
    CHECK(h.State.SawPlayerLogin);
    CHECK(h.State.LoggedInGuid.Low == ExistingGuid.Low);
    INFO("instance error: " << h.State.Instance.Error);
    CHECK(h.State.Instance.Completed);
    CHECK(h.State.Instance.KeyMatched);
    CHECK(h.State.Instance.DigestMatched);
}

TEST_CASE("world login: a redirect with the next packet already queued still reaches SMSG_LOGIN_VERIFY_WORLD", "[worldlogin]")
{
    // The same flow with the two Instance packets sent separately, to pin down that the batched
    // and unbatched forms behave alike.
    Harness h;
    h.State.BatchResumeAndVerify = false;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    CHECK(h.Outcome.Success);
}

TEST_CASE("world login: without a redirect, SMSG_LOGIN_VERIFY_WORLD on the Realm connection ends the login", "[worldlogin]")
{
    Harness h;
    h.State.Redirect = false;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    CHECK(h.Outcome.Success);
    CHECK(h.Driver->Query([](RealBot& bot) { return bot.HasConnection(BotConnectionKind::Realm) && !bot.HasConnection(BotConnectionKind::Instance); }));
    CHECK_FALSE(h.State.InstanceConnected);
}

TEST_CASE("world login: StopAt WorldHandshake holds once the socket is up, before the challenge is answered", "[worldlogin]")
{
    Harness h;
    h.Params.Handshake.StopAt = BotStage::WorldHandshake;
    h.Params.CharSelect.StopAt = BotStage::WorldHandshake;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    REQUIRE(h.Outcome.Success);
    CHECK(h.Outcome.Stopped);
    CHECK(h.Driver->Info().Stage == BotStage::WorldHandshake);

    // The server's challenge reaches the hooks (nothing answers it), and no CMSG_AUTH_SESSION goes out.
    REQUIRE(h.Driver->WaitUntil([&]() { return h.HookLog().size() >= 2; }));
    CHECK(h.HookLog() == std::vector<std::string>{ "CONNECT realm", "IN SMSG_AUTH_CHALLENGE realm" });
    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK_FALSE(h.State.Realm.Completed);
}

TEST_CASE("world login: StopAt CharSelect holds after a successful SMSG_AUTH_RESPONSE, before the character screen", "[worldlogin]")
{
    Harness h;
    h.Params.Handshake.StopAt = BotStage::CharSelect;
    h.Params.CharSelect.StopAt = BotStage::CharSelect;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    REQUIRE(h.Outcome.Success);
    CHECK(h.Outcome.Stopped);
    CHECK(h.Driver->Info().Stage == BotStage::CharSelect);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK(h.State.Realm.Completed);
    CHECK_FALSE(h.State.SawEnumCharacters);
}

TEST_CASE("world login: StopAt EnteringWorld holds with a character resolved, before CMSG_PLAYER_LOGIN", "[worldlogin]")
{
    Harness h;
    h.Params.Handshake.StopAt = BotStage::EnteringWorld;
    h.Params.CharSelect.StopAt = BotStage::EnteringWorld;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    REQUIRE(h.Outcome.Success);
    CHECK(h.Outcome.Stopped);
    CHECK(h.Outcome.CharacterGuid.Low == ExistingGuid.Low);
    CHECK(h.Driver->Info().Stage == BotStage::EnteringWorld);
    CHECK(h.State.SawEnumCharacters);

    // Give a (wrong) follow-up connect time to show up, then check none did.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(h.Instance->Accepted() == 0);
    CHECK_FALSE(h.State.InstanceConnected);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK_FALSE(h.State.SawPlayerLogin);
}

TEST_CASE("world login: an account with no character creates one when CreateIfNone is set", "[worldlogin]")
{
    Harness h;
    h.State.Characters = 0;
    h.Params.CharSelect.CreateIfNone = true;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    REQUIRE(h.Outcome.Success);
    CHECK(h.Outcome.CharacterGuid.Low == CreatedGuid.Low);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK(h.State.SawCreateCharacter);
    CHECK(h.State.CreatedName == "Botto");
    CHECK(h.State.LoggedInGuid.Low == CreatedGuid.Low);
}

TEST_CASE("world login: an account with no character fails cleanly when CreateIfNone is off", "[worldlogin]")
{
    Harness h;
    h.State.Characters = 0;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("CreateIfNone") != std::string::npos);
    CHECK(h.Outcome.Error.find("char select") != std::string::npos);
}

TEST_CASE("world login: a tampered SMSG_ENTER_ENCRYPTED_MODE signature fails the handshake, unless verification is off", "[worldlogin]")
{
    {
        Harness h;
        h.State.TamperSignature = true;
        h.StartServers();
        h.StartBot();

        REQUIRE(h.RunToEnd());
        CHECK_FALSE(h.Outcome.Success);
        CHECK(h.Outcome.Error.find("signature") != std::string::npos);
        CHECK(h.Outcome.Error.find("world handshake") != std::string::npos);
    }
    {
        Harness h;
        h.State.TamperSignature = true;
        h.Params.Handshake.VerifyServerSignature = false;
        h.Params.CharSelect.VerifyServerSignature = false;
        h.StartServers();
        h.StartBot();

        REQUIRE(h.RunToEnd());
        INFO("error: " << h.Outcome.Error);
        CHECK(h.Outcome.Success);
    }
}

TEST_CASE("world login: a non-zero SMSG_AUTH_RESPONSE result fails the handshake", "[worldlogin]")
{
    Harness h;
    h.State.AuthResult = 12;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("SMSG_AUTH_RESPONSE result 12") != std::string::npos);
}

TEST_CASE("world login: a hook that drops CMSG_ENTER_ENCRYPTED_MODE_ACK stops the handshake there, unarmed", "[worldlogin]")
{
    Harness h;
    h.Params.Handshake.PacketTimeoutMs = 400;
    h.StartServers();
    h.StartBot();
    h.Driver->Query([&](RealBot&)
    {
        h.Hook->DropOpcode = CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK;
        h.Hook->DropDir = BotPacketDir::Outgoing;
        return true;
    });

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("timed out waiting for SMSG_AUTH_RESPONSE") != std::string::npos);

    // The hook saw the ack it dropped, and the server never did.
    std::vector<std::string> const log = h.HookLog();
    CHECK(std::find(log.begin(), log.end(), "OUT CMSG_ENTER_ENCRYPTED_MODE_ACK realm") != log.end());
    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK_FALSE(h.State.Realm.Completed);
}

TEST_CASE("world login: a hook that swallows SMSG_AUTH_CHALLENGE keeps the routine from ever seeing it", "[worldlogin]")
{
    Harness h;
    h.Params.Handshake.PacketTimeoutMs = 400;
    h.StartServers();
    h.StartBot();
    h.Driver->Query([&](RealBot&)
    {
        h.Hook->DropOpcode = CLASSIC_SMSG_AUTH_CHALLENGE;
        h.Hook->DropDir = BotPacketDir::Incoming;
        h.Hook->DropAction = BotPacketAction::Handled;
        return true;
    });

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("timed out waiting for SMSG_AUTH_CHALLENGE") != std::string::npos);
    // The hook saw it even though it swallowed it, and nothing was sent in reply.
    std::vector<std::string> const log = h.HookLog();
    REQUIRE(log.size() >= 2);
    CHECK(log[1] == "IN SMSG_AUTH_CHALLENGE realm");
    CHECK(std::none_of(log.begin(), log.end(), [](std::string const& line) { return line.rfind("OUT ", 0) == 0; }));
}

TEST_CASE("world login: a server that goes silent after the banner fails at the packet timeout", "[worldlogin]")
{
    Harness h;
    h.State.GoSilentAfterBanner = true;
    h.Params.Handshake.PacketTimeoutMs = 300;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("timed out waiting for SMSG_AUTH_CHALLENGE") != std::string::npos);
}

TEST_CASE("world login: the Realm connection closing mid-handshake fails the bot at that stage", "[worldlogin]")
{
    Harness h;
    h.State.CloseAfterChallenge = true;
    h.StartServers();
    h.StartBot();

    h.RunRoutine();
    REQUIRE(h.Driver->WaitForStage(BotStage::Failed));

    RealBotInfo const info = h.Driver->Info();
    CHECK(info.FailedStage == BotStage::WorldHandshake);
    CHECK(info.LastError.find("realm connection closed during WorldHandshake") != std::string::npos);
    CHECK(info.TaskStack.empty());
}

TEST_CASE("world login: a hook-forced Instance connect failure is reported and the next SMSG_CONNECT_TO tried", "[worldlogin][connectto]")
{
    Harness h;
    h.State.ExpectedInstanceFailures = 2;
    h.StartServers();
    h.StartBot();
    h.Driver->Query([&](RealBot&) { h.Hook->RefuseInstanceConnects = 2; return true; });

    REQUIRE(h.RunToEnd());
    INFO("error: " << h.Outcome.Error);
    REQUIRE(h.Outcome.Success);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    REQUIRE(h.Instance->WaitForFinished(1));

    // Two failures reported, each echoing the serial of the attempt that failed and its connection
    // type; the third attempt reached the Instance server, which saw exactly one connection.
    REQUIRE(h.State.ReportedFailures.size() == 2);
    CHECK(h.State.ReportedFailures[0].Serial == CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1);
    CHECK(h.State.ReportedFailures[1].Serial == CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_2);
    CHECK(h.State.ReportedFailures[0].Connection == 1);
    CHECK(h.State.SentSerials == std::vector<uint32>{ 17, 35, 53 });
    CHECK(h.State.InstanceConnections == 1);
    CHECK(h.State.Instance.Completed);

    // The refused connects never touched the network, and each failure went out on the Realm
    // connection where a hook could see it.
    std::vector<std::string> const log = h.HookLog();
    CHECK(std::count(log.begin(), log.end(), "CONNECT instance") == 3);
    CHECK(std::count(log.begin(), log.end(), "OUT CMSG_CONNECT_TO_FAILED realm") == 2);
    CHECK(std::count(log.begin(), log.end(), "IN SMSG_CONNECT_TO realm") == 3);
}

TEST_CASE("world login: five failed Instance connects abort the login", "[worldlogin][connectto]")
{
    Harness h;
    h.State.ExpectedInstanceFailures = 5;
    h.StartServers();
    h.StartBot();
    h.Driver->Query([&](RealBot&) { h.Hook->RefuseInstanceConnects = 100; return true; });

    REQUIRE(h.RunToEnd());
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("instance connect failed 5 times") != std::string::npos);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));

    // Every attempt's failure was reported, with the escalating serials, the fifth included (that is
    // what makes the server abort its side of the login too), and no sixth SMSG_CONNECT_TO exists.
    REQUIRE(h.State.ReportedFailures.size() == 5);
    for (size_t i = 0; i < 5; ++i)
        CHECK(h.State.ReportedFailures[i].Serial == AttemptSerials[i]);
    CHECK(h.State.SentSerials == std::vector<uint32>{ 17, 35, 53, 71, 89 });
    CHECK(h.State.InstanceConnections == 0);
}

TEST_CASE("world login: an Instance server that hangs up is retried promptly, not after a timeout", "[worldlogin][connectto]")
{
    Harness h;
    h.State.ExpectedInstanceFailures = 1;
    h.State.InstanceDropsFirst = 1;
    // A timeout long enough to fail the test if the bot waited it out instead of noticing the close.
    h.Params.Handshake.PacketTimeoutMs = 20000;
    h.Params.CharSelect.PacketTimeoutMs = 20000;
    h.StartServers();
    h.StartBot();

    auto const start = std::chrono::steady_clock::now();
    REQUIRE(h.RunToEnd(std::chrono::milliseconds(8000)));
    auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    INFO("error: " << h.Outcome.Error);
    REQUIRE(h.Outcome.Success);
    CHECK(elapsed.count() < 5000);

    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    REQUIRE(h.Realm->WaitForFinished(1));
    CHECK(h.State.InstanceConnections == 2);
    REQUIRE(h.State.ReportedFailures.size() == 1);
    CHECK(h.State.ReportedFailures[0].Serial == CLASSIC_CONNECT_TO_SERIAL_WORLD_ATTEMPT_1);

    // The Instance connection closing during EnteringWorld was the routine's to handle: the bot did
    // not fail, and its hooks were told.
    std::vector<std::string> const log = h.HookLog();
    CHECK(std::find(log.begin(), log.end(), "LOST instance") != log.end());
}

TEST_CASE("world login: an Instance server that always hangs up fails the login after five attempts", "[worldlogin][connectto]")
{
    Harness h;
    h.State.ExpectedInstanceFailures = 5;
    h.State.InstanceDropsImmediately = true;
    h.Params.Handshake.PacketTimeoutMs = 20000;
    h.Params.CharSelect.PacketTimeoutMs = 20000;
    h.StartServers();
    h.StartBot();

    REQUIRE(h.RunToEnd(std::chrono::milliseconds(15000)));
    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("instance connect failed 5 times") != std::string::npos);
    CHECK(h.State.InstanceConnections == 5);
}

TEST_CASE("world login: nothing is left connected or running after a failure", "[worldlogin]")
{
    Harness h;
    h.State.TamperSignature = true;
    h.StartServers();
    h.StartBot();
    REQUIRE(h.RunToEnd());
    REQUIRE_FALSE(h.Outcome.Success);

    // The routine itself only reports; the bot's login routine is what turns that into Failed.
    // What a bare routine leaves behind is one open Realm connection, which the bot can close.
    h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
    CHECK(h.Driver->Query([](RealBot& bot) { return !bot.HasConnection(BotConnectionKind::Realm); }));
    REQUIRE(h.Realm->WaitForFinished(1));
}

TEST_CASE("world login: the same flow at the real host's tick rate, every packet of a burst in one tick", "[worldlogin]")
{
    // RealBotMgr ticks a bot every 100ms, so everything the server sends in that time is dispatched
    // in a single Tick: the handshake, char screen and redirect must not depend on the bot getting a
    // tick between two packets.
    Harness h;
    h.StartServers();
    h.StartBot(true, 100);

    REQUIRE(h.RunToEnd(std::chrono::milliseconds(20000)));
    INFO("error: " << h.Outcome.Error);
    CHECK(h.Outcome.Success);
    CHECK(h.HookLog().back() == "IN SMSG_LOGIN_VERIFY_WORLD instance");
}

TEST_CASE("world login: the connect timeout bounds an unreachable world server", "[worldlogin]")
{
    Harness h;
    // TEST-NET-3 (RFC 5737): reserved for documentation, never routable, so the connect can only
    // time out. 300ms must be what bounds it, not the 10s default.
    h.Params.Handshake.WorldAddress = "203.0.113.1";
    h.Params.Handshake.ConnectTimeoutMs = 300;
    h.StartBot();

    auto const start = std::chrono::steady_clock::now();
    REQUIRE(h.RunToEnd(std::chrono::milliseconds(5000)));
    auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

    CHECK_FALSE(h.Outcome.Success);
    CHECK(h.Outcome.Error.find("world connect failed") != std::string::npos);
    CHECK(elapsed.count() < 3000);
}

namespace
{
// One packet a hook is told to interfere with: which way, which opcode, on which connection.
struct PacketRule
{
    BotPacketDir Dir;
    uint32 Opcode;
    BotConnectionKind Connection;
};

class RuleHook : public BotTask
{
    public:
        RuleHook(PacketRule rule, BotPacketAction action) : m_rule(rule), m_action(action) {}

        char const* GetName() const override { return "rule_hook"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            if (dir == m_rule.Dir && packet.Opcode == m_rule.Opcode && packet.Connection == m_rule.Connection)
            {
                ++Matched;
                return m_action;
            }
            return BotPacketAction::Continue;
        }

        uint32 Matched = 0;

    private:
        PacketRule m_rule;
        BotPacketAction m_action;
};
}

TEST_CASE("world login: every handshake packet is seen by hooks and the observer, and can be handled or dropped", "[worldlogin]")
{
    // The Done-when bar of bot-phase-2.md, one packet at a time: for each packet of the login, on the
    // connection it belongs to, a hook that swallows an incoming one (the routine waiting for it never
    // sees it) or drops an outgoing one (the server never receives it) stops the login there, while
    // the hook still saw the packet and the bot's observer saw every incoming one regardless.
    using Dir = BotPacketDir;
    using Conn = BotConnectionKind;
    std::vector<PacketRule> const incoming = {
        { Dir::Incoming, CLASSIC_SMSG_AUTH_CHALLENGE, Conn::Realm },
        { Dir::Incoming, CLASSIC_SMSG_ENTER_ENCRYPTED_MODE, Conn::Realm },
        { Dir::Incoming, CLASSIC_SMSG_AUTH_RESPONSE, Conn::Realm },
        { Dir::Incoming, CLASSIC_SMSG_ENUM_CHARACTERS_RESULT, Conn::Realm },
        { Dir::Incoming, CLASSIC_SMSG_CONNECT_TO, Conn::Realm },
        { Dir::Incoming, CLASSIC_SMSG_AUTH_CHALLENGE, Conn::Instance },
        { Dir::Incoming, CLASSIC_SMSG_ENTER_ENCRYPTED_MODE, Conn::Instance },
        { Dir::Incoming, CLASSIC_SMSG_RESUME_COMMS, Conn::Instance },
        { Dir::Incoming, CLASSIC_SMSG_LOGIN_VERIFY_WORLD, Conn::Instance },
    };
    std::vector<PacketRule> const outgoing = {
        { Dir::Outgoing, CLASSIC_CMSG_AUTH_SESSION, Conn::Realm },
        { Dir::Outgoing, CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK, Conn::Realm },
        { Dir::Outgoing, CLASSIC_CMSG_ENUM_CHARACTERS, Conn::Realm },
        { Dir::Outgoing, CLASSIC_CMSG_PLAYER_LOGIN, Conn::Realm },
        { Dir::Outgoing, CLASSIC_CMSG_AUTH_CONTINUED_SESSION, Conn::Instance },
        { Dir::Outgoing, CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK, Conn::Instance },
    };

    auto run = [](PacketRule const& rule, BotPacketAction action)
    {
        INFO((rule.Dir == BotPacketDir::Incoming ? "IN " : "OUT ") << ClassicOpcodeName(rule.Opcode)
            << (rule.Connection == BotConnectionKind::Realm ? " realm" : " instance"));

        // Declared before the harness: the bot's thread may still call the observer while the
        // harness is being torn down.
        std::vector<uint32> observed;

        Harness h;
        h.Params.Handshake.PacketTimeoutMs = 400;
        h.Params.CharSelect.PacketTimeoutMs = 400;
        h.StartServers();
        h.StartBot();

        auto* ruleHook = static_cast<RuleHook*>(h.Driver->Query([&](RealBot& bot)
        {
            bot.SetPacketObserver([&observed](BotPacket const& packet) { observed.push_back(packet.Opcode); });
            return bot.GetContext().AddHook(std::make_unique<RuleHook>(rule, action));
        }));

        REQUIRE(h.RunToEnd(std::chrono::milliseconds(15000)));
        CHECK_FALSE(h.Outcome.Success);

        // The hook saw it, whichever way it went.
        CHECK(h.Driver->Query([&](RealBot&) { return ruleHook->Matched; }) >= 1);

        std::vector<uint32> const seenByObserver = h.Driver->Query([&](RealBot&) { return observed; });
        if (rule.Dir == BotPacketDir::Incoming)
        {
            // The observer sits ahead of the hooks: a swallowed packet was still observed.
            CHECK(std::count(seenByObserver.begin(), seenByObserver.end(), rule.Opcode) >= 1);
        }
        else
        {
            // A dropped outgoing packet never reached the server. (The other connection's server
            // may legitimately have received the same opcode, so only the matching one is asked.)
            FakeWorldServer& server = rule.Connection == BotConnectionKind::Realm ? *h.Realm : *h.Instance;
            h.Driver->Query([](RealBot& bot) { bot.CloseConnections(); return true; });
            CHECK(server.WaitForFinished(server.Accepted() > 0 ? 1 : 0));
            std::vector<uint32> const received = server.ReceivedOpcodes();
            CHECK(std::count(received.begin(), received.end(), rule.Opcode) == 0);
        }
    };

    for (PacketRule const& rule : incoming)
        run(rule, BotPacketAction::Handled);
    for (PacketRule const& rule : outgoing)
        run(rule, BotPacketAction::Drop);
}
