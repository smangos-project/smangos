// "world_socket/" scenarios (port-plan.md step 3: the V2 handshake, CMSG_AUTH_SESSION, encrypted mode,
// compression, ping/keepalive/time sync), against ConformanceRun::Target(). Where a scenario has to
// speak the handshake itself (to get it wrong on purpose) it holds a bot at StopAt(WorldHandshake) --
// socket up, banner exchanged, nothing answered -- and drives it by hand: the server's
// SMSG_AUTH_CHALLENGE is in the bot's history, the session info from the Battle.net half is published.
// Negative scenarios assert "rejected, and the connection closed" (the bot fails at WorldHandshake
// because its Realm connection dropped), never an exact result code.
#include "BotTask.h"
#include "ClassicPackets.h"
#include "ConformanceRun.h"
#include "RealBotTarget.h"
#include "TestBot.h"
#include "TestRoutine.h"
#include "WorldHandshakeCrypto.h"
#include <algorithm>
#include <catch_amalgamated.hpp>
#include <random>

namespace
{
// Drops one outgoing opcode: a bot that "never sends" it.
class DropOutgoingHook : public BotTask
{
    public:
        explicit DropOutgoingHook(uint32_t opcode) : m_opcode(opcode) {}

        char const* GetName() const override { return "drop_outgoing"; }
        BotTaskResult Update(BotContext&, uint32) override { return BotTaskResult::Running; }

        BotPacketAction OnPacket(BotContext&, BotPacket& packet, BotPacketDir dir) override
        {
            return dir == BotPacketDir::Outgoing && packet.Opcode == m_opcode ? BotPacketAction::Drop : BotPacketAction::Continue;
        }

    private:
        uint32_t m_opcode;
};

std::array<uint8, 32> RandomChallenge()
{
    std::random_device device;
    std::array<uint8, 32> challenge{};
    for (uint8& byte : challenge)
        byte = uint8(device());
    return challenge;
}

// A CMSG_AUTH_SESSION the way the bot itself would send it for `bot`'s session and the server's
// `challenge`: correct digest, the session's ticket. Callers then break one thing.
ClassicAuthSession CorrectAuthSession(TestBot const& bot, ClassicAuthChallenge const& challenge)
{
    BotSessionInfo const session = bot.Info().Session;

    ClassicAuthSession out;
    out.RealmId = session.RealmId;
    out.LocalChallenge = RandomChallenge();
    out.Digest = WorldHandshakeCrypto::ComputeAuthCheckDigest(session.JoinSessionKey, out.LocalChallenge, challenge.ServerChallenge);
    out.RealmJoinTicket = session.RealmJoinTicket;
    return out;
}

uint32_t ServerCloseTimeoutMs()
{
    return ConformanceRun::TargetConfig().ServerCloseTimeoutMs;
}
}

TEST_CASE("world_socket: encrypted mode is entered and SMSG_AUTH_RESPONSE is OK", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // Held as CharSelect begins: the V2 banner, SMSG_AUTH_CHALLENGE -> CMSG_AUTH_SESSION ->
        // SMSG_ENTER_ENCRYPTED_MODE -> ack (encryption armed) -> SMSG_AUTH_RESPONSE all happened.
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        std::optional<ClassicEnterEncryptedMode> enter = bot.LastReceived<ClassicEnterEncryptedMode>();
        REQUIRE(enter);
        CHECK(enter->Enabled);

        std::optional<ClassicAuthResponse> response = bot.LastReceived<ClassicAuthResponse>();
        REQUIRE(response);
        CHECK(response->Result == 0);
    });
}

TEST_CASE("world_socket: the server's SMSG_ENTER_ENCRYPTED_MODE signature verifies", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        // VerifyServerSignature is what makes the bot check the Ed25519 signature over the encryption
        // key digest (WorldHandshakeCrypto); a bad one fails the handshake before any ack goes out.
        TestBot bot(TestBotOptions().VerifyServerSignature(true).StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));
        CHECK(bot.GetLastError().empty());
    });
}

TEST_CASE("world_socket: a wrong CMSG_AUTH_SESSION digest is rejected and the connection closes", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldHandshake));
        REQUIRE(co_await bot.Held(BotStage::WorldHandshake, 30000));
        REQUIRE(co_await bot.WaitReceived(CLASSIC_SMSG_AUTH_CHALLENGE, 10000));

        std::optional<ClassicAuthChallenge> challenge = bot.LastReceived<ClassicAuthChallenge>();
        REQUIRE(challenge);

        ClassicAuthSession session = CorrectAuthSession(bot, *challenge);
        session.Digest[0] ^= 0xFF; // one flipped byte
        bot.Send(session);

        // Rejected: the server hangs up (the bot's Realm connection drops, failing it mid-handshake).
        REQUIRE(co_await bot.Stage(BotStage::Failed, ServerCloseTimeoutMs()));
        CHECK(bot.GetFailedStage() == BotStage::WorldHandshake);
        INFO("bot said: " << bot.GetLastError());
    });
}

TEST_CASE("world_socket: a join ticket for a different account is rejected", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::WorldHandshake));
        REQUIRE(co_await bot.Held(BotStage::WorldHandshake, 30000));
        REQUIRE(co_await bot.WaitReceived(CLASSIC_SMSG_AUTH_CHALLENGE, 10000));

        std::optional<ClassicAuthChallenge> challenge = bot.LastReceived<ClassicAuthChallenge>();
        REQUIRE(challenge);

        // This server's join ticket is the account id as a decimal string; the next id is somebody else.
        ClassicAuthSession session = CorrectAuthSession(bot, *challenge);
        REQUIRE_FALSE(session.RealmJoinTicket.empty());
        session.RealmJoinTicket = std::to_string(std::stoull(session.RealmJoinTicket) + 1);
        bot.Send(session);

        REQUIRE(co_await bot.Stage(BotStage::Failed, ServerCloseTimeoutMs()));
        CHECK(bot.GetFailedStage() == BotStage::WorldHandshake);
        INFO("bot said: " << bot.GetLastError());
    });
}

TEST_CASE("world_socket: never acking SMSG_ENTER_ENCRYPTED_MODE gets the connection closed", "[protocol][slow]")
{
    RunTest([]() -> TestRoutine
    {
        // The bot must not be the one to give up: its own per-packet and per-stage timeouts are put
        // far beyond the server's, so the only way out of WorldHandshake is the server closing.
        TestBot bot(TestBotOptions()
            .Hook([]() { return std::make_unique<DropOutgoingHook>(CLASSIC_CMSG_ENTER_ENCRYPTED_MODE_ACK); })
            .PacketTimeoutMs(10 * 60 * 1000)
            .StageTimeoutMs(0));

        REQUIRE(co_await bot.Stage(BotStage::Failed, ServerCloseTimeoutMs()));
        CHECK(bot.GetFailedStage() == BotStage::WorldHandshake);
        CHECK(bot.GetLastError().find("connection closed") != std::string::npos);
    });
}

TEST_CASE("world_socket: an unknown opcode after auth does not drop the connection", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
        REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

        // An opcode no server handles, with a body: it is skipped, not fatal.
        ByteBuffer body;
        body << uint32(0xDEADBEEF);
        bot.SendRaw(0x3FFFEE, body);

        // Still there: a ping gets its pong.
        ClassicPing ping;
        ping.Serial = 0x60000001;
        bot.Send(ping);
        auto pong = co_await bot.Packet<ClassicPong>(10000, [](ClassicPong const& p) { return p.Serial == 0x60000001; });
        REQUIRE(pong);
        CHECK(bot.GetStage() == BotStage::CharSelect);
    });
}

TEST_CASE("world_socket: a truncated known packet is handled without killing the server", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        {
            TestBot bot(TestBotOptions().StopAt(BotStage::CharSelect));
            REQUIRE(co_await bot.Held(BotStage::CharSelect, 30000));

            // CMSG_PLAYER_LOGIN wants a packed guid and a float; three bytes is neither.
            ByteBuffer body;
            body << uint8(0x01) << uint8(0xA0) << uint8(0x03);
            bot.SendRaw(CLASSIC_CMSG_PLAYER_LOGIN, body);

            // Whatever the server does with this connection is its business (drop it, ignore it);
            // give it time to have done it.
            co_await TestDelay(1000);
        }

        // The server is still up and serving: the next bot logs in all the way.
        TestBot next;
        REQUIRE(co_await next.Stage(BotStage::InWorld, 30000));
    });
}

TEST_CASE("world_socket: a compressed packet from the server is decoded", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));
        co_await TestDelay(1500); // the initial burst is what arrives compressed

        // BotWorldConnection inflates SMSG_COMPRESSED_PACKET on the connection's one stateful deflate
        // stream and hands on the inner packet, flagged; a broken stream would have closed the
        // connection instead, and a server that never compresses gives nothing to assert.
        std::vector<BotPacket> const received = bot.Received();
        REQUIRE_FALSE(received.empty());
        CHECK(std::any_of(received.begin(), received.end(), [](BotPacket const& packet) { return packet.Compressed; }));
        CHECK(bot.GetStage() == BotStage::InWorld);
    });
}

TEST_CASE("world_socket: time sync requests arrive and their responses are accepted", "[protocol][slow]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // The bot answers each SMSG_TIME_SYNC_REQUEST with CMSG_TIME_SYNC_RESPONSE by default. If the
        // server did not accept them it would stop sending (or drop the session): a second request
        // arriving, on a bot still InWorld, is the proof.
        REQUIRE(co_await bot.Packet<ClassicTimeSyncRequest>(30000));
        REQUIRE(co_await bot.Packet<ClassicTimeSyncRequest>(30000));
        CHECK(bot.GetStage() == BotStage::InWorld);
    });
}

TEST_CASE("world_socket: SMSG_PONG comes back on the connection the ping was sent on", "[protocol]")
{
    RunTest([]() -> TestRoutine
    {
        TestBot bot;
        REQUIRE(co_await bot.Stage(BotStage::InWorld, 30000));

        // The ping is answered by the socket that received it (WorldSocket::_HandlePing writes the
        // pong itself, whatever route the opcode table lists for SMSG_PONG), so each connection has
        // its own conversation: one ping on each, each pong on its own.
        for (BotConnectionKind connection : { BotConnectionKind::Realm, BotConnectionKind::Instance })
        {
            uint32_t const serial = connection == BotConnectionKind::Realm ? 0x60000002u : 0x60000003u;

            ClassicPing ping;
            ping.Serial = serial;
            bot.Send(ping, connection);

            auto pong = co_await bot.Packet<ClassicPong>(10000, [serial](ClassicPong const& p) { return p.Serial == serial; });
            REQUIRE(pong);
            std::optional<BotPacket> raw = bot.LastReceived(CLASSIC_SMSG_PONG);
            REQUIRE(raw);
            CHECK(raw->Connection == connection);
        }
    });
}
