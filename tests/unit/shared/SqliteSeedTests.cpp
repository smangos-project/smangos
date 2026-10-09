#include "Persistence/SqliteSeed.h"

#include <catch_amalgamated.hpp>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sqlite3.h>

namespace
{
    std::filesystem::path TempPath(char const* name)
    {
        std::filesystem::path const path = std::filesystem::temp_directory_path() / name;
        std::filesystem::remove(path);
        std::filesystem::remove(path.string() + "-wal");
        std::filesystem::remove(path.string() + "-shm");
        return path;
    }

    void Exec(sqlite3* db, char const* sql)
    {
        char* message = nullptr;
        int const rc = sqlite3_exec(db, sql, nullptr, nullptr, &message);
        INFO(sql << ": " << (message ? message : ""));
        sqlite3_free(message);
        REQUIRE(rc == SQLITE_OK);
    }

    int64 QueryInt(sqlite3* db, char const* sql)
    {
        sqlite3_stmt* stmt = nullptr;
        REQUIRE(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK);
        REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
        int64 const value = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return value;
    }
}

TEST_CASE("EmptySqliteImage matches the stock empty database layout", "[shared][sqliteseed]")
{
    std::vector<uint8> const image = Persistence::EmptySqliteImage(4096, false);
    CHECK(image[18] == 1);
    CHECK(Persistence::EmptySqliteImage(4096, true)[18] == 2);
    REQUIRE(image.size() == 4096);
    CHECK(std::memcmp(image.data(), "SQLite format 3\0", 16) == 0);
    CHECK(image[16] == 0x10);
    CHECK(image[17] == 0x00);
    CHECK(image[21] == 64);
    CHECK(image[31] == 1);
    CHECK(image[100] == 0x0D);
    CHECK(image[105] == 0x10);

    std::vector<uint8> const big = Persistence::EmptySqliteImage(65536, false);
    CHECK(big[16] == 0x00);
    CHECK(big[17] == 0x01);
    CHECK(big[105] == 0x00);
    CHECK(big[106] == 0x00);
}

namespace
{
    std::string QueryText(sqlite3* db, char const* sql)
    {
        sqlite3_stmt* stmt = nullptr;
        REQUIRE(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK);
        REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
        std::string const value = reinterpret_cast<char const*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return value;
    }
}

TEST_CASE("OpenStockSqlite keeps the stock engine and WAL mode across reopen", "[shared][sqliteseed]")
{
    for (uint32 pageSize : { 4096u, 16384u })
    {
        std::filesystem::path const path = TempPath("mangos-sqliteseed-test.db");
        std::string error;

        sqlite3* db = Persistence::OpenStockSqlite(path.string(), pageSize, true, error);
        INFO(error);
        REQUIRE(db);
        Exec(db, "CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT)");
        Exec(db, "INSERT INTO t VALUES (1, 'a'), (2, 'b')");
        CHECK(QueryInt(db, "PRAGMA page_size") == pageSize);
        REQUIRE(sqlite3_close(db) == SQLITE_OK);

        char header[16] = {};
        std::ifstream(path, std::ios::binary).read(header, sizeof(header));
        CHECK(std::memcmp(header, "SQLite format 3\0", 16) == 0);

        db = Persistence::OpenStockSqlite(path.string(), pageSize, true, error);
        REQUIRE(db);
        CHECK(QueryText(db, "PRAGMA journal_mode") == "wal");
        CHECK(QueryInt(db, "SELECT count(*) FROM t") == 2);
        std::string engine;
        CHECK(Persistence::IsStockEngine(db, engine));
        sqlite3_close(db);
        std::filesystem::remove(path);
        std::filesystem::remove(path.string() + "-wal");
        std::filesystem::remove(path.string() + "-shm");
    }
}

TEST_CASE("EnsureStockSqliteFile rejects bad page sizes and keeps existing files", "[shared][sqliteseed]")
{
    std::filesystem::path const path = TempPath("mangos-sqliteseed-keep.db");
    std::string error;
    CHECK_FALSE(Persistence::EnsureStockSqliteFile(path.string(), 3000, false, error));
    CHECK_FALSE(Persistence::EnsureStockSqliteFile(path.string(), 256, false, error));

    std::ofstream(path, std::ios::binary) << "not a database";
    REQUIRE(Persistence::EnsureStockSqliteFile(path.string(), 4096, false, error));
    CHECK(std::filesystem::file_size(path) == 14);
    std::filesystem::remove(path);
}
