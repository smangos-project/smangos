#include "BotWorldConnection.h"
#include "ClassicOpcodes.h"
#include "Errors.h"
#include "Log.h"
#include <zlib.h>
#include <algorithm>
#include <cstring>

std::string const BotWorldConnection::ServerBanner = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2";
std::string const BotWorldConnection::ClientBanner = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2";

namespace
{
// Identical shape both directions -- see vibemangos's WorldSocket.h PacketHeader.
#pragma pack(push, 1)
struct PacketHeader
{
    uint32 size;
    uint8 tag[12];

    bool IsValidSize() const { return size < 0x40000; }
};

// Prefixes the deflate payload inside CLASSIC_SMSG_COMPRESSED_PACKET.
struct CompressedWorldPacket
{
    uint32 UncompressedSize;
    uint32 UncompressedAdler;
    uint32 CompressedAdler;
};
#pragma pack(pop)
}

std::shared_ptr<BotWorldConnection> BotWorldConnection::Create(IO::Networking::AsyncSocket socket,
    PacketCallback onPacket, DisconnectCallback onDisconnect)
{
    auto self = std::shared_ptr<BotWorldConnection>(new BotWorldConnection(std::move(socket), std::move(onPacket), std::move(onDisconnect)));
    IO::NetworkError err = self->m_socket.InitializeAndFixateMemoryLocation();
    MANGOS_ASSERT(!err);
    return self;
}

BotWorldConnection::BotWorldConnection(IO::Networking::AsyncSocket socket, PacketCallback onPacket, DisconnectCallback onDisconnect)
    : m_socket(std::move(socket)), m_crypt(PacketCryptRole::Client), m_onPacket(std::move(onPacket)), m_onDisconnect(std::move(onDisconnect))
{
}

BotWorldConnection::~BotWorldConnection()
{
    if (m_decompressionStream)
    {
        inflateEnd(m_decompressionStream);
        delete m_decompressionStream;
    }
}

void BotWorldConnection::StartHandshake()
{
    ReadServerBanner();
}

void BotWorldConnection::ReadServerBanner()
{
    size_t const expectedSize = ServerBanner.length() + 1;
    auto buffer = std::make_shared<std::vector<char>>(expectedSize);

    m_socket.Read(buffer->data(), buffer->size(), [self = shared_from_this(), buffer, expectedSize](IO::NetworkError const& error, size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error);
            return;
        }

        if (std::memcmp(buffer->data(), ServerBanner.data(), ServerBanner.length()) != 0 || (*buffer)[expectedSize - 1] != '\n')
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotWorldConnection: bad server banner from %s", self->m_socket.GetRemoteIpString().c_str());
            self->Close();
            return;
        }

        self->SendClientBanner();
    });
}

void BotWorldConnection::SendClientBanner()
{
    auto bannerBuffer = std::make_shared<ByteBuffer>();
    bannerBuffer->append(ClientBanner.c_str(), ClientBanner.length());
    bannerBuffer->append("\n", 1);

    m_socket.Write({ std::shared_ptr<ByteBuffer const>(bannerBuffer) }, [self = shared_from_this()](IO::NetworkError const& error)
    {
        if (error)
        {
            self->OnIoError(error);
            return;
        }

        // One stateful raw-inflate stream for the connection's lifetime, mirroring the server's
        // deflate side (windowBits=-15 for raw deflate, no zlib header) -- see WorldSocket.cpp
        // ReadClientBanner(). The bot never sends compressed packets of its own (its own traffic
        // never approaches MinSizeForCompression), so only the decompression half is needed.
        self->m_decompressionStream = new z_stream_s();
        self->m_decompressionStream->zalloc = nullptr;
        self->m_decompressionStream->zfree = nullptr;
        self->m_decompressionStream->opaque = nullptr;
        self->m_decompressionStream->avail_in = 0;
        self->m_decompressionStream->next_in = nullptr;
        if (inflateInit2(self->m_decompressionStream, -15) != Z_OK)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: can't initialize packet decompression (zlib: inflateInit2)");
            self->Close();
            return;
        }

        self->ReadNextHeader();
    });
}

void BotWorldConnection::ReadNextHeader()
{
    auto header = std::make_shared<PacketHeader>();

    m_socket.Read(reinterpret_cast<char*>(header.get()), sizeof(PacketHeader), [self = shared_from_this(), header](IO::NetworkError const& error, size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error);
            return;
        }

        if (!header->IsValidSize())
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: server sent malformed packet size = %u", header->size);
            self->Close();
            return;
        }

        std::array<uint8, 12> tag;
        std::memcpy(tag.data(), header->tag, tag.size());
        self->ReadBody(header->size, tag);
    });
}

void BotWorldConnection::ReadBody(uint32 bodySize, std::array<uint8, 12> tag)
{
    auto payload = std::make_shared<std::vector<uint8>>(bodySize);

    m_socket.Read(reinterpret_cast<char*>(payload->data()), payload->size(), [self = shared_from_this(), payload, tag](IO::NetworkError const& error, size_t) -> void
    {
        if (error)
        {
            self->OnIoError(error);
            return;
        }

        AES_GCM::Tag rawTag;
        std::memcpy(rawTag, tag.data(), sizeof(rawTag));
        if (!self->m_crypt.DecryptRecv(payload->data(), payload->size(), rawTag))
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: failed to decrypt/verify packet (size %zu)", payload->size());
            self->Close();
            return;
        }

        self->HandleCompletePacket(std::move(*payload));
        if (!self->IsClosing())
            self->ReadNextHeader();
    });
}

void BotWorldConnection::HandleCompletePacket(std::vector<uint8> plaintext)
{
    if (plaintext.size() < sizeof(uint32))
    {
        sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: packet too small to contain an opcode (size = %zu)", plaintext.size());
        Close();
        return;
    }

    uint32 opcode = uint32(plaintext[0]) | (uint32(plaintext[1]) << 8) | (uint32(plaintext[2]) << 16) | (uint32(plaintext[3]) << 24);
    uint8 const* bodyStart = plaintext.data() + sizeof(uint32);
    size_t bodySize = plaintext.size() - sizeof(uint32);

    if (opcode == CLASSIC_SMSG_COMPRESSED_PACKET)
    {
        if (bodySize < sizeof(CompressedWorldPacket))
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: compressed packet too small for its header");
            Close();
            return;
        }

        CompressedWorldPacket cmp;
        std::memcpy(&cmp, bodyStart, sizeof(cmp));

        std::vector<uint8> decompressed(cmp.UncompressedSize);
        m_decompressionStream->next_in = const_cast<Bytef*>(bodyStart + sizeof(cmp));
        m_decompressionStream->avail_in = uInt(bodySize - sizeof(cmp));
        m_decompressionStream->next_out = decompressed.data();
        m_decompressionStream->avail_out = uInt(decompressed.size());

        int z_res = inflate(m_decompressionStream, Z_SYNC_FLUSH);
        if (z_res != Z_OK && z_res != Z_STREAM_END)
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: can't decompress packet (zlib: inflate) error %d", z_res);
            Close();
            return;
        }

        if (decompressed.size() < sizeof(uint32))
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotWorldConnection: decompressed packet too small to contain an opcode");
            Close();
            return;
        }

        uint32 innerOpcode = uint32(decompressed[0]) | (uint32(decompressed[1]) << 8) | (uint32(decompressed[2]) << 16) | (uint32(decompressed[3]) << 24);

        BotPacket packet;
        packet.Opcode = innerOpcode;
        packet.Data.append(decompressed.data() + sizeof(uint32), decompressed.size() - sizeof(uint32));
        packet.Data.rpos(0);
        DispatchPacket(std::move(packet));
        return;
    }

    BotPacket packet;
    packet.Opcode = opcode;
    if (bodySize)
        packet.Data.append(bodyStart, bodySize);
    packet.Data.rpos(0);
    DispatchPacket(std::move(packet));
}

void BotWorldConnection::DispatchPacket(BotPacket packet)
{
    PacketCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackLock);
        callback = m_onPacket;
    }
    if (callback)
        callback(std::move(packet));
}

void BotWorldConnection::SetPacketCallback(PacketCallback onPacket)
{
    std::lock_guard<std::mutex> lock(m_callbackLock);
    m_onPacket = std::move(onPacket);
}

void BotWorldConnection::SetDisconnectCallback(DisconnectCallback onDisconnect)
{
    std::lock_guard<std::mutex> lock(m_callbackLock);
    m_onDisconnect = std::move(onDisconnect);
}

void BotWorldConnection::SendRaw(uint32 opcode, ByteBuffer const& body, AES_GCM::Key const* armEncryption)
{
    if (IsClosing())
        return;

    std::vector<uint8> plaintext(sizeof(uint32) + body.size());
    plaintext[0] = uint8(opcode & 0xFF);
    plaintext[1] = uint8((opcode >> 8) & 0xFF);
    plaintext[2] = uint8((opcode >> 16) & 0xFF);
    plaintext[3] = uint8((opcode >> 24) & 0xFF);
    if (!body.empty())
        std::memcpy(plaintext.data() + sizeof(uint32), body.contents(), body.size());

    PacketHeader header{};
    header.size = uint32(plaintext.size());

    std::vector<uint8> framed(sizeof(header) + plaintext.size());

    // Encrypted here, synchronously under m_sendQueueLock -- not deferred to flush time, which
    // used to race ArmEncryption(): a caller doing Send(ack) then ArmEncryption(key) (exactly
    // WorldHandshake's sequencing) could see Init() run before the still-queued Ack was ever
    // turned into bytes, encrypting a packet the server expects unencrypted. Locking Init() the
    // same way (below) makes the two calls atomic relative to each other: the Ack is fully
    // encoded under the old (possibly unarmed) state before Init() can take effect.
    std::lock_guard<std::mutex> lock(m_sendQueueLock);

    AES_GCM::Tag tag;
    m_crypt.EncryptSend(plaintext.data(), plaintext.size(), tag);
    std::memcpy(header.tag, tag, sizeof(header.tag));

    std::memcpy(framed.data(), &header, sizeof(header));
    if (!plaintext.empty())
        std::memcpy(framed.data() + sizeof(header), plaintext.data(), plaintext.size());

    // Still under the send lock and before the packet reaches the socket, so the server cannot
    // have answered it yet: nothing is decrypting concurrently with this Init().
    if (armEncryption)
        m_crypt.Init(*armEncryption);

    m_sendQueue.push_back(std::move(framed));
    if (!m_sendQueueRunning)
    {
        m_sendQueueRunning = true;
        m_socket.EnterIoContext([self = shared_from_this()](IO::NetworkError)
        {
            self->FlushSendQueue();
        });
    }
}

void BotWorldConnection::FlushSendQueue()
{
    std::vector<uint8> framed;
    {
        std::lock_guard<std::mutex> lock(m_sendQueueLock);
        if (m_sendQueue.empty())
        {
            m_sendQueueRunning = false;
            return;
        }
        framed = std::move(m_sendQueue.front());
        m_sendQueue.pop_front();
    }

    auto framedBuf = std::make_shared<std::vector<uint8>>(std::move(framed));
    m_socket.Write({ std::shared_ptr<std::vector<uint8> const>(framedBuf) }, [self = shared_from_this()](IO::NetworkError const& error)
    {
        if (error)
        {
            self->OnIoError(error);
            std::lock_guard<std::mutex> lock(self->m_sendQueueLock);
            self->m_sendQueueRunning = false;
            return;
        }

        self->FlushSendQueue();
    });
}

void BotWorldConnection::ArmEncryption(AES_GCM::Key const& key)
{
    std::lock_guard<std::mutex> lock(m_sendQueueLock);
    m_crypt.Init(key);
}

void BotWorldConnection::OnIoError(IO::NetworkError const& error)
{
    if (error.GetErrorType() != IO::NetworkError::ErrorType::SocketClosed || !IsClosing())
        sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotWorldConnection: IoError: %s", error.ToString().c_str());
    Close();
}

void BotWorldConnection::Close()
{
    // Close() can be called from any thread (a caller closing explicitly, or internally from
    // OnIoError on the IO thread). m_closed must gate under a lock -- checked and set together --
    // or two racing calls can both pass the check before either sets it, double-closing the
    // socket and double-dispatching m_onDisconnect.
    {
        std::lock_guard<std::mutex> lock(m_callbackLock);
        if (m_closed)
            return;
        m_closed = true;
    }

    m_socket.CloseSocket();

    DisconnectCallback callback;
    {
        std::lock_guard<std::mutex> lock(m_callbackLock);
        callback = m_onDisconnect;
    }
    if (callback)
        callback();
}
