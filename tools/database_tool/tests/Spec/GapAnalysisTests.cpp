#include "Spec/GapAnalysis.h"

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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_gaps_" + name))
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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_gaps_" + name + ".doltlite"))
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

    void WritePairsTsvRaw(std::filesystem::path const& specDir, std::string const& rows)
    {
        WriteFile(specDir / "pairs.tsv", "source\ttable\tcdbc_table\tjoin_kind\tsource_key\tcdbc_key\tnotes\n" + rows);
    }

    void WriteRulesTsvRaw(std::filesystem::path const& specDir)
    {
        WriteFile(specDir / "rules.tsv", "name\tkind\texpr\tinverse\tapplies_to\nidentity\tvalue\t{v}\t{v}\tdefault\n");
    }

    void WriteEvidenceTsvRaw(std::filesystem::path const& specDir, std::string const& rows)
    {
        WriteFile(specDir / "evidence.tsv",
            "source\ttable\tcolumn\ttype\treaders\twire_hint\tcounterpart\tcounterpart_rule\tmatch_rate\tshape\n" + rows);
    }

    bool AnyGap(std::vector<SpecGapRow> const& gaps, std::string const& kind, std::string const& table, std::string const& column,
        std::string const& detailSubstring)
    {
        return std::any_of(gaps.begin(), gaps.end(), [&](SpecGapRow const& g)
        {
            return g.kind == kind && g.table == table && g.column == column && g.detail.find(detailSubstring) != std::string::npos;
        });
    }
}

TEST_CASE("BuildGaps reports removed columns and array length changes for an imported vdbc table", "[GapAnalysis]")
{
    TempDir definitions("schema_definitions");
    WriteFile(definitions.path / "Widget.dbd",
        "COLUMNS\n"
        "int ID\n"
        "int Value\n"
        "string Name\n"
        "\n"
        "BUILD 1.12.1.5875\n"
        "$id$ID<32>\n"
        "Value<32>\n"
        "Name\n"
        "\n"
        "BUILD 1.15.6.58797\n"
        "$id$ID<32>\n"
        "Value<32>[2]\n");
    WriteFile(definitions.path / "NotImported.dbd",
        "COLUMNS\n"
        "int ID\n"
        "\n"
        "BUILD 1.12.1.5875\n"
        "$id$ID<32>\n");

    TempDir specDir("schema_spec");
    TempDoltLiteFile reference("schema");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"vdbc_widget\" (\"ID\" INTEGER, \"Value\" INTEGER, \"Name\" TEXT)");

    SpecGapResult result = BuildGaps(reference.path, definitions.path, specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "removed_column", "widget", "Name", "removed"));
    CHECK(AnyGap(gaps, "array_length", "widget", "Value", "1 -> 2"));
    // NotImported has no vdbc_notimported table in the reference database, so it's out of scope.
    CHECK_FALSE(std::any_of(gaps.begin(), gaps.end(), [](SpecGapRow const& g) { return g.table == "notimported"; }));
}

TEST_CASE("BuildGaps reports a table with no 1.15 definition at all as removed_table", "[GapAnalysis]")
{
    TempDir definitions("removed_definitions");
    WriteFile(definitions.path / "Removed.dbd",
        "COLUMNS\n"
        "int ID\n"
        "\n"
        "BUILD 1.12.1.5875\n"
        "$id$ID<32>\n");

    TempDir specDir("removed_spec");
    TempDoltLiteFile reference("removed");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"vdbc_removed\" (\"ID\" INTEGER)");

    SpecGapResult result = BuildGaps(reference.path, definitions.path, specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "removed_table", "removed", "", "no 1.15.6.58797/58725 definition"));
}

TEST_CASE("BuildGaps lists vanilla-only ids missing from a joined cdbc table", "[GapAnalysis]")
{
    TempDir specDir("coverage_spec");
    WritePairsTsvRaw(specDir.path, "vdbc\twidget\twidget\tid\tID\tID\tnote\n");
    WriteRulesTsvRaw(specDir.path);

    TempDoltLiteFile reference("coverage");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"vdbc_widget\" (\"ID\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_widget\" (\"ID\" INTEGER)");
    db.Execute("INSERT INTO \"vdbc_widget\" VALUES (1), (2), (3)");
    db.Execute("INSERT INTO \"cdbc_widget\" VALUES (1), (2)");

    SpecGapResult result = BuildGaps(reference.path, "", specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "row_coverage", "vdbc_widget", "", "vanilla-only ids: 1 (3)"));
}

TEST_CASE("BuildGaps finds a counterpart candidate outside the declared pairs", "[GapAnalysis]")
{
    TempDir specDir("candidate_spec");
    WritePairsTsvRaw(specDir.path, "srv\tarea_template\tareatable\tid\tentry\tID\tnote\n");
    WriteRulesTsvRaw(specDir.path);
    WriteEvidenceTsvRaw(specDir.path, "srv\tarea_template\tflags\tINT\tAreaTemplate::flags (1 uses in 1 files)\t\t\t\t\t\n");

    TempDoltLiteFile reference("candidate");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_areatable\" (\"ID\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_otherplace\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (1, 5), (2, 6), (3, 7)");
    db.Execute("INSERT INTO \"cdbc_areatable\" VALUES (1), (2), (3)");
    db.Execute("INSERT INTO \"cdbc_otherplace\" VALUES (1, 5), (2, 6), (3, 7)");

    SpecGapResult result = BuildGaps(reference.path, "", specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "counterpart_candidate", "srv_area_template", "flags", "cdbc_otherplace.Flags via identity 3/3"));
}

TEST_CASE("BuildGaps does not search a constant column for a counterpart candidate", "[GapAnalysis]")
{
    // A constant (or all-null) column matches any other same-valued constant column at 100% by
    // coincidence -- the empty vanilla locale slots hit this in practice. shape's "distinct=1"
    // should keep it out of the search entirely, not just record a low-confidence hit.
    TempDir specDir("candidate_constant_spec");
    WritePairsTsvRaw(specDir.path, "srv\tarea_template\tareatable\tid\tentry\tID\tnote\n");
    WriteRulesTsvRaw(specDir.path);
    WriteEvidenceTsvRaw(specDir.path,
        "srv\tarea_template\tname\tTEXT\tAreaTemplate::name (1 uses in 1 files)\t\t\t\t\trows=3 distinct=1 null=0 const=\n");

    TempDoltLiteFile reference("candidate_constant");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"name\" TEXT)");
    db.Execute("CREATE TABLE \"cdbc_areatable\" (\"ID\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_otherplace\" (\"ID\" INTEGER, \"Name\" TEXT)");
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (1, ''), (2, ''), (3, '')");
    db.Execute("INSERT INTO \"cdbc_areatable\" VALUES (1), (2), (3)");
    db.Execute("INSERT INTO \"cdbc_otherplace\" VALUES (1, ''), (2, ''), (3, '')");

    SpecGapResult result = BuildGaps(reference.path, "", specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK_FALSE(AnyGap(gaps, "counterpart_candidate", "srv_area_template", "name", ""));
}

TEST_CASE("BuildGaps reports empty client tables with their known twin", "[GapAnalysis]")
{
    TempDir specDir("empty_spec");
    WritePairsTsvRaw(specDir.path, "srv\tbroadcast_text\tbroadcasttext\tid\tentry\tID\tnote\n");
    WriteRulesTsvRaw(specDir.path);

    TempDoltLiteFile reference("empty");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"cdbc_questobjective\" (\"ID\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_broadcasttext\" (\"ID\" INTEGER)");

    SpecGapResult result = BuildGaps(reference.path, "", specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "empty_client_table", "dbc_questobjective", "", "no known srv/vdbc twin"));
    CHECK(AnyGap(gaps, "empty_client_table", "dbc_broadcasttext", "", "srv holds: srv_broadcast_text"));
}

TEST_CASE("BuildGaps flags the known 1.15-only tables and fields mangosd will need", "[GapAnalysis]")
{
    TempDir specDir("known_spec");

    TempDoltLiteFile reference("known");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"cdbc_questv2\" (\"ID\" INTEGER, \"UniqueBitFlag\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_spellmisc\" (\"ID\" INTEGER, \"Attributes5\" INTEGER, \"SchoolMask\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_uimap\" (\"ID\" INTEGER, \"Name_lang\" TEXT)");

    SpecGapResult result = BuildGaps(reference.path, "", specDir.path);
    CHECK(result.error.empty());

    std::vector<SpecGapRow> gaps = ReadGapsTsv(specDir.path / "gaps.tsv");
    CHECK(AnyGap(gaps, "client_only_field", "dbc_questv2", "UniqueBitFlag", "data-plan.md section 4"));
    CHECK(AnyGap(gaps, "client_only_field", "dbc_spellmisc", "Attributes5", "data-plan.md section 4"));
    CHECK_FALSE(AnyGap(gaps, "client_only_field", "dbc_spellmisc", "SchoolMask", ""));
    CHECK(AnyGap(gaps, "client_only_field", "dbc_uimap", "", "data-plan.md section 4"));
}
