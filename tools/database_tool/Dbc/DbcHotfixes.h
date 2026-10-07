#ifndef DATABASE_TOOL_DBC_DBCHOTFIXES_H
#define DATABASE_TOOL_DBC_DBCHOTFIXES_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class DoltLiteDatabase;

struct DbcHotfixResult
{
    std::vector<std::string> files;
    uint64_t statements = 0;
    std::string error; // "<file>: <message>"; nothing is applied when set
};

// Runs every hotfixesDir/*.sql (generators/hotfix_sql.py) in name order, in one transaction.
// The files write "dbc_<table>"; tablePrefix replaces that prefix (e.g. "cdbc_" for the reference).
// A missing directory applies nothing.
DbcHotfixResult ApplyDbcHotfixes(DoltLiteDatabase& db, std::filesystem::path const& hotfixesDir,
                                 std::string const& tablePrefix = "dbc_");

#endif
