#include "DbcGenerator.h"
#include "DbdDefinitions.h"
#include "DbdResolve.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::vector<DbdBuild> DefaultBuilds()
    {
        return { DbdBuild::Parse("1.15.6.58797"), DbdBuild::Parse("1.15.6.58725") };
    }

    bool ParseArgs(int argc, char** argv, std::string& definitionsDir, std::vector<DbdBuild>& builds,
                   std::string& outDir)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--definitions" && i + 1 < argc)
            {
                definitionsDir = argv[++i];
            }
            else if (arg == "--build" && i + 1 < argc)
            {
                DbdBuild build;
                if (!DbdBuild::TryParse(argv[++i], build))
                {
                    std::cerr << "invalid --build value: " << argv[i] << "\n";
                    return false;
                }
                builds.push_back(build);
            }
            else if (arg == "--out" && i + 1 < argc)
            {
                outDir = argv[++i];
            }
            else
            {
                std::cerr << "unrecognized argument: " << arg << "\n";
                return false;
            }
        }
        return true;
    }
}

#ifndef DB_SCHEMA_SETUP_DEFINITIONS_DIR
#define DB_SCHEMA_SETUP_DEFINITIONS_DIR ""
#endif

#ifndef DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT
#define DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT ""
#endif

int main(int argc, char** argv)
{
    std::string definitionsDir = DB_SCHEMA_SETUP_DEFINITIONS_DIR;
    std::vector<DbdBuild> builds;
    std::string outDir;

    if (!ParseArgs(argc, argv, definitionsDir, builds, outDir))
        return 1;

    if (outDir.empty())
    {
        std::cerr << "--out <dir> is required\n";
        return 1;
    }

    if (builds.empty())
        builds = DefaultBuilds();

    DbdDefinitions definitions;
    try
    {
        definitions = LoadDbdDefinitions(definitionsDir);
    }
    catch (std::exception const& error)
    {
        std::cerr << error.what() << "\n";
        return 1;
    }

    bool hadErrors = !definitions.errors.empty();
    for (DbdDefinitionsLoadError const& error : definitions.errors)
        std::cerr << error.file << ":" << error.line << ": " << error.message << "\n";

    std::vector<DbdTable> resolvedTables;
    for (auto const& [name, file] : definitions.tables)
    {
        DbdResolveResult result = ResolveDbdTable(file, name, builds);
        if (result.status == DbdResolveStatus::Resolved)
        {
            resolvedTables.push_back(*result.table);
        }
        else if (result.status == DbdResolveStatus::Ambiguous)
        {
            std::cerr << name << ": ambiguous version definition for the requested build(s)\n";
            hadErrors = true;
        }
    }

    if (hadErrors)
        return 1;

    DbcGeneratorResult generated = GenerateDbcTablesSource(resolvedTables, DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT);
    if (!generated.errors.empty())
    {
        for (DbcGeneratorFieldError const& error : generated.errors)
            std::cerr << error.table << (error.field.empty() ? "" : ("." + error.field)) << ": " << error.message
                       << "\n";
        return 1;
    }

    std::string outPath = outDir + "/DbcTables.g.cpp";
    std::ofstream file(outPath, std::ios::binary);
    if (!file)
    {
        std::cerr << "failed to open " << outPath << " for writing\n";
        return 1;
    }
    file << generated.source;
    file.close();

    std::cout << "wrote " << resolvedTables.size() << " tables to " << outPath << "\n";
    return 0;
}
