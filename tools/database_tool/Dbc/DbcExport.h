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
    bool unchanged = false; // ExportDb2Options::onlyChanged: the existing file already holds these rows
    std::string error; // empty on success
};

struct DbcExportResult
{
    std::vector<DbcExportTableResult> tables;
    std::string error; // set only if the whole operation had to abort
};

struct ExportDb2Options
{
    // Leave a file alone when it already decodes to exactly the table's rows, so unchanged stock
    // files keep their original bytes.
    bool onlyChanged = false;
    // When set, a file about to be overwritten is first copied here (unless already there).
    std::filesystem::path backupDir;
};

// Exports dbc_<name> for every table in `tables` (GetDbcTables() by default), or just the ones
// named in `onlyTables` (case-insensitive) when it's non-empty, overwriting
// dbfilesclientDir/<fileStem>.db2 for each (dbc-formats.md). Header identity (TableHash,
// LayoutHash, Locale, Flags, schema string) comes from that table's dbc_files row; a table with
// no such row is reported (hasFilesRow=false) and skipped, not an error.
DbcExportResult ExportDb2Files(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables = {},
                                std::span<DbcTableMeta const> tables = GetDbcTables(),
                                ExportDb2Options const& options = {});

// Same as ExportDb2Files, but writes dbfilesclient/<fileStem>.dbc (WDBC) instead.
DbcExportResult ExportDbcFiles(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables = {},
                                std::span<DbcTableMeta const> tables = GetDbcTables());

#endif
