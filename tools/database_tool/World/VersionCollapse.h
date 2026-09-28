#ifndef DATABASE_TOOL_WORLD_VERSIONCOLLAPSE_H
#define DATABASE_TOOL_WORLD_VERSIONCOLLAPSE_H

#include <cstdint>
#include <string>
#include <vector>

class DoltLiteDatabase;

// The vanilla version every versioned server table collapses to (stuff-plan.md section 6):
// build 5875, patch 10. The server itself picks the same values (SUPPORTED_CLIENT_BUILD and the
// configured WowPatch), so both are parameters rather than literals baked into the SQL.
struct CollapseTarget
{
    uint32_t build = 5875;
    uint32_t patch = 10;
};

enum class VersionColumnKind
{
    None,       // no version column: the table is copied as-is
    Build,      // "build": keep the greatest value <= target.build per logical key
    Patch,      // "patch": keep the greatest value <= target.patch per logical key
    PatchRange, // "patch_min"/"patch_max": keep rows where patch_min <= target.patch <= patch_max
};

struct VersionColumnInfo
{
    VersionColumnKind kind = VersionColumnKind::None;
    std::string column;    // "build" or "patch"; empty for None/PatchRange
    std::string minColumn; // "patch_min"; only for PatchRange
    std::string maxColumn; // "patch_max"; only for PatchRange
};

// Looks for "build", "patch", or the "patch_min"/"patch_max" pair among `columns"
// (case-insensitive). A table has at most one of these (stuff-plan.md's 77 versioned tables).
VersionColumnInfo DetectVersionColumn(std::vector<std::string> const& columns);

struct CollapseReport
{
    std::string table;
    uint64_t rowsBefore = 0;
    uint64_t rowsAfter = 0;
    uint64_t distinctKeysBefore = 0;
    uint64_t keysWithNoRow = 0; // logical keys present before the collapse with no qualifying row
};

// The plain `SELECT <columns> FROM "table" ...` (no trailing semicolon) that reproduces
// `table`'s vanilla-version rows: every row kept as-is when `info.kind` is None; otherwise
// filtered per `info` and, for Build/Patch, deduplicated to one row per logical key
// (`keyColumns`, i.e. the primary key minus the version column) -- the greatest qualifying
// version wins, ties broken by rowid (SQLStorageImpl.h's LoadProgressive rule). The caller wraps
// this in an INSERT or runs it directly.
std::string BuildCollapseSelect(std::string const& table, std::vector<std::string> const& columns,
                                 std::vector<std::string> const& keyColumns, VersionColumnInfo const& info,
                                 CollapseTarget const& target = {});

// Runs BuildCollapseSelect against `db` and inserts the result into `destTable` (already created
// with the same `columns`, no rows yet). Returns a report; throws DoltLiteError on a SQL failure.
CollapseReport CollapseTableInto(DoltLiteDatabase& db, std::string const& sourceTable, std::string const& destTable,
                                  std::vector<std::string> const& columns, std::vector<std::string> const& keyColumns,
                                  VersionColumnInfo const& info, CollapseTarget const& target = {});

#endif
