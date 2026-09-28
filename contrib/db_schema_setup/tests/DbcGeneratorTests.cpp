#include "DbcGenerator.h"
#include "DbdDefinitions.h"
#include "DbdResolve.h"

#include <catch_amalgamated.hpp>

namespace
{
    std::string FixturePath(char const* name)
    {
        return std::string(DB_SCHEMA_SETUP_TEST_FIXTURES_DIR) + "/" + name;
    }

    DbdTable ResolveFixture(char const* fileName, char const* tableName, char const* build)
    {
        DbdFile file = ParseDbdFile(FixturePath(fileName));
        DbdResolveResult result = ResolveDbdTable(file, tableName, { DbdBuild::Parse(build) });
        REQUIRE(result.status == DbdResolveStatus::Resolved);
        return *result.table;
    }

    DbdResolvedField MakeField(std::string name, std::string type, int bits = 32, int arrayLength = 1)
    {
        DbdResolvedField field;
        field.name = std::move(name);
        field.type = std::move(type);
        field.bits = bits;
        field.arrayLength = arrayLength;
        return field;
    }
}

TEST_CASE("GenerateDbcTablesSource emits every field and table property", "[DbcGenerator]")
{
    DbdTable table = ResolveFixture("Grammar.dbd", "Grammar", "1.0.0.150");

    DbcGeneratorResult result = GenerateDbcTablesSource({ table }, "deadbeef");
    REQUIRE(result.errors.empty());

    CHECK(result.source.find("// WoWDBDefs commit: deadbeef") != std::string::npos);
    CHECK(result.source.find("#include \"Dbc/DbcTableMeta.h\"") != std::string::npos);

    // ID: noninline, id, int<32>
    CHECK(result.source.find("{ \"ID\", DbcFieldType::Int, 32, true, 1, true, false, true, nullptr, nullptr }") !=
          std::string::npos);
    // ContinentID: foreign key, unsigned 16-bit
    CHECK(result.source.find(
              "{ \"ContinentID\", DbcFieldType::Int, 16, false, 1, false, false, false, \"Map\", \"ID\" }") !=
          std::string::npos);
    // Flags: unsigned 32-bit array of 2
    CHECK(result.source.find("{ \"Flags\", DbcFieldType::Int, 32, false, 2, false, false, false, nullptr, nullptr }") !=
          std::string::npos);

    CHECK(result.source.find(
              "{ \"Grammar\", \"grammar\", \"dbc_grammar\", g_dbcLayoutHashes_Grammar, 2u, g_dbcFields_Grammar, 6u },") !=
          std::string::npos);
    CHECK(result.source.find("static uint32_t const g_dbcLayoutHashes_Grammar[] = { 0xAABBCCDDu, 0x11223344u };") !=
          std::string::npos);
}

TEST_CASE("GenerateDbcTablesSource sorts tables by name", "[DbcGenerator]")
{
    DbdTable b;
    b.name = "Bravo";
    b.fields = { MakeField("ID", "int") };

    DbdTable a;
    a.name = "Alpha";
    a.fields = { MakeField("ID", "int") };

    DbcGeneratorResult result = GenerateDbcTablesSource({ b, a }, "");
    REQUIRE(result.errors.empty());

    CHECK(result.source.find("g_dbcFields_Alpha") < result.source.find("g_dbcFields_Bravo"));
}

TEST_CASE("GenerateDbcTablesSource flags an int field with no size", "[DbcGenerator]")
{
    DbdTable table;
    table.name = "BadInt";
    table.fields = { MakeField("ID", "int", /*bits*/ 0) };

    DbcGeneratorResult result = GenerateDbcTablesSource({ table }, "");

    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].table == "BadInt");
    CHECK(result.errors[0].field == "ID");
    CHECK(result.source.empty());
}

TEST_CASE("GenerateDbcTablesSource takes the vanilla set's names and prefix", "[DbcGenerator]")
{
    DbdTable table = ResolveFixture("Grammar.dbd", "Grammar", "1.0.0.150");

    DbcGeneratorNames names;
    names.sqlTablePrefix = "vdbc_";
    names.getTablesFunction = "GetVanillaDbcTables";
    names.findTableFunction = "FindVanillaDbcTable";
    names.emitWowDbDefsCommitFunction = false;

    DbcGeneratorResult result = GenerateDbcTablesSource({ table }, "deadbeef", names);
    REQUIRE(result.errors.empty());

    CHECK(result.source.find("\"vdbc_grammar\"") != std::string::npos);
    CHECK(result.source.find("std::span<DbcTableMeta const> GetVanillaDbcTables()") != std::string::npos);
    CHECK(result.source.find("DbcTableMeta const* FindVanillaDbcTable(std::string_view name)") != std::string::npos);
    CHECK(result.source.find("GetWowDbDefsCommit") == std::string::npos);
    CHECK(result.source.find("\"dbc_grammar\"") == std::string::npos);
}

TEST_CASE("GenerateDbcTablesSource resolves a real fixture at the vanilla build", "[DbcGenerator]")
{
    DbdTable table = ResolveFixture("AreaTable.dbd", "AreaTable", "1.12.1.5875");

    DbcGeneratorNames names;
    names.sqlTablePrefix = "vdbc_";
    names.getTablesFunction = "GetVanillaDbcTables";
    names.findTableFunction = "FindVanillaDbcTable";
    names.emitWowDbDefsCommitFunction = false;

    DbcGeneratorResult result = GenerateDbcTablesSource({ table }, "deadbeef", names);
    REQUIRE(result.errors.empty());

    CHECK(result.source.find(
              "{ \"AreaTable\", \"areatable\", \"vdbc_areatable\", nullptr, 0u, g_dbcFields_AreaTable, 14u },") !=
          std::string::npos);
    CHECK(result.source.find(
              "{ \"LiquidTypeID\", DbcFieldType::Int, 32, true, 4, false, false, false, \"LiquidType\", \"ID\" },") !=
          std::string::npos);
}

TEST_CASE("GenerateDbcTablesSource flags a flattened column collision", "[DbcGenerator]")
{
    DbdTable table;
    table.name = "Collide";
    table.fields = { MakeField("Value1", "int"), MakeField("Value", "int", 32, 2) };

    DbcGeneratorResult result = GenerateDbcTablesSource({ table }, "");

    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].table == "Collide");
    CHECK(result.errors[0].message.find("Value1") != std::string::npos);
}
