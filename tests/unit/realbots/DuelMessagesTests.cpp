// Round trips for the modern duel wire.
#include "Packets/DuelMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("CanDuel round-trips", "[packets]")
{
    CanDuel packet;
    packet.Target = { 0x1234, 0 };
    packet.ToTheDeath = true;
    RoundTrip(packet, [](CanDuel const& a, CanDuel const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.ToTheDeath == b.ToTheDeath);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DuelResponse round-trips", "[packets]")
{
    DuelResponse packet;
    packet.Arbiter = { 0x77, 0x2000000000000000 };
    packet.Accepted = true;
    packet.Forfeited = true;
    RoundTrip(packet, [](DuelResponse const& a, DuelResponse const& b)
    {
        CHECK(a.Arbiter == b.Arbiter);
        CHECK(a.Accepted == b.Accepted);
        CHECK(a.Forfeited == b.Forfeited);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CanDuelResult round-trips", "[packets]")
{
    CanDuelResult packet;
    packet.Target = { 0x4321, 0 };
    packet.Result = true;
    RoundTrip(packet, [](CanDuelResult const& a, CanDuelResult const& b)
    {
        CHECK(a.Target == b.Target);
        CHECK(a.Result == b.Result);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DuelRequested round-trips", "[packets]")
{
    DuelRequested packet;
    packet.Arbiter = { 0x10, 0x1000000000000000 };
    packet.RequestedBy = { 0x20, 0 };
    packet.RequestedByAccount = { 0x30, 0 };
    packet.ToTheDeath = true;
    RoundTrip(packet, [](DuelRequested const& a, DuelRequested const& b)
    {
        CHECK(a.Arbiter == b.Arbiter);
        CHECK(a.RequestedBy == b.RequestedBy);
        CHECK(a.RequestedByAccount == b.RequestedByAccount);
        CHECK(a.ToTheDeath == b.ToTheDeath);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DuelCountdown round-trips", "[packets]")
{
    DuelCountdown packet;
    packet.Countdown = 3000;
    RoundTrip(packet, [](DuelCountdown const& a, DuelCountdown const& b)
    {
        CHECK(a.Countdown == b.Countdown);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DuelCompleteNotice round-trips", "[packets]")
{
    DuelCompleteNotice packet;
    packet.Started = true;
    RoundTrip(packet, [](DuelCompleteNotice const& a, DuelCompleteNotice const& b)
    {
        CHECK(a.Started == b.Started);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DuelWinner round-trips", "[packets]")
{
    DuelWinner packet;
    packet.BeatenName = "Loser";
    packet.WinnerName = "Winner";
    packet.BeatenVirtualRealmAddress = 5;
    packet.WinnerVirtualRealmAddress = 6;
    packet.Fled = true;
    RoundTrip(packet, [](DuelWinner const& a, DuelWinner const& b)
    {
        CHECK(a.BeatenName == b.BeatenName);
        CHECK(a.WinnerName == b.WinnerName);
        CHECK(a.BeatenVirtualRealmAddress == b.BeatenVirtualRealmAddress);
        CHECK(a.WinnerVirtualRealmAddress == b.WinnerVirtualRealmAddress);
        CHECK(a.Fled == b.Fled);
    });
    RejectsEveryTruncation(packet);
}
