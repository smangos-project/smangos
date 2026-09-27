#include "PortMigrationFile.h"

#include "Migrations/MigrationFile.h"
#include "Sql/MigrationPorter.h"

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

PortMigrationFileResult PortMigrationFile(std::filesystem::path const& path)
{
    PortMigrationFileResult result;

    std::string id, db;
    if (!TryParseMigrationFilename(path.filename().string(), id, db) || db != "world")
    {
        result.error = "not a <id>_world.sql migration filename: " + path.filename().string();
        return result;
    }

    PortedMigration ported = PortMysqlMigration(id, ReadFile(path));
    if (ported.flagged)
    {
        result.flagged = true;
        result.flagReason = ported.flagReason;
        return result;
    }

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << ported.content;
    return result;
}
