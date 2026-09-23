#ifndef REALBOTS_BOTWORLDCONNECTION_H
#define REALBOTS_BOTWORLDCONNECTION_H

#include "BotPacket.h"
#include "ByteBuffer.h"
#include "Crypto/Encryption/PacketCrypt.h"
#include "IO/Networking/AsyncSocket.h"
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

struct z_stream_s;

// The client (bot) side of the modern world socket: the V2 banner handshake, PacketHeader
// framing, PacketCrypt, and incoming decompression. Mirrors vibemangos's WorldSocket
// (src/game/Server/WorldSocket.cpp), from the other end of the same wire -- see that file for
// the parts of the protocol this is symmetric with (packet header shape, banner strings, the
// stateful per-connection deflate window).
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

        ~BotWorldConnection();
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

        void SendRaw(uint32 opcode, ByteBuffer const& body);

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
        void OnIoError(IO::NetworkError const& error);

        void FlushSendQueue();

        static std::string const ServerBanner;
        static std::string const ClientBanner;

        IO::Networking::AsyncSocket m_socket;
        PacketCrypt m_crypt;
        z_stream_s* m_decompressionStream = nullptr;

        PacketCallback m_onPacket;
        DisconnectCallback m_onDisconnect;

        std::mutex m_sendQueueLock;
        std::deque<std::vector<uint8>> m_sendQueue;
        bool m_sendQueueRunning = false;

        bool m_closed = false;
};

#endif
