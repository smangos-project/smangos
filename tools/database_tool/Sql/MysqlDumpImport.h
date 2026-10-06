#ifndef DATABASE_TOOL_MYSQLDUMPIMPORT_H
#define DATABASE_TOOL_MYSQLDUMPIMPORT_H

#include <cstdint>
#include <filesystem>
#include <string>

class DoltLiteDatabase;

struct DumpImportResult
{
    bool ok = false;
    std::string error;       // includes "line <n>: " when tied to one dump statement
    uint32_t tablesCreated = 0;
    uint64_t rowsInserted = 0;
};

// Imports a HeidiSQL-style MySQL dump (the world base dump) into db: `DROP TABLE IF EXISTS`
// runs as-is (SQLite accepts backtick identifiers), `CREATE TABLE` is
// translated with MysqlDdlTranslator (secondary indexes are deferred until
// every table's rows are in), `INSERT INTO ... VALUES ...` rows are parsed
// with ParseMysqlInsert and bound to one cached prepared statement per
// table, and `/*! ... */`/`SET ...` statements are skipped. Anything else is
// an error naming the dump's line number.
//
// Runs in one transaction. Before committing, checks that every imported
// table's row count matches how many rows were parsed for it, and that
// PRAGMA integrity_check reports "ok".
DumpImportResult ImportMysqlDump(DoltLiteDatabase& db, std::filesystem::path const& dumpPath);

#endif
