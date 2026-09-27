// Round trips for the modern death and resurrection wire.
#include "Packets/DeathMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("RepopRequest round-trips", "[packets]")
{
    RepopRequest packet;
    packet.CheckInstance = true;
    RoundTrip(packet, [](RepopRequest const& a, RepopRequest const& b)
    {
        CHECK(a.CheckInstance == b.CheckInstance);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ResurrectResponse round-trips", "[packets]")
{
    ResurrectResponse packet;
    packet.Resurrecter = { 0x5566, 0 };
    packet.Response = 1;
    RoundTrip(packet, [](ResurrectResponse const& a, ResurrectResponse const& b)
    {
        CHECK(a.Resurrecter == b.Resurrecter);
        CHECK(a.Response == b.Response);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ReclaimCorpse round-trips", "[packets]")
{
    ReclaimCorpse packet;
    packet.Corpse = { 0x7788, 0 };
    RoundTrip(packet, [](ReclaimCorpse const& a, ReclaimCorpse const& b)
    {
        CHECK(a.Corpse == b.Corpse);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ResurrectRequest round-trips", "[packets]")
{
    ResurrectRequest packet;
    packet.Offerer = { 0x99aa, 0 };
    packet.PetNumber = 3;
    packet.SpellId = 20770;
    packet.UseTimer = true;
    packet.Sickness = true;
    packet.Name = "Spirit Healer";
    RoundTrip(packet, [](ResurrectRequest const& a, ResurrectRequest const& b)
    {
        CHECK(a.Offerer == b.Offerer);
        CHECK(a.VirtualRealmAddress == b.VirtualRealmAddress);
        CHECK(a.PetNumber == b.PetNumber);
        CHECK(a.SpellId == b.SpellId);
        CHECK(a.UseTimer == b.UseTimer);
        CHECK(a.Sickness == b.Sickness);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("ResurrectRequest with no name round-trips", "[packets]")
{
    ResurrectRequest packet;
    packet.Offerer = { 0x99aa, 0 };
    packet.SpellId = 2006;
    RoundTrip(packet, [](ResurrectRequest const& a, ResurrectRequest const& b)
    {
        CHECK(b.Name.empty());
        CHECK_FALSE(b.UseTimer);
        CHECK_FALSE(b.Sickness);
        CHECK(a.SpellId == b.SpellId);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DeathReleaseLoc round-trips", "[packets]")
{
    DeathReleaseLoc packet;
    packet.MapId = 1;
    packet.X = -618.5f;
    packet.Y = -4251.75f;
    packet.Z = 38.75f;
    RoundTrip(packet, [](DeathReleaseLoc const& a, DeathReleaseLoc const& b)
    {
        CHECK(a.MapId == b.MapId);
        CHECK(a.X == b.X);
        CHECK(a.Y == b.Y);
        CHECK(a.Z == b.Z);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("CorpseReclaimDelay round-trips", "[packets]")
{
    CorpseReclaimDelay packet;
    packet.Remaining = 30000;
    RoundTrip(packet, [](CorpseReclaimDelay const& a, CorpseReclaimDelay const& b)
    {
        CHECK(a.Remaining == b.Remaining);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DurabilityDamageDeath round-trips", "[packets]")
{
    DurabilityDamageDeath packet;
    packet.Percent = 10;
    RoundTrip(packet, [](DurabilityDamageDeath const& a, DurabilityDamageDeath const& b)
    {
        CHECK(a.Percent == b.Percent);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SelfRes round-trips", "[packets]")
{
    SelfRes packet;
    packet.SpellID = 20707;
    RoundTrip(packet, [](SelfRes const& a, SelfRes const& b) { CHECK(a.SpellID == b.SpellID); });
    RejectsEveryTruncation(packet);
}
