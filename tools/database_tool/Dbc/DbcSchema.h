#ifndef DATABASE_TOOL_DBC_DBCSCHEMA_H
#define DATABASE_TOOL_DBC_DBCSCHEMA_H

#include "Dbc/DbcTableMeta.h"

#include <cstdint>
#include <string>
#include <vector>

class DoltLiteDatabase;

// The SQL column name(s) one metadata field maps to: the field name itself for a scalar field,
// or "Name1".."NameN" for an array. Shared with the WDC5/WDBC import/export path.
std::vector<std::string> DbcColumnNames(DbcFieldMeta const& field);

// CREATE TABLE for one metadata table (e.g. "dbc_areatable"), per dbc-formats.md's type mapping.
// Doesn't include its secondary indexes.
std::string BuildDbcTableDdl(DbcTableMeta const& table);

// CREATE INDEX for every foreign-key field in `table`. Foreign keys aren't enforced (client
// data has dangling references), so these are plain indexes, just to keep joins fast.
std::vector<std::string> BuildDbcTableIndexDdl(DbcTableMeta const& table);

// The bookkeeping table recording one row per imported file (dbc-formats.md).
std::string BuildDbcFilesTableDdl();

struct DbcSchemaResult
{
    uint32_t tablesCreated = 0;
    std::string error;
};

// Drops (if present) and recreates every dbc_* table plus dbc_files, in one transaction, then
// commits `world` with `message`.
DbcSchemaResult CreateDbcTables(DoltLiteDatabase& world, std::string const& message);

#endif
