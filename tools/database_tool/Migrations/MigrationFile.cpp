#include "MigrationFile.h"

#include "Sql/SqlStatementSplitter.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

bool TryParseMigrationFilename(std::string const& filename, std::string& id, std::string& db)
{
    std::string const suffix = ".sql";
    if (filename.size() <= suffix.size() || filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) != 0)
        return false;

    std::string stem = filename.substr(0, filename.size() - suffix.size());
    size_t underscore = stem.find('_');
    if (underscore == std::string::npos || underscore == 0 || underscore + 1 == stem.size())
        return false;

    std::string idPart = stem.substr(0, underscore);
    std::string dbPart = stem.substr(underscore + 1);

    if (!std::all_of(idPart.begin(), idPart.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
        return false;
    if (dbPart.empty() || !std::all_of(dbPart.begin(), dbPart.end(), [](unsigned char c) { return std::isalpha(c) != 0; }))
        return false;

    id = idPart;
    db = dbPart;
    return true;
}

namespace
{
    bool StartsWithCreateProcedure(std::string const& statementText)
    {
        static std::string const keyword = "create procedure";
        if (statementText.size() < keyword.size())
            return false;
        for (size_t i = 0; i < keyword.size(); ++i)
        {
            if (std::tolower(static_cast<unsigned char>(statementText[i])) != keyword[i])
                return false;
        }
        return true;
    }

    bool HasLegacyEnvelope(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();

        for (SqlScriptStatement const& statement : SplitMysqlStatements(buffer.str()))
        {
            if (StartsWithCreateProcedure(statement.text))
                return true;
        }
        return false;
    }
}

std::vector<MigrationFile> DiscoverMigrations(std::filesystem::path const& directory, std::string const& db)
{
    std::vector<MigrationFile> files;
    for (auto const& entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
            continue;

        std::string id, fileDb;
        if (!TryParseMigrationFilename(entry.path().filename().string(), id, fileDb))
            continue;
        if (fileDb != db)
            continue;

        MigrationFile migration;
        migration.id = id;
        migration.db = fileDb;
        migration.path = entry.path();
        migration.hasLegacyEnvelope = HasLegacyEnvelope(entry.path());
        files.push_back(std::move(migration));
    }

    std::sort(files.begin(), files.end(), [](MigrationFile const& a, MigrationFile const& b) { return a.id < b.id; });
    return files;
}
