// Round trips for the far-teleport (map change) wire.
#include "Packets/WorldTransfer.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("TransferPending round-trips with and without a ship or spell", "[packets]")
{
    TransferPending pending;
    pending.MapId = 1;
    pending.X = 1.0f;
    pending.Y = 2.0f;
    pending.Z = 3.0f;
    RoundTrip(pending, [](TransferPending const& a, TransferPending const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.ShipEntry == b.ShipEntry);
        CHECK(a.ShipOriginMapId == b.ShipOriginMapId);
        CHECK(a.TransferSpellId == b.TransferSpellId);
    });
    RejectsEveryTruncation(pending);

    pending.ShipEntry = 42;
    pending.ShipOriginMapId = 0;
    pending.TransferSpellId = 556;
    RoundTrip(pending, [](TransferPending const& a, TransferPending const& b)
    {
        CHECK(a.ShipEntry == b.ShipEntry);
        CHECK(a.ShipOriginMapId == b.ShipOriginMapId);
        CHECK(a.TransferSpellId == b.TransferSpellId);
    });
    RejectsEveryTruncation(pending);
}

TEST_CASE("TransferAborted round-trips", "[packets]")
{
    TransferAborted aborted;
    aborted.MapId = 269;
    aborted.Arg = 1;
    aborted.MapDifficultyXConditionId = 0;
    aborted.Reason = 5;
    RoundTrip(aborted, [](TransferAborted const& a, TransferAborted const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.Arg == b.Arg);
        CHECK(a.MapDifficultyXConditionId == b.MapDifficultyXConditionId);
        CHECK(a.Reason == b.Reason);
    });
    RejectsEveryTruncation(aborted);
}

TEST_CASE("NewWorld round-trips", "[packets]")
{
    NewWorld world;
    world.MapId = 1;
    world.X = 10.0f;
    world.Y = 20.0f;
    world.Z = 30.0f;
    world.O = 1.5f;
    world.Counter = 3;
    RoundTrip(world, [](NewWorld const& a, NewWorld const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
        CHECK(a.O == b.O);
        CHECK(a.Reason == b.Reason);
        CHECK(a.OffsetX == b.OffsetX);
        CHECK(a.OffsetY == b.OffsetY);
        CHECK(a.OffsetZ == b.OffsetZ);
        CHECK(a.Counter == b.Counter);
    });
    RejectsEveryTruncation(world);
}

TEST_CASE("WorldPortResponse round-trips its empty body", "[packets]")
{
    WorldPortResponse response;
    RoundTrip(response, [](WorldPortResponse const&, WorldPortResponse const&) {});
}
