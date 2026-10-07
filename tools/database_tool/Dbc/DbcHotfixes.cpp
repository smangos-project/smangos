#include "DbcHotfixes.h"

#include "Sql/SqlStatementSplitter.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <fstream>
#include <sstream>

DbcHotfixResult ApplyDbcHotfixes(DoltLiteDatabase& db, std::filesystem::path const& hotfixesDir,
                                 std::string const& tablePrefix)
{
    DbcHotfixResult result;
    if (!std::filesystem::is_directory(hotfixesDir))
        return result;

    std::vector<std::filesystem::path> paths;
    for (auto const& entry : std::filesystem::directory_iterator(hotfixesDir))
        if (entry.is_regular_file() && entry.path().extension() == ".sql")
            paths.push_back(entry.path());
    std::sort(paths.begin(), paths.end());

    std::string current;
    try
    {
        db.Begin();
        for (std::filesystem::path const& path : paths)
        {
            current = path.filename().string();
            std::ifstream file(path, std::ios::binary);
            std::ostringstream buffer;
            buffer << file.rdbuf();
            for (SqlScriptStatement statement : SplitMysqlStatements(buffer.str()))
            {
                if (tablePrefix != "dbc_")
                {
                    size_t table = statement.text.find("\"dbc_");
                    if (table != std::string::npos)
                        statement.text.replace(table + 1, 4, tablePrefix);
                }
                db.Execute(statement.text);
                ++result.statements;
            }
            result.files.push_back(current);
        }
        db.Commit();
    }
    catch (std::exception const& e)
    {
        try { db.Rollback(); } catch (std::exception const&) {}
        result.error = current + ": " + e.what();
        result.files.clear();
        result.statements = 0;
    }
    return result;
}
