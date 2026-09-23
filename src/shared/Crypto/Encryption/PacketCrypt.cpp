#include "PacketCrypt.h"
#include <array>
#include <cstring>

namespace
{
// Server->client magic: 0x52565253 ("SRVR" byte-swapped as a little-endian uint32).
// Client->server magic: 0x544E4C43 ("CLNT" byte-swapped likewise). Matches
// WorldSocket.cs/WorldPacketCrypt.cpp exactly -- these are fixed protocol constants, not
// derived from anything.
constexpr uint32 SERVER_TO_CLIENT_MAGIC = 0x52565253;
constexpr uint32 CLIENT_TO_SERVER_MAGIC = 0x544E4C43;

AES_GCM::IV MakeIV(uint64 counter, uint32 magic)
{
    AES_GCM::IV iv;
    memcpy(iv.data(), &counter, sizeof(uint64));
    memcpy(iv.data() + sizeof(uint64), &magic, sizeof(uint32));
    return iv;
}
}

PacketCrypt::PacketCrypt(PacketCryptRole role)
    : m_decrypt(false), m_encrypt(true), m_sendCounter(0), m_recvCounter(0), m_initialized(false)
{
    if (role == PacketCryptRole::Server)
    {
        m_sendMagic = SERVER_TO_CLIENT_MAGIC;
        m_recvMagic = CLIENT_TO_SERVER_MAGIC;
    }
    else
    {
        m_sendMagic = CLIENT_TO_SERVER_MAGIC;
        m_recvMagic = SERVER_TO_CLIENT_MAGIC;
    }
}

PacketCrypt::PacketCrypt(PacketCryptRole role, AES_GCM::Key const& key)
    : PacketCrypt(role)
{
    Init(key);
}

void PacketCrypt::Init(AES_GCM::Key const& key)
{
    m_decrypt.Init(key);
    m_encrypt.Init(key);
    m_initialized = true;
}

bool PacketCrypt::PeekDecryptRecv(uint8* data, size_t length)
{
    if (m_initialized)
    {
        AES_GCM::IV iv = MakeIV(m_recvCounter, m_recvMagic);
        if (!m_decrypt.ProcessNoIntegrityCheck(iv, data, length))
            return false;
    }

    return true;
}

bool PacketCrypt::DecryptRecv(uint8* data, size_t length, AES_GCM::Tag& tag)
{
    if (m_initialized)
    {
        AES_GCM::IV iv = MakeIV(m_recvCounter, m_recvMagic);
        if (!m_decrypt.Process(iv, data, length, tag))
            return false;
    }
    else
        memset(tag, 0, AES_GCM::TAG_SIZE_BYTES);

    ++m_recvCounter;
    return true;
}

bool PacketCrypt::EncryptSend(uint8* data, size_t length, AES_GCM::Tag& tag)
{
    if (m_initialized)
    {
        AES_GCM::IV iv = MakeIV(m_sendCounter, m_sendMagic);
        if (!m_encrypt.Process(iv, data, length, tag))
            return false;
    }
    else
        memset(tag, 0, AES_GCM::TAG_SIZE_BYTES);

    ++m_sendCounter;
    return true;
}
