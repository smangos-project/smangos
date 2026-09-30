#include "PacketFraming.h"

#include <catch_amalgamated.hpp>
#include <zlib.h>

TEST_CASE("PacketFraming round-trips a plaintext build/parse", "[classicpacketframing]")
{
    std::vector<uint8> const body = { 1, 2, 3, 4, 5 };
    std::vector<uint8> const plaintext = PacketFraming::BuildPlaintext(0x390016, body.data(), body.size());

    uint32 opcode = 0;
    uint8 const* bodyStart = nullptr;
    size_t bodySize = 0;
    REQUIRE(PacketFraming::ParsePlaintext(plaintext, opcode, bodyStart, bodySize));
    CHECK(opcode == 0x390016u);
    CHECK(bodySize == body.size());
    CHECK(std::vector<uint8>(bodyStart, bodyStart + bodySize) == body);
}

TEST_CASE("PacketFraming round-trips a plaintext with an empty body", "[classicpacketframing]")
{
    std::vector<uint8> const plaintext = PacketFraming::BuildPlaintext(0x3A0004, nullptr, 0);

    uint32 opcode = 0;
    uint8 const* bodyStart = nullptr;
    size_t bodySize = 0;
    REQUIRE(PacketFraming::ParsePlaintext(plaintext, opcode, bodyStart, bodySize));
    CHECK(opcode == 0x3A0004u);
    CHECK(bodySize == 0);
}

TEST_CASE("PacketFraming::ParsePlaintext rejects a buffer too small for an opcode", "[classicpacketframing]")
{
    std::vector<uint8> const tooSmall = { 1, 2, 3 };
    uint32 opcode = 0;
    uint8 const* bodyStart = nullptr;
    size_t bodySize = 0;
    CHECK_FALSE(PacketFraming::ParsePlaintext(tooSmall, opcode, bodyStart, bodySize));
}

TEST_CASE("PacketFraming::BuildCompressedHeader computes both adler32 checksums", "[classicpacketframing]")
{
    std::vector<uint8> const uncompressed = { 10, 20, 30, 40, 50 };
    std::vector<uint8> const compressed = { 9, 8, 7 };

    CompressedPacketHeader const header = PacketFraming::BuildCompressedHeader(
        uncompressed.data(), uncompressed.size(), compressed.data(), compressed.size());

    CHECK(header.UncompressedSize == uncompressed.size());
    CHECK(header.UncompressedAdler == uint32(adler32(0x9827D8F1, uncompressed.data(), uInt(uncompressed.size()))));
    CHECK(header.CompressedAdler == uint32(adler32(0x9827D8F1, compressed.data(), uInt(compressed.size()))));
}

TEST_CASE("PacketFraming::VerifyCompressedHeader accepts the client's seed and rejects zlib's default", "[classicpacketframing]")
{
    std::vector<uint8> const uncompressed = { 10, 20, 30, 40, 50 };
    std::vector<uint8> const compressed = { 9, 8, 7 };

    CompressedPacketHeader header = PacketFraming::BuildCompressedHeader(
        uncompressed.data(), uncompressed.size(), compressed.data(), compressed.size());
    CHECK(PacketFraming::VerifyCompressedHeader(header, uncompressed.data(), uncompressed.size(), compressed.data(), compressed.size()));

    // What the server sent before it was fixed: checksums seeded with zlib's default.
    CompressedPacketHeader wrongSeed = header;
    wrongSeed.UncompressedAdler = uint32(adler32(adler32(0L, nullptr, 0), uncompressed.data(), uInt(uncompressed.size())));
    wrongSeed.CompressedAdler = uint32(adler32(adler32(0L, nullptr, 0), compressed.data(), uInt(compressed.size())));
    CHECK_FALSE(PacketFraming::VerifyCompressedHeader(wrongSeed, uncompressed.data(), uncompressed.size(), compressed.data(), compressed.size()));

    CompressedPacketHeader wrongSize = header;
    wrongSize.UncompressedSize += 1;
    CHECK_FALSE(PacketFraming::VerifyCompressedHeader(wrongSize, uncompressed.data(), uncompressed.size(), compressed.data(), compressed.size()));
}

// The half-against-half proof for the stateful raw-deflate/-inflate windows: one
// PacketFraming instance deflating, a separate one inflating, across several packets in a
// row -- proves both the raw (windowBits=-15, no zlib header) framing and that each instance's
// sliding window stays in lockstep call to call, not just for a single message.
TEST_CASE("PacketFraming: a deflate instance and a separate inflate instance stay in lockstep across several packets", "[classicpacketframing]")
{
    PacketFraming compressor;
    PacketFraming decompressor;

    std::vector<std::string> const messages = {
        "the quick brown fox jumps over the lazy dog",
        "the quick brown fox jumps over the lazy dog again",
        "something completely different this time",
    };

    for (std::string const& message : messages)
    {
        std::vector<uint8> const plaintext(message.begin(), message.end());

        std::vector<uint8> compressed;
        REQUIRE(compressor.Deflate(plaintext.data(), plaintext.size(), compressed));

        std::vector<uint8> decompressed;
        REQUIRE(decompressor.Inflate(compressed.data(), compressed.size(), uint32(plaintext.size()), decompressed));
        CHECK(decompressed == plaintext);
    }
}
