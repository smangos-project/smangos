#include "CreateWorldDatabase.h"

#include "Migrations/MigrationFile.h"
#include "Migrations/MigrationRunner.h"
#include "Migrations/MigrationState.h"
#include "World/DoltLiteDatabase.h"

#include <filesystem>

WorldUpdateResult UpdateWorldDatabase(DoltLiteDatabase& world, std::filesystem::path const& migrationsDirectory)
{
    WorldUpdateResult result;

    std::vector<std::string> appliedIds;
    DoltLiteStatement appliedStmt = world.Prepare("SELECT id FROM `migrations`");
    while (appliedStmt.Step())
        appliedIds.push_back(appliedStmt.ColumnText(0));

    std::vector<MigrationFile> files = DiscoverMigrations(migrationsDirectory, "world");
    MigrationState state = CompareMigrations(files, appliedIds);

    MigrationRunResult runResult = RunDoltLiteMigrations(world, state.pending);
    result.appliedMigrationIds = std::move(runResult.appliedIds);
    result.failedFile = std::move(runResult.failedFile);
    result.error = std::move(runResult.error);
    return result;
}

WorldCreateResult CreateWorldDatabase(std::filesystem::path const& worldPath, std::filesystem::path const& cacheDirectory,
    std::filesystem::path const& migrationsDirectory, std::filesystem::path const& dbfilesclientDir, bool importDbc)
{
    WorldCreateResult result;

    if (std::filesystem::exists(worldPath))
    {
        result.alreadyExisted = true;
        result.error = "already exists: " + worldPath.string() + " (delete .world/ to recreate it)";
        return result;
    }

    result.archive = FetchAndUnpackWorldArchive(WorldBaseDumpArchiveSpec(), cacheDirectory);
    if (!result.archive.error.empty())
    {
        result.error = result.archive.error;
        return result;
    }

    DoltLiteDatabase world(worldPath.string());
    result.dumpImport = ImportMysqlDump(world, result.archive.sqlPath, "world: import world_full_14_june_2021");
    if (!result.dumpImport.ok)
    {
        result.error = result.dumpImport.error;
        return result;
    }

    result.dbcSchema = CreateDbcTables(world,
        std::string("world: create dbc tables (") + GetWowDbDefsCommit() + ")");
    if (!result.dbcSchema.error.empty())
    {
        result.error = result.dbcSchema.error;
        return result;
    }

    if (importDbc)
    {
        result.dbcImport = ImportDb2Files(world, dbfilesclientDir);
        if (!result.dbcImport.error.empty())
        {
            result.error = result.dbcImport.error;
            return result;
        }
    }

    // Last: migrations may edit dbc_* rows, so the tables and their rows must already exist.
    result.update = UpdateWorldDatabase(world, migrationsDirectory);
    if (!result.update.error.empty())
    {
        result.error = result.update.failedFile + ": " + result.update.error;
        return result;
    }

    return result;
}
