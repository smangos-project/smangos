#include "Verify/RowCompare.h"
#include "Verify/Verify.h"

#include "Dbc/DbcSchema.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>

struct VerifyTempDir
{
    std::filesystem::path path;

    explicit VerifyTempDir(std::string const& name)
        : path(std::filesystem::temp_directory_path() / ("database_tool_test_verify_" + name))
    {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }

    ~VerifyTempDir() { std::filesystem::remove_all(path); }
};

static void WriteVerifyTestFile(std::filesystem::path const& path, std::string const& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << content;
}

TEST_CASE("CompareRowSets matches rows by key and reports each differing column", "[Verify]")
{
    VerifyTempDir dir("compare");
    DoltLiteDatabase db((dir.path / "db.doltlite").string());
    db.Execute("CREATE TABLE a (id INTEGER, n INTEGER, f REAL, t TEXT)");
    db.Execute("CREATE TABLE b (id INTEGER, n INTEGER, f REAL, t TEXT)");
    db.Execute("INSERT INTO a VALUES (1, 5, 0.1, 'x'), (2, 7, 2.0, 'y'), (3, NULL, 1.0, NULL), (4, 1, 1.0, 'q')");
    db.Execute("INSERT INTO b VALUES (1, 5, 0.100000001, 'x'), (2, 8, 2, 'Y'), (3, NULL, 1.0, NULL), (5, 1, 1.0, 'q')");

    RowCompareResult result = CompareRowSets(db, "SELECT * FROM a", "SELECT * FROM b", { 0 });
    CHECK(result.expectedRows == 4);
    CHECK(result.actualRows == 4);
    CHECK(result.missingRows == 1);
    CHECK(result.firstMissingKey == "id=4");
    CHECK(result.extraRows == 1);
    CHECK(result.firstExtraKey == "id=5");
    REQUIRE(result.differences.size() == 2);
    CHECK(result.differences[0].column == "n");
    CHECK(result.differences[0].rows == 1);
    CHECK(result.differences[0].expected == "7");
    CHECK(result.differences[0].actual == "8");
    CHECK(result.differences[1].column == "t");
    CHECK(result.differences[1].actual == "'Y'");
}

TEST_CASE("CompareRowSets pairs duplicate keys and compares keyless tables as multisets", "[Verify]")
{
    VerifyTempDir dir("multiset");
    DoltLiteDatabase db((dir.path / "db.doltlite").string());
    db.Execute("CREATE TABLE a (k INTEGER, v TEXT)");
    db.Execute("CREATE TABLE b (k INTEGER, v TEXT)");
    db.Execute("INSERT INTO a VALUES (1, 'x'), (1, 'y'), (2, 'z'), (2, 'z')");
    db.Execute("INSERT INTO b VALUES (1, 'y'), (1, 'x'), (2, 'z')");

    RowCompareResult keyed = CompareRowSets(db, "SELECT * FROM a", "SELECT * FROM b", { 0 });
    CHECK(keyed.differences.empty());
    CHECK(keyed.missingRows == 1);

    RowCompareResult keyless = CompareRowSets(db, "SELECT * FROM a", "SELECT * FROM b", {});
    CHECK(keyless.differences.empty());
    CHECK(keyless.missingRows == 1);
    CHECK(keyless.extraRows == 0);
}

TEST_CASE("ReadVerifyMapping reads the key line and the SELECT", "[Verify]")
{
    VerifyTempDir dir("mapping");
    std::filesystem::path path = dir.path / "t.sql";
    WriteVerifyTestFile(path, "-- rebuilds t\n-- key: a, b\nSELECT a,\n       b\nFROM t;\n");

    VerifyMapping mapping = ReadVerifyMapping(path);
    CHECK(mapping.key == std::vector<std::string>{ "a", "b" });
    CHECK(mapping.select == "SELECT a,\n       b\nFROM t");

    WriteVerifyMapping(path, {}, mapping);
    VerifyMapping reread = ReadVerifyMapping(path);
    CHECK(reread.key == mapping.key);
    CHECK(reread.select == mapping.select);
}

TEST_CASE("V1 rebuilds each srv_ table from its mapping and honours dropped columns", "[Verify]")
{
    VerifyTempDir dir("v1");
    std::filesystem::path worldPath = dir.path / "world.doltlite";
    std::filesystem::path referencePath = dir.path / "reference.doltlite";
    std::filesystem::path verifyDir = dir.path / "Verify";
    {
        DoltLiteDatabase reference(referencePath.string());
        reference.Execute("CREATE TABLE srv_thing (id INTEGER PRIMARY KEY, name TEXT, junk INTEGER)");
        reference.Execute("INSERT INTO srv_thing VALUES (1, 'a', 0), (2, 'b', 0)");
        reference.Execute("CREATE TABLE srv_other (id INTEGER, patch INTEGER, v INTEGER, PRIMARY KEY(id, patch))");
        reference.Execute("INSERT INTO srv_other VALUES (1, 2, 11)");
    }
    {
        DoltLiteDatabase world(worldPath.string());
        world.Execute("CREATE TABLE dbc_thing (ID INTEGER PRIMARY KEY, Name TEXT)");
        world.Execute("INSERT INTO dbc_thing VALUES (1, 'a'), (2, 'c')");
        world.Execute("CREATE TABLE other (id INTEGER, patch INTEGER, v INTEGER, PRIMARY KEY(id, patch))");
        world.Execute("INSERT INTO other VALUES (1, 0, 10), (1, 2, 11), (1, 11, 12)");
    }

    VerifyInitResult init = InitVerifyMappings(referencePath, verifyDir);
    REQUIRE(init.error.empty());
    CHECK(init.written == 2);
    CHECK(ReadVerifyMapping(verifyDir / "srv" / "other.sql").key == std::vector<std::string>{ "id" });

    WriteVerifyTestFile(verifyDir / "srv" / "thing.sql", "-- key: id\nSELECT ID AS id, Name AS name FROM dbc_thing\n");

    VerifyResult undropped = RunVerify(worldPath, referencePath, verifyDir, { false, false, { "V1" }, {} });
    REQUIRE(undropped.error.empty());
    REQUIRE(undropped.problems.size() == 1);
    CHECK(undropped.problems[0].table == "srv_thing");
    CHECK(undropped.problems[0].message == "column junk is neither mapped nor dropped");

    WriteVerifyTestFile(verifyDir / "srv" / "dropped.tsv", "table\tcolumn\treason\nthing\tjunk\tno reader\n");
    VerifyResult dropped = RunVerify(worldPath, referencePath, verifyDir, { false, false, { "V1" }, { "thing" } });
    REQUIRE(dropped.problems.size() == 1);
    CHECK(dropped.problems[0].message == "name differs in 1 rows, first id=2: original 'b', rebuilt 'c'");
}

TEST_CASE("V3 counts the dbc_ rows that differ from the client against client-delta.tsv", "[Verify]")
{
    VerifyTempDir dir("v3");
    std::filesystem::path worldPath = dir.path / "world.doltlite";
    std::filesystem::path referencePath = dir.path / "reference.doltlite";
    std::filesystem::path verifyDir = dir.path / "Verify";
    {
        DoltLiteDatabase reference(referencePath.string());
        reference.Execute(BuildDbcTableDdl(*FindDbcTable("Map"), "cdbc_map"));
        reference.Execute("INSERT INTO cdbc_map (ID) VALUES (1)");
    }
    {
        DoltLiteDatabase world(worldPath.string());
        REQUIRE(CreateDbcTables(world, "dbc tables").error.empty());
    }

    VerifyOptions options{ false, false, { "V3" }, { "dbc_map", "dbc_chrraces" } };
    VerifyResult result = RunVerify(worldPath, referencePath, verifyDir, options);
    REQUIRE(result.error.empty());
    REQUIRE(result.problems.size() == 1);
    CHECK(result.problems[0].table == "dbc_map");
    CHECK(result.problems[0].message == "0 inserted, 0 changed, 1 removed; client-delta.tsv says 0, 0, 0");

    {
        DoltLiteDatabase world(worldPath.string());
        world.Execute("INSERT INTO dbc_map (ID, Directory) VALUES (1, 'x'), (2, 'y')");
        world.Execute("INSERT INTO dbc_chrraces (ID) VALUES (3)");
    }
    options.updateDelta = true;
    CHECK(RunVerify(worldPath, referencePath, verifyDir, options).problems.empty());

    std::ifstream in(verifyDir / "client-delta.tsv");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(content == "table\tinserted\tchanged\tremoved\ndbc_chrraces\t1\t0\t0\ndbc_map\t1\t1\t0\n");

    options.updateDelta = false;
    CHECK(RunVerify(worldPath, referencePath, verifyDir, options).problems.empty());
}
