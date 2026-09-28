#include "DbcGenerator.h"
#include "DbdDefinitions.h"
#include "DbdResolve.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    // Everything that differs between metadata sets: the default build(s) resolved when
    // `--build` isn't given, the output file, and the symbols the generator emits.
    struct SetConfig
    {
        std::vector<DbdBuild> defaultBuilds;
        std::string outFile;
        DbcGeneratorNames names;
    };

    bool ResolveSetConfig(std::string const& set, SetConfig& config)
    {
        if (set == "client")
        {
            config.defaultBuilds = { DbdBuild::Parse("1.15.6.58797"), DbdBuild::Parse("1.15.6.58725") };
            config.outFile = "DbcTables.g.cpp";
            config.names = DbcGeneratorNames{};
            return true;
        }
        if (set == "vanilla")
        {
            config.defaultBuilds = { DbdBuild::Parse("1.12.1.5875") };
            config.outFile = "VanillaDbcTables.g.cpp";
            config.names.sqlTablePrefix = "vdbc_";
            config.names.getTablesFunction = "GetVanillaDbcTables";
            config.names.findTableFunction = "FindVanillaDbcTable";
            config.names.emitWowDbDefsCommitFunction = false;
            return true;
        }
        return false;
    }

    bool ParseArgs(int argc, char** argv, std::string& definitionsDir, std::string& overridesDir,
                   std::vector<DbdBuild>& builds, std::string& outDir, std::string& set)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "--definitions" && i + 1 < argc)
            {
                definitionsDir = argv[++i];
            }
            else if (arg == "--overrides" && i + 1 < argc)
            {
                overridesDir = argv[++i];
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
            else if (arg == "--set" && i + 1 < argc)
            {
                set = argv[++i];
            }
            else
            {
                std::cerr << "unrecognized argument: " << arg << "\n";
                return false;
            }
        }
        return true;
    }

    // `overrides` entries replace `base` entries of the same table name; a table only in
    // `overrides` is added. Parse errors from either directory are kept.
    void ApplyOverrides(DbdDefinitions& base, DbdDefinitions const& overrides)
    {
        for (auto const& [name, file] : overrides.tables)
            base.tables[name] = file;
        base.errors.insert(base.errors.end(), overrides.errors.begin(), overrides.errors.end());
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
    std::string overridesDir;
    std::vector<DbdBuild> builds;
    std::string outDir;
    std::string set = "client";

    if (!ParseArgs(argc, argv, definitionsDir, overridesDir, builds, outDir, set))
        return 1;

    if (outDir.empty())
    {
        std::cerr << "--out <dir> is required\n";
        return 1;
    }

    SetConfig setConfig;
    if (!ResolveSetConfig(set, setConfig))
    {
        std::cerr << "unknown --set value: " << set << " (expected client or vanilla)\n";
        return 1;
    }

    if (builds.empty())
        builds = setConfig.defaultBuilds;

    DbdDefinitions definitions;
    try
    {
        definitions = LoadDbdDefinitions(definitionsDir);
        if (!overridesDir.empty())
            ApplyOverrides(definitions, LoadDbdDefinitions(overridesDir));
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

    DbcGeneratorResult generated =
        GenerateDbcTablesSource(resolvedTables, DB_SCHEMA_SETUP_WOWDBDEFS_COMMIT, setConfig.names);
    if (!generated.errors.empty())
    {
        for (DbcGeneratorFieldError const& error : generated.errors)
            std::cerr << error.table << (error.field.empty() ? "" : ("." + error.field)) << ": " << error.message
                       << "\n";
        return 1;
    }

    std::string outPath = outDir + "/" + setConfig.outFile;
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
