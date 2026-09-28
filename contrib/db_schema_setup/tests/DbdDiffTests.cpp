#include "DbdDiff.h"

#include "DbdDefinitions.h"

#include <catch_amalgamated.hpp>

#include <algorithm>

namespace
{
    std::string FixturesDir()
    {
        return std::string(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR);
    }
}

TEST_CASE("DiffDbdTables reports a removed field and a changed array length", "[DbdDiff]")
{
    DbdDefinitions definitions = LoadDbdDefinitions(FixturesDir());
    std::vector<DbdTableDiff> diffs = DiffDbdTables(definitions, { DbdBuild::Parse("1.0.0.1") }, { DbdBuild::Parse("2.0.0.1") });

    auto it = std::find_if(diffs.begin(), diffs.end(), [](DbdTableDiff const& d) { return d.table == "DiffSample"; });
    REQUIRE(it != diffs.end());
    CHECK_FALSE(it->removedEntirely);
    REQUIRE(it->fields.size() == 2);

    auto nameChange = std::find_if(it->fields.begin(), it->fields.end(), [](DbdFieldDiff const& f) { return f.field == "Name"; });
    REQUIRE(nameChange != it->fields.end());
    CHECK(nameChange->kind == "removed");

    auto valueChange = std::find_if(it->fields.begin(), it->fields.end(), [](DbdFieldDiff const& f) { return f.field == "Value"; });
    REQUIRE(valueChange != it->fields.end());
    CHECK(valueChange->kind == "array_length");
    CHECK(valueChange->detail == "1 -> 2");
}

TEST_CASE("DiffDbdTables reports a table with no definition at the target build as removed entirely", "[DbdDiff]")
{
    DbdDefinitions definitions = LoadDbdDefinitions(FixturesDir());
    std::vector<DbdTableDiff> diffs = DiffDbdTables(definitions, { DbdBuild::Parse("1.0.0.100") }, { DbdBuild::Parse("9.9.9.9999") });

    auto it = std::find_if(diffs.begin(), diffs.end(), [](DbdTableDiff const& d) { return d.table == "ResolveSample"; });
    REQUIRE(it != diffs.end());
    CHECK(it->removedEntirely);
    CHECK(it->fields.empty());
}

TEST_CASE("DiffDbdTables skips a table that doesn't resolve at the source build", "[DbdDiff]")
{
    DbdDefinitions definitions = LoadDbdDefinitions(FixturesDir());
    std::vector<DbdTableDiff> diffs = DiffDbdTables(definitions, { DbdBuild::Parse("9.9.9.9999") }, { DbdBuild::Parse("1.0.0.1") });

    CHECK(std::find_if(diffs.begin(), diffs.end(), [](DbdTableDiff const& d) { return d.table == "DiffSample"; }) == diffs.end());
}
