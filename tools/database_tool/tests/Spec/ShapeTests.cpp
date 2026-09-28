#include "Spec/Shape.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

namespace
{
    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_shape_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };
}

TEST_CASE("ComputeShape reports row/distinct/null counts", "[Shape]")
{
    TempDoltLiteFile reference("counts");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"name\" TEXT)");
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (1, 'a'), (2, NULL), (3, '')");

    ComputeShapeResult result = ComputeShape(db);

    auto entryIt = result.shapes.find("srv\tarea_template\tentry");
    REQUIRE(entryIt != result.shapes.end());
    CHECK(entryIt->second == "rows=3 distinct=3 null=0");

    auto nameIt = result.shapes.find("srv\tarea_template\tname");
    REQUIRE(nameIt != result.shapes.end());
    CHECK(nameIt->second == "rows=3 distinct=2 null=2");
}

TEST_CASE("ComputeShape records the value when a column is constant", "[Shape]")
{
    TempDoltLiteFile reference("constant");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"vdbc_widget\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
    db.Execute("INSERT INTO \"vdbc_widget\" VALUES (1, 7), (2, 7), (3, 7)");

    ComputeShapeResult result = ComputeShape(db);

    auto it = result.shapes.find("vdbc\twidget\tFlags");
    REQUIRE(it != result.shapes.end());
    CHECK(it->second == "rows=3 distinct=1 null=0 const=7");
}

TEST_CASE("ComputeShape finds the effect_base_dice_from_die_sides formula", "[Shape]")
{
    TempDoltLiteFile reference("dice");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_spell_template\" (\"entry\" INTEGER, \"effectBaseDice1\" INTEGER, \"effectDieSides1\" INTEGER)");
    // 3 rows agree with baseDice = dieSides ? 1 : 0, 1 row (id 4) doesn't.
    db.Execute("INSERT INTO \"srv_spell_template\" VALUES (1, 0, 0), (2, 1, 3), (3, 1, 1), (4, 0, 2)");

    ComputeShapeResult result = ComputeShape(db);

    auto it = result.shapes.find("srv\tspell_template\teffectBaseDice1");
    REQUIRE(it != result.shapes.end());
    CHECK(it->second.find("formula=effect_base_dice_from_die_sides:3/4") != std::string::npos);
}

TEST_CASE("ComputeShape finds the best-matching sibling array slot", "[Shape]")
{
    TempDoltLiteFile reference("siblings");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_spell_template\" (\"entry\" INTEGER, "
               "\"effectRadiusIndex1\" INTEGER, \"effectRadiusIndex2\" INTEGER, \"effectRadiusIndex3\" INTEGER)");
    // radius2 equals radius1 in 3/3 rows, radius3 in 0/3 -- radius1 should win.
    db.Execute("INSERT INTO \"srv_spell_template\" VALUES (1, 5, 5, 9), (2, 6, 6, 9), (3, 7, 7, 9)");

    ComputeShapeResult result = ComputeShape(db);

    auto it = result.shapes.find("srv\tspell_template\teffectRadiusIndex2");
    REQUIRE(it != result.shapes.end());
    CHECK(it->second.find("formula=equals_effectRadiusIndex1:3/3") != std::string::npos);
}

TEST_CASE("ComputeShape leaves a plain column with no formula hint", "[Shape]")
{
    TempDoltLiteFile reference("noformula");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (1, 5), (2, 6)");

    ComputeShapeResult result = ComputeShape(db);

    auto it = result.shapes.find("srv\tarea_template\tflags");
    REQUIRE(it != result.shapes.end());
    CHECK(it->second.find("formula=") == std::string::npos);
}

TEST_CASE("ComputeShape is empty for a table with no rows", "[Shape]")
{
    TempDoltLiteFile reference("emptytable");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");

    ComputeShapeResult result = ComputeShape(db);

    auto it = result.shapes.find("srv\tarea_template\tentry");
    REQUIRE(it != result.shapes.end());
    CHECK(it->second == "rows=0 distinct=0 null=0");
}
