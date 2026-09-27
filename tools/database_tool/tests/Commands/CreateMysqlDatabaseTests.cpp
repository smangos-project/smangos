#include "Commands/CreateMysqlDatabase.h"

#include "Migrations/MigrationFile.h"

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
}

TEST_CASE("CreateAndUpdateMysqlDatabase creates a database, imports the base script, and applies migrations", "[CreateMysqlDatabase][mysql]")
{
    char const* connection = MysqlConnectionEnv();
    if (!connection)
    {
        WARN("DATABASE_TOOL_TEST_MYSQL_CONNECTION not set, skipping (needs a real MySQL/MariaDB server)");
        return;
    }

    // DatabaseMysql's ctor/dtor reference-count mysql_library_init()/_end()
    // (see src/shared/Database/DatabaseMysql.cpp): the library is torn down
    // once the last DatabaseMysql in the process is destroyed, and
    // re-initializing it afterward corrupts later connections. Keeping this
    // one alive for the whole test keeps the count above zero throughout, so
    // CreateAndUpdateMysqlDatabase's own internal connections (constructed
    // and destroyed per call) never trigger that teardown/reinit.
    DatabaseMysql keepAlive;

    std::string const dbName = "test_database_tool_logs";
    std::string const connectionInfo = std::string(connection) + ";" + dbName;
    REQUIRE(keepAlive.Initialize(connection, 1, 0));
    keepAlive.DirectExecute("DROP DATABASE IF EXISTS `" + dbName + "`");

    std::filesystem::path const baseScript = std::filesystem::path(DATABASE_TOOL_TEST_SQL_DIR) / "logs.sql";
    std::filesystem::path const migrationsDir = DATABASE_TOOL_TEST_MIGRATIONS_DIR;

    MysqlSetupResult first = CreateAndUpdateMysqlDatabase(connectionInfo, baseScript, migrationsDir, "logs");
    INFO(first.error);
    REQUIRE(first.error.empty());
    CHECK(first.databaseCreated);
    CHECK(first.baseScriptImported);

    auto realFiles = DiscoverMigrations(migrationsDir, "logs");
    CHECK(first.appliedMigrationIds.size() == realFiles.size());

    // A second run against the now-current database should be a clean no-op.
    MysqlSetupResult second = CreateAndUpdateMysqlDatabase(connectionInfo, baseScript, migrationsDir, "logs");
    INFO(second.error);
    REQUIRE(second.error.empty());
    CHECK_FALSE(second.databaseCreated);
    CHECK_FALSE(second.baseScriptImported);
    CHECK(second.appliedMigrationIds.empty());

    // Spot-check the base script and a migration actually landed.
    auto tables = keepAlive.Query("SHOW TABLES FROM `" + dbName + "` LIKE 'instance_creature_kills'");
    REQUIRE(tables);
    CHECK(tables->GetRowCount() == 1);

    keepAlive.DirectExecute("DROP DATABASE IF EXISTS `" + dbName + "`");
}
