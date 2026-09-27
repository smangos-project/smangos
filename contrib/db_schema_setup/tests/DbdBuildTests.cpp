#include "DbdBuild.h"

#include <catch_amalgamated.hpp>

TEST_CASE("DbdBuild parses x.y.z.b", "[DbdBuild]")
{
    DbdBuild build = DbdBuild::Parse("1.15.6.58797");
    CHECK(build.expansion == 1);
    CHECK(build.major == 15);
    CHECK(build.minor == 6);
    CHECK(build.build == 58797);
    CHECK(build.ToString() == "1.15.6.58797");
}

TEST_CASE("DbdBuild::TryParse rejects malformed strings", "[DbdBuild]")
{
    DbdBuild build;
    CHECK_FALSE(DbdBuild::TryParse("1.15.6", build));
    CHECK_FALSE(DbdBuild::TryParse("1.15.6.x", build));
    CHECK_FALSE(DbdBuild::TryParse("1.15.6.58797.1", build));
    CHECK_FALSE(DbdBuild::TryParse("", build));
}

TEST_CASE("DbdBuild orders by expansion, major, minor, then build", "[DbdBuild]")
{
    CHECK(DbdBuild::Parse("1.0.0.0") < DbdBuild::Parse("2.0.0.0"));
    CHECK(DbdBuild::Parse("1.0.0.0") < DbdBuild::Parse("1.1.0.0"));
    CHECK(DbdBuild::Parse("1.1.0.0") < DbdBuild::Parse("1.1.1.0"));
    CHECK(DbdBuild::Parse("1.1.1.0") < DbdBuild::Parse("1.1.1.1"));
    CHECK(DbdBuild::Parse("1.15.6.58797") == DbdBuild::Parse("1.15.6.58797"));
}

TEST_CASE("DbdBuildRange parses a-b", "[DbdBuild]")
{
    DbdBuildRange range = DbdBuildRange::Parse("1.15.6.58185-1.15.6.59415");
    CHECK(range.min == DbdBuild::Parse("1.15.6.58185"));
    CHECK(range.max == DbdBuild::Parse("1.15.6.59415"));
}

// Classic Era, Cata Classic and retail build numbers interleave, so a bare build-number
// comparison across ranges from different expansions/majors is meaningless without also
// checking those fields.
TEST_CASE("DbdBuildRange::Contains matches on expansion, major and build, but not minor", "[DbdBuild]")
{
    DbdBuildRange range = DbdBuildRange::Parse("1.15.3.55112-1.15.3.56626");

    CHECK(range.Contains(DbdBuild::Parse("1.15.3.55917")));
    CHECK_FALSE(range.Contains(DbdBuild::Parse("1.15.3.55111"))); // below min build
    CHECK_FALSE(range.Contains(DbdBuild::Parse("1.15.3.56627"))); // above max build
    CHECK_FALSE(range.Contains(DbdBuild::Parse("1.16.3.55917"))); // wrong major
    CHECK_FALSE(range.Contains(DbdBuild::Parse("2.15.3.55917"))); // wrong expansion

    // minor (the third component) is not checked at all: a build with a minor version outside
    // [1.15.3, 1.15.3] but with matching expansion/major/build is still "contained".
    CHECK(range.Contains(DbdBuild::Parse("1.15.9.55917")));
}
