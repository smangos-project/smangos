#include "DbdFile.h"

#include <filesystem>
#include <iostream>
#include <string>

#ifndef DB_SCHEMA_SETUP_DEFINITIONS_DIR
#define DB_SCHEMA_SETUP_DEFINITIONS_DIR ""
#endif

int main(int argc, char** argv)
{
    std::string definitionsDir = DB_SCHEMA_SETUP_DEFINITIONS_DIR;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--definitions" && i + 1 < argc)
            definitionsDir = argv[++i];
    }

    if (!std::filesystem::is_directory(definitionsDir))
    {
        std::cerr << "definitions directory not found: " << definitionsDir << "\n";
        return 1;
    }

    int parsed = 0;
    int failed = 0;

    for (auto const& entry : std::filesystem::directory_iterator(definitionsDir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".dbd")
            continue;

        try
        {
            ParseDbdFile(entry.path().string());
            ++parsed;
        }
        catch (DbdParseError const& error)
        {
            ++failed;
            std::cerr << error.what() << "\n";
        }
    }

    std::cout << "parsed " << parsed << " files (" << failed << " failed)\n";
    return failed != 0 ? 1 : 0;
}
