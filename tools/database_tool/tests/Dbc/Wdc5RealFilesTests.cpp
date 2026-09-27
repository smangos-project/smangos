#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>

TEST_CASE("ReadWdc5File decodes every real dbfilesclient file with a resolved definition", "[Wdc5Reader][dbfilesclient]")
{
    // Opt-in: DATABASE_TOOL_DBFILESCLIENT points at a copy of the real client's dbfilesclient
    // directory (dbc-formats.md's own test plan for this).
    char const* dir = std::getenv("DATABASE_TOOL_DBFILESCLIENT");
    if (!dir)
    {
        WARN("DATABASE_TOOL_DBFILESCLIENT not set, skipping (needs a copy of the real dbfilesclient directory)");
        return;
    }

    uint32_t checked = 0;
    uint32_t missing = 0;
    std::vector<std::string> failures;

    for (DbcTableMeta const& table : GetDbcTables())
    {
        std::filesystem::path filePath = std::filesystem::path(dir) / (std::string(table.fileStem) + ".db2");
        if (!std::filesystem::exists(filePath))
        {
            ++missing;
            continue;
        }

        ++checked;
        Wdc5ReadResult result = ReadWdc5File(filePath, table);
        if (!result.error.empty())
            failures.push_back(std::string(table.name) + ": " + result.error);
    }

    INFO(checked << " files checked, " << missing << " tables with no file");
    for (std::string const& failure : failures)
        UNSCOPED_INFO(failure);
    CHECK(failures.empty());
    CHECK(checked > 0);
}
