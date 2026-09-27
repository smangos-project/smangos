#include "Commands/CreateWorldDatabase.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_createworld_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir()
        {
            std::filesystem::remove_all(path);
        }
    };

    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_createworld_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };

    void WriteFile(std::filesystem::path const& path, std::string const& contents)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file << contents;
    }
}

TEST_CASE("UpdateWorldDatabase applies pending migrations it discovers itself", "[CreateWorldDatabase]")
{
    TempDoltLiteFile world("update_ok");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");
    db.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY, `name` TEXT)");

    TempDir migrations("update_ok");
    WriteFile(migrations.path / "20210101000000_world.sql",
        "INSERT INTO `migrations` VALUES ('20210101000000');\n"
        "INSERT INTO `t` (`id`, `name`) VALUES (1, 'a');\n");
    WriteFile(migrations.path / "20210102000000_world.sql",
        "INSERT INTO `migrations` VALUES ('20210102000000');\n"
        "INSERT INTO `t` (`id`, `name`) VALUES (2, 'b');\n");

    WorldUpdateResult result = UpdateWorldDatabase(db, migrations.path);

    INFO(result.error);
    CHECK(result.failedFile.empty());
    REQUIRE(result.appliedMigrationIds.size() == 2);
    CHECK(result.appliedMigrationIds[0] == "20210101000000");
    CHECK(result.appliedMigrationIds[1] == "20210102000000");

    DoltLiteStatement count = db.Prepare("SELECT COUNT(*) FROM `t`");
    REQUIRE(count.Step());
    CHECK(count.ColumnInt64(0) == 2);
}

TEST_CASE("UpdateWorldDatabase skips already-applied migrations and only runs pending ones", "[CreateWorldDatabase]")
{
    TempDoltLiteFile world("update_partial");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");
    db.Execute("INSERT INTO `migrations` VALUES ('20210101000000')");
    db.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY)");

    TempDir migrations("update_partial");
    WriteFile(migrations.path / "20210101000000_world.sql",
        "INSERT INTO `migrations` VALUES ('20210101000000');\n"
        "INSERT INTO `t` (`id`) VALUES (999);\n"); // already applied -- must not run again
    WriteFile(migrations.path / "20210102000000_world.sql",
        "INSERT INTO `migrations` VALUES ('20210102000000');\n"
        "INSERT INTO `t` (`id`) VALUES (2);\n");

    WorldUpdateResult result = UpdateWorldDatabase(db, migrations.path);

    INFO(result.error);
    CHECK(result.failedFile.empty());
    REQUIRE(result.appliedMigrationIds.size() == 1);
    CHECK(result.appliedMigrationIds[0] == "20210102000000");

    DoltLiteStatement rows = db.Prepare("SELECT id FROM `t` ORDER BY id");
    REQUIRE(rows.Step());
    CHECK(rows.ColumnInt64(0) == 2);
    CHECK_FALSE(rows.Step());
}

TEST_CASE("UpdateWorldDatabase stops at an unported (legacy-envelope) file", "[CreateWorldDatabase]")
{
    TempDoltLiteFile world("update_legacy");
    DoltLiteDatabase db(world.path.string());
    db.Execute("CREATE TABLE `migrations` (`id` TEXT PRIMARY KEY)");

    TempDir migrations("update_legacy");
    WriteFile(migrations.path / "20210101000000_world.sql",
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n"
        "DELIMITER ??\n"
        "CREATE PROCEDURE `Migration_20210101000000`()\n"
        "BEGIN\n"
        "  DECLARE v INT;\n"
        "  SELECT COUNT(*) INTO v FROM `migrations` WHERE `id` = '20210101000000';\n"
        "  IF v=0 THEN\n"
        "    INSERT INTO `migrations` VALUES ('20210101000000');\n"
        "  END IF;\n"
        "END??\n"
        "DELIMITER ;\n"
        "CALL `Migration_20210101000000`();\n"
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n");

    WorldUpdateResult result = UpdateWorldDatabase(db, migrations.path);

    CHECK(result.appliedMigrationIds.empty());
    CHECK(result.failedFile == "20210101000000_world.sql");
    CHECK(result.error.find("ported") != std::string::npos);
}

TEST_CASE("CreateWorldDatabase refuses to overwrite an existing .world", "[CreateWorldDatabase]")
{
    TempDoltLiteFile world("exists");
    { DoltLiteDatabase db(world.path.string()); } // create the file

    TempDir cache("exists_cache");
    TempDir migrations("exists_migrations");

    WorldCreateResult result = CreateWorldDatabase(world.path, cache.path, migrations.path);

    CHECK(result.alreadyExisted);
    CHECK(result.error.find("already exists") != std::string::npos);
}

TEST_CASE("CreateWorldDatabase builds the real pinned world database end to end", "[CreateWorldDatabase][network]")
{
    // Opt-in: downloads the real 16MB archive and imports its 912,894 rows.
    if (!std::getenv("DATABASE_TOOL_WORLD_DUMP"))
    {
        WARN("DATABASE_TOOL_WORLD_DUMP not set, skipping (downloads and imports the real world dump)");
        return;
    }

    TempDoltLiteFile world("real");
    TempDir cache("real_cache");

    WorldCreateResult result = CreateWorldDatabase(world.path, cache.path, DATABASE_TOOL_TEST_MIGRATIONS_DIR);

    REQUIRE_FALSE(result.alreadyExisted);
    CHECK(result.dumpImport.ok);
    CHECK(result.dumpImport.tablesCreated == 169);
    CHECK(result.dumpImport.rowsInserted == 912894);
    // The real migrations directory is a moving target as step 13 ports more
    // of it to SQLite: either every migration so far applies cleanly, or the
    // loop stops at whichever one isn't ported yet (never any other error).
    INFO(result.update.failedFile << ": " << result.update.error);
    CHECK((result.update.error.empty() || result.update.error.find("ported") != std::string::npos));
}
