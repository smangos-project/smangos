#include "DbdDefinitions.h"
#include "DbdDiff.h"
#include "DbdResolve.h"

#include <algorithm>
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
        std::string& diffFrom, std::string& diffTo)
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
            else if (arg == "--diff" && i + 2 < argc)
            {
                diffFrom = argv[++i];
                diffTo = argv[++i];
            }
            else
            {
                std::cerr << "unrecognized argument: " << arg << "\n";
                return false;
            }
        }
        return true;
    }

    // phase0-plan.md step 4.5.1: reports, per table resolved at `diffFrom`, what a resolution at
    // `diffTo` no longer has -- fields removed, array lengths changed, types changed, or the
    // whole table gone. Informational; always exits 0.
    int RunDiff(DbdDefinitions const& definitions, std::string const& diffFrom, std::string const& diffTo)
    {
        DbdBuild from, to;
        if (!DbdBuild::TryParse(diffFrom, from) || !DbdBuild::TryParse(diffTo, to))
        {
            std::cerr << "invalid --diff build\n";
            return 1;
        }

        std::vector<DbdTableDiff> diffs = DiffDbdTables(definitions, { from }, { to });
        for (DbdTableDiff const& diff : diffs)
        {
            if (diff.removedEntirely)
            {
                std::cout << diff.table << ": removed entirely (no definition for " << to.ToString() << ")\n";
                continue;
            }
            for (DbdFieldDiff const& field : diff.fields)
                std::cout << diff.table << "." << field.field << ": " << field.kind
                           << (field.detail.empty() ? "" : " (" + field.detail + ")") << "\n";
        }
        std::cout << diffs.size() << " tables differ from " << from.ToString() << " to " << to.ToString() << "\n";
        return 0;
    }
}

#ifndef DB_SCHEMA_SETUP_DEFINITIONS_DIR
#define DB_SCHEMA_SETUP_DEFINITIONS_DIR ""
#endif

int main(int argc, char** argv)
{
    std::string definitionsDir = DB_SCHEMA_SETUP_DEFINITIONS_DIR;
    std::vector<DbdBuild> builds;
    std::string diffFrom, diffTo;

    if (!ParseArgs(argc, argv, definitionsDir, builds, diffFrom, diffTo))
        return 1;

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

    if (!diffFrom.empty())
        return RunDiff(definitions, diffFrom, diffTo);

    bool hadErrors = !definitions.errors.empty();
    for (DbdDefinitionsLoadError const& error : definitions.errors)
        std::cerr << error.file << ":" << error.line << ": " << error.message << "\n";

    std::vector<std::string> unresolved;
    int resolvedCount = 0;

    for (auto const& [name, file] : definitions.tables)
    {
        DbdResolveResult result = ResolveDbdTable(file, name, builds);
        if (result.status == DbdResolveStatus::Resolved)
        {
            ++resolvedCount;
        }
        else if (result.status == DbdResolveStatus::Ambiguous)
        {
            std::cerr << name << ": ambiguous version definition for the requested build(s)\n";
            hadErrors = true;
        }
        else
        {
            unresolved.push_back(name);
        }
    }

    std::sort(unresolved.begin(), unresolved.end());

    std::cout << "parsed " << definitions.tables.size() << " files ("
               << definitions.errors.size() << " failed)\n";
    std::cout << resolvedCount << " tables resolved, " << unresolved.size() << " with no definition:\n";
    for (std::string const& name : unresolved)
        std::cout << "  " << name << "\n";

    return hadErrors ? 1 : 0;
}
