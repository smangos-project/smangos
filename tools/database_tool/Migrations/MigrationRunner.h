#ifndef DATABASE_TOOL_MIGRATIONRUNNER_H
#define DATABASE_TOOL_MIGRATIONRUNNER_H

#include "MigrationFile.h"

#include <string>
#include <vector>

class Database;

struct MigrationRunResult
{
    std::vector<std::string> appliedIds;
    std::string failedFile;  // empty on full success
    std::string error;
};

// Runs each pending file's statements against a MySQL connection, in
// filename order, stopping at the first failing statement. Each file records
// its own id in the `migrations` table (inside its stored-procedure
// envelope's own guard), so this never writes that row itself.
MigrationRunResult RunMysqlMigrations(Database& db, std::vector<MigrationFile> const& pending);

#endif
