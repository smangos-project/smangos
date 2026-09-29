#include "Spec/SpecCheck.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <filesystem>

namespace
{
    struct TempFixture
    {
        std::filesystem::path dir;
        std::filesystem::path reference;

        explicit TempFixture(std::string const& name)
            : dir(std::filesystem::temp_directory_path() / ("database_tool_test_speccheck_" + name))
            , reference(dir / "reference.doltlite")
        {
            std::filesystem::remove_all(dir);
            std::filesystem::create_directories(dir);

            DoltLiteDatabase db(reference.string());
            db.Execute("CREATE TABLE \"srv_foo\" (\"id\" INTEGER, \"spellid_1\" INTEGER, \"spellid_2\" INTEGER)");
            db.Execute("CREATE TABLE \"vdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
            db.Execute("CREATE TABLE \"cdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER, \"NewCol\" INTEGER)");

            WriteRulesTsv(dir / "rules.tsv", { { "identity", "value", "{v}", "{v}", "" } });
            WriteEvidenceTsv(dir / "evidence.tsv", {
                { "vdbc", "bar", "ID", "INTEGER", "none", "", "cdbc_bar.ID", "identity", "1.0", "" },
                { "vdbc", "bar", "Flags", "INTEGER", "none", "", "cdbc_bar.Flags", "identity", "1.0", "" },
            });
            WriteGapsTsv(dir / "gaps.tsv", {});
            WriteDbcColumnsTsv(dir / "dbc-columns.tsv", { { "dbc_bar", "NewCol", "", "0" } });
            WriteColumnsTsv(dir / "columns.tsv", GoodColumns());
            WriteTablesTsv(dir / "tables.tsv", GoodTables());
        }

        ~TempFixture() { std::filesystem::remove_all(dir); }

        static std::vector<SpecColumnRow> GoodColumns()
        {
            return {
                { "srv", "foo", "id", "server", "", "", "primary key", "yes" },
                { "srv", "foo", "spellid_1", "drop", "", "", "unused", "yes" },
                { "srv", "foo", "spellid_2", "drop", "", "", "unused", "yes" },
                { "vdbc", "bar", "ID", "dbc", "dbc_bar.ID", "identity", "", "yes" },
                { "vdbc", "bar", "Flags", "dbc", "dbc_bar.Flags", "identity", "", "yes" },
            };
        }

        static std::vector<SpecTableRow> GoodTables()
        {
            return {
                { "foo", "server", "id", "srv:foo", "", "", "" },
                { "dbc_bar", "dbc", "ID", "vdbc:bar", "", "", "" },
            };
        }

        SpecCheckResult Check(bool final = false) const { return CheckSpec(reference, dir, final); }

        void SetColumns(std::vector<SpecColumnRow> const& rows) const { WriteColumnsTsv(dir / "columns.tsv", rows); }
    };

    bool HasProblem(SpecCheckResult const& result, uint32_t check, std::string const& text)
    {
        return std::any_of(result.problems.begin(), result.problems.end(), [&](SpecCheckProblem const& problem)
        { return problem.check == check && problem.message.find(text) != std::string::npos; });
    }
}

TEST_CASE("CheckSpec errors when the reference database doesn't exist", "[SpecCheck]")
{
    TempFixture fixture("missing");
    CHECK_FALSE(CheckSpec(fixture.dir / "nope.doltlite", fixture.dir, false).error.empty());
}

TEST_CASE("CheckSpec passes a complete spec", "[SpecCheck]")
{
    TempFixture fixture("good");
    SpecCheckResult result = fixture.Check(true);
    CHECK(result.error.empty());
    CHECK(result.problems.empty());
}

TEST_CASE("CheckSpec flags a missing, duplicate and unreal row (rule 1)", "[SpecCheck]")
{
    TempFixture fixture("inventory");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows.erase(rows.begin());                                          // srv:foo.id
    rows.push_back(rows.back());                                       // vdbc:bar.Flags twice
    rows.push_back({ "srv", "foo", "ghost", "drop", "", "", "x", "" }); // no such column
    fixture.SetColumns(rows);

    SpecCheckResult result = fixture.Check();
    CHECK(HasProblem(result, 1, "srv:foo.id has no row"));
    CHECK(HasProblem(result, 1, "vdbc:bar.Flags has 2 rows"));
    CHECK(HasProblem(result, 1, "srv:foo.ghost names no column"));
}

TEST_CASE("CheckSpec flags an empty disposition (rule 1)", "[SpecCheck]")
{
    TempFixture fixture("disposition");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[0].disposition.clear();
    fixture.SetColumns(rows);
    CHECK(HasProblem(fixture.Check(), 1, "no valid disposition"));
}

TEST_CASE("CheckSpec flags a bad dbc target and rule (rule 2)", "[SpecCheck]")
{
    TempFixture fixture("dbc");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[3].target = "dbc_bar.Missing";
    rows[4].rule = "no_such_rule";
    fixture.SetColumns(rows);
    WriteEvidenceTsv(fixture.dir / "evidence.tsv", {});

    SpecCheckResult result = fixture.Check();
    CHECK(HasProblem(result, 2, "dbc_bar.Missing"));
    CHECK(HasProblem(result, 2, "no_such_rule"));
    CHECK(HasProblem(result, 2, "vdbc:bar.ID has no recorded match rate"));
}

TEST_CASE("CheckSpec flags an unread server column and empty derived, constant and drop rows (rules 3, 4)", "[SpecCheck]")
{
    TempFixture fixture("server");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[0].reason.clear();
    rows[1] = { "srv", "foo", "spellid_1", "derived", "", "", "", "yes" };
    rows[2] = { "srv", "foo", "spellid_2", "constant", "", "", "", "yes" };
    rows[3] = { "vdbc", "bar", "ID", "drop", "", "", "", "yes" };
    rows.pop_back();
    rows.push_back({ "vdbc", "bar", "Flags", "drop", "", "", "", "yes" });
    fixture.SetColumns(rows);

    SpecCheckResult result = fixture.Check();
    CHECK(HasProblem(result, 3, "srv:foo.id"));
    CHECK(HasProblem(result, 4, "srv:foo.spellid_1 is derived"));
    CHECK(HasProblem(result, 4, "srv:foo.spellid_2 is constant"));
    CHECK(HasProblem(result, 4, "vdbc:bar.ID is dropped"));
}

TEST_CASE("CheckSpec flags two rows filling one target, except a shared-rule pair and array slots (rule 5)", "[SpecCheck]")
{
    TempFixture fixture("targets");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[0] = { "srv", "foo", "id", "dbc", "dbc_bar.ID", "identity", "x", "yes" }; // srv + vdbc, same rule
    rows[1] = { "srv", "foo", "spellid_1", "dbc", "dbc_bar.NewCol", "identity", "x", "yes" };
    rows[2] = { "srv", "foo", "spellid_2", "dbc", "dbc_bar.NewCol", "identity", "x", "yes" };
    rows[4].target = "dbc_bar.Flags";
    rows.push_back({ "vdbc", "bar", "Extra", "dbc", "dbc_bar.Flags", "identity", "x", "yes" });
    fixture.SetColumns(rows);

    SpecCheckResult result = fixture.Check();
    CHECK_FALSE(HasProblem(result, 5, "dbc_bar.ID"));
    CHECK_FALSE(HasProblem(result, 5, "dbc_bar.NewCol"));
    CHECK(HasProblem(result, 5, "dbc_bar.Flags is filled by"));
}

TEST_CASE("CheckSpec lets an override table fill its base table's target with the same rule (rule 5)", "[SpecCheck]")
{
    TempFixture fixture("override");
    DoltLiteDatabase(fixture.reference.string()).Execute("CREATE TABLE \"srv_foo_mod\" (\"id\" INTEGER, \"Flags\" INTEGER, \"Other\" INTEGER)");
    WriteTablesTsv(fixture.dir / "tables.tsv", { { "foo_mod", "gone", "id", "", "dbc_bar", "foo", "" } });
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[1] = { "srv", "foo", "spellid_1", "dbc", "dbc_bar.NewCol", "identity", "x", "yes" };
    rows.push_back({ "srv", "foo_mod", "id", "drop", "", "", "key", "yes" });
    rows.push_back({ "srv", "foo_mod", "Flags", "dbc", "dbc_bar.NewCol", "identity", "x", "yes" });
    rows.push_back({ "srv", "foo_mod", "Other", "dbc", "dbc_bar.Flags", "identity", "x", "yes" });
    rows.push_back({ "srv", "foo_mod", "Sole", "dbc", "dbc_bar.Unwritten", "identity", "x", "yes" });
    fixture.SetColumns(rows);

    SpecCheckResult result = fixture.Check();
    CHECK_FALSE(HasProblem(result, 5, "dbc_bar.NewCol"));
    CHECK(HasProblem(result, 5, "srv:foo_mod.Other overrides foo, which doesn't fill dbc_bar.Flags"));
    CHECK_FALSE(HasProblem(result, 5, "srv:foo_mod.Sole"));
}

TEST_CASE("CheckSpec wants tables.tsv to agree with columns.tsv (rule 6)", "[SpecCheck]")
{
    TempFixture fixture("tables");
    DoltLiteDatabase(fixture.reference.string()).Execute("CREATE TABLE \"cdbc_holder\" (\"ID\" INTEGER)");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[1] = { "srv", "foo", "spellid_1", "dbc", "dbc_bar.NewCol", "identity", "x", "yes" };
    rows[2] = { "srv", "foo", "spellid_2", "server", "kept.spell", "", "x", "yes" };
    rows.push_back({ "new", "tagged", "tag", "server", "", "", "x", "yes" });
    fixture.SetColumns(rows);
    WriteTablesTsv(fixture.dir / "tables.tsv", {
        { "foo", "gone", "", "", "dbc_b*", "", "" },
        { "dbc_bar", "dbc", "ID", "vdbc:bar, srv:other", "", "", "" },
        { "kept", "server", "missing", "srv:foo", "", "", "" },
        { "dbc_holder", "placeholder", "ID", "foo", "", "", "" },
        { "dbc_holder", "dbc", "ID", "", "", "", "" },
    });

    SpecCheckResult result = fixture.Check();
    CHECK(HasProblem(result, 6, "foo is gone but keeps columns in place"));
    CHECK(HasProblem(result, 6, "foo writes kept, which its into doesn't name"));
    CHECK_FALSE(HasProblem(result, 6, "foo has into dbc_b*"));
    CHECK(HasProblem(result, 6, "dbc_bar sources lack srv:foo"));
    CHECK(HasProblem(result, 6, "dbc_bar sources list srv:other"));
    CHECK(HasProblem(result, 6, "kept has key column missing"));
    CHECK(HasProblem(result, 6, "placeholder dbc_holder names foo, which isn't a server table"));
    CHECK(HasProblem(result, 6, "dbc_holder has more than one tables.tsv row"));
    CHECK(HasProblem(result, 6, "tagged has new columns and no server row"));
}

TEST_CASE("CheckSpec wants a vanilla_value on 1.15-only columns of written tables (rule 7)", "[SpecCheck]")
{
    TempFixture fixture("vanilla");
    WriteDbcColumnsTsv(fixture.dir / "dbc-columns.tsv", { { "dbc_bar", "NewCol", "", "" }, { "dbc_bar", "Flags", "", "" } });

    SpecCheckResult result = fixture.Check();
    CHECK(HasProblem(result, 7, "dbc_bar.NewCol"));
    CHECK_FALSE(HasProblem(result, 7, "dbc_bar.Flags")); // a dbc row fills it
}

TEST_CASE("CheckSpec wants every removed table and column dispositioned (rule 8)", "[SpecCheck]")
{
    TempFixture fixture("gaps");
    WriteGapsTsv(fixture.dir / "gaps.tsv", {
        { "removed_column", "bar", "Flags", "removed" },
        { "removed_column", "bar", "Gone", "removed" },
        { "removed_table", "lost", "", "removed" },
        { "empty_client_table", "dbc_other", "", "informational" },
    });

    SpecCheckResult result = fixture.Check();
    CHECK_FALSE(HasProblem(result, 8, "bar.Flags"));
    CHECK(HasProblem(result, 8, "removed_column bar.Gone"));
    CHECK(HasProblem(result, 8, "removed_table lost"));
    CHECK_FALSE(HasProblem(result, 8, "dbc_other"));
}

TEST_CASE("CheckSpec --final wants every row reviewed (rule 9)", "[SpecCheck]")
{
    TempFixture fixture("final");
    std::vector<SpecColumnRow> rows = TempFixture::GoodColumns();
    rows[0].reviewed.clear();
    fixture.SetColumns(rows);

    CHECK(fixture.Check(false).problems.empty());
    CHECK(HasProblem(fixture.Check(true), 9, "srv:foo.id is not reviewed"));
}
