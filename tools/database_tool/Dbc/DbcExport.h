#ifndef DATABASE_TOOL_DBC_DBCEXPORT_H
#define DATABASE_TOOL_DBC_DBCEXPORT_H

#include "Dbc/DbcTableMeta.h"

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

class DoltLiteDatabase;

struct DbcExportTableResult
{
    std::string tableName;
    bool hasFilesRow = false; // false when dbc_files has no row for this table (skipped)
    uint32_t rowsExported = 0;
    std::string error; // empty on success
};

struct DbcExportResult
{
    std::vector<DbcExportTableResult> tables;
    std::string error; // set only if the whole operation had to abort
};

// Exports dbc_<name> for every table in `tables` (GetDbcTables() by default), or just the ones
// named in `onlyTables` (case-insensitive) when it's non-empty, overwriting
// dbfilesclientDir/<fileStem>.db2 for each (dbc-formats.md). Header identity (TableHash,
// LayoutHash, Locale, Flags, schema string) comes from that table's dbc_files row; a table with
// no such row is reported (hasFilesRow=false) and skipped, not an error.
DbcExportResult ExportDb2Files(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables = {},
                                std::span<DbcTableMeta const> tables = GetDbcTables());

#endif
