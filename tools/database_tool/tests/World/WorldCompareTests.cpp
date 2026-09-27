#include "World/WorldCompare.h"

#include "World/DoltLiteDatabase.h"

#include "Database/DatabaseEnv.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>

namespace
{
    // These tests need a real MySQL/MariaDB server and are opt-in: set
    // DATABASE_TOOL_TEST_MYSQL_CONNECTION to "host;port;user;pass" (no
    // database name -- these tests create their own, uniquely prefixed, so
    // they never touch whatever databases that server already has).
    char const* MysqlConnectionEnv()
    {
        return std::getenv("DATABASE_TOOL_TEST_MYSQL_CONNECTION");
    }

    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_worldcompare_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile()
        {
            std::filesystem::remove(path);
        }
    };

    // Owns both the admin connection (to CREATE/DROP the scratch database)
    // and the connection to it, so a test only has to build the schema and
    // call CompareWorldWithMysql.
    struct MysqlFixture
    {
        std::string const dbName = "test_database_tool_worldcompare";
        DatabaseMysql keepAlive; // see CreateMysqlDatabaseTests.cpp for why this stays alive
        DatabaseMysql db;

        explicit MysqlFixture(char const* connection)
        {
            REQUIRE(keepAlive.Initialize(connection, 1, 0));
            keepAlive.DirectExecute("DROP DATABASE IF EXISTS `" + dbName + "`");
            keepAlive.DirectExecute("CREATE DATABASE `" + dbName + "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci");

            std::string const connectionInfo = std::string(connection) + ";" + dbName;
            REQUIRE(db.Initialize(connectionInfo.c_str(), 1, 0));
        }

        ~MysqlFixture()
        {
            keepAlive.DirectExecute("DROP DATABASE IF EXISTS `" + dbName + "`");
        }
    };
}

TEST_CASE("CompareWorldWithMysql reports identical tables as identical", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `t` (`id` INT NOT NULL, `name` VARCHAR(50), `score` FLOAT, PRIMARY KEY (`id`))"));
    REQUIRE(mysql.db.DirectExecute("INSERT INTO `t` (`id`, `name`, `score`) VALUES (1, 'a', 1.5), (2, NULL, -2.25)"));

    TempDoltLiteFile worldFile("identical");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY, `name` TEXT, `score` REAL)");
    world.Execute("INSERT INTO `t` (`id`, `name`, `score`) VALUES (1, 'a', 1.5), (2, NULL, -2.25)");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    CHECK(result.tablesOnlyInWorld.empty());
    CHECK(result.tablesOnlyInMysql.empty());
    REQUIRE(result.tableComparisons.size() == 1);

    WorldTableComparison const& comparison = result.tableComparisons[0];
    CHECK(comparison.table == "t");
    CHECK(comparison.columnsMatch);
    CHECK(comparison.worldRowCount == 2);
    CHECK(comparison.mysqlRowCount == 2);
    CHECK(comparison.rowCountMatches);
    CHECK(comparison.rowDifferences.empty());
    CHECK(result.Identical());
}

TEST_CASE("CompareWorldWithMysql lists tables that exist on only one side", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `only_in_mysql` (`id` INT NOT NULL, PRIMARY KEY (`id`))"));

    TempDoltLiteFile worldFile("table_sets");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `only_in_world` (`id` INTEGER PRIMARY KEY)");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    REQUIRE(result.tablesOnlyInWorld.size() == 1);
    CHECK(result.tablesOnlyInWorld[0] == "only_in_world");
    REQUIRE(result.tablesOnlyInMysql.size() == 1);
    CHECK(result.tablesOnlyInMysql[0] == "only_in_mysql");
    CHECK(result.tableComparisons.empty());
    CHECK_FALSE(result.Identical());
}

TEST_CASE("CompareWorldWithMysql reports a column mismatch without comparing rows", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `t` (`id` INT NOT NULL, `extra` INT, PRIMARY KEY (`id`))"));
    REQUIRE(mysql.db.DirectExecute("INSERT INTO `t` (`id`, `extra`) VALUES (1, 1)"));

    TempDoltLiteFile worldFile("column_mismatch");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY)");
    world.Execute("INSERT INTO `t` (`id`) VALUES (1)");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    REQUIRE(result.tableComparisons.size() == 1);
    WorldTableComparison const& comparison = result.tableComparisons[0];
    CHECK_FALSE(comparison.columnsMatch);
    CHECK(comparison.worldColumns == std::vector<std::string>{ "id" });
    CHECK(comparison.mysqlColumns == std::vector<std::string>{ "id", "extra" });
    CHECK(comparison.worldRowCount == 0); // never queried once columns disagree
    CHECK_FALSE(result.Identical());
}

TEST_CASE("CompareWorldWithMysql reports a row count mismatch", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `t` (`id` INT NOT NULL, PRIMARY KEY (`id`))"));
    REQUIRE(mysql.db.DirectExecute("INSERT INTO `t` (`id`) VALUES (1), (2)"));

    TempDoltLiteFile worldFile("row_count_mismatch");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY)");
    world.Execute("INSERT INTO `t` (`id`) VALUES (1)");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    REQUIRE(result.tableComparisons.size() == 1);
    WorldTableComparison const& comparison = result.tableComparisons[0];
    CHECK(comparison.columnsMatch);
    CHECK(comparison.worldRowCount == 1);
    CHECK(comparison.mysqlRowCount == 2);
    CHECK_FALSE(comparison.rowCountMatches);
    CHECK_FALSE(result.Identical());
}

TEST_CASE("CompareWorldWithMysql reports the first differing row, by primary key order", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `t` (`id` INT NOT NULL, `name` VARCHAR(50), PRIMARY KEY (`id`))"));
    REQUIRE(mysql.db.DirectExecute("INSERT INTO `t` (`id`, `name`) VALUES (1, 'a'), (2, 'mysql')"));

    TempDoltLiteFile worldFile("row_mismatch");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY, `name` TEXT)");
    world.Execute("INSERT INTO `t` (`id`, `name`) VALUES (1, 'a'), (2, 'world')");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    REQUIRE(result.tableComparisons.size() == 1);
    WorldTableComparison const& comparison = result.tableComparisons[0];
    CHECK(comparison.rowCountMatches);
    REQUIRE(comparison.rowDifferences.size() == 1);
    CHECK(comparison.rowDifferences[0].rowIndex == 1); // second row, ordered by id
    CHECK(comparison.rowDifferences[0].column == "name");
    CHECK(comparison.rowDifferences[0].worldValue == "world");
    CHECK(comparison.rowDifferences[0].mysqlValue == "mysql");
    CHECK_FALSE(result.Identical());
}

TEST_CASE("CompareWorldWithMysql compares float columns as float32, not as text", "[WorldCompare][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    MysqlFixture mysql(connection);
    REQUIRE(mysql.db.DirectExecute("CREATE TABLE `t` (`id` INT NOT NULL, `score` FLOAT, PRIMARY KEY (`id`))"));
    REQUIRE(mysql.db.DirectExecute("INSERT INTO `t` (`id`, `score`) VALUES (1, 1.1)"));

    TempDoltLiteFile worldFile("float_precision");
    DoltLiteDatabase world(worldFile.path.string());
    world.Execute("CREATE TABLE `t` (`id` INTEGER PRIMARY KEY, `score` REAL)");
    // A double literal, same as MysqlDdlTranslator/MysqlDumpImport store: SQLite
    // keeps the full double precision of 1.1, which MySQL's single-precision
    // FLOAT column never had. Comparing raw text would see these differ.
    world.Execute("INSERT INTO `t` (`id`, `score`) VALUES (1, 1.1)");

    WorldCompareResult result = CompareWorldWithMysql(world, mysql.db);

    REQUIRE(result.tableComparisons.size() == 1);
    CHECK(result.tableComparisons[0].rowDifferences.empty());
    CHECK(result.Identical());
}
