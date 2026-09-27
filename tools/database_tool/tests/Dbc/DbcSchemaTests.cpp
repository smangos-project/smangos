#include "Dbc/DbcSchema.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

namespace
{
    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dbcschema_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };

    int64_t CountUserTables(DoltLiteDatabase& db)
    {
        DoltLiteStatement count = db.Prepare(
            "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name LIKE 'dbc_%' AND name != 'dbc_files'");
        count.Step();
        return count.ColumnInt64(0);
    }
}

TEST_CASE("DbcColumnNames returns the field name for a scalar field", "[DbcSchema]")
{
    DbcFieldMeta field{ "Value", DbcFieldType::Int, 32, true, 1, false, false, false, nullptr, nullptr };
    CHECK(DbcColumnNames(field) == std::vector<std::string>{ "Value" });
}

TEST_CASE("DbcColumnNames flattens an array field into Name1..NameN", "[DbcSchema]")
{
    DbcFieldMeta field{ "Flags", DbcFieldType::Int, 32, false, 3, false, false, false, nullptr, nullptr };
    CHECK(DbcColumnNames(field) == std::vector<std::string>{ "Flags1", "Flags2", "Flags3" });
}

TEST_CASE("BuildDbcTableDdl maps field types and marks the id column as the primary key", "[DbcSchema]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, true, 1, true, false, false, nullptr, nullptr },
        { "Name_lang", DbcFieldType::LocString, 0, true, 1, false, false, false, nullptr, nullptr },
        { "Weight", DbcFieldType::Float, 0, true, 1, false, false, false, nullptr, nullptr },
        { "Flags", DbcFieldType::Int, 32, false, 2, false, false, false, nullptr, nullptr },
    };
    DbcTableMeta table{ "Sample", "sample", "dbc_sample", nullptr, 0, fields, 4 };

    std::string ddl = BuildDbcTableDdl(table);

    CHECK(ddl == "CREATE TABLE \"dbc_sample\" (\"ID\" INTEGER NOT NULL DEFAULT 0 PRIMARY KEY, "
                 "\"Name_lang\" TEXT NOT NULL DEFAULT '', \"Weight\" REAL NOT NULL DEFAULT 0, "
                 "\"Flags1\" INTEGER NOT NULL DEFAULT 0, \"Flags2\" INTEGER NOT NULL DEFAULT 0)");
}

TEST_CASE("BuildDbcTableIndexDdl indexes only foreign-key fields", "[DbcSchema]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, true, 1, true, false, false, nullptr, nullptr },
        { "MapID", DbcFieldType::Int, 32, true, 1, false, false, false, "Map", "ID" },
    };
    DbcTableMeta table{ "Sample", "sample", "dbc_sample", nullptr, 0, fields, 2 };

    std::vector<std::string> indexes = BuildDbcTableIndexDdl(table);

    REQUIRE(indexes.size() == 1);
    CHECK(indexes[0] == "CREATE INDEX \"dbc_sample__MapID\" ON \"dbc_sample\" (\"MapID\")");
}

TEST_CASE("BuildDbcFilesTableDdl declares the bookkeeping columns", "[DbcSchema]")
{
    std::string ddl = BuildDbcFilesTableDdl();
    for (char const* column : { "table_name", "format", "layout_hash", "table_hash", "locale", "flags", "schema",
                                 "row_count" })
        CHECK(ddl.find(column) != std::string::npos);
}

TEST_CASE("CreateDbcTables creates dbc_files plus one table for every real metadata table", "[DbcSchema]")
{
    TempDoltLiteFile file("real_metadata");
    DoltLiteDatabase db(file.path.string());

    DbcSchemaResult result = CreateDbcTables(db, "test: create dbc tables");

    INFO(result.error);
    CHECK(result.error.empty());
    CHECK(result.tablesCreated == GetDbcTables().size());
    CHECK(CountUserTables(db) == static_cast<int64_t>(GetDbcTables().size()));

    DoltLiteStatement filesTable = db.Prepare("SELECT COUNT(*) FROM sqlite_master WHERE name = 'dbc_files'");
    REQUIRE(filesTable.Step());
    CHECK(filesTable.ColumnInt64(0) == 1);

    DbcTableMeta const* map = FindDbcTable("Map");
    REQUIRE(map != nullptr);
    DoltLiteStatement columns = db.Prepare(std::string("PRAGMA table_info(\"") + map->sqlTable + "\")");
    int64_t columnCount = 0;
    while (columns.Step())
        ++columnCount;
    CHECK(columnCount > 0);
}

TEST_CASE("CreateDbcTables recreates dbc_* tables from scratch on a second run", "[DbcSchema]")
{
    TempDoltLiteFile file("rerun");
    DoltLiteDatabase db(file.path.string());

    REQUIRE(CreateDbcTables(db, "first").error.empty());

    DbcTableMeta const* map = FindDbcTable("Map");
    REQUIRE(map != nullptr);
    db.Execute(std::string("INSERT INTO \"") + map->sqlTable + "\" (\"ID\") VALUES (1)");

    DbcSchemaResult second = CreateDbcTables(db, "second");
    INFO(second.error);
    CHECK(second.error.empty());

    DoltLiteStatement count = db.Prepare(std::string("SELECT COUNT(*) FROM \"") + map->sqlTable + "\"");
    REQUIRE(count.Step());
    CHECK(count.ColumnInt64(0) == 0);
}
