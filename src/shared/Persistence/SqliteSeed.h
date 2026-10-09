#ifndef MANGOS_PERSISTENCE_SQLITESEED_H
#define MANGOS_PERSISTENCE_SQLITESEED_H

#include "Common.h"

#include <string>
#include <vector>

struct sqlite3;

// The linked sqlite library is doltlite, which creates its own (prolly) format for any new or
// empty file but keeps using the stock b-tree engine for an existing stock sqlite file. These
// helpers seed such a file so the character store stays plain sqlite.
namespace Persistence
{
    // Bytes stock sqlite writes for an empty database: the 100-byte header and an empty page 1.
    // pageSize is a power of two in [512, 65536]. wal sets the header's WAL file format: doltlite
    // does not persist PRAGMA journal_mode=WAL on a stock file, so WAL mode has to be seeded.
    std::vector<uint8> EmptySqliteImage(uint32 pageSize, bool wal);

    // Writes EmptySqliteImage(pageSize, wal) to path when the file is missing or 0 bytes.
    bool EnsureStockSqliteFile(std::string const& path, uint32 pageSize, bool wal, std::string& error);

    // True when db runs on the stock b-tree engine (doltlite_engine() is 'orig' or absent).
    bool IsStockEngine(sqlite3* db, std::string& engine);

    // Seeds path if needed and opens it; fails unless the stock engine is in use.
    sqlite3* OpenStockSqlite(std::string const& path, uint32 pageSize, bool wal, std::string& error);
}

#endif
