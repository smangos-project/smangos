#include "Commands/GenerateWms.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

TEST_CASE("generate-wms: names are PascalCased from snake_case and plain lowercase", "[generate-wms]")
{
    CHECK(WmsPascalCase("dbc_areatable") == "DbcAreatable");
    CHECK(WmsPascalCase("spawn_flags") == "SpawnFlags");
    CHECK(WmsPascalCase("MapName_lang") == "MapNameLang");
    CHECK(WmsPascalCase("1st") == "_1st");
}

TEST_CASE("generate-wms: integer columns widen only past the int range", "[generate-wms]")
{
    CHECK(WmsIntegerType(0, 2147483647) == "int");
    CHECK(WmsIntegerType(-5, 10) == "int");
    CHECK(WmsIntegerType(0, 4294967295LL) == "long");
}

TEST_CASE("generate-wms: tables read their key, types and nullability", "[generate-wms]")
{
    auto const path = std::filesystem::temp_directory_path() / "generate_wms_test.doltlite";
    std::filesystem::remove(path);
    {
        DoltLiteDatabase db(path.string());
        db.Execute("CREATE TABLE t_one (entry INTEGER NOT NULL, name TEXT NOT NULL DEFAULT '', x REAL, big INTEGER NOT NULL, PRIMARY KEY (entry))");
        db.Execute("CREATE TABLE t_two (a INTEGER NOT NULL, b INTEGER NOT NULL, PRIMARY KEY (b, a))");
        db.Execute("CREATE TABLE wms_hidden (id INTEGER PRIMARY KEY)");
        db.Execute("INSERT INTO t_one VALUES (1, 'a', NULL, 4294967295)");

        std::vector<WmsTable> const tables = ReadWmsTables(db);
        REQUIRE(tables.size() == 2);
        CHECK(tables[0].record == "TOneRecord");
        CHECK(tables[0].columns[0].clrType == "int");
        CHECK(tables[0].columns[2].clrType == "float");
        CHECK(!tables[0].columns[2].notNull);
        CHECK(tables[0].columns[3].clrType == "long");

        std::string const source = RenderWmsTable(tables[1]);
        CHECK(source.find("[PrimaryKey(nameof(B), nameof(A))]") != std::string::npos);
        std::string const one = RenderWmsTable(tables[0]);
        CHECK(one.find("public float? X { get; set; }") != std::string::npos);
        CHECK(one.find("public string Name { get; set; } = string.Empty;") != std::string::npos);
    }
    std::filesystem::remove(path);
}
