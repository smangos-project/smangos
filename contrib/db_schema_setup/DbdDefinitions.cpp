#include "DbdDefinitions.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

DbdDefinitions LoadDbdDefinitions(std::string const& directory)
{
    if (!std::filesystem::is_directory(directory))
        throw std::runtime_error("definitions directory not found: " + directory);

    std::vector<std::filesystem::path> files;
    for (auto const& entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".dbd")
            files.push_back(entry.path());
    }

    std::sort(files.begin(), files.end());

    DbdDefinitions definitions;
    for (std::filesystem::path const& path : files)
    {
        std::string tableName = path.stem().string();
        try
        {
            definitions.tables.emplace(tableName, ParseDbdFile(path.string()));
        }
        catch (DbdParseError const& error)
        {
            definitions.errors.push_back({ error.file, error.line, error.what() });
        }
    }

    return definitions;
}
