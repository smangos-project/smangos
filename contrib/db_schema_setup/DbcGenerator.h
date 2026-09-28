#ifndef DB_SCHEMA_SETUP_DBCGENERATOR_H
#define DB_SCHEMA_SETUP_DBCGENERATOR_H

#include "DbdResolve.h"

#include <string>
#include <vector>

// One field-level problem found while generating a table's metadata.
struct DbcGeneratorFieldError
{
    std::string table;
    std::string field; // empty when the error isn't about one field
    std::string message;
};

struct DbcGeneratorResult
{
    std::string source; // only meaningful when errors is empty
    std::vector<DbcGeneratorFieldError> errors;
};

// Names a metadata set generates, so the same generator produces both the client set
// (tools/database_tool/generated/DbcTables.g.cpp) and the vanilla set
// (.../VanillaDbcTables.g.cpp) as separate translation units linked into one binary; the
// defaults are the client set's, unchanged from before this existed.
struct DbcGeneratorNames
{
    std::string sqlTablePrefix = "dbc_";
    std::string getTablesFunction = "GetDbcTables";
    std::string findTableFunction = "FindDbcTable";
    // Only one metadata set needs to define GetWowDbDefsCommit(); every set is generated from
    // the same pinned commit, and the symbol is non-static, so a second definition would
    // collide at link time.
    bool emitWowDbDefsCommitFunction = true;
};

// Renders one generated/*.g.cpp for every table in `tables` (already resolved for our build,
// e.g. via ResolveDbdTable), sorted by name regardless of input order. `wowdbdefsCommit` is
// recorded in the header comment only.
DbcGeneratorResult GenerateDbcTablesSource(std::vector<DbdTable> const& tables, std::string const& wowdbdefsCommit,
                                            DbcGeneratorNames const& names = {});

#endif
