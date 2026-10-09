#include "Commands/CharStoreImport.h"

#include "World/DoltLiteDatabase.h"

#include "Database/DatabaseEnv.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>

TEST_CASE("WithoutRowidIfComposite only touches composite primary keys", "[charstore]")
{
    std::string const composite = "CREATE TABLE `a` (\n  `x` INTEGER,\n  `y` INTEGER,\n  PRIMARY KEY (`x`, `y`)\n)";
    CHECK(WithoutRowidIfComposite(composite) == composite + " WITHOUT ROWID");

    std::string const single = "CREATE TABLE `b` (\n  `x` INTEGER,\n  `y` TEXT,\n  PRIMARY KEY (`x`)\n)";
    CHECK(WithoutRowidIfComposite(single) == single);

    std::string const none = "CREATE TABLE `c` (\n  `x` INTEGER,\n  `y` TEXT\n)";
    CHECK(WithoutRowidIfComposite(none) == none);
}

TEST_CASE("ImportCharStore copies schema and rows into a stock sqlite file", "[charstore][mysql]")
{
    char const* connection = std::getenv("DATABASE_TOOL_TEST_MYSQL_CONNECTION");
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    DatabaseMysql mysql;
    REQUIRE(mysql.Initialize(connection, 1, 0));
    std::string const dbName = "test_database_tool_charstore";
    mysql.DirectExecute("DROP DATABASE IF EXISTS `" + dbName + "`");
    mysql.DirectExecute("CREATE DATABASE `" + dbName + "`");
    mysql.DirectExecute("CREATE TABLE `" + dbName + "`.`chars` (`guid` INT UNSIGNED NOT NULL, `name` VARCHAR(12) NOT NULL, "
        "`money` BIGINT UNSIGNED NOT NULL, `x` FLOAT NOT NULL, `note` TEXT, PRIMARY KEY (`guid`), KEY `idx_name` (`name`))");
    mysql.DirectExecute("CREATE TABLE `" + dbName + "`.`spells` (`guid` INT UNSIGNED NOT NULL, `spell` INT UNSIGNED NOT NULL, "
        "`active` TINYINT NOT NULL DEFAULT 1, PRIMARY KEY (`guid`, `spell`))");
    mysql.DirectExecute("CREATE TABLE `" + dbName + "`.`blobs` (`id` INT NOT NULL, `data` LONGBLOB, PRIMARY KEY (`id`))");
    mysql.DirectExecute("INSERT INTO `" + dbName + "`.`chars` VALUES (1, 'Alice', 18446744073709551615, -8949.5, NULL), "
        "(2, 'Bob', 0, 1.25, 'hi')");
    mysql.DirectExecute("INSERT INTO `" + dbName + "`.`spells` VALUES (1, 100, 1), (1, 200, 0), (2, 100, 1)");
    mysql.DirectExecute("INSERT INTO `" + dbName + "`.`blobs` VALUES (1, X'00FF0041'), (2, NULL)");

    std::filesystem::path const out = std::filesystem::temp_directory_path() / "database_tool_charstore_test.db";
    std::filesystem::remove(out);
    std::filesystem::remove(out.string() + "-wal");
    std::filesystem::remove(out.string() + "-shm");

    CharStoreImportOptions options;
    std::string const info = connection;
    options.mysqlInfo = info + ";" + dbName;
    options.out = out;
    options.withoutRowid = true;
    CharStoreImportResult result = ImportCharStore(options);
    INFO(result.error);
    REQUIRE(result.error.empty());
    REQUIRE(result.tables.size() == 3);

    {
        DoltLiteDatabase db(out.string());
        DoltLiteStatement engine = db.Prepare("SELECT doltlite_engine()");
        REQUIRE(engine.Step());
        CHECK(engine.ColumnText(0) == "orig");

        DoltLiteStatement chars = db.Prepare("SELECT guid, name, money, x, note FROM chars ORDER BY guid");
        REQUIRE(chars.Step());
        CHECK(chars.ColumnInt64(0) == 1);
        CHECK(chars.ColumnText(1) == "Alice");
        CHECK(static_cast<uint64_t>(chars.ColumnInt64(2)) == 18446744073709551615ull);
        CHECK(chars.ColumnDouble(3) == Catch::Approx(-8949.5));
        CHECK(chars.ColumnIsNull(4));
        REQUIRE(chars.Step());
        CHECK(chars.ColumnText(4) == "hi");

        DoltLiteStatement blob = db.Prepare("SELECT typeof(data), length(data), hex(data) FROM blobs WHERE id = 1");
        REQUIRE(blob.Step());
        CHECK(blob.ColumnText(0) == "blob");
        CHECK(blob.ColumnText(2) == "00FF0041");

        DoltLiteStatement spells = db.Prepare("SELECT count(*), (SELECT sql FROM sqlite_master WHERE name = 'spells') FROM spells");
        REQUIRE(spells.Step());
        CHECK(spells.ColumnInt64(0) == 3);
        CHECK(spells.ColumnText(1).find("WITHOUT ROWID") != std::string::npos);

        DoltLiteStatement index = db.Prepare("SELECT count(*) FROM sqlite_master WHERE type = 'index' AND tbl_name = 'chars'");
        REQUIRE(index.Step());
        CHECK(index.ColumnInt64(0) == 1);
    }

    CHECK_FALSE(ImportCharStore(options).error.empty()); // refuses to overwrite

    mysql.DirectExecute("DROP DATABASE `" + dbName + "`");
    std::filesystem::remove(out);
    std::filesystem::remove(out.string() + "-wal");
    std::filesystem::remove(out.string() + "-shm");
}
