#include "Dbc/DbcHotfixes.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_hotfixes_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path / "hotfixes");
        }

        ~TempDir()
        {
            std::filesystem::remove_all(path);
        }

        void Write(std::string const& file, std::string const& text) const
        {
            std::ofstream(path / "hotfixes" / file) << text;
        }
    };

    int64_t Scalar(DoltLiteDatabase& db, std::string const& sql)
    {
        DoltLiteStatement statement = db.Prepare(sql);
        statement.Step();
        return statement.ColumnInt64(0);
    }
}

TEST_CASE("ApplyDbcHotfixes runs the files in name order", "[DbcHotfixes]")
{
    TempDir dir("order");
    dir.Write("2.sql", "INSERT OR REPLACE INTO \"dbc_t\" (\"ID\", \"V\") VALUES (1, 20);\n");
    dir.Write("1.sql", "INSERT OR REPLACE INTO \"dbc_t\" (\"ID\", \"V\") VALUES (1, 10);\nDELETE FROM \"dbc_t\" WHERE \"ID\" = 2;\n");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    db.Execute("CREATE TABLE \"dbc_t\" (\"ID\" INT PRIMARY KEY, \"V\" INT)");
    db.Execute("INSERT INTO \"dbc_t\" VALUES (2, 5)");

    DbcHotfixResult result = ApplyDbcHotfixes(db, dir.path / "hotfixes");
    REQUIRE(result.error.empty());
    CHECK(result.files == std::vector<std::string>{ "1.sql", "2.sql" });
    CHECK(result.statements == 3);
    CHECK(Scalar(db, "SELECT V FROM dbc_t WHERE ID = 1") == 20);
    CHECK(Scalar(db, "SELECT COUNT(*) FROM dbc_t") == 1);
}

TEST_CASE("ApplyDbcHotfixes swaps the table prefix", "[DbcHotfixes]")
{
    TempDir dir("prefix");
    dir.Write("1.sql", "INSERT OR REPLACE INTO \"dbc_t\" (\"ID\", \"V\") VALUES (1, 'a\"dbc_x');\n");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    db.Execute("CREATE TABLE \"cdbc_t\" (\"ID\" INT PRIMARY KEY, \"V\" TEXT)");

    DbcHotfixResult result = ApplyDbcHotfixes(db, dir.path / "hotfixes", "cdbc_");
    REQUIRE(result.error.empty());
    DoltLiteStatement value = db.Prepare("SELECT V FROM cdbc_t WHERE ID = 1");
    REQUIRE(value.Step());
    CHECK(value.ColumnText(0) == "a\"dbc_x");
}

TEST_CASE("ApplyDbcHotfixes rolls back everything on an error", "[DbcHotfixes]")
{
    TempDir dir("error");
    dir.Write("1.sql", "INSERT INTO \"dbc_t\" (\"ID\") VALUES (1);\nINSERT INTO \"dbc_missing\" (\"ID\") VALUES (1);\n");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    db.Execute("CREATE TABLE \"dbc_t\" (\"ID\" INT PRIMARY KEY)");

    DbcHotfixResult result = ApplyDbcHotfixes(db, dir.path / "hotfixes");
    CHECK(result.error.rfind("1.sql: ", 0) == 0);
    CHECK(result.statements == 0);
    CHECK(Scalar(db, "SELECT COUNT(*) FROM dbc_t") == 0);
}

TEST_CASE("ApplyDbcHotfixes applies nothing without a directory", "[DbcHotfixes]")
{
    TempDir dir("none");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    DbcHotfixResult result = ApplyDbcHotfixes(db, dir.path / "absent");
    CHECK(result.error.empty());
    CHECK(result.files.empty());
}
