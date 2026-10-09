#ifndef DATABASE_TOOL_CHARSTOREIMPORT_H
#define DATABASE_TOOL_CHARSTOREIMPORT_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct CharStoreImportOptions
{
    std::string mysqlInfo;            // "host;port;user;pass;db"
    std::filesystem::path out;        // must not exist yet (or be empty)
    std::vector<std::string> tables;  // empty: every base table
    uint32_t pageSize = 4096;
    bool schemaOnly = false;
    bool withoutRowid = false;        // composite primary key tables become WITHOUT ROWID
};

struct CharStoreTableResult
{
    std::string table;
    uint64_t rows = 0;
    std::vector<std::string> warnings;
};

struct CharStoreImportResult
{
    std::string error;
    std::vector<CharStoreTableResult> tables;
};

// Copies a MariaDB character database into a new stock sqlite file (seeded so the linked
// doltlite keeps the b-tree engine): schema through TranslateMysqlCreateTable, rows in one
// transaction, secondary indexes after the rows, then row counts and integrity are checked.
CharStoreImportResult ImportCharStore(CharStoreImportOptions const& options);

// Appends WITHOUT ROWID to a translated CREATE TABLE whose primary key has several columns.
std::string WithoutRowidIfComposite(std::string const& createTable);

#endif
