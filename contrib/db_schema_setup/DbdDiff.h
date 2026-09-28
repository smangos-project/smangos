#ifndef DB_SCHEMA_SETUP_DBDDIFF_H
#define DB_SCHEMA_SETUP_DBDDIFF_H

#include "DbdBuild.h"
#include "DbdDefinitions.h"

#include <string>
#include <vector>

// One field-level change between the same table resolved at two builds.
struct DbdFieldDiff
{
    std::string field;
    std::string kind;   // "removed", "array_length" or "type"
    std::string detail; // e.g. "4 -> 8", "string -> int"; empty for "removed"
};

// One table's diff between a `fromBuilds` resolution and a `toBuilds` resolution.
struct DbdTableDiff
{
    std::string table;
    bool removedEntirely = false; // resolved at fromBuilds, no definition at all at toBuilds
    std::vector<DbdFieldDiff> fields;
};

// For every table in `definitions` that resolves against `fromBuilds` (each build tried in
// order, like ResolveDbdTable), resolves it again against `toBuilds` and reports what changed:
// fields present at `fromBuilds` and gone at `toBuilds`, array lengths that changed, and type
// changes. A table with no `toBuilds` resolution at all (NoDefinition or Ambiguous) is reported
// as `removedEntirely` instead of a field list. Tables that don't resolve at `fromBuilds` are
// skipped -- there is nothing to diff from. Only tables with at least one reported change are
// returned, sorted by name.
std::vector<DbdTableDiff> DiffDbdTables(DbdDefinitions const& definitions,
    std::vector<DbdBuild> const& fromBuilds, std::vector<DbdBuild> const& toBuilds);

#endif
