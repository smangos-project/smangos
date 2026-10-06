#include "CreateWorldDatabase.h"

#include "Migrations/MigrationFile.h"
#include "Migrations/MigrationRunner.h"
#include "Migrations/MigrationState.h"
#include "Reference/ReferenceBuild.h"
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
    std::filesystem::path const& migrationsDirectory, std::filesystem::path const& dbfilesclientDir, bool importDbc,
    std::filesystem::path const& vanillaDbcDir)
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
    result.dumpImport = ImportMysqlDump(world, result.archive.sqlPath);
    if (!result.dumpImport.ok)
    {
        result.error = result.dumpImport.error;
        return result;
    }

    result.dbcSchema = CreateDbcTables(world);
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

    std::vector<std::string> stagingTables;
    if (!vanillaDbcDir.empty())
    {
        ReferenceBuildResult staged;
        LoadVanillaDbcTables(world, vanillaDbcDir, staged);
        result.stagingErrors = std::move(staged.tableErrors);
        for (ReferenceTableInfo const& info : staged.tables)
            stagingTables.push_back(info.table);
    }

    // Migrations may edit dbc_* rows, so the tables and their rows must already exist.
    result.update = UpdateWorldDatabase(world, migrationsDirectory);
    if (!result.update.error.empty())
    {
        result.error = result.update.failedFile + ": " + result.update.error;
        return result;
    }

    if (!stagingTables.empty())
    {
        world.Begin();
        for (std::string const& table : stagingTables)
            world.Execute("DROP TABLE \"" + table + "\"");
        world.Commit();
    }

    return result;
}
