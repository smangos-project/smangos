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

// Renders tools/database_tool/generated/DbcTables.g.cpp for every table in `tables` (already
// resolved for our build, e.g. via ResolveDbdTable), sorted by name regardless of input order.
// `wowdbdefsCommit` is recorded in the header comment only.
DbcGeneratorResult GenerateDbcTablesSource(std::vector<DbdTable> const& tables, std::string const& wowdbdefsCommit);

#endif
