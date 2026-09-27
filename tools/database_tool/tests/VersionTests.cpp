#include "Version.h"

#include <catch_amalgamated.hpp>
#include <string>

TEST_CASE("DatabaseToolVersion returns a non-empty string", "[Version]")
{
    CHECK(std::string(DatabaseToolVersion()).size() > 0);
}
