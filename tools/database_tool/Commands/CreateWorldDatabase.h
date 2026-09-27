#ifndef DATABASE_TOOL_CREATEWORLDDATABASE_H
#define DATABASE_TOOL_CREATEWORLDDATABASE_H

#include "Dbc/DbcSchema.h"
#include "Sql/MysqlDumpImport.h"
#include "World/WorldArchive.h"

#include <filesystem>
#include <string>
#include <vector>

class DoltLiteDatabase;

struct WorldUpdateResult
{
    std::vector<std::string> appliedMigrationIds;
    std::string failedFile; // empty on full success
    std::string error;
};

// Applies every pending `_world` migration under migrationsDirectory to an
// already-open .world (RunDoltLiteMigrations): one DoltLite transaction plus
// dolt_commit per file, in filename order, stopping at the first failure --
// including a file that still has the legacy stored-procedure envelope,
// reported by name rather than run.
WorldUpdateResult UpdateWorldDatabase(DoltLiteDatabase& world, std::filesystem::path const& migrationsDirectory);

struct WorldCreateResult
{
    bool alreadyExisted = false;
    WorldArchiveResult archive;
    DumpImportResult dumpImport;
    WorldUpdateResult update;
    DbcSchemaResult dbcSchema;
    std::string error; // empty on success; sourced from whichever step failed
};

// Creates a brand new .world at worldPath: refuses if it already exists.
// Fetches the pinned base dump into cacheDirectory (WorldArchive), imports
// it (ImportMysqlDump), applies every pending `_world` migration
// (UpdateWorldDatabase) -- vmangos' own recipe, in SQLite (world-sql.md) --
// then creates the dbc_* tables (CreateDbcTables). dbc_* import
// (plan.md's create step 6) isn't implemented yet.
WorldCreateResult CreateWorldDatabase(std::filesystem::path const& worldPath, std::filesystem::path const& cacheDirectory, std::filesystem::path const& migrationsDirectory);

#endif
