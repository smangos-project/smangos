#include "Sql/MysqlDumpImport.h"

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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dumpimport_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };

    // Shaped like the real dump: every statement has a `-- Dumping ...`
    // comment (no delimiter of its own) glued onto its front by the
    // splitter, and data sections are wrapped in versioned
    // `/*!40000 ALTER TABLE ... {DIS,EN}ABLE KEYS */` statements.
    std::string const kFixtureDump =
        "-- --------------------------------------------------------\n"
        "-- Host: 127.0.0.1\n"
        "-- --------------------------------------------------------\n"
        "\n"
        "/*!40101 SET NAMES utf8 */;\n"
        "\n"
        "-- Dumping structure for table mangos.t\n"
        "DROP TABLE IF EXISTS `t`;\n"
        "CREATE TABLE IF NOT EXISTS `t` (\n"
        "  `id` int(10) NOT NULL,\n"
        "  `name` varchar(50) DEFAULT NULL,\n"
        "  `score` float DEFAULT NULL,\n"
        "  PRIMARY KEY (`id`),\n"
        "  KEY `name_idx` (`name`)\n"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8;\n"
        "\n"
        "-- Dumping data for table mangos.t: 3 rows\n"
        "/*!40000 ALTER TABLE `t` DISABLE KEYS */;\n"
        "INSERT INTO `t` (`id`,`name`,`score`) VALUES (1,'a',1.5),(2,NULL,-2.25),(3,'it''s',0);\n"
        "/*!40000 ALTER TABLE `t` ENABLE KEYS */;\n";
}

TEST_CASE("ImportMysqlDump imports tables, rows and a secondary index", "[MysqlDumpImport]")
{
    TempFile dump("database_tool_test_dump.sql", kFixtureDump);
    TempDoltLiteFile world("ok");
    DoltLiteDatabase db(world.path.string());

    DumpImportResult result = ImportMysqlDump(db, dump.path);

    INFO(result.error);
    REQUIRE(result.ok);
    CHECK(result.tablesCreated == 1);
    CHECK(result.rowsInserted == 3);

    DoltLiteStatement select = db.Prepare("SELECT id, name, score FROM t ORDER BY id");
    REQUIRE(select.Step());
    CHECK(select.ColumnInt64(0) == 1);
    CHECK(select.ColumnText(1) == "a");
    CHECK(select.ColumnDouble(2) == Catch::Approx(1.5));

    REQUIRE(select.Step());
    CHECK(select.ColumnInt64(0) == 2);
    CHECK(select.ColumnIsNull(1));
    CHECK(select.ColumnDouble(2) == Catch::Approx(-2.25));

    REQUIRE(select.Step());
    CHECK(select.ColumnInt64(0) == 3);
    CHECK(select.ColumnText(1) == "it's");

    CHECK_FALSE(select.Step());

    DoltLiteStatement indexList = db.Prepare(
        "SELECT name FROM sqlite_master WHERE type = 'index' AND name = 't__name_idx'");
    CHECK(indexList.Step());
}

TEST_CASE("ImportMysqlDump reports an unrecognized statement with its line number and rolls back", "[MysqlDumpImport]")
{
    TempFile dump("database_tool_test_dump_bad.sql",
        "DROP TABLE IF EXISTS `t`;\n"
        "CREATE TABLE IF NOT EXISTS `t` (`id` int(10) NOT NULL, PRIMARY KEY (`id`));\n"
        "ALTER TABLE `t` ADD COLUMN `x` int(10);\n");
    TempDoltLiteFile world("bad");
    DoltLiteDatabase db(world.path.string());

    DumpImportResult result = ImportMysqlDump(db, dump.path);

    CHECK_FALSE(result.ok);
    CHECK(result.error.find("line 3") != std::string::npos);

    DoltLiteStatement tables = db.Prepare("SELECT name FROM sqlite_master WHERE type = 'table' AND name = 't'");
    CHECK_FALSE(tables.Step());
}
