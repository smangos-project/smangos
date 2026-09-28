#include "DbdDefinitions.h"

#include <catch_amalgamated.hpp>

#include <algorithm>

namespace
{
    bool HasError(DbdDefinitions const& definitions, std::string const& fileStem)
    {
        return std::any_of(definitions.errors.begin(), definitions.errors.end(),
            [&](DbdDefinitionsLoadError const& e) { return e.file.find(fileStem) != std::string::npos; });
    }
}

TEST_CASE("LoadDbdDefinitions parses every table and collects per-file errors", "[DbdDefinitions]")
{
    DbdDefinitions definitions = LoadDbdDefinitions(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR);

    // 18 .dbd fixtures live alongside this test: 7 are deliberately malformed (see DbdFileTests),
    // the rest (including ErrorAmbiguousBuild.dbd, which parses fine -- its ambiguity is a
    // resolve-time concern) parse successfully.
    CHECK(definitions.tables.size() == 11);
    CHECK(definitions.errors.size() == 7);

    CHECK(definitions.tables.contains("Map"));
    CHECK(definitions.tables.contains("AreaTable"));
    CHECK(definitions.tables.contains("SpellName"));
    CHECK(definitions.tables.contains("ItemSparse"));
    CHECK(definitions.tables.contains("ChrRaces"));
    CHECK(definitions.tables.contains("Grammar"));
    CHECK(definitions.tables.contains("ResolveSample"));
    CHECK(definitions.tables.contains("DiffSample"));
    CHECK(definitions.tables.contains("ErrorAmbiguousBuild"));

    CHECK(HasError(definitions, "ErrorMissingSpace"));
    CHECK(HasError(definitions, "ErrorInvalidType"));
    CHECK(HasError(definitions, "ErrorBadForeignKey"));
    CHECK(HasError(definitions, "ErrorUnknownColumn"));
    CHECK(HasError(definitions, "ErrorNoBuildOrLayout"));
    CHECK(HasError(definitions, "ErrorBadArrayLength"));
    CHECK(HasError(definitions, "ErrorNotColumns"));
}

TEST_CASE("LoadDbdDefinitions throws for a missing directory", "[DbdDefinitions]")
{
    CHECK_THROWS_AS(LoadDbdDefinitions(std::string(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR) + "/does-not-exist"),
        std::runtime_error);
}
