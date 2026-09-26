// Round trips for the modern taxi wire.
#include "Packets/TaxiMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("TaxiNodeStatusQuery round-trips", "[packets]")
{
    TaxiNodeStatusQuery query;
    query.UnitGuid = { 0x12345678, 0 };
    RoundTrip(query, [](TaxiNodeStatusQuery const& a, TaxiNodeStatusQuery const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("TaxiNodeStatus round-trips", "[packets]")
{
    TaxiNodeStatus status;
    status.UnitGuid = { 0x87654321, 0 };
    status.Status = CLASSIC_TAXISTATUS_LEARNED;
    RoundTrip(status, [](TaxiNodeStatus const& a, TaxiNodeStatus const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.Status == b.Status);
    });
    RejectsEveryTruncation(status);
}

TEST_CASE("TaxiQueryAvailableNodes round-trips", "[packets]")
{
    TaxiQueryAvailableNodes query;
    query.UnitGuid = { 42, 0 };
    RoundTrip(query, [](TaxiQueryAvailableNodes const& a, TaxiQueryAvailableNodes const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
    });
    RejectsEveryTruncation(query);
}

TEST_CASE("EnableTaxiNode round-trips", "[packets]")
{
    EnableTaxiNode enable;
    enable.UnitGuid = { 7, 0 };
    RoundTrip(enable, [](EnableTaxiNode const& a, EnableTaxiNode const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
    });
    RejectsEveryTruncation(enable);
}

TEST_CASE("ShowTaxiNodes round-trips with and without a window", "[packets]")
{
    ShowTaxiNodes nodes;
    nodes.CanLandNodes[0] = 0xFF;
    nodes.CanUseNodes[3] = 0x0F;
    RoundTrip(nodes, [](ShowTaxiNodes const& a, ShowTaxiNodes const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.CurrentNode == b.CurrentNode);
        CHECK(a.CanLandNodes == b.CanLandNodes);
        CHECK(a.CanUseNodes == b.CanUseNodes);
    });
    RejectsEveryTruncation(nodes);

    nodes.UnitGuid = { 99, 0 };
    nodes.CurrentNode = 45;
    RoundTrip(nodes, [](ShowTaxiNodes const& a, ShowTaxiNodes const& b)
    {
        CHECK(a.UnitGuid == b.UnitGuid);
        CHECK(a.CurrentNode == b.CurrentNode);
        CHECK(a.CanLandNodes == b.CanLandNodes);
        CHECK(a.CanUseNodes == b.CanUseNodes);
    });
    RejectsEveryTruncation(nodes);
}

TEST_CASE("ActivateTaxi round-trips", "[packets]")
{
    ActivateTaxi activate;
    activate.Vendor = { 123, 0 };
    activate.Node = 45;
    activate.GroundMountID = 0;
    activate.FlyingMountID = 0;
    RoundTrip(activate, [](ActivateTaxi const& a, ActivateTaxi const& b)
    {
        CHECK(a.Vendor == b.Vendor);
        CHECK(a.Node == b.Node);
        CHECK(a.GroundMountID == b.GroundMountID);
        CHECK(a.FlyingMountID == b.FlyingMountID);
    });
    RejectsEveryTruncation(activate);
}

TEST_CASE("NewTaxiPath round-trips", "[packets]")
{
    NewTaxiPath path;
    path.TaxiNodesId = 45;
    RoundTrip(path, [](NewTaxiPath const& a, NewTaxiPath const& b)
    {
        CHECK(a.TaxiNodesId == b.TaxiNodesId);
    });
    RejectsEveryTruncation(path);
}

TEST_CASE("ActivateTaxiReply round-trips", "[packets]")
{
    ActivateTaxiReply reply;
    reply.Reply = 6;
    RoundTrip(reply, [](ActivateTaxiReply const& a, ActivateTaxiReply const& b)
    {
        CHECK(a.Reply == b.Reply);
    });
    RejectsEveryTruncation(reply);
}
