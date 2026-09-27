#include "DbdFile.h"

#include <catch_amalgamated.hpp>

namespace
{
    std::string FixturePath(char const* name)
    {
        return std::string(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR) + "/" + name;
    }
}

TEST_CASE("ParseDbdFile reads every grammar item in one file", "[DbdFile]")
{
    DbdFile file = ParseDbdFile(FixturePath("Grammar.dbd"));

    REQUIRE(file.columns.size() == 6);
    REQUIRE(file.versions.size() == 1);

    DbdColumn const& id = file.columns.at("ID");
    CHECK(id.type == "int");
    CHECK(id.verified);

    DbdColumn const& continentId = file.columns.at("ContinentID");
    CHECK(continentId.foreignTable == "Map");
    CHECK(continentId.foreignColumn == "ID");

    DbdColumn const& name = file.columns.at("Name");
    CHECK(name.type == "string");
    CHECK_FALSE(name.verified); // trailing '?'

    DbdColumn const& nameLang = file.columns.at("Name_lang");
    CHECK(nameLang.type == "locstring");
    CHECK(nameLang.comment == "display name");

    DbdColumn const& weight = file.columns.at("Weight");
    CHECK(weight.type == "float");

    DbdVersionBlock const& version = file.versions[0];
    CHECK(version.comment == "test version block");
    REQUIRE(version.builds.size() == 2);
    CHECK(version.builds[0] == DbdBuild::Parse("1.0.1.300"));
    CHECK(version.builds[1] == DbdBuild::Parse("1.0.1.301"));
    REQUIRE(version.buildRanges.size() == 1);
    CHECK(version.buildRanges[0] == DbdBuildRange::Parse("1.0.0.100-1.0.0.200"));
    REQUIRE(version.layoutHashes.size() == 2);
    CHECK(version.layoutHashes[0] == "AABBCCDD");
    CHECK(version.layoutHashes[1] == "11223344");

    REQUIRE(version.fields.size() == 6);

    DbdFieldDefinition const& idField = version.fields[0];
    CHECK(idField.name == "ID");
    CHECK(idField.bits == 32);
    CHECK_FALSE(idField.isUnsigned);
    CHECK(idField.isId);
    CHECK(idField.isNonInline);
    CHECK_FALSE(idField.isRelation);

    DbdFieldDefinition const& continentIdField = version.fields[1];
    CHECK(continentIdField.name == "ContinentID");
    CHECK(continentIdField.bits == 16);
    CHECK(continentIdField.isUnsigned);

    DbdFieldDefinition const& weightField = version.fields[4];
    CHECK(weightField.name == "Weight");
    CHECK(weightField.bits == 0);
    CHECK(weightField.comment == "in kg");

    DbdFieldDefinition const& flagsField = version.fields[5];
    CHECK(flagsField.name == "Flags");
    CHECK(flagsField.bits == 32);
    CHECK(flagsField.isUnsigned);
    CHECK(flagsField.arrayLength == 2);
}

TEST_CASE("ParseDbdFile accepts a UTF-8 BOM and CRLF line endings", "[DbdFile]")
{
    DbdFile file = ParseDbdFile(FixturePath("CrlfBom.dbd"));

    REQUIRE(file.columns.size() == 1);
    REQUIRE(file.versions.size() == 1);
    CHECK(file.versions[0].builds[0] == DbdBuild::Parse("1.0.0.1"));
}

TEST_CASE("ParseDbdFile treats runs of blank lines as a single separator", "[DbdFile]")
{
    DbdFile file = ParseDbdFile(FixturePath("MultipleBlankLines.dbd"));

    REQUIRE(file.versions.size() == 2);
    CHECK(file.versions[0].builds[0] == DbdBuild::Parse("1.0.0.1"));
    CHECK(file.versions[1].builds[0] == DbdBuild::Parse("1.0.0.2")); // flushed at EOF, no trailing blank line
}

TEST_CASE("ParseDbdFile reports file:line for malformed input", "[DbdFile]")
{
    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorMissingSpace.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 2; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorInvalidType.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 2; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorBadForeignKey.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 2; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorUnknownColumn.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 6; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorNoBuildOrLayout.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 5; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorBadArrayLength.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 5; }));

    CHECK_THROWS_MATCHES(ParseDbdFile(FixturePath("ErrorNotColumns.dbd")), DbdParseError,
        Catch::Matchers::Predicate<DbdParseError>([](DbdParseError const& e) { return e.line == 1; }));
}
