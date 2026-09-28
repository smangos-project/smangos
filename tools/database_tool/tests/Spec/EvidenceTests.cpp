#include "Spec/Evidence.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_evidence_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir() { std::filesystem::remove_all(path); }
    };

    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_evidence_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };

    void WriteFile(std::filesystem::path const& path, std::string const& content)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path);
        out << content;
    }

    // A small fake mangosd source tree exercising all three readers paths (4.1): a loaded DBC
    // (Widget), an unlinked DBC fmt (Unused), an SQLStorage table (addon) and two hand-written
    // loaders (a clean one and a `SELECT *` one).
    void MakeFixtureRepo(std::filesystem::path const& root)
    {
        WriteFile(root / "src" / "game" / "Database" / "DBCfmt.h",
            "char const WidgetEntryfmt[]=\"ni\";\n"
            "char const UnusedEntryfmt[]=\"ni\";\n");

        WriteFile(root / "src" / "game" / "Database" / "DBCStores.cpp",
            "#include \"DBCfmt.h\"\n"
            "DBCStorage <WidgetEntry> sWidgetStore(WidgetEntryfmt);\n"
            "void LoadDBCStores(std::string const& dataPath)\n"
            "{\n"
            "    LoadDBC(locales, bar, errlist, sWidgetStore, dbcPath, \"Widget.dbc\");\n"
            "}\n");

        WriteFile(root / "src" / "game" / "Database" / "DBCStructure.h",
            "struct WidgetEntry\n"
            "{\n"
            "    uint32 id;\n"
            "    uint32 value;\n"
            "};\n");

        WriteFile(root / "src" / "game" / "Database" / "SQLStorages.cpp",
            "char const AddonEntryfmt[] = \"ii\";\n"
            "SQLStorage sAddonStore(AddonEntryfmt, \"id\", \"addon\");\n");

        WriteFile(root / "src" / "game" / "AddonDefs.h",
            "struct AddonEntry\n"
            "{\n"
            "    uint32 id;\n"
            "    uint32 flags;\n"
            "};\n");

        WriteFile(root / "src" / "game" / "AddonUser.cpp",
            "void UseAddon()\n"
            "{\n"
            "    AddonEntry const* a = sAddonStore.LookupEntry<AddonEntry>(1);\n"
            "    DoSomething(a->flags);\n"
            // AddonEntry has no "value" member; this exercises the name-only usage index's
            // documented collision noise -- it's what gives WidgetEntry::value its one use.
            "    DoSomethingElse(a->value);\n"
            "}\n");

        WriteFile(root / "src" / "game" / "ManualLoader.cpp",
            "void LoadManual()\n"
            "{\n"
            "    result = WorldDatabase.Query(\"SELECT `x`, `y` FROM `manual_table`\");\n"
            "    Field* fields = result->Fetch();\n"
            "    data.x = fields[0].GetUInt32();\n"
            "}\n"
            "void LoadStar()\n"
            "{\n"
            "    result = WorldDatabase.Query(\"SELECT * FROM `star_table`\");\n"
            "}\n");
    }

    void MakeFixtureReference(DoltLiteDatabase& db)
    {
        db.Execute("CREATE TABLE \"vdbc_widget\" (\"ID\" INTEGER, \"Value\" INTEGER)");
        db.Execute("CREATE TABLE \"vdbc_unused\" (\"ID\" INTEGER, \"Value\" INTEGER)");
        db.Execute("CREATE TABLE \"srv_addon\" (\"id\" INTEGER, \"flags\" INTEGER)");
        db.Execute("CREATE TABLE \"srv_manual_table\" (\"x\" INTEGER, \"y\" INTEGER)");
        db.Execute("CREATE TABLE \"srv_star_table\" (\"a\" INTEGER)");
    }

    SpecEvidenceRow const& FindRow(std::vector<SpecEvidenceRow> const& rows, std::string const& source, std::string const& table, std::string const& column)
    {
        auto it = std::find_if(rows.begin(), rows.end(), [&](SpecEvidenceRow const& row)
        { return row.source == source && row.table == table && row.column == column; });
        REQUIRE(it != rows.end());
        return *it;
    }
}

TEST_CASE("BuildEvidence errors when the reference database doesn't exist", "[Evidence]")
{
    TempDir specDir("missing_reference");
    SpecEvidenceResult result = BuildEvidence(specDir.path / "does-not-exist.doltlite", specDir.path, specDir.path);
    CHECK_FALSE(result.error.empty());
}

TEST_CASE("BuildEvidence resolves DBC, SQLStorage and hand-written-loader readers", "[Evidence]")
{
    TempDir repo("repo");
    MakeFixtureRepo(repo.path);

    TempDoltLiteFile reference("basic");
    {
        DoltLiteDatabase db(reference.path.string());
        MakeFixtureReference(db);
    }
    TempDir specDir("basic");

    SpecEvidenceResult result = BuildEvidence(reference.path, repo.path, specDir.path);
    CHECK(result.error.empty());
    CHECK(result.rowsWritten == 9); // 2+2 vdbc + 2+2+1 srv

    std::vector<SpecEvidenceRow> rows = ReadEvidenceTsv(specDir.path / "evidence.tsv");
    REQUIRE(rows.size() == 9);

    // Widget.dbc is loaded into WidgetEntry; "value" is read elsewhere, "id" never is.
    CHECK(FindRow(rows, "vdbc", "widget", "ID").readers == "none");
    CHECK(FindRow(rows, "vdbc", "widget", "Value").readers == "WidgetEntry::value (1 uses in 1 files)");

    // UnusedEntryfmt has no DBCStorage/LoadDBC link at all.
    CHECK(FindRow(rows, "vdbc", "unused", "ID").readers == "none");
    CHECK(FindRow(rows, "vdbc", "unused", "Value").readers == "none");

    // addon is SQLStorage-loaded into AddonEntry; "flags" is read, "id" never is.
    CHECK(FindRow(rows, "srv", "addon", "id").readers == "none");
    CHECK(FindRow(rows, "srv", "addon", "flags").readers == "AddonEntry::flags (1 uses in 1 files)");

    // manual_table: a plain SELECT list, x is read via fields[0], y never is.
    std::string xReaders = FindRow(rows, "srv", "manual_table", "x").readers;
    CHECK(xReaders.rfind("manual:", 0) == 0);
    CHECK(xReaders.find("ManualLoader.cpp:5") != std::string::npos);
    CHECK(FindRow(rows, "srv", "manual_table", "y").readers == "none");

    // star_table: `SELECT *` isn't parsable, so every column gets a whole-table manual:.
    std::string starReaders = FindRow(rows, "srv", "star_table", "a").readers;
    CHECK(starReaders.rfind("manual:", 0) == 0);
    CHECK(starReaders.find("ManualLoader.cpp:9") != std::string::npos);

    for (SpecEvidenceRow const& row : rows)
    {
        CHECK(row.wireHint.empty());
        CHECK(row.counterpart.empty());
        CHECK(row.counterpartRule.empty());
        CHECK(row.matchRate.empty());
        CHECK(row.shape.empty());
    }
}

TEST_CASE("BuildEvidence is deterministic on rerun", "[Evidence]")
{
    TempDir repo("repo_rerun");
    MakeFixtureRepo(repo.path);

    TempDoltLiteFile reference("rerun");
    {
        DoltLiteDatabase db(reference.path.string());
        MakeFixtureReference(db);
    }
    TempDir specDir("rerun");

    BuildEvidence(reference.path, repo.path, specDir.path);
    std::vector<SpecEvidenceRow> first = ReadEvidenceTsv(specDir.path / "evidence.tsv");

    BuildEvidence(reference.path, repo.path, specDir.path);
    std::vector<SpecEvidenceRow> second = ReadEvidenceTsv(specDir.path / "evidence.tsv");

    REQUIRE(first.size() == second.size());
    for (size_t i = 0; i < first.size(); ++i)
    {
        CHECK(first[i].table == second[i].table);
        CHECK(first[i].column == second[i].column);
        CHECK(first[i].readers == second[i].readers);
    }
}

// Opt-in: runs the real scan against this repo's own src/game and src/scripts, against the
// real reference database (built locally by `database_tool reference`, gitignored like the
// rest of .world/). Skips when that database doesn't exist, e.g. in CI.
TEST_CASE("BuildEvidence against the real reference database and source tree", "[Evidence][real]")
{
#ifndef DATABASE_TOOL_TEST_REPO_ROOT_DIR
    WARN("DATABASE_TOOL_TEST_REPO_ROOT_DIR not configured, skipping");
#else
    std::filesystem::path repoRoot = DATABASE_TOOL_TEST_REPO_ROOT_DIR;
    std::filesystem::path referencePath = repoRoot / ".world" / "reference.doltlite";
    if (!std::filesystem::exists(referencePath))
    {
        WARN(".world/reference.doltlite not found, skipping (run `database_tool reference` first)");
        return;
    }

    TempDir specDir("real");
    SpecEvidenceResult result = BuildEvidence(referencePath, repoRoot, specDir.path);
    CHECK(result.error.empty());
    CHECK(result.rowsWritten > 0);

    std::vector<SpecEvidenceRow> rows = ReadEvidenceTsv(specDir.path / "evidence.tsv");
    REQUIRE(rows.size() == result.rowsWritten);

    // AreaTableEntryfmt has no DBCStorage/LoadDBC link (phase0-plan.md 4.1): every column of
    // its vdbc table is unread.
    bool sawAreaTableColumn = false;
    for (SpecEvidenceRow const& row : rows)
    {
        if (row.source == "vdbc" && row.table == "areatable")
        {
            sawAreaTableColumn = true;
            CHECK(row.readers == "none");
        }
    }
    CHECK(sawAreaTableColumn);

    // WMOAreaTable is loaded, and its areaId field (SQL column AreaTableID) is read elsewhere
    // (GetWMOAreaTableEntryByTripple).
    bool sawWmoAreaId = false;
    for (SpecEvidenceRow const& row : rows)
    {
        if (row.source == "vdbc" && row.table == "wmoareatable" && row.column == "AreaTableID")
        {
            sawWmoAreaId = true;
            CHECK(row.readers != "none");
            CHECK(row.readers.rfind("manual:", 0) != 0);
            CHECK(row.readers.rfind("WMOAreaTableEntry::areaId", 0) == 0);
        }
    }
    CHECK(sawWmoAreaId);

    // Row count matches columns.tsv's srv+vdbc universe (phase0-plan.md 3.1: same universe).
    std::vector<SpecColumnRow> columns = ReadColumnsTsv(repoRoot / "tools" / "database_tool" / "Spec" / "columns.tsv");
    size_t srvVdbcCount = std::count_if(columns.begin(), columns.end(), [](SpecColumnRow const& row)
    { return row.source == "srv" || row.source == "vdbc"; });
    CHECK(rows.size() == srvVdbcCount);
#endif
}
