// Round trips for the modern guild-charter wire.
#include "Packets/PetitionMessages.h"
#include "TestPacketRoundTrip.h"
#include <catch_amalgamated.hpp>

TEST_CASE("PetitionShow round-trips", "[packets]")
{
    PetitionShow packet;
    packet.PetitionerGuid = { 0x1234, 0 };
    RoundTrip(packet, [](PetitionShow const& a, PetitionShow const& b) { CHECK(a.PetitionerGuid == b.PetitionerGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PetitionBuy round-trips", "[packets]")
{
    PetitionBuy packet;
    packet.NpcGuid = { 0x4321, 0 };
    packet.Name = "Realbots Test Guild";
    RoundTrip(packet, [](PetitionBuy const& a, PetitionBuy const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.Name == b.Name);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PetitionShowSignatures round-trips", "[packets]")
{
    PetitionShowSignatures packet;
    packet.ItemGuid = { 0x5555, 0 };
    RoundTrip(packet, [](PetitionShowSignatures const& a, PetitionShowSignatures const& b) { CHECK(a.ItemGuid == b.ItemGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("QueryPetition round-trips", "[packets]")
{
    QueryPetition packet;
    packet.PetitionId = 42;
    packet.ItemGuid = { 0x6666, 0 };
    RoundTrip(packet, [](QueryPetition const& a, QueryPetition const& b)
    {
        CHECK(a.PetitionId == b.PetitionId);
        CHECK(a.ItemGuid == b.ItemGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("SignPetition round-trips", "[packets]")
{
    SignPetition packet;
    packet.ItemGuid = { 0x7777, 0 };
    RoundTrip(packet, [](SignPetition const& a, SignPetition const& b) { CHECK(a.ItemGuid == b.ItemGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("DeclinePetition round-trips", "[packets]")
{
    DeclinePetition packet;
    packet.ItemGuid = { 0x8888, 0 };
    RoundTrip(packet, [](DeclinePetition const& a, DeclinePetition const& b) { CHECK(a.ItemGuid == b.ItemGuid); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("OfferPetition round-trips", "[packets]")
{
    OfferPetition packet;
    packet.UnusedId = 3;
    packet.ItemGuid = { 0x9999, 0 };
    packet.PlayerGuid = { 0xaaaa, 0 };
    RoundTrip(packet, [](OfferPetition const& a, OfferPetition const& b)
    {
        CHECK(a.UnusedId == b.UnusedId);
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.PlayerGuid == b.PlayerGuid);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("TurnInPetition round-trips with and without the tabard tail", "[packets]")
{
    TurnInPetition packet;
    packet.ItemGuid = { 0xbbbb, 0 };
    packet.BackgroundColor = 1;
    packet.EmblemStyle = 2;
    packet.EmblemColor = 3;
    packet.BorderStyle = 4;
    packet.BorderColor = 5;
    RoundTrip(packet, [](TurnInPetition const& a, TurnInPetition const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.BackgroundColor == b.BackgroundColor);
        CHECK(a.EmblemStyle == b.EmblemStyle);
        CHECK(a.EmblemColor == b.EmblemColor);
        CHECK(a.BorderStyle == b.BorderStyle);
        CHECK(a.BorderColor == b.BorderColor);
    });

    ByteBuffer shortBody;
    packet.ItemGuid.Write(shortBody);
    TurnInPetition parsed;
    REQUIRE(parsed.Read(shortBody));
    CHECK(parsed.ItemGuid == packet.ItemGuid);
    CHECK(parsed.BackgroundColor == 0);
}

TEST_CASE("RenamePetition round-trips", "[packets]")
{
    RenamePetition packet;
    packet.ItemGuid = { 0xcccc, 0 };
    packet.NewName = "Renamed Guild";
    RoundTrip(packet, [](RenamePetition const& a, RenamePetition const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.NewName == b.NewName);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PetitionShowList round-trips", "[packets]")
{
    PetitionShowList packet;
    packet.NpcGuid = { 0xdddd, 0 };
    packet.Price = 1000;
    RoundTrip(packet, [](PetitionShowList const& a, PetitionShowList const& b)
    {
        CHECK(a.NpcGuid == b.NpcGuid);
        CHECK(a.Price == b.Price);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PetitionSignatures round-trips with no and with several signers", "[packets]")
{
    PetitionSignatures packet;
    packet.ItemGuid = { 0xeeee, 0 };
    packet.OwnerGuid = { 0xffff, 0 };
    packet.OwnerAccountGuid = { 0x1111, 0 };
    packet.PetitionId = 7;
    RoundTrip(packet, [](PetitionSignatures const& a, PetitionSignatures const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.OwnerGuid == b.OwnerGuid);
        CHECK(a.OwnerAccountGuid == b.OwnerAccountGuid);
        CHECK(a.PetitionId == b.PetitionId);
        CHECK(a.Signatures.size() == b.Signatures.size());
    });
    RejectsEveryTruncation(packet);

    packet.Signatures.push_back({ ClassicGuid{ 0x2222, 0 }, 0 });
    packet.Signatures.push_back({ ClassicGuid{ 0x3333, 0 }, 0 });
    RoundTrip(packet, [](PetitionSignatures const& a, PetitionSignatures const& b)
    {
        REQUIRE(a.Signatures.size() == b.Signatures.size());
        for (size_t i = 0; i < a.Signatures.size(); ++i)
        {
            CHECK(a.Signatures[i].SignerGuid == b.Signatures[i].SignerGuid);
            CHECK(a.Signatures[i].Choice == b.Signatures[i].Choice);
        }
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("PetitionSignResult round-trips", "[packets]")
{
    PetitionSignResult packet;
    packet.ItemGuid = { 0x4444, 0 };
    packet.PlayerGuid = { 0x5555, 0 };
    packet.Result = PetitionSignResult::AlreadySigned;
    RoundTrip(packet, [](PetitionSignResult const& a, PetitionSignResult const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.PlayerGuid == b.PlayerGuid);
        CHECK(a.Result == b.Result);
    });
    RejectsEveryTruncation(packet);
}

TEST_CASE("TurnInPetitionResult round-trips", "[packets]")
{
    TurnInPetitionResult packet;
    packet.Result = TurnInPetitionResult::NeedMoreSignatures;
    RoundTrip(packet, [](TurnInPetitionResult const& a, TurnInPetitionResult const& b) { CHECK(a.Result == b.Result); });
    RejectsEveryTruncation(packet);
}

TEST_CASE("QueryPetitionResponse round-trips when disallowed and when allowed", "[packets]")
{
    QueryPetitionResponse disallowed;
    disallowed.PetitionId = 9;
    disallowed.Allow = false;
    RoundTrip(disallowed, [](QueryPetitionResponse const& a, QueryPetitionResponse const& b)
    {
        CHECK(a.PetitionId == b.PetitionId);
        CHECK(a.Allow == b.Allow);
    });
    RejectsEveryTruncation(disallowed);

    QueryPetitionResponse allowed;
    allowed.PetitionId = 9;
    allowed.Allow = true;
    allowed.OwnerGuid = { 0x6666, 0 };
    allowed.Name = "Realbots Test Guild";
    allowed.BodyText = "";
    allowed.MinSignatures = 9;
    allowed.MaxSignatures = 9;
    RoundTrip(allowed, [](QueryPetitionResponse const& a, QueryPetitionResponse const& b)
    {
        CHECK(a.PetitionId == b.PetitionId);
        CHECK(a.Allow == b.Allow);
        CHECK(a.OwnerGuid == b.OwnerGuid);
        CHECK(a.Name == b.Name);
        CHECK(a.BodyText == b.BodyText);
        CHECK(a.MinSignatures == b.MinSignatures);
        CHECK(a.MaxSignatures == b.MaxSignatures);
    });
    RejectsEveryTruncation(allowed);
}

TEST_CASE("RenamePetitionResult round-trips", "[packets]")
{
    RenamePetitionResult packet;
    packet.ItemGuid = { 0x7777, 0 };
    packet.NewName = "Renamed Guild";
    RoundTrip(packet, [](RenamePetitionResult const& a, RenamePetitionResult const& b)
    {
        CHECK(a.ItemGuid == b.ItemGuid);
        CHECK(a.NewName == b.NewName);
    });
    RejectsEveryTruncation(packet);
}
