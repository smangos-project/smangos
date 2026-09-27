#include "Version.h"

#include "Commands/CreateWorldDatabase.h"
#include "Commands/PortMigrationFile.h"
#include "World/DoltLiteDatabase.h"

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

        WorldCreateResult result = CreateWorldDatabase(worldPath, cacheDirectory, migrationsDirectory);
        if (!result.error.empty())
        {
            std::printf("create world: %s\n", result.error.c_str());
            return 1;
        }

        std::printf("create world: imported %u tables, %llu rows; applied %zu migrations\n",
            result.dumpImport.tablesCreated, static_cast<unsigned long long>(result.dumpImport.rowsInserted),
            result.update.appliedMigrationIds.size());
        return 0;
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

    std::printf("unknown command: %s\n", command.c_str());
    return 1;
}
