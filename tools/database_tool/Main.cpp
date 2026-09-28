#include "Version.h"

#include "Commands/CreateWorldDatabase.h"
#include "Commands/PortMigrationFile.h"
#include "Dbc/DbcExport.h"
#include "Dbc/DbcImport.h"
#include "Reference/ReferenceBuild.h"
#include "Spec/SpecInventory.h"
#include "World/DoltLiteDatabase.h"
#include "World/WorldCompare.h"

#include "Database/DatabaseEnv.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    std::filesystem::path SourceRoot()
    {
        return std::filesystem::path(DATABASE_TOOL_SOURCE_DIR);
    }

    std::filesystem::path DbfilesclientDir()
    {
        return SourceRoot() / ".client" / "Content" / "dbfilesclient";
    }

    bool HasFlag(std::vector<std::string> const& args, std::string const& flag)
    {
        return std::find(args.begin(), args.end(), flag) != args.end();
    }

    // Returns the number of tables that failed to import.
    uint32_t PrintDbcImportSummary(char const* format, DbcImportResult const& result)
    {
        uint32_t imported = 0, missing = 0, failed = 0;
        for (DbcImportTableResult const& table : result.tables)
        {
            if (!table.fileFound)
                ++missing;
            else if (!table.error.empty())
            {
                std::printf("%s: %s\n", table.tableName.c_str(), table.error.c_str());
                ++failed;
            }
            else
                ++imported;
        }
        std::printf("import %s: %u tables imported, %u missing, %u failed\n", format, imported, missing, failed);
        return failed;
    }

    // Empty means "every database".
    bool WantsDatabase(std::vector<std::string> const& databases, std::string const& name)
    {
        return databases.empty() || std::find(databases.begin(), databases.end(), name) != databases.end();
    }

    std::vector<std::string> SplitCsv(std::string const& csv)
    {
        std::vector<std::string> result;
        std::string current;
        for (char c : csv)
        {
            if (c == ',')
            {
                result.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
        result.push_back(current);
        return result;
    }

    std::vector<std::string> ParseDatabasesFlag(std::vector<std::string> const& args)
    {
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == "--databases" && i + 1 < args.size())
                return SplitCsv(args[i + 1]);
        return {};
    }

    int RunCreate(std::vector<std::string> const& args)
    {
        if (!WantsDatabase(ParseDatabasesFlag(args), "world"))
        {
            std::printf("create: only --databases world is implemented so far\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        std::filesystem::path const cacheDirectory = SourceRoot() / ".world" / "cache";
        std::filesystem::path const migrationsDirectory = SourceRoot() / "sql" / "migrations";
        bool const importDbc = !HasFlag(args, "--no-dbc-import");

        WorldCreateResult result = CreateWorldDatabase(worldPath, cacheDirectory, migrationsDirectory, DbfilesclientDir(), importDbc);
        if (!result.error.empty())
        {
            std::printf("create world: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("create world: imported %u tables, %llu rows; applied %zu migrations\n",
            result.dumpImport.tablesCreated, static_cast<unsigned long long>(result.dumpImport.rowsInserted),
            result.update.appliedMigrationIds.size());
        if (importDbc)
            PrintDbcImportSummary("db2", result.dbcImport);
        return 0;
    }

    // import --format db2|dbc [--tables A,B]
    int RunImport(std::vector<std::string> const& args)
    {
        std::string format;
        std::vector<std::string> onlyTables;
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--format" && i + 1 < args.size())
                format = args[++i];
            else if (args[i] == "--tables" && i + 1 < args.size())
                onlyTables = SplitCsv(args[++i]);
        }

        if (format != "db2" && format != "dbc")
        {
            std::printf("import: --format db2|dbc is required\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("import: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        DbcImportResult result = format == "db2"
            ? ImportDb2Files(world, DbfilesclientDir(), onlyTables)
            : ImportDbcFiles(world, DbfilesclientDir(), onlyTables);
        if (!result.error.empty())
        {
            std::printf("import: %s\n", result.error.c_str());
            return 1;
        }

        return PrintDbcImportSummary(format.c_str(), result) ? 1 : 0;
    }

    // export --format db2|dbc [--tables A,B]
    int RunExport(std::vector<std::string> const& args)
    {
        std::string format;
        std::vector<std::string> onlyTables;
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "--format" && i + 1 < args.size())
                format = args[++i];
            else if (args[i] == "--tables" && i + 1 < args.size())
                onlyTables = SplitCsv(args[++i]);
        }

        if (format != "db2" && format != "dbc")
        {
            std::printf("export: --format db2|dbc is required\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("export: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        DbcExportResult result = format == "db2"
            ? ExportDb2Files(world, DbfilesclientDir(), onlyTables)
            : ExportDbcFiles(world, DbfilesclientDir(), onlyTables);
        if (!result.error.empty())
        {
            std::printf("export: %s\n", result.error.c_str());
            return 1;
        }

        uint32_t exported = 0, missing = 0, failed = 0;
        for (DbcExportTableResult const& table : result.tables)
        {
            if (!table.hasFilesRow)
                ++missing;
            else if (!table.error.empty())
            {
                std::printf("%s: %s\n", table.tableName.c_str(), table.error.c_str());
                ++failed;
            }
            else
                ++exported;
        }
        std::printf("export %s: %u tables exported, %u with no dbc_files row, %u failed\n",
            format.c_str(), exported, missing, failed);
        return failed ? 1 : 0;
    }

    int RunUpdate(std::vector<std::string> const& args)
    {
        if (!WantsDatabase(ParseDatabasesFlag(args), "world"))
        {
            std::printf("update: only --databases world is implemented so far\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        std::filesystem::path const migrationsDirectory = SourceRoot() / "sql" / "migrations";

        if (!std::filesystem::exists(worldPath))
        {
            std::printf("update world: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        WorldUpdateResult result = UpdateWorldDatabase(world, migrationsDirectory);
        if (!result.error.empty())
        {
            std::printf("update world: %s: %s\n", result.failedFile.c_str(), result.error.c_str());
            return 1;
        }

        std::printf("update world: applied %zu migrations\n", result.appliedMigrationIds.size());
        return 0;
    }

    // compare-world --mysql "<host;port;user;pass;db>"
    int RunCompareWorld(std::vector<std::string> const& args)
    {
        std::string mysqlInfo;
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == "--mysql" && i + 1 < args.size())
                mysqlInfo = args[i + 1];

        if (mysqlInfo.empty())
        {
            std::printf("compare-world: --mysql \"<host;port;user;pass;db>\" is required\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("compare-world: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());

        DatabaseMysql mysql;
        if (!mysql.Initialize(mysqlInfo.c_str(), 1, 0))
        {
            std::printf("compare-world: could not connect to MySQL: %s\n", mysqlInfo.c_str());
            return 1;
        }

        WorldCompareResult result = CompareWorldWithMysql(world, mysql);

        for (std::string const& table : result.tablesOnlyInWorld)
            std::printf("only in .world: %s\n", table.c_str());
        for (std::string const& table : result.tablesOnlyInMysql)
            std::printf("only in mysql: %s\n", table.c_str());

        for (WorldTableComparison const& table : result.tableComparisons)
        {
            if (!table.columnsMatch)
            {
                std::printf("%s: columns differ\n  .world: ", table.table.c_str());
                for (std::string const& col : table.worldColumns)
                    std::printf("%s ", col.c_str());
                std::printf("\n  mysql:  ");
                for (std::string const& col : table.mysqlColumns)
                    std::printf("%s ", col.c_str());
                std::printf("\n");
                continue;
            }

            if (!table.rowCountMatches)
                std::printf("%s: row count differs (.world=%llu, mysql=%llu)\n", table.table.c_str(),
                    static_cast<unsigned long long>(table.worldRowCount),
                    static_cast<unsigned long long>(table.mysqlRowCount));

            for (WorldRowDifference const& diff : table.rowDifferences)
                std::printf("%s: row %llu, column %s differs (.world=%s, mysql=%s)\n", table.table.c_str(),
                    static_cast<unsigned long long>(diff.rowIndex), diff.column.c_str(),
                    diff.worldValue.c_str(), diff.mysqlValue.c_str());
        }

        std::printf("%zu tables compared, %zu only in .world, %zu only in mysql\n",
            result.tableComparisons.size(), result.tablesOnlyInWorld.size(), result.tablesOnlyInMysql.size());

        if (result.Identical())
        {
            std::printf("compare-world: identical\n");
            return 0;
        }
        return 1;
    }

    // reference
    int RunReference(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        std::filesystem::path const referencePath = SourceRoot() / ".world" / "reference.doltlite";
        std::filesystem::path const vanillaDbcDir = SourceRoot() / ".client" / "vanilla" / "dbc";

        if (!std::filesystem::exists(worldPath))
        {
            std::printf("reference: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        ReferenceBuildResult result = BuildReferenceDatabase(referencePath, worldPath, vanillaDbcDir, DbfilesclientDir());
        for (std::string const& error : result.tableErrors)
            std::printf("%s\n", error.c_str());
        if (!result.error.empty())
        {
            std::printf("reference: %s\n", result.error.c_str());
            return 1;
        }

        uint32_t srv = 0, vdbc = 0, cdbc = 0;
        for (ReferenceTableInfo const& table : result.tables)
        {
            if (table.source == "srv") ++srv;
            else if (table.source == "vdbc") ++vdbc;
            else if (table.source == "cdbc") ++cdbc;
        }
        std::printf("reference: %u srv_* tables, %u vdbc_* tables, %u cdbc_* tables, %zu errors\n", srv, vdbc, cdbc,
            result.tableErrors.size());
        return result.tableErrors.empty() ? 0 : 1;
    }

    std::filesystem::path SpecDir()
    {
        return SourceRoot() / "tools" / "database_tool" / "Spec";
    }

    // spec init
    int RunSpecInit(std::vector<std::string> const& args)
    {
        (void)args;

        std::filesystem::path const referencePath = SourceRoot() / ".world" / "reference.doltlite";
        SpecInitResult result = UpdateSpecInventory(referencePath, SpecDir());
        if (!result.error.empty())
        {
            std::printf("spec init: %s\n", result.error.c_str());
            return 1;
        }

        for (std::string const& missing : result.missingColumns)
            std::printf("columns.tsv: %s is no longer in the reference database\n", missing.c_str());
        for (std::string const& missing : result.missingDbcColumns)
            std::printf("dbc-columns.tsv: %s is no longer in the reference database\n", missing.c_str());
        std::printf("spec init: %u columns.tsv rows added, %u dbc-columns.tsv rows added\n",
            result.columnsAdded, result.dbcColumnsAdded);
        return 0;
    }

    // spec set --source S --table T [--column C] [--disposition D] [--target T] [--rule R]
    //          [--reason "..."] [--reviewed yes|no] [--overwrite]
    int RunSpecSet(std::vector<std::string> const& args)
    {
        SpecSetFilter filter;
        SpecSetValues values;
        bool overwrite = false;

        for (size_t i = 0; i < args.size(); ++i)
        {
            auto takeValue = [&]() -> std::string { return i + 1 < args.size() ? args[++i] : std::string(); };
            if (args[i] == "--source") filter.source = takeValue();
            else if (args[i] == "--table") filter.table = takeValue();
            else if (args[i] == "--column") filter.column = takeValue();
            else if (args[i] == "--disposition") values.disposition = takeValue();
            else if (args[i] == "--target") values.target = takeValue();
            else if (args[i] == "--rule") values.rule = takeValue();
            else if (args[i] == "--reason") values.reason = takeValue();
            else if (args[i] == "--reviewed") values.reviewed = takeValue();
            else if (args[i] == "--overwrite") overwrite = true;
        }

        if (filter.source.empty() || filter.table.empty())
        {
            std::printf("spec set: --source and --table are required\n");
            return 1;
        }

        SpecSetResult result = SetSpecColumns(SpecDir(), filter, values, overwrite);
        if (!result.error.empty())
        {
            std::printf("spec set: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("spec set: %u rows matched, %u updated\n", result.matched, result.updated);
        return 0;
    }

    int RunSpec(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("spec: subcommand required (init, set)\n");
            return 1;
        }

        std::string const subcommand = args[0];
        std::vector<std::string> const rest(args.begin() + 1, args.end());

        if (subcommand == "init")
            return RunSpecInit(rest);
        if (subcommand == "set")
            return RunSpecSet(rest);

        std::printf("spec: unknown subcommand: %s\n", subcommand.c_str());
        return 1;
    }

    int RunPortMigration(std::vector<std::string> const& args)
    {
        if (args.empty())
        {
            std::printf("port-migration: no files given\n");
            return 1;
        }

        bool anyFlagged = false;
        for (std::string const& arg : args)
        {
            PortMigrationFileResult result = PortMigrationFile(arg);
            if (!result.error.empty())
            {
                std::printf("%s: %s\n", arg.c_str(), result.error.c_str());
                anyFlagged = true;
            }
            else if (result.flagged)
            {
                std::printf("%s: flagged: %s\n", arg.c_str(), result.flagReason.c_str());
                anyFlagged = true;
            }
        }

        return anyFlagged ? 2 : 0;
    }
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty())
    {
        std::printf("database_tool %s\n", DatabaseToolVersion());
        return 0;
    }

    std::string const command = args[0];
    std::vector<std::string> const rest(args.begin() + 1, args.end());

    if (command == "create")
        return RunCreate(rest);
    if (command == "update")
        return RunUpdate(rest);
    if (command == "port-migration")
        return RunPortMigration(rest);
    if (command == "compare-world")
        return RunCompareWorld(rest);
    if (command == "reference")
        return RunReference(rest);
    if (command == "spec")
        return RunSpec(rest);
    if (command == "import")
        return RunImport(rest);
    if (command == "export")
        return RunExport(rest);

    std::printf("unknown command: %s\n", command.c_str());
    return 1;
}
