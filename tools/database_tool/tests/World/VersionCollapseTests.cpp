#include "World/VersionCollapse.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

namespace
{
    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_versioncollapse_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };

    // Every column left untyped is enough for these tests; SQLite/DoltLite stores whatever
    // affinity the inserted value has.
    void MakeDestTable(DoltLiteDatabase& db, std::string const& name, std::vector<std::string> const& columns)
    {
        std::string ddl = "CREATE TABLE \"" + name + "\" (";
        for (size_t i = 0; i < columns.size(); ++i)
            ddl += (i ? ", \"" : "\"") + columns[i] + "\"";
        db.Execute(ddl + ")");
    }
}

TEST_CASE("DetectVersionColumn recognizes build, patch, a patch range, a build range, and none", "[VersionCollapse]")
{
    CHECK(DetectVersionColumn({ "entry", "build", "name" }).kind == VersionColumnKind::Build);
    CHECK(DetectVersionColumn({ "entry", "patch", "name" }).kind == VersionColumnKind::Patch);

    VersionColumnInfo range = DetectVersionColumn({ "guid", "patch_min", "patch_max" });
    CHECK(range.kind == VersionColumnKind::PatchRange);
    CHECK(range.minColumn == "patch_min");
    CHECK(range.maxColumn == "patch_max");

    VersionColumnInfo buildRange = DetectVersionColumn({ "entry", "build_min", "build_max" });
    CHECK(buildRange.kind == VersionColumnKind::BuildRange);
    CHECK(buildRange.minColumn == "build_min");
    CHECK(buildRange.maxColumn == "build_max");

    CHECK(DetectVersionColumn({ "entry", "name" }).kind == VersionColumnKind::None);
}

TEST_CASE("CollapseTableInto keeps the greatest build <= target per key", "[VersionCollapse]")
{
    TempDoltLiteFile file("build");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"entry\" INTEGER, \"build\" INTEGER, \"name\" TEXT)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 5000, 'old'), (1, 5875, 'vanilla'), (1, 6005, 'tbc')");
    MakeDestTable(db, "dst", { "entry", "build", "name" });

    VersionColumnInfo info = DetectVersionColumn({ "entry", "build", "name" });
    CollapseReport report = CollapseTableInto(db, "src", "dst", { "entry", "build", "name" }, { "entry" }, info);

    CHECK(report.rowsBefore == 3);
    CHECK(report.rowsAfter == 1);
    CHECK(report.distinctKeysBefore == 1);
    CHECK(report.keysWithNoRow == 0);

    DoltLiteStatement row = db.Prepare("SELECT \"name\" FROM \"dst\"");
    REQUIRE(row.Step());
    CHECK(row.ColumnText(0) == "vanilla");
}

TEST_CASE("CollapseTableInto drops a key with no row at or below the target build", "[VersionCollapse]")
{
    TempDoltLiteFile file("no_row");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"entry\" INTEGER, \"build\" INTEGER)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 5875), (2, 6005), (2, 8000)");
    MakeDestTable(db, "dst", { "entry", "build" });

    VersionColumnInfo info = DetectVersionColumn({ "entry", "build" });
    CollapseReport report = CollapseTableInto(db, "src", "dst", { "entry", "build" }, { "entry" }, info);

    CHECK(report.rowsBefore == 3);
    CHECK(report.distinctKeysBefore == 2);
    CHECK(report.rowsAfter == 1); // only entry 1 has a row <= 5875
    CHECK(report.keysWithNoRow == 1); // entry 2's rows are both above the target
}

TEST_CASE("CollapseTableInto breaks a tie between two rows at the same version", "[VersionCollapse]")
{
    TempDoltLiteFile file("tie");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"entry\" INTEGER, \"build\" INTEGER, \"name\" TEXT)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 5875, 'first'), (1, 5875, 'second')");
    MakeDestTable(db, "dst", { "entry", "build", "name" });

    VersionColumnInfo info = DetectVersionColumn({ "entry", "build", "name" });
    CollapseReport report = CollapseTableInto(db, "src", "dst", { "entry", "build", "name" }, { "entry" }, info);

    CHECK(report.rowsBefore == 2);
    CHECK(report.distinctKeysBefore == 1);
    CHECK(report.rowsAfter == 1); // a tie still yields exactly one row, not two
    CHECK(report.keysWithNoRow == 0);
}

TEST_CASE("CollapseTableInto filters a patch_min/patch_max table without deduplicating", "[VersionCollapse]")
{
    TempDoltLiteFile file("range");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"guid\" INTEGER, \"patch_min\" INTEGER, \"patch_max\" INTEGER)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 0, 10), (2, 0, 6), (3, 8, 10)");
    MakeDestTable(db, "dst", { "guid", "patch_min", "patch_max" });

    VersionColumnInfo info = DetectVersionColumn({ "guid", "patch_min", "patch_max" });
    CollapseReport report =
        CollapseTableInto(db, "src", "dst", { "guid", "patch_min", "patch_max" }, { "guid" }, info);

    CHECK(report.rowsBefore == 3);
    CHECK(report.rowsAfter == 2); // guid 2's range (0-6) excludes patch 10
    CHECK(report.keysWithNoRow == 1);
}

TEST_CASE("CollapseTableInto filters a build_min/build_max table without deduplicating", "[VersionCollapse]")
{
    TempDoltLiteFile file("build_range");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"entry\" INTEGER, \"build_min\" INTEGER, \"build_max\" INTEGER)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 0, 5875), (2, 0, 5000), (3, 6000, 9999)");
    MakeDestTable(db, "dst", { "entry", "build_min", "build_max" });

    VersionColumnInfo info = DetectVersionColumn({ "entry", "build_min", "build_max" });
    CollapseReport report =
        CollapseTableInto(db, "src", "dst", { "entry", "build_min", "build_max" }, { "entry" }, info);

    CHECK(report.rowsBefore == 3);
    CHECK(report.rowsAfter == 1); // only entry 1's range (0-5875) covers build 5875
    CHECK(report.keysWithNoRow == 2);
}

TEST_CASE("CollapseTableInto copies a table with no version column as-is", "[VersionCollapse]")
{
    TempDoltLiteFile file("none");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE \"src\" (\"entry\" INTEGER, \"name\" TEXT)");
    db.Execute("INSERT INTO \"src\" VALUES (1, 'a'), (2, 'b')");
    MakeDestTable(db, "dst", { "entry", "name" });

    VersionColumnInfo info = DetectVersionColumn({ "entry", "name" });
    CHECK(info.kind == VersionColumnKind::None);
    CollapseReport report = CollapseTableInto(db, "src", "dst", { "entry", "name" }, { "entry" }, info);

    CHECK(report.rowsBefore == 2);
    CHECK(report.rowsAfter == 2);
    CHECK(report.keysWithNoRow == 0);
}
