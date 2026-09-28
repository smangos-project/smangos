#include "Spec/SpecInventory.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_specinventory_" + name))
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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_specinventory_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };

    // srv_foo(id, name), vdbc_bar(ID, Flags), cdbc_bar(ID, Flags, NewCol) -- has a vanilla
    // counterpart (vdbc_bar) -- and cdbc_baz(ID) which does not.
    void MakeReferenceFixture(DoltLiteDatabase& db)
    {
        db.Execute("CREATE TABLE \"srv_foo\" (\"id\" INTEGER, \"name\" TEXT)");
        db.Execute("CREATE TABLE \"vdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
        db.Execute("CREATE TABLE \"cdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER, \"NewCol\" INTEGER)");
        db.Execute("CREATE TABLE \"cdbc_baz\" (\"ID\" INTEGER)");
    }

    bool HasColumnRow(std::vector<SpecColumnRow> const& rows, std::string const& source, std::string const& table, std::string const& column)
    {
        return std::any_of(rows.begin(), rows.end(), [&](SpecColumnRow const& row)
        { return row.source == source && row.table == table && row.column == column; });
    }

    bool HasDbcColumnRow(std::vector<SpecDbcColumnRow> const& rows, std::string const& table, std::string const& column)
    {
        return std::any_of(rows.begin(), rows.end(), [&](SpecDbcColumnRow const& row)
        { return row.table == table && row.column == column; });
    }
}

TEST_CASE("UpdateSpecInventory errors when the reference database doesn't exist", "[SpecInventory]")
{
    TempDir specDir("missing_reference");
    SpecInitResult result = UpdateSpecInventory(specDir.path / "does-not-exist.doltlite", specDir.path);
    CHECK_FALSE(result.error.empty());
}

TEST_CASE("UpdateSpecInventory inventories srv_*/vdbc_* columns and cdbc_* columns with a vanilla counterpart", "[SpecInventory]")
{
    TempDoltLiteFile reference("basic");
    {
        DoltLiteDatabase db(reference.path.string());
        MakeReferenceFixture(db);
    }
    TempDir specDir("basic");

    SpecInitResult result = UpdateSpecInventory(reference.path, specDir.path);
    CHECK(result.error.empty());
    CHECK(result.columnsAdded == 4);   // srv_foo.id, srv_foo.name, vdbc_bar.ID, vdbc_bar.Flags
    CHECK(result.dbcColumnsAdded == 3); // dbc_bar.ID, dbc_bar.Flags, dbc_bar.NewCol
    CHECK(result.missingColumns.empty());
    CHECK(result.missingDbcColumns.empty());

    std::vector<SpecColumnRow> columns = ReadColumnsTsv(specDir.path / "columns.tsv");
    REQUIRE(columns.size() == 4);
    CHECK(HasColumnRow(columns, "srv", "foo", "id"));
    CHECK(HasColumnRow(columns, "srv", "foo", "name"));
    CHECK(HasColumnRow(columns, "vdbc", "bar", "ID"));
    CHECK(HasColumnRow(columns, "vdbc", "bar", "Flags"));
    for (SpecColumnRow const& row : columns)
    {
        CHECK(row.disposition.empty());
        CHECK(row.target.empty());
    }
    // srv sorts before vdbc (phase0-plan.md 3.1's source order).
    CHECK(columns.front().source == "srv");
    CHECK(columns.back().source == "vdbc");

    std::vector<SpecDbcColumnRow> dbcColumns = ReadDbcColumnsTsv(specDir.path / "dbc-columns.tsv");
    REQUIRE(dbcColumns.size() == 3);
    CHECK(HasDbcColumnRow(dbcColumns, "dbc_bar", "ID"));
    CHECK(HasDbcColumnRow(dbcColumns, "dbc_bar", "Flags"));
    CHECK(HasDbcColumnRow(dbcColumns, "dbc_bar", "NewCol"));
    // cdbc_baz has no vdbc_baz counterpart, so it never receives a vanilla row and is excluded.
    CHECK_FALSE(HasDbcColumnRow(dbcColumns, "dbc_baz", "ID"));
}

TEST_CASE("UpdateSpecInventory is idempotent and preserves hand-filled dispositions", "[SpecInventory]")
{
    TempDoltLiteFile reference("idempotent");
    {
        DoltLiteDatabase db(reference.path.string());
        MakeReferenceFixture(db);
    }
    TempDir specDir("idempotent");

    UpdateSpecInventory(reference.path, specDir.path);

    // Hand-review fills in a disposition.
    std::vector<SpecColumnRow> columns = ReadColumnsTsv(specDir.path / "columns.tsv");
    for (SpecColumnRow& row : columns)
        if (row.source == "srv" && row.table == "foo" && row.column == "id")
            row.disposition = "server";
    WriteColumnsTsv(specDir.path / "columns.tsv", columns);

    SpecInitResult second = UpdateSpecInventory(reference.path, specDir.path);
    CHECK(second.columnsAdded == 0);
    CHECK(second.dbcColumnsAdded == 0);

    std::vector<SpecColumnRow> after = ReadColumnsTsv(specDir.path / "columns.tsv");
    auto it = std::find_if(after.begin(), after.end(), [](SpecColumnRow const& row)
    { return row.source == "srv" && row.table == "foo" && row.column == "id"; });
    REQUIRE(it != after.end());
    CHECK(it->disposition == "server"); // not overwritten
}

TEST_CASE("UpdateSpecInventory reports, but keeps, a column the reference database no longer has", "[SpecInventory]")
{
    TempDir specDir("removed_column");
    {
        TempDoltLiteFile reference("removed_column_v1");
        DoltLiteDatabase db(reference.path.string());
        MakeReferenceFixture(db);
        UpdateSpecInventory(reference.path, specDir.path);
    }

    TempDoltLiteFile reference2("removed_column_v2");
    {
        DoltLiteDatabase db(reference2.path.string());
        db.Execute("CREATE TABLE \"srv_foo\" (\"id\" INTEGER)"); // "name" dropped
        db.Execute("CREATE TABLE \"vdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
        db.Execute("CREATE TABLE \"cdbc_bar\" (\"ID\" INTEGER, \"Flags\" INTEGER, \"NewCol\" INTEGER)");
    }

    SpecInitResult result = UpdateSpecInventory(reference2.path, specDir.path);
    CHECK(result.columnsAdded == 0);
    REQUIRE(result.missingColumns.size() == 1);
    CHECK(result.missingColumns[0] == "srv:foo.name");

    // Still in the file, per phase0-plan.md 3.2: "reported, not deleted".
    std::vector<SpecColumnRow> columns = ReadColumnsTsv(specDir.path / "columns.tsv");
    CHECK(HasColumnRow(columns, "srv", "foo", "name"));
}

TEST_CASE("SetSpecColumns fills only empty rows unless --overwrite", "[SpecInventory]")
{
    TempDir specDir("set");
    WriteColumnsTsv(specDir.path / "columns.tsv", {
        { "vdbc", "attackanimkits", "ID", "", "", "", "", "" },
        { "vdbc", "attackanimkits", "AnimID", "server", "", "", "already reviewed", "" },
        { "srv", "other_table", "id", "", "", "", "", "" },
    });

    SpecSetResult first = SetSpecColumns(specDir.path, { "vdbc", "attackanimkits", "" },
        { std::string("drop"), std::nullopt, std::nullopt, std::string("not imported"), std::nullopt }, false);
    CHECK(first.matched == 2);
    CHECK(first.updated == 1); // AnimID already has a disposition, left alone

    std::vector<SpecColumnRow> columns = ReadColumnsTsv(specDir.path / "columns.tsv");
    auto find = [&](std::string const& column) -> SpecColumnRow const&
    {
        auto it = std::find_if(columns.begin(), columns.end(), [&](SpecColumnRow const& row) { return row.column == column; });
        REQUIRE(it != columns.end());
        return *it;
    };
    CHECK(find("ID").disposition == "drop");
    CHECK(find("ID").reason == "not imported");
    CHECK(find("AnimID").disposition == "server"); // untouched
    CHECK(find("AnimID").reason == "already reviewed");

    SpecSetResult overwritten = SetSpecColumns(specDir.path, { "vdbc", "attackanimkits", "AnimID" },
        { std::string("drop"), std::nullopt, std::nullopt, std::string("superseded"), std::nullopt }, true);
    CHECK(overwritten.matched == 1);
    CHECK(overwritten.updated == 1);
    columns = ReadColumnsTsv(specDir.path / "columns.tsv");
    CHECK(find("AnimID").disposition == "drop");
    CHECK(find("AnimID").reason == "superseded");
}

TEST_CASE("SetSpecColumns reports an error when nothing matches", "[SpecInventory]")
{
    TempDir specDir("set_nomatch");
    WriteColumnsTsv(specDir.path / "columns.tsv", { { "srv", "foo", "id", "", "", "", "", "" } });

    SpecSetResult result = SetSpecColumns(specDir.path, { "srv", "does_not_exist", "" }, {}, false);
    CHECK(result.matched == 0);
    CHECK_FALSE(result.error.empty());
}
