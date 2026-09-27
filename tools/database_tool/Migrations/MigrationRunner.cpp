#include "MigrationRunner.h"

#include "Sql/SqlStatementSplitter.h"

#include "World/DoltLiteDatabase.h"

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

MigrationRunResult RunDoltLiteMigrations(DoltLiteDatabase& db, std::vector<MigrationFile> const& pending)
{
    MigrationRunResult result;
    for (MigrationFile const& file : pending)
    {
        if (file.hasLegacyEnvelope)
        {
            result.failedFile = file.path.filename().string();
            result.error = "not yet ported to SQLite dialect (still has the stored-procedure envelope)";
            return result;
        }

        std::string content = ReadFile(file.path);
        try
        {
            db.Begin();
            for (SqlScriptStatement const& statement : SplitMysqlStatements(content))
                db.Execute(statement.text);

            DoltLiteStatement check = db.Prepare("SELECT 1 FROM `migrations` WHERE `id` = ?");
            check.BindText(1, file.id);
            if (!check.Step())
                throw DoltLiteError("missing INSERT INTO `migrations` for id " + file.id);

            db.Commit();
        }
        catch (std::exception const& e)
        {
            try { db.Rollback(); } catch (std::exception const&) {}
            result.failedFile = file.path.filename().string();
            result.error = e.what();
            return result;
        }

        db.DoltCommit("migration " + file.id);
        result.appliedIds.push_back(file.id);
    }
    return result;
}
