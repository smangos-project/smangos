#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <string>

namespace
{
    // A DoltLite file at a unique temp path, removed on both ends of the test.
    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };
}

TEST_CASE("DoltLiteDatabase executes statements and reads rows back", "[DoltLiteDatabase]")
{
    TempDoltLiteFile file("execute");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");
    db.Execute("INSERT INTO t(id, name) VALUES (1, 'hello')");

    DoltLiteStatement statement = db.Prepare("SELECT id, name FROM t WHERE id = 1");
    REQUIRE(statement.Step());
    CHECK(statement.ColumnInt64(0) == 1);
    CHECK(statement.ColumnText(1) == "hello");
    CHECK_FALSE(statement.Step());
}

TEST_CASE("DoltLiteDatabase binds parameters of every supported type", "[DoltLiteDatabase]")
{
    TempDoltLiteFile file("bind");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE t(i INTEGER, d REAL, s TEXT, n TEXT)");

    DoltLiteStatement insert = db.Prepare("INSERT INTO t(i, d, s, n) VALUES (?, ?, ?, ?)");
    insert.BindInt64(1, 42);
    insert.BindDouble(2, 3.5);
    insert.BindText(3, "text");
    insert.BindNull(4);
    CHECK_FALSE(insert.Step());

    DoltLiteStatement select = db.Prepare("SELECT i, d, s, n FROM t");
    REQUIRE(select.Step());
    CHECK(select.ColumnInt64(0) == 42);
    CHECK(select.ColumnDouble(1) == 3.5);
    CHECK(select.ColumnText(2) == "text");
    CHECK(select.ColumnIsNull(3));
}

TEST_CASE("DoltLiteDatabase rolls back a transaction", "[DoltLiteDatabase]")
{
    TempDoltLiteFile file("rollback");
    DoltLiteDatabase db(file.path.string());

    db.Execute("CREATE TABLE t(id INTEGER PRIMARY KEY)");

    db.Begin();
    db.Execute("INSERT INTO t(id) VALUES (1)");
    db.Rollback();

    DoltLiteStatement statement = db.Prepare("SELECT COUNT(*) FROM t");
    REQUIRE(statement.Step());
    CHECK(statement.ColumnInt64(0) == 0);
}

TEST_CASE("DoltLiteDatabase reports sqlite errors", "[DoltLiteDatabase]")
{
    TempDoltLiteFile file("error");
    DoltLiteDatabase db(file.path.string());

    CHECK_THROWS_AS(db.Execute("NOT VALID SQL"), DoltLiteError);
}
