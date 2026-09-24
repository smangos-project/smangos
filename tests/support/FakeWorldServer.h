#ifndef REALBOTS_TESTS_FAKEWORLDSERVER_H
#define REALBOTS_TESTS_FAKEWORLDSERVER_H

// A scripted fake of the modern world server for unit tests (bot-phase-2.md, "Handshake through
// the normal dispatch"): one script per accepted connection, written as straight-line code against
// a blocking FakeWorldPeer, so a test can play a Realm connection, an Instance connection after an
// SMSG_CONNECT_TO, or anything in between without nesting AsyncSocket callbacks. The wire side is
// independent of how the client is built (it only knows the protocol), which is what lets the same
// scripts test the bot's handshake before and after it is rewritten.
//
// Scripts run on their own thread per connection, blocking on the IO thread's completions. They
// never use Catch assertions (not thread-safe); they record what they saw into a result struct the
// test checks afterwards. Every blocking call has a timeout so a wedged client fails the test
// instead of hanging it.
#include "BotPacket.h"
#include "ByteBuffer.h"
#include "ClassicPackets.h"
#include "Crypto/BigNumber.h"
#include "Crypto/Ed25519.h"
#include "Crypto/Encryption/PacketCrypt.h"
#include "Errors.h"
#include "IO/Context/IoContext.h"
#include "IO/Networking/AsyncSocket.h"
#include "IO/Networking/AsyncSocketAcceptor.h"
#include "WorldHandshakeCrypto.h"
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// One completion of an async IO call, waited on from a script thread.
class FakeWorldIoWait
{
    public:
        void Complete(bool failed)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done = true;
            m_failed = failed;
            m_cv.notify_all();
        }

        // false on timeout; `failed` reports the IO error otherwise.
        bool WaitFor(std::chrono::milliseconds timeout, bool& failed)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (!m_cv.wait_for(lock, timeout, [this]() { return m_done; }))
                return false;
            failed = m_failed;
            return true;
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_done = false;
        bool m_failed = false;
};

class FakeWorldPeer
{
    public:
        static constexpr std::chrono::milliseconds DefaultTimeout{ 5000 };

        FakeWorldPeer(IO::IoContext* ctx, IO::Networking::SocketDescriptor descriptor)
            : m_socket(std::make_shared<IO::Networking::AsyncSocket>(ctx, std::move(descriptor))), m_crypt(PacketCryptRole::Server)
        {
            MANGOS_ASSERT(!m_socket->InitializeAndFixateMemoryLocation());
        }

        ~FakeWorldPeer() { Close(); }

        void Close() { m_socket->CloseSocket(); }

        PacketCrypt& Crypt() { return m_crypt; }
        std::string const& Error() const { return m_error; }
        bool Ok() const { return m_error.empty(); }

        // The plain V2 banner exchange: the server's line, then the client's line back.
        bool ExchangeBanners()
        {
            std::string const banner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
            if (!WriteAll(std::vector<uint8>(banner.begin(), banner.end())))
                return false;

            std::string const expected = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";
            std::vector<uint8> received(expected.size());
            if (!ReadExact(received.data(), received.size()))
                return false;
            if (std::memcmp(received.data(), expected.data(), expected.size()) != 0)
                return Fail("bad client banner");
            return true;
        }

        // Frames `body` under the opcode through the connection's PacketCrypt (unarmed until Init,
        // when it just zero-tags -- its counter still advances, which is what keeps both sides in
        // step, see PacketCrypt.h).
        bool SendPacket(uint32 opcode, ByteBuffer const& body)
        {
            std::vector<uint8> framed;
            return FramePacket(opcode, body, framed) && WriteAll(std::move(framed));
        }

        template<typename T>
        bool Send(T const& packet)
        {
            ByteBuffer body;
            packet.Write(body);
            return SendPacket(T::Opcode, body);
        }

        // Frames every packet in order and writes them as one chunk, so the client's socket
        // delivers them back to back (what a busy server does).
        bool SendBatch(std::vector<std::pair<uint32, ByteBuffer>> const& packets)
        {
            std::vector<uint8> all;
            for (auto const& [opcode, body] : packets)
            {
                std::vector<uint8> framed;
                if (!FramePacket(opcode, body, framed))
                    return false;
                all.insert(all.end(), framed.begin(), framed.end());
            }
            return WriteAll(std::move(all));
        }

        // Reads and decrypts the next client packet. false on timeout, IO error or a bad tag.
        bool ReadPacket(BotPacket& out, std::chrono::milliseconds timeout = DefaultTimeout)
        {
            WireHeader header{};
            if (!ReadExact(reinterpret_cast<uint8*>(&header), sizeof(header), timeout))
                return false;
            if (header.size < sizeof(uint32) || header.size >= 0x40000)
                return Fail("bad client packet size " + std::to_string(header.size));

            std::vector<uint8> body(header.size);
            if (!ReadExact(body.data(), body.size(), timeout))
                return false;

            AES_GCM::Tag tag;
            std::memcpy(tag, header.tag, sizeof(tag));
            if (!m_crypt.DecryptRecv(body.data(), body.size(), tag))
                return Fail("DecryptRecv failed");

            std::memcpy(&out.Opcode, body.data(), sizeof(uint32));
            out.Data.clear();
            if (body.size() > sizeof(uint32))
                out.Data.append(body.data() + sizeof(uint32), body.size() - sizeof(uint32));
            out.Data.rpos(0);
            return true;
        }

        // Reads the next client packet and parses it as T; a different opcode or a body T::Read
        // rejects fails the peer, with which opcode arrived recorded in Error().
        template<typename T>
        bool Expect(T& out, std::chrono::milliseconds timeout = DefaultTimeout)
        {
            BotPacket packet;
            if (!ReadPacket(packet, timeout))
                return false;
            m_lastOpcode = packet.Opcode;
            if (packet.Opcode != T::Opcode)
                return Fail("expected opcode " + Hex(T::Opcode) + ", got " + Hex(packet.Opcode));
            if (!out.Read(packet.Data))
                return Fail("malformed body for opcode " + Hex(packet.Opcode));
            return true;
        }

        uint32 LastOpcode() const { return m_lastOpcode; }

        // True once the client has closed its end (or an error/timeout hit): a read of one byte
        // would fail. Only meant for "the client closed the connection" scenarios.
        bool WaitForClientClose(std::chrono::milliseconds timeout = DefaultTimeout)
        {
            bool failed = false;
            auto buffer = std::make_shared<std::vector<char>>(1);
            auto wait = std::make_shared<FakeWorldIoWait>();
            m_socket->Read(buffer->data(), 1, [wait, buffer](IO::NetworkError const& error, size_t) { wait->Complete(bool(error)); });
            return wait->WaitFor(timeout, failed) && failed;
        }

        bool ReadExact(uint8* out, size_t size, std::chrono::milliseconds timeout = DefaultTimeout)
        {
            if (size == 0)
                return true;

            auto buffer = std::make_shared<std::vector<char>>(size);
            auto wait = std::make_shared<FakeWorldIoWait>();
            m_socket->Read(buffer->data(), size, [wait, buffer](IO::NetworkError const& error, size_t) { wait->Complete(bool(error)); });

            bool failed = false;
            if (!wait->WaitFor(timeout, failed))
                return Fail("timed out reading " + std::to_string(size) + " bytes");
            if (failed)
                return Fail("socket error or close while reading");

            std::memcpy(out, buffer->data(), size);
            return true;
        }

        bool WriteAll(std::vector<uint8> bytes, std::chrono::milliseconds timeout = DefaultTimeout)
        {
            auto buffer = std::make_shared<std::vector<uint8>>(std::move(bytes));
            auto wait = std::make_shared<FakeWorldIoWait>();
            m_socket->Write({ std::shared_ptr<std::vector<uint8> const>(buffer) }, [wait, buffer](IO::NetworkError const& error) { wait->Complete(bool(error)); });

            bool failed = false;
            if (!wait->WaitFor(timeout, failed))
                return Fail("timed out writing");
            if (failed)
                return Fail("socket error or close while writing");
            return true;
        }

    private:
#pragma pack(push, 1)
        struct WireHeader
        {
            uint32 size;
            uint8 tag[12];
        };
#pragma pack(pop)

        bool FramePacket(uint32 opcode, ByteBuffer const& body, std::vector<uint8>& framed)
        {
            std::vector<uint8> plaintext(sizeof(uint32) + body.size());
            std::memcpy(plaintext.data(), &opcode, sizeof(opcode));
            if (!body.empty())
                std::memcpy(plaintext.data() + sizeof(uint32), body.contents(), body.size());

            AES_GCM::Tag tag;
            if (!m_crypt.EncryptSend(plaintext.data(), plaintext.size(), tag))
                return Fail("EncryptSend failed");

            framed.resize(sizeof(WireHeader) + plaintext.size());
            WireHeader header{};
            header.size = uint32(plaintext.size());
            std::memcpy(header.tag, tag, sizeof(header.tag));
            std::memcpy(framed.data(), &header, sizeof(header));
            std::memcpy(framed.data() + sizeof(header), plaintext.data(), plaintext.size());
            return true;
        }

        static std::string Hex(uint32 value)
        {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "0x%X", value);
            return buffer;
        }

        bool Fail(std::string error)
        {
            if (m_error.empty())
                m_error = std::move(error);
            return false;
        }

        std::shared_ptr<IO::Networking::AsyncSocket> m_socket;
        PacketCrypt m_crypt;
        std::string m_error;
        uint32 m_lastOpcode = 0;
};

// Accepts connections on a fixed port and runs `script` on a thread of its own for each. Destroy it
// before the IoContext shuts down; the destructor closes every peer (which unblocks any script
// still waiting on one) and joins the threads.
class FakeWorldServer
{
    public:
        using Script = std::function<void(FakeWorldPeer&)>;

        FakeWorldServer(IO::IoContext* ctx, uint16_t port, Script script)
            : m_ctx(ctx), m_script(std::move(script))
        {
            m_acceptor = IO::Networking::AsyncSocketAcceptor::CreateAndBindServer(ctx, "127.0.0.1", port);
            MANGOS_ASSERT(m_acceptor != nullptr);

            m_acceptor->AutoAcceptSocketsUntilClose([this](IO::Networking::SocketDescriptor descriptor)
            {
                auto peer = std::make_shared<FakeWorldPeer>(m_ctx, std::move(descriptor));
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_stopping)
                    return;
                m_peers.push_back(peer);
                ++m_accepted;
                m_threads.emplace_back([this, peer]()
                {
                    m_script(*peer);
                    m_finished.fetch_add(1);
                });
            });
        }

        ~FakeWorldServer()
        {
            m_acceptor->ClosePortAndStopAcceptingNewConnections();

            std::vector<std::shared_ptr<FakeWorldPeer>> peers;
            std::vector<std::thread> threads;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_stopping = true;
                peers = m_peers;
                threads = std::move(m_threads);
            }
            for (auto& peer : peers)
                peer->Close();
            for (auto& thread : threads)
                thread.join();
        }

        // Scripts started / scripts that have returned so far.
        int Accepted() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_accepted;
        }
        int Finished() const { return m_finished.load(); }

        // Polls until `count` scripts have returned; false on timeout.
        bool WaitForFinished(int count, std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) const
        {
            auto const deadline = std::chrono::steady_clock::now() + timeout;
            while (m_finished.load() < count)
            {
                if (std::chrono::steady_clock::now() >= deadline)
                    return false;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            return true;
        }

    private:
        IO::IoContext* m_ctx;
        Script m_script;
        std::unique_ptr<IO::Networking::AsyncSocketAcceptor> m_acceptor;

        mutable std::mutex m_mutex;
        bool m_stopping = false;
        int m_accepted = 0;
        std::atomic<int> m_finished{ 0 };
        std::vector<std::shared_ptr<FakeWorldPeer>> m_peers;
        std::vector<std::thread> m_threads;
};

// What a fake Realm connection observed, filled in by FakeWorldRunRealmHandshake.
struct FakeWorldRealmOptions
{
    std::vector<uint8> JoinSessionKey;
    std::string ExpectedTicket;
    uint32 AuthResult = 0; // BattlenetRpcErrorCode sent in SMSG_AUTH_RESPONSE
    bool TamperSignature = false;
    // Stops after SMSG_ENTER_ENCRYPTED_MODE, never reading the client's ack -- for "the client
    // must not proceed" checks.
    bool StopAfterEnterEncryptedMode = false;
};

struct FakeWorldRealmResult
{
    bool Completed = false; // every step up to and including SMSG_AUTH_RESPONSE was played
    bool DigestMatched = false;
    bool TicketMatched = false;
    std::array<uint8, 40> SessionKey{};
    std::string Error;
};

namespace FakeWorldDetail
{
inline std::array<uint8, 32> RandomChallenge()
{
    BigNumber bn;
    bn.SetRand(32 * 8);
    std::vector<uint8> bytes = bn.AsByteArray(32, false);
    std::array<uint8, 32> out{};
    std::copy(bytes.begin(), bytes.end(), out.begin());
    return out;
}

inline std::array<uint8, 64> SignEnterEncryptedMode(std::array<uint8, 32> const& encryptKey, bool tamper)
{
    auto toSign = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(encryptKey, true);

    Ed25519 ed25519;
    MANGOS_ASSERT(ed25519.LoadFromByteArray(WorldHandshakeCrypto::EnterEncryptedModePrivateKey()));
    std::vector<uint8> context(WorldHandshakeCrypto::EnableEncryptionContext().begin(), WorldHandshakeCrypto::EnableEncryptionContext().end());
    std::vector<uint8> signature = ed25519.SignWithContext(toSign, context);
    MANGOS_ASSERT(signature.size() == 64);
    if (tamper)
        signature[0] ^= 0xFF;

    std::array<uint8, 64> out{};
    std::copy(signature.begin(), signature.end(), out.begin());
    return out;
}

inline AES_GCM::Key ToKey(std::array<uint8, 32> const& encryptKey)
{
    AES_GCM::Key key;
    std::copy(encryptKey.begin(), encryptKey.end(), key.begin());
    return key;
}
}

// Plays a Realm connection from the V2 banner through SMSG_AUTH_RESPONSE, checking the client's
// digest and ticket on the way. The connection is left encrypted (Crypt() armed) and open, so the
// caller's script continues with CharSelect on it.
inline FakeWorldRealmResult FakeWorldRunRealmHandshake(FakeWorldPeer& peer, FakeWorldRealmOptions const& options)
{
    FakeWorldRealmResult result;
    auto fail = [&]() { result.Error = peer.Error(); return result; };

    if (!peer.ExchangeBanners())
        return fail();

    ClassicAuthChallenge challenge;
    challenge.ServerChallenge = FakeWorldDetail::RandomChallenge();
    if (!peer.Send(challenge))
        return fail();

    ClassicAuthSession session;
    if (!peer.Expect(session))
        return fail();

    result.TicketMatched = (session.RealmJoinTicket == options.ExpectedTicket);
    result.DigestMatched = (session.Digest == WorldHandshakeCrypto::ComputeAuthCheckDigest(options.JoinSessionKey, session.LocalChallenge, challenge.ServerChallenge));

    result.SessionKey = WorldHandshakeCrypto::ComputeSessionKey(options.JoinSessionKey, session.LocalChallenge, challenge.ServerChallenge);
    auto encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(result.SessionKey, session.LocalChallenge, challenge.ServerChallenge);

    ClassicEnterEncryptedMode enter;
    enter.Signature = FakeWorldDetail::SignEnterEncryptedMode(encryptKey, options.TamperSignature);
    enter.Enabled = true;
    if (!peer.Send(enter))
        return fail();
    if (options.StopAfterEnterEncryptedMode)
        return result;

    ClassicEnterEncryptedModeAck ack;
    if (!peer.Expect(ack))
        return fail();
    peer.Crypt().Init(FakeWorldDetail::ToKey(encryptKey));

    ClassicAuthResponse response;
    response.Result = options.AuthResult;
    if (!peer.Send(response))
        return fail();

    result.Completed = true;
    return result;
}

struct FakeWorldInstanceOptions
{
    std::array<uint8, 40> SessionKey{}; // the Realm connection's session key
    uint64 ExpectedPackedKey = 0;       // the ClassicConnectTo::Key the redirect carried
    bool TamperSignature = false;
    // Stops after the client's ack, sending no SMSG_RESUME_COMMS.
    bool SkipResumeComms = false;
};

struct FakeWorldInstanceResult
{
    bool Completed = false;
    bool DigestMatched = false;
    bool KeyMatched = false;
    std::string Error;
};

// Plays an Instance connection: banner, SMSG_AUTH_CHALLENGE, CMSG_AUTH_CONTINUED_SESSION (digest and
// packed key checked), SMSG_ENTER_ENCRYPTED_MODE, the ack, then SMSG_RESUME_COMMS. Left encrypted
// and open for the caller to continue on (SMSG_LOGIN_VERIFY_WORLD and beyond).
inline FakeWorldInstanceResult FakeWorldRunInstanceHandshake(FakeWorldPeer& peer, FakeWorldInstanceOptions const& options)
{
    FakeWorldInstanceResult result;
    auto fail = [&]() { result.Error = peer.Error(); return result; };

    if (!peer.ExchangeBanners())
        return fail();

    ClassicAuthChallenge challenge;
    challenge.ServerChallenge = FakeWorldDetail::RandomChallenge();
    if (!peer.Send(challenge))
        return fail();

    ClassicAuthContinuedSession session;
    if (!peer.Expect(session))
        return fail();

    result.KeyMatched = (session.Key == options.ExpectedPackedKey);
    result.DigestMatched = (session.Digest == WorldHandshakeCrypto::ComputeContinuedSessionDigest(options.SessionKey, session.Key, session.LocalChallenge, challenge.ServerChallenge));

    auto encryptKey = WorldHandshakeCrypto::ComputeEncryptKey(options.SessionKey, session.LocalChallenge, challenge.ServerChallenge);

    ClassicEnterEncryptedMode enter;
    enter.Signature = FakeWorldDetail::SignEnterEncryptedMode(encryptKey, options.TamperSignature);
    enter.Enabled = true;
    if (!peer.Send(enter))
        return fail();

    ClassicEnterEncryptedModeAck ack;
    if (!peer.Expect(ack))
        return fail();
    peer.Crypt().Init(FakeWorldDetail::ToKey(encryptKey));

    if (!options.SkipResumeComms && !peer.Send(ClassicResumeComms{}))
        return fail();

    result.Completed = true;
    return result;
}

#endif
