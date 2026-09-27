#include "Sql/MysqlDdlTranslator.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

namespace
{
    bool Contains(std::string const& haystack, std::string const& needle)
    {
        return haystack.find(needle) != std::string::npos;
    }
}

TEST_CASE("MysqlDdlTranslator maps every column type", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `a` tinyint(3) unsigned,\n"
        "  `b` smallint(5),\n"
        "  `c` mediumint(8),\n"
        "  `d` int(10) unsigned,\n"
        "  `e` bigint(20),\n"
        "  `f` float,\n"
        "  `g` double,\n"
        "  `h` decimal(10,2),\n"
        "  `i` char(3),\n"
        "  `j` varchar(50),\n"
        "  `k` tinytext,\n"
        "  `l` text,\n"
        "  `m` mediumtext,\n"
        "  `n` longtext,\n"
        "  `o` date,\n"
        "  `p` datetime,\n"
        "  `q` timestamp,\n"
        "  `r` blob,\n"
        "  `s` varbinary(16)\n"
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8;");

    REQUIRE(table.warnings.empty());
    std::string const& ddl = table.statements[0];
    CHECK(Contains(ddl, "`a` INTEGER"));
    CHECK(Contains(ddl, "`e` INTEGER"));
    CHECK(Contains(ddl, "`f` REAL"));
    CHECK(Contains(ddl, "`g` REAL"));
    CHECK(Contains(ddl, "`h` NUMERIC"));
    CHECK(Contains(ddl, "`i` TEXT"));
    CHECK(Contains(ddl, "`n` TEXT"));
    CHECK(Contains(ddl, "`o` TEXT"));
    CHECK(Contains(ddl, "`q` TEXT"));
    CHECK(Contains(ddl, "`r` BLOB"));
    CHECK(Contains(ddl, "`s` BLOB"));

    // Table options (ENGINE, CHARSET, ...) are dropped, not translated.
    CHECK_FALSE(Contains(ddl, "ENGINE"));
    CHECK_FALSE(Contains(ddl, "CHARSET"));
}

TEST_CASE("MysqlDdlTranslator fills in MySQL's implicit non-strict defaults", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `a` int(10) NOT NULL,\n"
        "  `b` varchar(50) NOT NULL,\n"
        "  `c` timestamp NOT NULL,\n"
        "  `d` int(10) NOT NULL DEFAULT '5',\n"
        "  `e` varchar(10) DEFAULT NULL\n"
        ") ENGINE=InnoDB;");

    std::string const& ddl = table.statements[0];
    CHECK(Contains(ddl, "`a` INTEGER NOT NULL DEFAULT 0"));
    CHECK(Contains(ddl, "`b` TEXT NOT NULL DEFAULT '' COLLATE NOCASE"));
    CHECK(Contains(ddl, "`c` TEXT NOT NULL DEFAULT '0000-00-00 00:00:00' COLLATE NOCASE"));
    CHECK(Contains(ddl, "`d` INTEGER NOT NULL DEFAULT 5")); // quotes stripped from a numeric default
    CHECK(Contains(ddl, "`e` TEXT DEFAULT NULL COLLATE NOCASE"));
}

TEST_CASE("MysqlDdlTranslator maps a single-column AUTO_INCREMENT primary key to INTEGER PRIMARY KEY", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\n"
        "  `name` varchar(50) NOT NULL DEFAULT '',\n"
        "  PRIMARY KEY (`id`),\n"
        "  KEY `idx_name` (`name`),\n"
        "  UNIQUE KEY `idx_name_unique` (`name`)\n"
        ") ENGINE=MyISAM AUTO_INCREMENT=42 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='a table';");

    REQUIRE(table.warnings.empty());
    REQUIRE(table.statements.size() == 3);

    std::string const& ddl = table.statements[0];
    CHECK(Contains(ddl, "`id` INTEGER PRIMARY KEY NOT NULL"));
    CHECK_FALSE(Contains(ddl, "PRIMARY KEY (`id`)")); // not also a separate table constraint

    CHECK(table.statements[1] == R"(CREATE INDEX "t__idx_name" ON `t` (`name`))");
    CHECK(table.statements[2] == R"(CREATE UNIQUE INDEX "t__idx_name_unique" ON `t` (`name`))");
}

TEST_CASE("MysqlDdlTranslator keeps a multi-column primary key as a table constraint", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `a` int(10) NOT NULL,\n"
        "  `b` int(10) NOT NULL,\n"
        "  PRIMARY KEY (`a`, `b`)\n"
        ") ENGINE=MyISAM;");

    CHECK(Contains(table.statements[0], "PRIMARY KEY (`a`, `b`)"));
}

TEST_CASE("MysqlDdlTranslator warns on AUTO_INCREMENT that isn't a simple integer primary key", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `a` int(10) NOT NULL AUTO_INCREMENT,\n"
        "  `b` int(10) NOT NULL,\n"
        "  PRIMARY KEY (`a`, `b`)\n"
        ") ENGINE=MyISAM;");

    REQUIRE(table.warnings.size() == 1);
    CHECK(Contains(table.warnings[0], "AUTO_INCREMENT"));
}

TEST_CASE("MysqlDdlTranslator handles an inline PRIMARY KEY with no separate table constraint", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `code` char(3) NOT NULL PRIMARY KEY,\n"
        "  `name` varchar(50) NOT NULL DEFAULT ''\n"
        ") ENGINE=MyISAM;");

    REQUIRE(table.warnings.empty());
    CHECK(Contains(table.statements[0], "PRIMARY KEY (`code`)"));
}

TEST_CASE("MysqlDdlTranslator drops a secondary index's prefix length", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `t` (\n"
        "  `name` varchar(50) NOT NULL DEFAULT '',\n"
        "  KEY `idx_name` (`name`(20))\n"
        ") ENGINE=MyISAM;");

    REQUIRE(table.statements.size() == 2);
    CHECK(table.statements[1] == R"(CREATE INDEX "t__idx_name" ON `t` (`name`))");
}

TEST_CASE("MysqlDdlTranslator produces DDL that DoltLite actually accepts", "[MysqlDdlTranslator]")
{
    auto table = TranslateMysqlCreateTable(
        "CREATE TABLE `creature` (\n"
        "  `guid` int(10) unsigned NOT NULL AUTO_INCREMENT,\n"
        "  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',\n"
        "  `position_x` float NOT NULL DEFAULT '0',\n"
        "  `spawn_flags` tinytext,\n"
        "  `comment` text,\n"
        "  PRIMARY KEY (`guid`),\n"
        "  KEY `idx_id` (`id`)\n"
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8;");

    auto path = std::filesystem::temp_directory_path() / "database_tool_test_ddl_translator.doltlite";
    std::filesystem::remove(path);
    DoltLiteDatabase db(path.string());
    for (std::string const& statement : table.statements)
        db.Execute(statement);

    // MySQL, run non-strict, lets an INSERT omit a NOT NULL column with no
    // explicit default; the implicit default we filled in must let SQLite
    // accept the same INSERT.
    db.Execute("INSERT INTO `creature` (`guid`) VALUES (1)");

    DoltLiteStatement select = db.Prepare("SELECT `id`, `position_x`, `spawn_flags` FROM `creature` WHERE `guid` = 1");
    REQUIRE(select.Step());
    CHECK(select.ColumnInt64(0) == 0);
    CHECK(select.ColumnDouble(1) == 0.0);
    CHECK(select.ColumnIsNull(2));
}
