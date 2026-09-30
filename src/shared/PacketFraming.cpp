#include "PacketFraming.h"
#include <zlib.h>
#include <cstring>

namespace
{
    constexpr uLong CompressedPacketAdlerSeed = 0x9827D8F1;
}

std::vector<uint8> PacketFraming::BuildPlaintext(uint32 opcode, uint8 const* body, size_t bodySize)
{
    std::vector<uint8> plaintext(sizeof(uint32) + bodySize);
    plaintext[0] = uint8(opcode & 0xFF);
    plaintext[1] = uint8((opcode >> 8) & 0xFF);
    plaintext[2] = uint8((opcode >> 16) & 0xFF);
    plaintext[3] = uint8((opcode >> 24) & 0xFF);
    if (bodySize)
        std::memcpy(plaintext.data() + sizeof(uint32), body, bodySize);
    return plaintext;
}

bool PacketFraming::ParsePlaintext(std::vector<uint8> const& plaintext, uint32& opcode, uint8 const*& bodyStart, size_t& bodySize)
{
    if (plaintext.size() < sizeof(uint32))
        return false;

    opcode = uint32(plaintext[0]) | (uint32(plaintext[1]) << 8) | (uint32(plaintext[2]) << 16) | (uint32(plaintext[3]) << 24);
    bodyStart = plaintext.data() + sizeof(uint32);
    bodySize = plaintext.size() - sizeof(uint32);
    return true;
}

CompressedPacketHeader PacketFraming::BuildCompressedHeader(uint8 const* uncompressed, size_t uncompressedSize,
    uint8 const* compressed, size_t compressedSize)
{
    CompressedPacketHeader header{};
    header.UncompressedSize = uint32(uncompressedSize);
    // The client checks both against this seed, not zlib's default of 1 (TrinityCore's WorldSocket does the same).
    header.UncompressedAdler = uint32(adler32(CompressedPacketAdlerSeed, uncompressed, uInt(uncompressedSize)));
    header.CompressedAdler = uint32(adler32(CompressedPacketAdlerSeed, compressed, uInt(compressedSize)));
    return header;
}

PacketFraming::PacketFraming()
{
    // One stateful raw-inflate stream for the connection's lifetime (windowBits=-15 for raw
    // deflate, no zlib header) -- mirrors the other side's deflate stream below.
    m_inflateStream = new z_stream_s();
    m_inflateStream->zalloc = nullptr;
    m_inflateStream->zfree = nullptr;
    m_inflateStream->opaque = nullptr;
    m_inflateStream->avail_in = 0;
    m_inflateStream->next_in = nullptr;
    if (inflateInit2(m_inflateStream, -15) != Z_OK)
    {
        delete m_inflateStream;
        m_inflateStream = nullptr;
    }

    m_deflateStream = new z_stream_s();
    m_deflateStream->zalloc = nullptr;
    m_deflateStream->zfree = nullptr;
    m_deflateStream->opaque = nullptr;
    if (deflateInit2(m_deflateStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        delete m_deflateStream;
        m_deflateStream = nullptr;
    }
}

PacketFraming::~PacketFraming()
{
    if (m_inflateStream)
    {
        inflateEnd(m_inflateStream);
        delete m_inflateStream;
    }
    if (m_deflateStream)
    {
        deflateEnd(m_deflateStream);
        delete m_deflateStream;
    }
}

bool PacketFraming::Inflate(uint8 const* compressed, size_t compressedSize, uint32 uncompressedSize, std::vector<uint8>& out)
{
    if (!m_inflateStream)
        return false;

    out.resize(uncompressedSize);
    m_inflateStream->next_in = const_cast<Bytef*>(compressed);
    m_inflateStream->avail_in = uInt(compressedSize);
    m_inflateStream->next_out = out.data();
    m_inflateStream->avail_out = uInt(out.size());

    int result = inflate(m_inflateStream, Z_SYNC_FLUSH);
    return result == Z_OK || result == Z_STREAM_END;
}

bool PacketFraming::Deflate(uint8 const* plaintext, size_t plaintextSize, std::vector<uint8>& out)
{
    if (!m_deflateStream)
        return false;

    out.resize(deflateBound(m_deflateStream, uLong(plaintextSize)));
    m_deflateStream->next_in = const_cast<Bytef*>(plaintext);
    m_deflateStream->avail_in = uInt(plaintextSize);
    m_deflateStream->next_out = out.data();
    m_deflateStream->avail_out = uInt(out.size());

    int result = deflate(m_deflateStream, Z_SYNC_FLUSH);
    if (result != Z_OK && result != Z_STREAM_END)
        return false;

    out.resize(out.size() - m_deflateStream->avail_out);
    return true;
}
