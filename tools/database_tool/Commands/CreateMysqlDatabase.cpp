#include "CreateMysqlDatabase.h"

#include "Migrations/MigrationFile.h"
#include "Migrations/MigrationRunner.h"
#include "Migrations/MigrationState.h"
#include "Sql/SqlStatementSplitter.h"

#include "Database/DatabaseEnv.h"

#include <fstream>
#include <sstream>

namespace
{
    struct ParsedConnectionInfo
    {
        std::string host, port, user, password, database;
    };

    ParsedConnectionInfo ParseConnectionInfo(std::string const& info)
    {
        std::vector<std::string> parts;
        std::string current;
        for (char c : info)
        {
            if (c == ';')
            {
                parts.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
        parts.push_back(current);

        ParsedConnectionInfo result;
        if (parts.size() > 0) result.host = parts[0];
        if (parts.size() > 1) result.port = parts[1];
        if (parts.size() > 2) result.user = parts[2];
        if (parts.size() > 3) result.password = parts[3];
        if (parts.size() > 4) result.database = parts[4];
        return result;
    }

    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool RunScript(Database& db, std::string const& content, std::string& error)
    {
        for (SqlScriptStatement const& statement : SplitMysqlStatements(content))
        {
            if (!db.DirectExecute(statement.text))
            {
                error = "statement failed (see the DBErrors log): " + statement.text.substr(0, 200);
                return false;
            }
        }
        return true;
    }

    bool HasMigrationsTable(Database& db)
    {
        auto result = db.Query("SHOW TABLES LIKE 'migrations'");
        return result && result->GetRowCount() > 0;
    }
}

MysqlSetupResult CreateAndUpdateMysqlDatabase(
    std::string const& connectionInfo,
    std::filesystem::path const& baseScriptPath,
    std::filesystem::path const& migrationsDirectory,
    std::string const& migrationDbSuffix)
{
    MysqlSetupResult result;
    ParsedConnectionInfo parsed = ParseConnectionInfo(connectionInfo);
    if (parsed.database.empty())
    {
        result.error = "connection info has no database name: " + connectionInfo;
        return result;
    }

    std::string noDbInfo = parsed.host + ";" + parsed.port + ";" + parsed.user + ";" + parsed.password;

    // Kept alive for the whole call (never destroyed before db below is
    // constructed): DatabaseMysql's ctor/dtor call the process-global
    // mysql_server_init()/mysql_server_end(), and destroying one instance
    // before constructing the next has been observed to corrupt later
    // connections in this same process.
    DatabaseMysql adminDb;
    if (!adminDb.Initialize(noDbInfo.c_str(), 1, 0))
    {
        result.error = "could not connect to the MySQL server";
        return result;
    }
    if (!adminDb.DirectExecute(
            "CREATE DATABASE IF NOT EXISTS `" + parsed.database + "` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci"))
    {
        result.error = "CREATE DATABASE failed";
        return result;
    }

    DatabaseMysql db;
    if (!db.Initialize(connectionInfo.c_str(), 1, 0))
    {
        result.error = "could not connect to `" + parsed.database + "`";
        return result;
    }

    bool freshDatabase = !HasMigrationsTable(db);
    result.databaseCreated = freshDatabase;

    if (freshDatabase)
    {
        std::string baseScript = ReadFile(baseScriptPath);
        if (!RunScript(db, baseScript, result.error))
            return result;
        result.baseScriptImported = true;
    }

    std::vector<std::string> appliedIds;
    if (auto appliedResult = db.Query("SELECT id FROM migrations"))
    {
        do
        {
            appliedIds.push_back(appliedResult->Fetch()[0].GetCppString());
        } while (appliedResult->NextRow());
    }

    std::vector<MigrationFile> files = DiscoverMigrations(migrationsDirectory, migrationDbSuffix);
    MigrationState state = CompareMigrations(files, appliedIds);

    MigrationRunResult runResult = RunMysqlMigrations(db, state.pending);
    result.appliedMigrationIds = std::move(runResult.appliedIds);
    if (!runResult.failedFile.empty())
        result.error = runResult.failedFile + ": " + runResult.error;

    return result;
}
