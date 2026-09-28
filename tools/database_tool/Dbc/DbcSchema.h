#ifndef DATABASE_TOOL_DBC_DBCSCHEMA_H
#define DATABASE_TOOL_DBC_DBCSCHEMA_H

#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h" // Wdc5Row, reused by BindDbcRow

#include <cstdint>
#include <string>
#include <vector>

class DoltLiteDatabase;
class DoltLiteStatement;

// The SQL column name(s) one metadata field maps to: the field name itself for a scalar field,
// or "Name1".."NameN" for an array. Shared with the WDC5/WDBC import/export path.
// `locStringSlots` mirrors WdbcLayout: a locstring field with more than one slot expands to
// "Name_lang" (slot 0), "Name_lang_1".."_N-2" and "Name_lang_mask", matching ReadWdbcFile.
std::vector<std::string> DbcColumnNames(DbcFieldMeta const& field, uint32_t locStringSlots = 1);

// CREATE TABLE for one metadata table, per dbc-formats.md's type mapping. Doesn't include its
// secondary indexes. Named "dbc_<name>" (table.sqlTable) unless `sqlTableName` overrides it, and
// a locstring field is one column unless `locStringSlots` overrides it -- WdbcLayout's twin, for
// the reference database's 9-slot vanilla tables ("vdbc_"/"cdbc_" share the client's field
// metadata, just under a different table name and locstring layout).
std::string BuildDbcTableDdl(DbcTableMeta const& table, std::string const& sqlTableName = {},
                              uint32_t locStringSlots = 1);

// CREATE INDEX for every foreign-key field in `table`. Foreign keys aren't enforced (client
// data has dangling references), so these are plain indexes, just to keep joins fast.
std::vector<std::string> BuildDbcTableIndexDdl(DbcTableMeta const& table, std::string const& sqlTableName = {},
                                                uint32_t locStringSlots = 1);

// Binds one row's values into a prepared "INSERT ... VALUES (?, ?, ...)" statement, one bind per
// flattened column in DbcColumnNames' order, then steps and resets it. `locStringSlots` mirrors
// WdbcLayout: a locstring field's mask slot (the last one, when there is more than one) binds as
// an integer rather than text. Shared by the WDC5/WDBC import path and the reference database
// builder.
void BindDbcRow(DoltLiteStatement& statement, DbcTableMeta const& table, Wdc5Row const& row,
                 uint32_t locStringSlots = 1);

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
