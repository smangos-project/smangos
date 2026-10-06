#ifndef DATABASE_TOOL_MIGRATIONRUNNER_H
#define DATABASE_TOOL_MIGRATIONRUNNER_H

#include "MigrationFile.h"

#include <string>
#include <vector>

class Database;
class DoltLiteDatabase;

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

// Runs each pending world migration against a DoltLite database, one SQLite
// transaction per file, in filename order, stopping at the first failure.
// A ported file records its own id with its own `INSERT INTO migrations`
// statement (MigrationPorter, world-sql.md); this checks that the row landed
// rather than trusting the file, so a failing statement or a missing insert
// both leave the file's whole transaction rolled back. Only once that check
// passes is the transaction committed. A file that still has the legacy stored-procedure envelope hasn't
// been ported yet and is reported as a failure naming the file, not run.
MigrationRunResult RunDoltLiteMigrations(DoltLiteDatabase& db, std::vector<MigrationFile> const& pending);

#endif
