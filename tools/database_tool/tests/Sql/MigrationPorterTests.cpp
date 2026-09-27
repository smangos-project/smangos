#include "Sql/MigrationPorter.h"

#include "Migrations/MigrationFile.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

namespace
{
    std::string WrapInEnvelope(std::string const& id, std::string const& body)
    {
        return
            "DROP PROCEDURE IF EXISTS add_migration;\n"
            "delimiter ??\n"
            "CREATE PROCEDURE `add_migration`()\n"
            "BEGIN\n"
            "DECLARE v INT DEFAULT 1;\n"
            "SET v = (SELECT COUNT(*) FROM `migrations` WHERE `id`='" + id + "');\n"
            "IF v=0 THEN\n"
            "INSERT INTO `migrations` VALUES ('" + id + "');\n"
            "-- Add your query below.\n"
            "\n" + body + "\n"
            "-- End of migration.\n"
            "END IF;\n"
            "END??\n"
            "delimiter ;\n"
            "CALL add_migration();\n"
            "DROP PROCEDURE IF EXISTS add_migration;\n";
    }

    bool Contains(std::string const& haystack, std::string const& needle)
    {
        return haystack.find(needle) != std::string::npos;
    }

    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

TEST_CASE("PortMysqlMigration strips the envelope and keeps the migrations line and body", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("20990101000000",
        WrapInEnvelope("20990101000000", "UPDATE `creature_template` SET `faction` = 21 WHERE `entry` = 10389;"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(result.content.find("INSERT INTO `migrations` VALUES ('20990101000000');") == 0);
    CHECK(Contains(result.content, "UPDATE `creature_template` SET `faction` = 21 WHERE `entry` = 10389"));
    CHECK_FALSE(Contains(result.content, "DROP PROCEDURE"));
    CHECK_FALSE(Contains(result.content, "DECLARE"));
    CHECK_FALSE(Contains(result.content, "END IF"));
}

TEST_CASE("PortMysqlMigration substitutes a literal @variable assignment", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1",
            "SET @OGUID = 22202;\n"
            "INSERT INTO `gameobject` (`guid`, `id`) VALUES (@OGUID+1, 100);"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK_FALSE(Contains(result.content, "SET @OGUID"));
    CHECK(Contains(result.content, "VALUES (22202+1, 100)"));
}

TEST_CASE("PortMysqlMigration substitutes a variable copy and an increment", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1",
            "SET @A = 10;\n"
            "SET @B = @A;\n"
            "SET @A = @A + 5;\n"
            "INSERT INTO `t` (`a`, `b`) VALUES (@A, @B);"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "VALUES (15, 10)"));
}

TEST_CASE("PortMysqlMigration flags an @variable assignment it can't evaluate", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "SET @X = (SELECT COUNT(*) FROM `t`);"));

    CHECK(result.flagged);
    CHECK(result.content.empty());
    CHECK(Contains(result.flagReason, "@X"));
}

TEST_CASE("PortMysqlMigration flags a reference to an unknown @variable", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "INSERT INTO `t` (`a`) VALUES (@NEVER_SET);"));

    CHECK(result.flagged);
    CHECK(result.content.empty());
}

TEST_CASE("PortMysqlMigration re-escapes string literals for SQLite", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1",
            "INSERT INTO `t` (`a`) VALUES ('it\\'s here');\n"
            "INSERT INTO `t` (`a`) VALUES (\"double quoted\");\n"
            "INSERT INTO `t` (`a`) VALUES ('line1\\nline2\\ttabbed');"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "'it''s here'"));
    CHECK(Contains(result.content, "'double quoted'"));
    CHECK(Contains(result.content, "'line1\nline2' || char(9) || 'tabbed'"));
}

TEST_CASE("PortMysqlMigration rewrites TRUNCATE to DELETE FROM", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1", WrapInEnvelope("1", "TRUNCATE `locales_item`;"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "DELETE FROM `locales_item`"));
    CHECK_FALSE(Contains(result.content, "TRUNCATE"));
}

TEST_CASE("PortMysqlMigration rewrites CONCAT to ||", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "UPDATE `t` SET `content` = CONCAT( `content` , ' ');"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "(`content` || ' ')"));
    CHECK_FALSE(Contains(result.content, "CONCAT"));
}

TEST_CASE("PortMysqlMigration rewrites INSERT IGNORE to INSERT OR IGNORE", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "INSERT IGNORE INTO `t` (`a`) VALUES (1);"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "INSERT OR IGNORE INTO"));
}

TEST_CASE("PortMysqlMigration rewrites && to AND", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "UPDATE `t` SET `x` = 1 WHERE `a` = 1 && `b` = 2;"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "AND"));
    CHECK_FALSE(Contains(result.content, "&&"));
}

TEST_CASE("PortMysqlMigration flags a literal || as ambiguous", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "UPDATE `t` SET `x` = 1 WHERE `a` = 1 || `b` = 2;"));

    CHECK(result.flagged);
    CHECK(result.content.empty());
    CHECK(Contains(result.flagReason, "||"));
}

TEST_CASE("PortMysqlMigration translates an embedded CREATE TABLE", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "CREATE TABLE IF NOT EXISTS `t` (`id` int(10) NOT NULL, PRIMARY KEY (`id`));"));

    INFO(result.flagReason);
    REQUIRE_FALSE(result.flagged);
    CHECK(Contains(result.content, "CREATE TABLE `t`"));
    CHECK(Contains(result.content, "INTEGER"));
}

TEST_CASE("PortMysqlMigration flags ALTER TABLE for a hand port", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "ALTER TABLE `t` ADD COLUMN `x` int(10);"));

    CHECK(result.flagged);
    CHECK(result.content.empty());
    CHECK(Contains(result.flagReason, "ALTER TABLE"));
}

TEST_CASE("PortMysqlMigration flags a multi-table UPDATE for a hand port", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1",
        WrapInEnvelope("1", "UPDATE `a`, `b` SET `a`.`x` = `b`.`x` WHERE `a`.`id` = `b`.`id`;"));

    CHECK(result.flagged);
    CHECK(result.content.empty());
    CHECK(Contains(result.flagReason, "multi-table"));
}

TEST_CASE("PortMysqlMigration flags a file with no CREATE PROCEDURE envelope", "[MigrationPorter]")
{
    PortedMigration result = PortMysqlMigration("1", "SELECT 1;\n");

    CHECK(result.flagged);
    CHECK(Contains(result.flagReason, "envelope"));
}

TEST_CASE("PortMysqlMigration handles every real world migration without crashing", "[MigrationPorter][real-files]")
{
    size_t total = 0;
    size_t ported = 0;
    size_t flagged = 0;
    std::set<std::string> flagReasons;

    for (auto const& entry : std::filesystem::directory_iterator(DATABASE_TOOL_TEST_MIGRATIONS_DIR))
    {
        std::string id, db;
        if (!TryParseMigrationFilename(entry.path().filename().string(), id, db) || db != "world")
            continue;

        ++total;
        PortedMigration result = PortMysqlMigration(id, ReadFile(entry.path()));
        if (result.flagged)
        {
            ++flagged;
            flagReasons.insert(result.flagReason.substr(result.flagReason.find(':') + 2));
        }
        else
        {
            ++ported;
            CHECK(result.content.find("INSERT INTO `migrations` VALUES ('" + id + "');") == 0);
        }
    }

    CHECK(total > 1000);
    CHECK(ported > 0);
    CHECK(flagged > 0);

    {
        std::string summary = "total=" + std::to_string(total) + " ported=" + std::to_string(ported) +
            " flagged=" + std::to_string(flagged);
        for (std::string const& reason : flagReasons)
            summary += "\n  - " + reason;
        WARN(summary);
    }

    // Known real files, checked by id so the assertions survive future
    // migrations landing with lower/higher/equal ids.
    auto PortById = [](std::string const& id)
    {
        std::filesystem::path path = std::filesystem::path(DATABASE_TOOL_TEST_MIGRATIONS_DIR) / (id + "_world.sql");
        REQUIRE(std::filesystem::exists(path));
        return PortMysqlMigration(id, ReadFile(path));
    };

    // Has a trailing ALTER TABLE: must be flagged, not partially ported.
    PortedMigration alterTable = PortById("20210615223353");
    CHECK(alterTable.flagged);
    CHECK(alterTable.content.empty());

    // Uses CONCAT(...): must port successfully.
    PortedMigration concatFile = PortById("20230921171542");
    INFO(concatFile.flagReason);
    CHECK_FALSE(concatFile.flagged);

    // Uses TRUNCATE: must port successfully.
    PortedMigration truncateFile = PortById("20221107183131");
    INFO(truncateFile.flagReason);
    CHECK_FALSE(truncateFile.flagged);

    // Uses @variables (SET @X = <int>, then @X+<int> in an INSERT): must port successfully.
    PortedMigration variableFile = PortById("20210620150858");
    INFO(variableFile.flagReason);
    CHECK_FALSE(variableFile.flagged);
}
