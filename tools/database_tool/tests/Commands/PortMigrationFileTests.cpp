#include "Commands/PortMigrationFile.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

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

    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

TEST_CASE("PortMigrationFile overwrites a clean file with its ported content", "[PortMigrationFile]")
{
    TempFile migration("20210101000000_world.sql",
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n"
        "DELIMITER ??\n"
        "CREATE PROCEDURE `Migration_20210101000000`()\n"
        "BEGIN\n"
        "  DECLARE v INT;\n"
        "  SELECT COUNT(*) INTO v FROM `migrations` WHERE `id` = '20210101000000';\n"
        "  IF v=0 THEN\n"
        "    INSERT INTO `migrations` VALUES ('20210101000000');\n"
        "    UPDATE `creature_template` SET `faction` = 21 WHERE `entry` = 1;\n"
        "  END IF;\n"
        "END??\n"
        "DELIMITER ;\n"
        "CALL `Migration_20210101000000`();\n"
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n");

    PortMigrationFileResult result = PortMigrationFile(migration.path);

    INFO(result.flagReason);
    CHECK(result.error.empty());
    CHECK_FALSE(result.flagged);

    std::string content = ReadFile(migration.path);
    CHECK(content.find("INSERT INTO `migrations` VALUES ('20210101000000');") != std::string::npos);
    CHECK(content.find("UPDATE `creature_template` SET `faction` = 21 WHERE `entry` = 1;") != std::string::npos);
    CHECK(content.find("DELIMITER") == std::string::npos);
    CHECK(content.find("CREATE PROCEDURE") == std::string::npos);
}

TEST_CASE("PortMigrationFile leaves a flagged file untouched", "[PortMigrationFile]")
{
    std::string const original =
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n"
        "DELIMITER ??\n"
        "CREATE PROCEDURE `Migration_20210101000000`()\n"
        "BEGIN\n"
        "  DECLARE v INT;\n"
        "  SELECT COUNT(*) INTO v FROM `migrations` WHERE `id` = '20210101000000';\n"
        "  IF v=0 THEN\n"
        "    INSERT INTO `migrations` VALUES ('20210101000000');\n"
        "    ALTER TABLE `creature_template` ADD COLUMN `x` int(10);\n"
        "  END IF;\n"
        "END??\n"
        "DELIMITER ;\n"
        "CALL `Migration_20210101000000`();\n"
        "DROP PROCEDURE IF EXISTS `Migration_20210101000000`;\n";
    TempFile migration("20210101000000_world.sql", original);

    PortMigrationFileResult result = PortMigrationFile(migration.path);

    CHECK(result.error.empty());
    CHECK(result.flagged);
    CHECK(result.flagReason.find("ALTER TABLE") != std::string::npos);
    CHECK(ReadFile(migration.path) == original);
}

TEST_CASE("PortMigrationFile rejects a path that isn't a world migration filename", "[PortMigrationFile]")
{
    TempFile notAMigration("database_tool_test_not_a_migration.sql", "SELECT 1;\n");

    PortMigrationFileResult result = PortMigrationFile(notAMigration.path);

    CHECK_FALSE(result.flagged);
    CHECK_FALSE(result.error.empty());
}

TEST_CASE("PortMigrationFile rejects a non-world database suffix", "[PortMigrationFile]")
{
    TempFile logsMigration("20210101000000_logs.sql", "SELECT 1;\n");

    PortMigrationFileResult result = PortMigrationFile(logsMigration.path);

    CHECK_FALSE(result.flagged);
    CHECK_FALSE(result.error.empty());
}
