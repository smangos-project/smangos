#ifndef DB_SCHEMA_SETUP_DBDDEFINITIONS_H
#define DB_SCHEMA_SETUP_DBDDEFINITIONS_H

#include "DbdFile.h"

#include <string>
#include <unordered_map>
#include <vector>

struct DbdDefinitionsLoadError
{
    std::string file;
    int line;
    std::string message;
};

struct DbdDefinitions
{
    std::unordered_map<std::string, DbdFile> tables; // keyed by table name (the .dbd filename stem)
    std::vector<DbdDefinitionsLoadError> errors;      // one entry per file that failed to parse
};

// Parses every `*.dbd` file directly inside `directory`. A file that fails to parse is recorded
// in `errors` and skipped; it does not stop the rest of the directory from loading.
DbdDefinitions LoadDbdDefinitions(std::string const& directory);

#endif
