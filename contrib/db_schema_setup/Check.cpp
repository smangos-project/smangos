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

    int count = 0;
    for (auto const& entry : std::filesystem::directory_iterator(definitionsDir))
        if (entry.is_regular_file() && entry.path().extension() == ".dbd")
            ++count;

    std::cout << "found " << count << " .dbd files in " << definitionsDir << "\n";
    return 0;
}
