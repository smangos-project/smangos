#ifndef DATABASE_TOOL_MIGRATIONSTATE_H
#define DATABASE_TOOL_MIGRATIONSTATE_H

#include "MigrationFile.h"

#include <string>
#include <vector>

struct MigrationState
{
    std::vector<MigrationFile> pending; // files whose id isn't recorded as applied
    std::vector<std::string> orphaned;   // applied ids with no matching file
};

// Compares a database's migration files against the ids its own migrations
// table already has recorded. Both inputs are compared purely by id text, so
// callers decide how appliedIds was fetched (a MySQL query, a DoltLite
// query, ...).
MigrationState CompareMigrations(std::vector<MigrationFile> const& files, std::vector<std::string> const& appliedIds);

#endif
