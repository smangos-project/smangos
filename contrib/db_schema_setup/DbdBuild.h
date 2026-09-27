#ifndef DB_SCHEMA_SETUP_DBDBUILD_H
#define DB_SCHEMA_SETUP_DBDBUILD_H

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>

// A WoW build number in x.y.z.b form (expansion.major.minor.build).
struct DbdBuild
{
    int32_t expansion = 0;
    int32_t major = 0;
    int32_t minor = 0;
    uint32_t build = 0;

    auto operator<=>(DbdBuild const&) const = default;

    std::string ToString() const;

    static bool TryParse(std::string_view text, DbdBuild& out);
    static DbdBuild Parse(std::string_view text);
};

// An inclusive [min, max] range of builds, as written by a DBD `BUILD a-b` line.
struct DbdBuildRange
{
    DbdBuild min;
    DbdBuild max;

    auto operator<=>(DbdBuildRange const&) const = default;

    // Ported from WoWDBDefs' DBDefsLib BuildRange.Contains: compares expansion, major and
    // build, but not minor. That is an upstream quirk, not an oversight here, and is kept
    // intact so resolution matches what WoWDBDefs itself would report.
    bool Contains(DbdBuild const& build) const;

    static bool TryParse(std::string_view text, DbdBuildRange& out);
    static DbdBuildRange Parse(std::string_view text);
};

#endif
