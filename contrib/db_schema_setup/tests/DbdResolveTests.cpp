#include "DbdResolve.h"

#include <catch_amalgamated.hpp>

namespace
{
    std::string FixturePath(char const* name)
    {
        return std::string(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR) + "/" + name;
    }

    DbdFile LoadFixture(char const* name)
    {
        return ParseDbdFile(FixturePath(name));
    }

    // The build our client build (1.15.6.58797) and its dataBuild (1.15.6.58725) resolve
    // against, in the priority order the check tool uses by default.
    std::vector<DbdBuild> OurBuilds()
    {
        return { DbdBuild::Parse("1.15.6.58797"), DbdBuild::Parse("1.15.6.58725") };
    }
}

TEST_CASE("ResolveDbdTable matches an exact build", "[DbdResolve]")
{
    DbdFile file = LoadFixture("ResolveSample.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ResolveSample", { DbdBuild::Parse("1.0.0.100") });

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    CHECK(result.table->name == "ResolveSample");
    REQUIRE(result.table->fields.size() == 2);
    CHECK(result.table->fields[1].name == "Value");
    CHECK(result.table->fields[1].bits == 32);
    CHECK_FALSE(result.table->fields[1].isUnsigned);
}

TEST_CASE("ResolveDbdTable matches a build range", "[DbdResolve]")
{
    DbdFile file = LoadFixture("ResolveSample.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ResolveSample", { DbdBuild::Parse("2.0.0.25") });

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    CHECK(result.table->fields[1].bits == 16);
    CHECK(result.table->fields[1].isUnsigned);
}

TEST_CASE("ResolveDbdTable falls through a build priority list in order", "[DbdResolve]")
{
    DbdFile file = LoadFixture("ResolveSample.dbd");

    // 3.0.0.1 matches nothing; the resolver should fall through to 1.0.0.100.
    DbdResolveResult result = ResolveDbdTable(file, "ResolveSample",
        { DbdBuild::Parse("3.0.0.1"), DbdBuild::Parse("1.0.0.100") });

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    CHECK(result.table->fields[1].bits == 32);
}

TEST_CASE("ResolveDbdTable reports no definition when nothing matches", "[DbdResolve]")
{
    DbdFile file = LoadFixture("ResolveSample.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ResolveSample", { DbdBuild::Parse("3.0.0.1") });

    CHECK(result.status == DbdResolveStatus::NoDefinition);
    CHECK_FALSE(result.table.has_value());
}

TEST_CASE("ResolveDbdTable reports Ambiguous when a build matches two blocks", "[DbdResolve]")
{
    DbdFile file = LoadFixture("ErrorAmbiguousBuild.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ErrorAmbiguousBuild", { DbdBuild::Parse("1.0.0.2") });

    CHECK(result.status == DbdResolveStatus::Ambiguous);
    CHECK_FALSE(result.table.has_value());
}

// A handful of well-known tables, resolved against the pinned WoWDBDefs definitions for our
// client build (or its dataBuild), with fields checked against the .dbd source.

TEST_CASE("Map resolves for our build", "[DbdResolve][WellKnownTables]")
{
    DbdFile file = LoadFixture("Map.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "Map", OurBuilds());

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    REQUIRE(result.table->layoutHashes.size() == 2);
    CHECK(result.table->layoutHashes[0] == "32401DC5");
    CHECK(result.table->layoutHashes[1] == "B47C6F05");

    REQUIRE(result.table->fields.size() == 24);
    CHECK(result.table->fields.front().name == "ID");
    CHECK(result.table->fields.front().bits == 32);
    CHECK(result.table->fields.front().isId);
    CHECK(result.table->fields.front().isNonInline);

    CHECK(result.table->fields.back().name == "Flags");
    CHECK(result.table->fields.back().bits == 32);
    CHECK(result.table->fields.back().arrayLength == 3);
    CHECK_FALSE(result.table->fields.back().isUnsigned);
}

TEST_CASE("AreaTable resolves for our build", "[DbdResolve][WellKnownTables]")
{
    DbdFile file = LoadFixture("AreaTable.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "AreaTable", OurBuilds());

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    REQUIRE(result.table->layoutHashes.size() == 1);
    CHECK(result.table->layoutHashes[0] == "0A6204DD");
    REQUIRE(result.table->fields.size() == 24);
    CHECK(result.table->fields.back().name == "LiquidTypeID");
    CHECK(result.table->fields.back().arrayLength == 4);
    CHECK(result.table->fields.back().isUnsigned);
}

TEST_CASE("SpellName resolves for our build", "[DbdResolve][WellKnownTables]")
{
    DbdFile file = LoadFixture("SpellName.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "SpellName", OurBuilds());

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    REQUIRE(result.table->fields.size() == 2);
    CHECK(result.table->fields[0].name == "ID");
    CHECK(result.table->fields[1].name == "Name_lang");
}

TEST_CASE("ItemSparse resolves for our build", "[DbdResolve][WellKnownTables]")
{
    DbdFile file = LoadFixture("ItemSparse.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ItemSparse", OurBuilds());

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    REQUIRE(result.table->layoutHashes.size() == 1);
    CHECK(result.table->layoutHashes[0] == "DF925D47");
    CHECK(result.table->fields.size() == 75);
    CHECK(result.table->fields.front().name == "ID");
}

TEST_CASE("ChrRaces resolves for our build", "[DbdResolve][WellKnownTables]")
{
    DbdFile file = LoadFixture("ChrRaces.dbd");
    DbdResolveResult result = ResolveDbdTable(file, "ChrRaces", OurBuilds());

    REQUIRE(result.status == DbdResolveStatus::Resolved);
    REQUIRE(result.table->layoutHashes.size() == 1);
    CHECK(result.table->layoutHashes[0] == "E8A71CAF");
    CHECK(result.table->fields.size() == 58);
    CHECK(result.table->fields.front().name == "ID");
}
