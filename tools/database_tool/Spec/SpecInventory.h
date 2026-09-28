#ifndef DATABASE_TOOL_SPEC_SPECINVENTORY_H
#define DATABASE_TOOL_SPEC_SPECINVENTORY_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct SpecInitResult
{
    uint32_t columnsAdded = 0;
    uint32_t dbcColumnsAdded = 0;
    // "<source>:<table>.<column>" / "<table>.<column>" rows the spec files still list that no
    // longer exist in the reference database -- reported, not deleted (phase0-plan.md 3.2).
    std::vector<std::string> missingColumns;
    std::vector<std::string> missingDbcColumns;
    std::string error;
};

// database_tool spec init (phase0-plan.md step 3.2): reads `referencePath` and appends, to
// `specDir`/columns.tsv, any column of a srv_*/vdbc_* table not already listed there, and to
// `specDir`/dbc-columns.tsv, any column of a cdbc_* table that has a same-named vdbc_* table
// (i.e. can receive vanilla rows) not already listed there. New rows get every hand-filled
// field empty. Never rewrites a filled row; safe to re-run as the reference database grows.
SpecInitResult UpdateSpecInventory(std::filesystem::path const& referencePath, std::filesystem::path const& specDir);

// A (source, table[, column]) match for `spec set`; an empty `column` means every column of
// that table.
struct SpecSetFilter
{
    std::string source;
    std::string table;
    std::string column;
};

// Fields to apply; unset ones are left untouched on rows spec set does update.
struct SpecSetValues
{
    std::optional<std::string> disposition;
    std::optional<std::string> target;
    std::optional<std::string> rule;
    std::optional<std::string> reason;
    std::optional<std::string> reviewed;
};

struct SpecSetResult
{
    uint32_t matched = 0; // rows the filter selected
    uint32_t updated = 0; // rows actually written (matched, and either empty or --overwrite)
    std::string error;
};

// database_tool spec set (phase0-plan.md step 3.2): applies `values` to every row of
// `specDir`/columns.tsv matching `filter`. A row whose disposition is already non-empty is
// left alone unless `overwrite` is set.
SpecSetResult SetSpecColumns(std::filesystem::path const& specDir, SpecSetFilter const& filter, SpecSetValues const& values, bool overwrite);

#endif
