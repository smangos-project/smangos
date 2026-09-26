// BuildMonsterMove's own path packing (packet_builder.cpp), pulled out as a pure function so it
// can be tested without a Unit or a live MoveSpline.
#include "Movement/spline/MonsterMovePacking.h"
#include <catch_amalgamated.hpp>
#include <cmath>

using namespace Movement;

namespace
{
// Undoes PackXYZ: an 11/11/10-bit two's-complement triple at quarter-yard precision.
Vector3 UnpackDelta(uint32 packed)
{
    auto signExtend = [](uint32 value, uint32 bits) -> int32
    {
        uint32 const signBit = 1u << (bits - 1);
        value &= (1u << bits) - 1;
        if (value & signBit)
            value |= ~((1u << bits) - 1);
        return int32(value);
    };

    int32 const x = signExtend(packed & 0x7FF, 11);
    int32 const y = signExtend((packed >> 11) & 0x7FF, 11);
    int32 const z = signExtend((packed >> 22) & 0x3FF, 10);
    return Vector3(x / 4.0f, y / 4.0f, z / 4.0f);
}
}

TEST_CASE("MonsterMovePacking: a packed delta round-trips within quarter-yard precision", "[movement]")
{
    Vector3 const start(0.0f, 0.0f, 0.0f);
    Vector3 const interior(10.0f, -5.0f, 2.0f);
    Vector3 const destination(100.0f, 0.0f, 0.0f);

    MonsterMove packet;
    PackMonsterMovePath(start, { interior, destination }, false, packet);

    REQUIRE(packet.PackedDeltas.size() == 1);

    Vector3 const midpoint = (start + destination) * 0.5f;
    Vector3 const expectedDelta = midpoint - interior;
    Vector3 const decoded = UnpackDelta(packet.PackedDeltas[0]);

    CHECK(std::abs(decoded.x - expectedDelta.x) <= 0.26f);
    CHECK(std::abs(decoded.y - expectedDelta.y) <= 0.26f);
    CHECK(std::abs(decoded.z - expectedDelta.z) <= 0.26f);
}

TEST_CASE("MonsterMovePacking: the destination is placed verbatim, the midpoint used for deltas", "[movement]")
{
    Vector3 const start(0.0f, 0.0f, 0.0f);
    Vector3 const destination(100.0f, 0.0f, 0.0f);
    // Exactly at the start/destination midpoint: its delta packs to zero.
    Vector3 const atMidpoint(50.0f, 0.0f, 0.0f);

    MonsterMove packet;
    PackMonsterMovePath(start, { atMidpoint, destination }, false, packet);

    REQUIRE(packet.Points.size() == 1);
    CHECK(packet.Points[0].X == destination.x);
    CHECK(packet.Points[0].Y == destination.y);
    CHECK(packet.Points[0].Z == destination.z);

    REQUIRE(packet.PackedDeltas.size() == 1);
    CHECK(packet.PackedDeltas[0] == 0);
}

TEST_CASE("MonsterMovePacking: a flying path is written uncompressed, node for node", "[movement]")
{
    Vector3 const start(0.0f, 0.0f, 0.0f);
    std::vector<Vector3> const path =
    {
        Vector3(1.0f, 2.0f, 3.0f),
        Vector3(4.0f, 5.0f, 6.0f),
        Vector3(7.0f, 8.0f, 9.0f),
    };

    MonsterMove packet;
    PackMonsterMovePath(start, path, true, packet);

    CHECK(packet.PackedDeltas.empty());
    REQUIRE(packet.Points.size() == path.size());
    for (size_t i = 0; i < path.size(); ++i)
    {
        CHECK(packet.Points[i].X == path[i].x);
        CHECK(packet.Points[i].Y == path[i].y);
        CHECK(packet.Points[i].Z == path[i].z);
    }
}
