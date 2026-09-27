#include "Migrations/MigrationRunner.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    struct TempFile
    {
        std::filesystem::path path;

        TempFile(std::string const& name, std::string const& contents)
            : path(std::filesystem::temp_directory_path() / name)
        {
            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            file << contents;
        }

        ~TempFile()
        {
            std::filesystem::remove(path);
        }
    };

    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_migrationrunner_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };

    MigrationFile File(std::string id, std::filesystem::path path, bool legacyEnvelope = false)
    {
        MigrationFile file;
        file.id = std::move(id);
        file.db = "world";
        file.path = std::move(path);
        file.hasLegacyEnvelope = legacyEnvelope;
        return file;
    }
}

TEST_CASE("RunDoltLiteMigrations applies a file's statements and commits it to dolt history", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("apply");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");
    db.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY, `name` TEXT)");

    TempFile migration("database_tool_test_migrationrunner_ok.sql",
        "INSERT INTO `migrations` VALUES ('1');\n"
        "INSERT INTO `t` (`id`, `name`) VALUES (1, 'a');\n");

    MigrationRunResult result = RunDoltLiteMigrations(db, { File("1", migration.path) });

    INFO(result.error);
    CHECK(result.failedFile.empty());
    REQUIRE(result.appliedIds.size() == 1);
    CHECK(result.appliedIds[0] == "1");

    DoltLiteStatement select = db.Prepare("SELECT name FROM `t` WHERE id = 1");
    REQUIRE(select.Step());
    CHECK(select.ColumnText(0) == "a");

    DoltLiteStatement log = db.Prepare("SELECT COUNT(*) FROM dolt_log WHERE message = 'migration 1'");
    REQUIRE(log.Step());
    CHECK(log.ColumnInt64(0) == 1);
}

TEST_CASE("RunDoltLiteMigrations applies pending files in order, one dolt_log entry each", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("order");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");

    TempFile first("database_tool_test_migrationrunner_first.sql", "INSERT INTO `migrations` VALUES ('1');\n");
    TempFile second("database_tool_test_migrationrunner_second.sql", "INSERT INTO `migrations` VALUES ('2');\n");

    MigrationRunResult result = RunDoltLiteMigrations(db, { File("1", first.path), File("2", second.path) });

    INFO(result.error);
    CHECK(result.failedFile.empty());
    REQUIRE(result.appliedIds.size() == 2);
    CHECK(result.appliedIds[0] == "1");
    CHECK(result.appliedIds[1] == "2");

    DoltLiteStatement log = db.Prepare("SELECT COUNT(*) FROM dolt_log WHERE message LIKE 'migration %'");
    REQUIRE(log.Step());
    CHECK(log.ColumnInt64(0) == 2);
}

TEST_CASE("RunDoltLiteMigrations rolls back a failing statement, leaving no data and no migrations row", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("fail_statement");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");
    db.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY)");

    TempFile migration("database_tool_test_migrationrunner_bad_statement.sql",
        "INSERT INTO `migrations` VALUES ('1');\n"
        "INSERT INTO `t` (`id`) VALUES (1);\n"
        "NOT VALID SQL;\n");

    MigrationRunResult result = RunDoltLiteMigrations(db, { File("1", migration.path) });

    CHECK(result.appliedIds.empty());
    CHECK(result.failedFile == migration.path.filename().string());

    DoltLiteStatement rows = db.Prepare("SELECT COUNT(*) FROM `t`");
    REQUIRE(rows.Step());
    CHECK(rows.ColumnInt64(0) == 0);

    DoltLiteStatement applied = db.Prepare("SELECT COUNT(*) FROM `migrations`");
    REQUIRE(applied.Step());
    CHECK(applied.ColumnInt64(0) == 0);
}

TEST_CASE("RunDoltLiteMigrations fails a file that never inserts its own migrations row", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("missing_row");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");
    db.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY)");

    TempFile migration("database_tool_test_migrationrunner_missing_row.sql",
        "INSERT INTO `t` (`id`) VALUES (1);\n");

    MigrationRunResult result = RunDoltLiteMigrations(db, { File("1", migration.path) });

    CHECK(result.appliedIds.empty());
    CHECK(result.failedFile == migration.path.filename().string());
    CHECK(result.error.find("migrations") != std::string::npos);

    DoltLiteStatement rows = db.Prepare("SELECT COUNT(*) FROM `t`");
    REQUIRE(rows.Step());
    CHECK(rows.ColumnInt64(0) == 0);
}

TEST_CASE("RunDoltLiteMigrations reports an unported file without running it", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("legacy");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");

    MigrationFile legacy = File("1", std::filesystem::path("does_not_exist_world.sql"), true);

    MigrationRunResult result = RunDoltLiteMigrations(db, { legacy });

    CHECK(result.appliedIds.empty());
    CHECK(result.failedFile == "does_not_exist_world.sql");
    CHECK(result.error.find("ported") != std::string::npos);
}

TEST_CASE("RunDoltLiteMigrations stops at the first failure and does not run later files", "[MigrationRunner][DoltLite]")
{
    TempDoltLiteFile world("stop");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");

    TempFile bad("database_tool_test_migrationrunner_stop_bad.sql", "NOT VALID SQL;\n");
    TempFile after("database_tool_test_migrationrunner_stop_after.sql", "INSERT INTO `migrations` VALUES ('2');\n");

    MigrationRunResult result = RunDoltLiteMigrations(db, { File("1", bad.path), File("2", after.path) });

    CHECK(result.appliedIds.empty());
    CHECK(result.failedFile == bad.path.filename().string());

    DoltLiteStatement applied = db.Prepare("SELECT COUNT(*) FROM `migrations`");
    REQUIRE(applied.Step());
    CHECK(applied.ColumnInt64(0) == 0);
}
