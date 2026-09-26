// SMSG_UPDATE_OBJECT round trips. BlockData is opaque bytes here: the create/values/movement block
// layout belongs to Objects/Updates, not to this struct.
#include "Packets/ObjectUpdate.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("UpdateObject round-trips with no blocks and no guid lists", "[packets]")
{
    UpdateObject out;
    out.MapId = 0;
    out.BlockCount = 0;

    RoundTrip(out, [](UpdateObject const& a, UpdateObject const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.BlockCount == b.BlockCount);
        CHECK(b.DestroyGUIDs.empty());
        CHECK(b.OutOfRangeGUIDs.empty());
        CHECK(b.BlockData.empty());
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("UpdateObject round-trips block data", "[packets]")
{
    UpdateObject out;
    out.MapId = 1;
    out.BlockCount = 2;
    out.BlockData = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    RoundTrip(out, [](UpdateObject const& a, UpdateObject const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.BlockCount == b.BlockCount);
        CHECK(a.BlockData == b.BlockData);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("UpdateObject round-trips destroy and out-of-range guid lists", "[packets]")
{
    UpdateObject out;
    out.MapId = 0;
    out.BlockCount = 0;

    ClassicGuid destroyed;
    destroyed.Low = 0x42;
    destroyed.High = 0x0800000000000000ull;
    out.DestroyGUIDs.push_back(destroyed);

    ClassicGuid outOfRange1;
    outOfRange1.Low = 0x99;
    out.OutOfRangeGUIDs.push_back(outOfRange1);

    ClassicGuid outOfRange2;
    outOfRange2.Low = 0x17;
    outOfRange2.High = 0xF000000000000000ull;
    out.OutOfRangeGUIDs.push_back(outOfRange2);

    RoundTrip(out, [](UpdateObject const& a, UpdateObject const& b)
    {
        REQUIRE(a.DestroyGUIDs.size() == b.DestroyGUIDs.size());
        for (size_t i = 0; i < a.DestroyGUIDs.size(); ++i)
            CHECK(a.DestroyGUIDs[i] == b.DestroyGUIDs[i]);

        REQUIRE(a.OutOfRangeGUIDs.size() == b.OutOfRangeGUIDs.size());
        for (size_t i = 0; i < a.OutOfRangeGUIDs.size(); ++i)
            CHECK(a.OutOfRangeGUIDs[i] == b.OutOfRangeGUIDs[i]);
    });
    RejectsEveryTruncation(out);
}

TEST_CASE("UpdateObject round-trips guid lists together with block data", "[packets]")
{
    UpdateObject out;
    out.MapId = 530;
    out.BlockCount = 1;
    out.BlockData = { 0xAA, 0xBB };

    ClassicGuid destroyed;
    destroyed.Low = 0x1234;
    out.DestroyGUIDs.push_back(destroyed);

    ClassicGuid outOfRange;
    outOfRange.Low = 0x5678;
    out.OutOfRangeGUIDs.push_back(outOfRange);

    RoundTrip(out, [](UpdateObject const& a, UpdateObject const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.BlockCount == b.BlockCount);
        CHECK(a.BlockData == b.BlockData);
        REQUIRE(b.DestroyGUIDs.size() == 1);
        CHECK(a.DestroyGUIDs[0] == b.DestroyGUIDs[0]);
        REQUIRE(b.OutOfRangeGUIDs.size() == 1);
        CHECK(a.OutOfRangeGUIDs[0] == b.OutOfRangeGUIDs[0]);
    });
    RejectsEveryTruncation(out);
}
