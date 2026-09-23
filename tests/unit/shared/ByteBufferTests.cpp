#include "ByteBuffer.h"
#include <catch_amalgamated.hpp>

TEST_CASE("ByteBuffer: WriteBit/ReadBit round-trips a bit sequence", "[bytebuffer]")
{
    ByteBuffer buf;
    buf.WriteBit(true);
    buf.WriteBit(false);
    buf.WriteBit(true);
    buf.WriteBit(true);
    buf.FlushBits();
    buf << uint32(0xDEADBEEF); // byte-aligned write after bits, must not corrupt them

    CHECK(buf.ReadBit() == true);
    CHECK(buf.ReadBit() == false);
    CHECK(buf.ReadBit() == true);
    CHECK(buf.ReadBit() == true);
    buf.ResetBitPos();
    CHECK(buf.read<uint32>() == 0xDEADBEEF);
}

TEST_CASE("ByteBuffer: WriteBits/ReadBits round-trips a multi-bit value", "[bytebuffer]")
{
    ByteBuffer buf;
    buf.WriteBits(0x2A, 7); // 42, fits in 7 bits
    buf.FlushBits();

    uint32 value = buf.ReadBits(7);
    CHECK(value == 0x2A);
}

TEST_CASE("ByteBuffer: WriteSizedString/ReadString round-trips a string", "[bytebuffer]")
{
    ByteBuffer buf;
    std::string original = "hello bot";
    buf.WriteSizedString(original, 7);

    uint32 length = buf.ReadBits(7);
    buf.ResetBitPos();
    std::string read = buf.ReadString(length);

    CHECK(read == original);
}

TEST_CASE("ByteBuffer: a byte-aligned write auto-flushes pending bits", "[bytebuffer]")
{
    ByteBuffer buf;
    buf.WriteBit(true); // only 1 of 8 bits written; append() below must flush it first
    buf << uint8(0x7F);

    CHECK(buf.size() == 2);
    CHECK(buf.ReadBit() == true);
}
