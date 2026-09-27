#include "MigrationRunner.h"

#include "Sql/SqlStatementSplitter.h"

#include "Database/DatabaseEnv.h"

#include <fstream>
#include <sstream>

namespace
{
    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

MigrationRunResult RunMysqlMigrations(Database& db, std::vector<MigrationFile> const& pending)
{
    MigrationRunResult result;
    for (MigrationFile const& file : pending)
    {
        std::string content = ReadFile(file.path);
        for (SqlScriptStatement const& statement : SplitMysqlStatements(content))
        {
            if (!db.DirectExecute(statement.text))
            {
                result.failedFile = file.path.filename().string();
                result.error = "statement failed (see the DBErrors log): " + statement.text.substr(0, 200);
                return result;
            }
        }
        result.appliedIds.push_back(file.id);
    }
    return result;
}
