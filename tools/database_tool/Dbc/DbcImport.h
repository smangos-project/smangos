#ifndef DATABASE_TOOL_DBC_DBCIMPORT_H
#define DATABASE_TOOL_DBC_DBCIMPORT_H

#include "Dbc/DbcTableMeta.h"

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

class DoltLiteDatabase;

struct DbcImportTableResult
{
    std::string tableName;
    bool fileFound = false;
    uint32_t rowsImported = 0;
    std::string error; // empty on success; a per-table read/import problem, not fatal to the rest
};

struct DbcImportResult
{
    std::vector<DbcImportTableResult> tables;
    std::string error; // set only if the whole operation had to abort (e.g. the transaction failed)
};

// Imports dbfilesclient/<fileStem>.db2 for every table in `tables` (GetDbcTables() by default),
// or just the ones named in `onlyTables` (case-insensitive) when it's non-empty, into `world`:
// recreates dbc_<name>, bulk-inserts every row and upserts its dbc_files bookkeeping row, all in
// one transaction (dbc-formats.md). A table with no file on disk is
// reported (fileFound=false) and left untouched, not an error; a table whose file fails to
// decode is reported with its error and also left untouched.
DbcImportResult ImportDb2Files(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables = {},
                                std::span<DbcTableMeta const> tables = GetDbcTables());

// Same as ImportDb2Files, but for dbfilesclient/<fileStem>.dbc (WDBC); dbc_files.format is 'dbc'.
DbcImportResult ImportDbcFiles(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables = {},
                                std::span<DbcTableMeta const> tables = GetDbcTables());

#endif
