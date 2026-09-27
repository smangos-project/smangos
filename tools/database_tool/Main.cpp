#include "Version.h"

#include "Commands/CreateWorldDatabase.h"
#include "Commands/PortMigrationFile.h"
#include "Dbc/DbcImport.h"
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
    uint32_t PrintDbcImportSummary(DbcImportResult const& result)
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
        std::printf("import db2: %u tables imported, %u missing, %u failed\n", imported, missing, failed);
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
            PrintDbcImportSummary(result.dbcImport);
        return 0;
    }

    // import --format db2 [--tables A,B]
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

        if (format != "db2")
        {
            std::printf("import: --format db2 is required (dbc isn't implemented yet)\n");
            return 1;
        }

        std::filesystem::path const worldPath = SourceRoot() / ".world" / "world.doltlite";
        if (!std::filesystem::exists(worldPath))
        {
            std::printf("import: %s does not exist (run create first)\n", worldPath.string().c_str());
            return 1;
        }

        DoltLiteDatabase world(worldPath.string());
        DbcImportResult result = ImportDb2Files(world, DbfilesclientDir(), onlyTables);
        if (!result.error.empty())
        {
            std::printf("import: %s\n", result.error.c_str());
            return 1;
        }

        return PrintDbcImportSummary(result) ? 1 : 0;
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
    if (command == "import")
        return RunImport(rest);

    std::printf("unknown command: %s\n", command.c_str());
    return 1;
}
