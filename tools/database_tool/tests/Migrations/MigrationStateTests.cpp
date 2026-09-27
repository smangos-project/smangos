#include "Migrations/MigrationState.h"

#include <catch_amalgamated.hpp>

namespace
{
    MigrationFile File(std::string id)
    {
        MigrationFile file;
        file.id = std::move(id);
        file.db = "world";
        return file;
    }
}

TEST_CASE("CompareMigrations finds no pending or orphaned ids when everything matches", "[MigrationState]")
{
    auto state = CompareMigrations({ File("1"), File("2") }, { "1", "2" });
    CHECK(state.pending.empty());
    CHECK(state.orphaned.empty());
}

TEST_CASE("CompareMigrations reports files not yet applied as pending", "[MigrationState]")
{
    auto state = CompareMigrations({ File("1"), File("2"), File("3") }, { "1" });
    REQUIRE(state.pending.size() == 2);
    CHECK(state.pending[0].id == "2");
    CHECK(state.pending[1].id == "3");
    CHECK(state.orphaned.empty());
}

TEST_CASE("CompareMigrations reports applied ids with no file as orphaned", "[MigrationState]")
{
    auto state = CompareMigrations({ File("1") }, { "1", "2" });
    CHECK(state.pending.empty());
    REQUIRE(state.orphaned.size() == 1);
    CHECK(state.orphaned[0] == "2");
}

TEST_CASE("CompareMigrations handles an empty migrations table", "[MigrationState]")
{
    auto state = CompareMigrations({ File("1"), File("2") }, {});
    CHECK(state.pending.size() == 2);
    CHECK(state.orphaned.empty());
}
