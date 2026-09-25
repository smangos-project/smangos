#ifndef REALBOTS_BOTWORLDCONNECTION_H
#define REALBOTS_BOTWORLDCONNECTION_H

#include "BotPacket.h"
#include "ByteBuffer.h"
#include "ClassicPacketFraming.h"
#include "Crypto/Encryption/PacketCrypt.h"
#include "IO/Networking/AsyncSocket.h"
#include "Packets/Classic/Auth.h"
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

// The client (bot) side of the modern world socket: the V2 banner handshake, packet header
// framing, PacketCrypt, and incoming decompression -- the other end of the wire a server's
// WorldSocket speaks (see ClassicPacketFraming for the shared wire shape).
//
// Owns no bot/session state: every decoded packet is handed to onPacket as an opaque BotPacket;
// what a caller does with it (feed a coroutine await, dispatch to task hooks) is not this
// class's concern (bot-plan.md, "Bots touch no server runtime state").
class BotWorldConnection : public std::enable_shared_from_this<BotWorldConnection>
{
    public:
        using PacketCallback = std::function<void(BotPacket)>;
        using DisconnectCallback = std::function<void()>;

        static std::shared_ptr<BotWorldConnection> Create(IO::Networking::AsyncSocket socket,
            PacketCallback onPacket, DisconnectCallback onDisconnect);

        BotWorldConnection(BotWorldConnection const&) = delete;
        BotWorldConnection& operator=(BotWorldConnection const&) = delete;

        // Sends the client's V2 banner, waits for the server's, then starts the packet loop.
        void StartHandshake();

        template<typename T>
        void Send(T const& packet)
        {
            ByteBuffer body;
            packet.Write(body);
            SendRaw(T::Opcode, body);
        }

        // armEncryption: if set, PacketCrypt is armed with this key right after this packet is
        // encoded (so it goes out in the old, unarmed state) and before it is queued to the socket
        // -- the CMSG_ENTER_ENCRYPTED_MODE_ACK case. Arming from another thread after a plain
        // SendRaw would race the server's already-encrypted reply arriving on the IO thread.
        void SendRaw(uint32 opcode, ByteBuffer const& body, AES_GCM::Key const* armEncryption = nullptr);

        // Retargets who gets the next incoming packet/disconnect -- how a connection is handed
        // from one login stage to the next (e.g. BotWorldHandshake -> BotCharSelect) before
        // there's a BotBrain task stack to do this instead (bot-phase-1.md's future model).
        // Thread-safe against a packet/disconnect arriving concurrently on the IO thread: the
        // callback in effect at the moment a packet completes is the one that runs for it, never
        // a half-updated one.
        void SetPacketCallback(PacketCallback onPacket);
        void SetDisconnectCallback(DisconnectCallback onDisconnect);

        // Arms PacketCrypt with the session's derived AES-256 key (see WorldHandshakeCrypto).
        // Called once, right after sending ClassicEnterEncryptedModeAck -- every packet from
        // either side after that point is encrypted (bot-phase-1.md, "WorldHandshake").
        void ArmEncryption(AES_GCM::Key const& key);

        void Close();
        bool IsClosing() const { return m_socket.IsClosing(); }

    private:
        explicit BotWorldConnection(IO::Networking::AsyncSocket socket, PacketCallback onPacket, DisconnectCallback onDisconnect);

        void ReadServerBanner();
        void SendClientBanner();
        void ReadNextHeader();
        void ReadBody(uint32 bodySize, std::array<uint8, 12> tag);
        void HandleCompletePacket(std::vector<uint8> plaintext);
        void DispatchPacket(BotPacket packet);
        void OnIoError(IO::NetworkError const& error);

        void FlushSendQueue();

        static std::string const ServerBanner;
        static std::string const ClientBanner;

        IO::Networking::AsyncSocket m_socket;
        PacketCrypt m_crypt;
        ClassicPacketFraming m_framing;

        std::mutex m_callbackLock;
        PacketCallback m_onPacket;
        DisconnectCallback m_onDisconnect;

        std::mutex m_sendQueueLock;
        std::deque<std::vector<uint8>> m_sendQueue;
        bool m_sendQueueRunning = false;

        bool m_closed = false;
};

#endif
