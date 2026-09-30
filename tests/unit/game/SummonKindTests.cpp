#include "Spells/SummonKind.h"
#include <catch_amalgamated.hpp>

TEST_CASE("Summon kind by properties id", "[spell]")
{
    CHECK(GetSummonKind(67, nullptr) == SUMMON_KIND_PET);
    CHECK(GetSummonKind(64, nullptr) == SUMMON_KIND_WILD);
    CHECK(GetSummonKind(61, nullptr) == SUMMON_KIND_GUARDIAN);
    CHECK(GetSummonKind(5121, nullptr) == SUMMON_KIND_GUARDIAN);
    CHECK(GetSummonKind(65, nullptr) == SUMMON_KIND_POSSESSED);
    CHECK(GetSummonKind(121, nullptr) == SUMMON_KIND_TOTEM);
    CHECK(GetSummonKind(5100, nullptr) == SUMMON_KIND_TOTEM);
    CHECK(GetSummonKind(63, nullptr) == SUMMON_KIND_TOTEM_SLOT1);
    CHECK(GetSummonKind(81, nullptr) == SUMMON_KIND_TOTEM_SLOT2);
    CHECK(GetSummonKind(82, nullptr) == SUMMON_KIND_TOTEM_SLOT3);
    CHECK(GetSummonKind(83, nullptr) == SUMMON_KIND_TOTEM_SLOT4);
    CHECK(GetSummonKind(41, nullptr) == SUMMON_KIND_CRITTER);
    CHECK(GetSummonKind(66, nullptr) == SUMMON_KIND_DEMON);
    CHECK(GetSummonKind(5878, nullptr) == SUMMON_KIND_TRANS_DOOR);
}

TEST_CASE("Summon kind fallback on the row fields", "[spell]")
{
    SummonPropertiesEntry row{};
    CHECK(GetSummonKind(9999, nullptr) == SUMMON_KIND_WILD);
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_WILD);

    row = {};
    row.Slot = 3;
    row.Control = 3;
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_TOTEM_SLOT3);

    row = {};
    row.Control = 3;
    row.Title = 5;
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_POSSESSED);

    row = {};
    row.Title = 5;
    row.Control = 2;
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_CRITTER);

    row = {};
    row.Control = 2;
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_PET);

    row = {};
    row.Control = 1;
    CHECK(GetSummonKind(9999, &row) == SUMMON_KIND_GUARDIAN);
}
