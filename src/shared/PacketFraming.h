#ifndef SHARED_PACKETFRAMING_H
#define SHARED_PACKETFRAMING_H

#include "Common.h"
#include <vector>

struct z_stream_s;

// The largest packet body either direction's framing accepts (PacketHeader::Size is checked
// against this on read; the same bound applies to what we send, since the shape is symmetric).
constexpr uint32 MAX_CLASSIC_PACKET_SIZE = 0x40000;

// Identical shape both directions: [size:u32 LE][tag:12 bytes], prefixing PacketCrypt's
// (Crypto/Encryption/PacketCrypt.h) ciphertext on every world-socket frame.
#pragma pack(push, 1)
struct PacketHeader
{
    uint32 Size;
    uint8 Tag[12];

    bool IsValidSize() const { return Size < MAX_CLASSIC_PACKET_SIZE; }
};

// Prefixes CLASSIC_SMSG_COMPRESSED_PACKET's deflate payload: adler32 checksums of the plaintext
// and the compressed bytes, and the plaintext's own size (inflate needs it up front to size the
// output buffer; a raw deflate stream carries no end marker of its own).
struct CompressedPacketHeader
{
    uint32 UncompressedSize;
    uint32 UncompressedAdler;
    uint32 CompressedAdler;
};
#pragma pack(pop)

// The modern world-socket wire format around PacketCrypt: plaintext framing (an opcode prefixing
// every encrypted packet body) and CLASSIC_SMSG_COMPRESSED_PACKET's own raw-deflate envelope
// around a whole inner packet. The wire-shape pieces are static and pure; the deflate/inflate
// windows are stateful for a connection's whole lifetime (not per-packet -- every compressed
// packet in a direction shares one sliding history, so both ends must process every packet
// through the same stream in order), so one instance of this class owns them, mirroring
// PacketCrypt's own per-connection state.
class PacketFraming
{
    public:
        // [opcode:u32 LE][body], the plaintext PacketCrypt::EncryptSend/DecryptRecv encrypts (or
        // decrypts) as one unit.
        static std::vector<uint8> BuildPlaintext(uint32 opcode, uint8 const* body, size_t bodySize);

        // Splits a decrypted plaintext into its opcode and body. False if too small to hold an
        // opcode; `bodyStart`/`bodySize` then point back into `plaintext`, so `plaintext` must
        // outlive them.
        static bool ParsePlaintext(std::vector<uint8> const& plaintext, uint32& opcode, uint8 const*& bodyStart, size_t& bodySize);

        // Computes both adler32 checksums for a CLASSIC_SMSG_COMPRESSED_PACKET header around
        // already-compressed bytes.
        static CompressedPacketHeader BuildCompressedHeader(uint8 const* uncompressed, size_t uncompressedSize,
            uint8 const* compressed, size_t compressedSize);

        // True when `header` matches the plaintext and compressed bytes it claims to describe: both
        // adler32 checksums (with the seed the client checks) and the plaintext size.
        static bool VerifyCompressedHeader(CompressedPacketHeader const& header, uint8 const* uncompressed, size_t uncompressedSize,
            uint8 const* compressed, size_t compressedSize);

        PacketFraming();
        ~PacketFraming();
        PacketFraming(PacketFraming const&) = delete;
        PacketFraming& operator=(PacketFraming const&) = delete;

        // Inflates `compressed` into exactly `uncompressedSize` bytes, continuing this instance's
        // ongoing raw-inflate window (windowBits=-15, no zlib header). False on a zlib error.
        bool Inflate(uint8 const* compressed, size_t compressedSize, uint32 uncompressedSize, std::vector<uint8>& out);

        // The mirror: deflates `plaintext`, continuing this instance's ongoing raw-deflate window.
        // False on a zlib error.
        bool Deflate(uint8 const* plaintext, size_t plaintextSize, std::vector<uint8>& out);

    private:
        z_stream_s* m_inflateStream = nullptr;
        z_stream_s* m_deflateStream = nullptr;
};

#endif
