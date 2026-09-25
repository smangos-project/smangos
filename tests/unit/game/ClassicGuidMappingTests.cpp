#include "Packets/Classic/GuidMapping.h"

#include <catch_amalgamated.hpp>

namespace
{
constexpr uint32 REALM = 1;

ObjectGuid Roundtrip(ObjectGuid const& guid, uint32 realmId = REALM)
{
    nonstd::optional<ObjectGuid> back = ClassicGuidMapping::FromClassic(ClassicGuidMapping::ToClassic(guid, realmId));
    REQUIRE(back.has_value());
    return *back;
}
}

TEST_CASE("A player guid maps to a realm-specific classic guid", "[classicguid]")
{
    ClassicGuid const classic = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, 5u), REALM);
    CHECK(classic.Low == 5);
    CHECK(classic.High == ((uint64(2) << 58) | (uint64(REALM) << 42)));
}

TEST_CASE("A creature guid carries its entry and the realm", "[classicguid]")
{
    ClassicGuid const classic = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_UNIT, 1234u, 99u), REALM);
    CHECK(classic.Low == 99);
    CHECK(classic.High == ((uint64(8) << 58) | (uint64(REALM) << 42) | (uint64(1234) << 6)));
}

TEST_CASE("A transport guid has no realm and no low part", "[classicguid]")
{
    ClassicGuid const classic = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_TRANSPORT, 176231u, 7u), REALM);
    CHECK(classic.Low == 0);
    CHECK(classic.High == ((uint64(6) << 58) | (uint64(7) << 38) | uint64(176231)));
}

TEST_CASE("The realm id is stamped into the guid, not hardcoded", "[classicguid]")
{
    for (uint32 realm : { 1u, 2u, 4097u })
    {
        ClassicGuid const classic = ClassicGuidMapping::ToClassic(ObjectGuid(HIGHGUID_PLAYER, 5u), realm);
        CHECK(((classic.High >> 42) & 0x1FFF) == realm);
    }
}

TEST_CASE("Every guid type in use round-trips", "[classicguid]")
{
    std::vector<ObjectGuid> const guids = {
        ObjectGuid(HIGHGUID_PLAYER, 1u),
        ObjectGuid(HIGHGUID_PLAYER, 0xFFFFFFFFu),
        ObjectGuid(HIGHGUID_ITEM, 42u),
        ObjectGuid(HIGHGUID_UNIT, 1234u, 99u),
        ObjectGuid(HIGHGUID_UNIT, 0x7FFFFFu, 0xFFFFFFu),
        ObjectGuid(HIGHGUID_PET, 416u, 3u),
        ObjectGuid(HIGHGUID_GAMEOBJECT, 2560u, 12u),
        ObjectGuid(HIGHGUID_DYNAMICOBJECT, 8u),
        ObjectGuid(HIGHGUID_CORPSE, 9u),
        ObjectGuid(HIGHGUID_TRANSPORT, 176231u, 7u),
    };

    for (ObjectGuid const& guid : guids)
    {
        INFO(guid.GetRawValue());
        CHECK(Roundtrip(guid) == guid);
    }
}

TEST_CASE("A mo-transport guid comes back as a plain transport", "[classicguid]")
{
    ObjectGuid const back = Roundtrip(ObjectGuid(HIGHGUID_MO_TRANSPORT, 3u));
    CHECK(back.GetHigh() == HIGHGUID_TRANSPORT);
    CHECK(back.GetCounter() == 3);
}

TEST_CASE("An empty guid stays empty both ways", "[classicguid]")
{
    CHECK(ClassicGuidMapping::ToClassic(ObjectGuid(), REALM).IsEmpty());

    nonstd::optional<ObjectGuid> const back = ClassicGuidMapping::FromClassic(ClassicGuid());
    REQUIRE(back.has_value());
    CHECK(back->IsEmpty());
}

TEST_CASE("A classic guid of a type the server has no guid for is rejected", "[classicguid]")
{
    ClassicGuid guild;
    guild.Low = 7;
    guild.High = (uint64(28) << 58) | (uint64(REALM) << 42);
    CHECK_FALSE(ClassicGuidMapping::FromClassic(guild).has_value());
}

TEST_CASE("A counter too large for the server's guid layout is rejected", "[classicguid]")
{
    ClassicGuid creature;
    creature.Low = 0x1000000;
    creature.High = (uint64(8) << 58) | (uint64(REALM) << 42) | (uint64(5) << 6);
    CHECK_FALSE(ClassicGuidMapping::FromClassic(creature).has_value());

    ClassicGuid player;
    player.Low = 0x100000000ULL;
    player.High = (uint64(2) << 58) | (uint64(REALM) << 42);
    CHECK_FALSE(ClassicGuidMapping::FromClassic(player).has_value());
}

TEST_CASE("A mapped guid survives the packed wire format", "[classicguid]")
{
    ObjectGuid const guid(HIGHGUID_UNIT, 1234u, 99u);

    ByteBuffer buffer;
    ClassicGuidMapping::ToClassic(guid, REALM).Write(buffer);

    ClassicGuid wire;
    REQUIRE(wire.Read(buffer));

    nonstd::optional<ObjectGuid> const back = ClassicGuidMapping::FromClassic(wire);
    REQUIRE(back.has_value());
    CHECK(*back == guid);
}
