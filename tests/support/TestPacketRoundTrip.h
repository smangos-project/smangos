#ifndef MANGOS_TEST_PACKET_ROUND_TRIP_H
#define MANGOS_TEST_PACKET_ROUND_TRIP_H

// Round-trip checks shared by the wire-struct unit tests.

#include "ByteBuffer.h"
#include <array>
#include <catch_amalgamated.hpp>

// Writes `out`, reads the result back into a fresh T, and hands both to `check`; also checks that
// Read consumed exactly what Write produced.
template<typename T, typename Check>
void RoundTrip(T const& out, Check check)
{
    ByteBuffer data;
    out.Write(data);

    T in;
    REQUIRE(in.Read(data));
    CHECK(data.rpos() == data.size());
    check(out, in);
}

// Every strict prefix of a valid body must be rejected, never crash and never read past the end.
template<typename T>
void RejectsEveryTruncation(T const& valid)
{
    ByteBuffer full;
    valid.Write(full);

    for (size_t length = 0; length < full.size(); ++length)
    {
        ByteBuffer truncated;
        if (length)
            truncated.append(full.contents(), length);

        T in;
        INFO("truncated to " << length << " of " << full.size() << " bytes");
        CHECK_FALSE(in.Read(truncated));
    }
}

template<size_t N>
std::array<uint8, N> Pattern(uint8 seed)
{
    std::array<uint8, N> out{};
    for (size_t i = 0; i < N; ++i)
        out[i] = uint8(seed + i * 7);
    return out;
}

#endif
