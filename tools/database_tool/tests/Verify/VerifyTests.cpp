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

    VerifyInitResult init = InitVerifyMappings(referencePath, dir.path, verifyDir);
    REQUIRE(init.error.empty());
    CHECK(init.written == 2);
    CHECK(ReadVerifyMapping(verifyDir / "srv" / "other.sql").key == std::vector<std::string>{ "id" });

    WriteVerifyTestFile(verifyDir / "srv" / "thing.sql", "-- key: id\nSELECT ID AS id, Name AS name FROM dbc_thing\n");

    VerifyResult undropped = RunVerify(worldPath, referencePath, dir.path, verifyDir, { false, false, { "V1" }, {} });
    REQUIRE(undropped.error.empty());
    REQUIRE(undropped.problems.size() == 1);
    CHECK(undropped.problems[0].table == "srv_thing");
    CHECK(undropped.problems[0].message == "column junk is neither mapped nor dropped");

    WriteVerifyTestFile(dir.path / "columns.tsv", "source\ttable\tcolumn\tdisposition\ttarget\trule\treason\treviewed\n"
                                                  "srv\tthing\tjunk\tdrop\t\t\tno reader\tyes\n");
    VerifyResult dropped = RunVerify(worldPath, referencePath, dir.path, verifyDir, { false, false, { "V1" }, { "thing" } });
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
    VerifyResult result = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    REQUIRE(result.error.empty());
    REQUIRE(result.problems.size() == 1);
    CHECK(result.problems[0].table == "dbc_map");
    CHECK(result.problems[0].message == "0 inserted, 0 changed, 1 removed; client-delta.tsv says 0, 0, 0");

    {
        DoltLiteDatabase world(worldPath.string());
        world.Execute("INSERT INTO dbc_map (ID, Directory) VALUES (1, 'x'), (2, 'y')");
        world.Execute("INSERT INTO dbc_chrraces (ID) VALUES (3)");
    }
    options.update = true;
    VerifyResult updated = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    CHECK(updated.error.empty());
    CHECK(updated.problems.empty());

    std::ifstream in(verifyDir / "client-delta.tsv");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(content == "table\tinserted\tchanged\tremoved\ndbc_chrraces\t1\t0\t0\ndbc_map\t1\t1\t0\n");

    options.update = false;
    VerifyResult rechecked = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    CHECK(rechecked.error.empty());
    CHECK(rechecked.problems.empty());
}

TEST_CASE("V2 rebuilds vanilla tables through the columns.tsv mapping, up to allow.tsv", "[Verify]")
{
    VerifyTempDir dir("v2");
    std::filesystem::path worldPath = dir.path / "world.doltlite";
    std::filesystem::path referencePath = dir.path / "reference.doltlite";
    std::filesystem::path verifyDir = dir.path / "Verify";
    {
        DoltLiteDatabase reference(referencePath.string());
        reference.Execute(BuildDbcTableDdl(*FindVanillaDbcTable("SpellCastTimes"), "vdbc_spellcasttimes", 9));
        reference.Execute("INSERT INTO vdbc_spellcasttimes VALUES (1, 0, 0, 0), (2, 250, 0, 250), (3, 500, 0, 500)");
    }
    {
        DoltLiteDatabase world(worldPath.string());
        world.Execute(BuildDbcTableDdl(*FindDbcTable("SpellCastTimes")));
        world.Execute("INSERT INTO dbc_spellcasttimes VALUES (1, 0, 0, 0), (2, 250, 0, 300), (4, 1, 0, 1)");
    }
    WriteVerifyTestFile(dir.path / "rules.tsv", "name\tkind\texpr\tinverse\tapplies_to\nidentity\tvalue\t{v}\t{v}\t\n");
    WriteVerifyTestFile(dir.path / "columns.tsv",
                        "source\ttable\tcolumn\tdisposition\ttarget\trule\treason\treviewed\n"
                        "vdbc\tspellcasttimes\tID\tdbc\tdbc_spellcasttimes.ID\tidentity\t\tyes\n"
                        "vdbc\tspellcasttimes\tBase\tdbc\tdbc_spellcasttimes.Base\tidentity\t\tyes\n"
                        "vdbc\tspellcasttimes\tPerLevel\tconstant\t0\t\t\tyes\n"
                        "vdbc\tspellcasttimes\tMinimum\tdbc\tdbc_spellcasttimes.Minimum\tidentity\t\tyes\n");

    VerifyOptions options{ false, false, { "V2" }, {} };
    VerifyResult unmapped = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    CHECK(unmapped.problems.empty());
    CHECK(unmapped.notes == std::vector<std::string>{ "V2: 1 vanilla tables not mapped yet: SpellCastTimes" });

    VerifyInitResult init = InitVerifyMappings(referencePath, dir.path, verifyDir, { "all" });
    REQUIRE(init.error.empty());
    CHECK(init.written == 1);
    CHECK(ReadVerifyMapping(verifyDir / "vdbc" / "SpellCastTimes.sql").select ==
          "SELECT d.\"ID\" AS \"ID\",\n       d.\"Base\" AS \"Base\",\n       0 AS \"PerLevel\",\n"
          "       d.\"Minimum\" AS \"Minimum\"\nFROM \"dbc_spellcasttimes\" d");

    VerifyResult mapped = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    REQUIRE(mapped.problems.size() == 2);
    CHECK(mapped.problems[0].message == "1 of 3 rows missing, first ID=3");
    CHECK(mapped.problems[1].message == "Minimum differs in 1 rows, first ID=2: original 250, rebuilt 300");

    WriteVerifyTestFile(verifyDir / "vdbc" / "allow.tsv", "table\tcolumn\trows\treason\n"
                                                          "SpellCastTimes\t(missing)\t1\tvmangos fix\n"
                                                          "SpellCastTimes\tMinimum\t2\tvmangos fix\n");
    VerifyResult allowed = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    REQUIRE(allowed.problems.size() == 1);
    CHECK(allowed.problems[0].message ==
          "Minimum differs in 1 rows, first ID=2: original 250, rebuilt 300 (allow.tsv allows 2)");
}

TEST_CASE("V4 counts dangling client foreign keys and declared server references", "[Verify]")
{
    VerifyTempDir dir("v4");
    std::filesystem::path worldPath = dir.path / "world.doltlite";
    std::filesystem::path referencePath = dir.path / "reference.doltlite";
    std::filesystem::path verifyDir = dir.path / "Verify";
    DoltLiteDatabase(referencePath.string()).Execute("CREATE TABLE unused (x INTEGER)");
    {
        DoltLiteDatabase world(worldPath.string());
        REQUIRE(CreateDbcTables(world, "dbc tables").error.empty());
        world.Execute("INSERT INTO dbc_taxinodes (ID) VALUES (1)");
        world.Execute("INSERT INTO dbc_taxipath (ID, FromTaxiNode, ToTaxiNode) VALUES (1, 1, 0), (2, 5, 0)");
        world.Execute("CREATE TABLE thing (id INTEGER, node INTEGER, kind INTEGER)");
        world.Execute("INSERT INTO thing VALUES (1, 1, 0), (2, 7, 0), (3, 8, 1)");
    }
    WriteVerifyTestFile(verifyDir / "references.tsv", "table\tcolumn\ttarget\twhere\tdangling\treason\n"
                                                      "thing\tnode\tdbc_taxinodes.ID\tkind = 0\t0\ttest\n");

    VerifyOptions options{ false, false, { "V4" }, { "dbc_taxipath", "thing" } };
    VerifyResult result = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    REQUIRE(result.error.empty());
    REQUIRE(result.problems.size() == 2);
    CHECK(result.problems[0].message == "FromTaxiNode -> dbc_taxinodes.ID: 1 values don't resolve; references.tsv says 0");
    CHECK(result.problems[1].message == "node -> dbc_taxinodes.ID: 1 values don't resolve; references.tsv says 0");

    options.update = true;
    VerifyResult updated = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    CHECK(updated.error.empty());
    CHECK(updated.problems.empty());
    options.update = false;
    VerifyResult rechecked = RunVerify(worldPath, referencePath, dir.path, verifyDir, options);
    CHECK(rechecked.error.empty());
    CHECK(rechecked.problems.empty());

    std::ifstream in(verifyDir / "references.tsv");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(content == "table\tcolumn\ttarget\twhere\tdangling\treason\n"
                     "dbc_taxipath\tFromTaxiNode\tdbc_taxinodes.ID\t\t1\t\n"
                     "thing\tnode\tdbc_taxinodes.ID\tkind = 0\t1\ttest\n");
}
