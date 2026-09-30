#include "Migrations/MigrationFile.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

TEST_CASE("TryParseMigrationFilename parses <id>_<db>.sql", "[MigrationFile]")
{
    std::string id, db;
    REQUIRE(TryParseMigrationFilename("20210515110157_world.sql", id, db));
    CHECK(id == "20210515110157");
    CHECK(db == "world");
}

TEST_CASE("TryParseMigrationFilename rejects anything else found in sql/migrations", "[MigrationFile]")
{
    std::string id, db;
    CHECK_FALSE(TryParseMigrationFilename("README", id, db));
    CHECK_FALSE(TryParseMigrationFilename("merge.sh", id, db));
    CHECK_FALSE(TryParseMigrationFilename("merge.bat", id, db));
    CHECK_FALSE(TryParseMigrationFilename("world_db_updates.sql", id, db));
    CHECK_FALSE(TryParseMigrationFilename("_world.sql", id, db));
    CHECK_FALSE(TryParseMigrationFilename("20210515110157_.sql", id, db));
    CHECK_FALSE(TryParseMigrationFilename("20210515110157world.sql", id, db));
    CHECK_FALSE(TryParseMigrationFilename("abc_world.sql", id, db));
    CHECK_FALSE(TryParseMigrationFilename("20210515110157_world.txt", id, db));
}

TEST_CASE("DiscoverMigrations finds every real migration for one database, sorted by id", "[MigrationFile]")
{
    auto world = DiscoverMigrations(DATABASE_TOOL_TEST_MIGRATIONS_DIR, "world");
    CHECK(world.size() == 1156);
    CHECK(std::is_sorted(world.begin(), world.end(), [](MigrationFile const& a, MigrationFile const& b) { return a.id < b.id; }));
    for (MigrationFile const& file : world)
    {
        CHECK(file.db == "world");

        // sql/migrations is live: earlier years are already committed ported
        // to SQLite (world-sql.md's per-year batches), so hasLegacyEnvelope
        // isn't uniformly true any more -- it must just match what's
        // actually on disk for this file.
        std::ifstream stream(file.path, std::ios::binary);
        std::ostringstream buffer;
        buffer << stream.rdbuf();
        bool actuallyHasEnvelope = buffer.str().find("DROP PROCEDURE IF EXISTS add_migration") != std::string::npos;
        CHECK(file.hasLegacyEnvelope == actuallyHasEnvelope);
    }

    CHECK(DiscoverMigrations(DATABASE_TOOL_TEST_MIGRATIONS_DIR, "characters").size() == 9);
    CHECK(DiscoverMigrations(DATABASE_TOOL_TEST_MIGRATIONS_DIR, "logon").size() == 11);
    CHECK(DiscoverMigrations(DATABASE_TOOL_TEST_MIGRATIONS_DIR, "logs").size() == 7);
}

TEST_CASE("DiscoverMigrations returns nothing for an unknown database suffix", "[MigrationFile]")
{
    CHECK(DiscoverMigrations(DATABASE_TOOL_TEST_MIGRATIONS_DIR, "nope").empty());
}
