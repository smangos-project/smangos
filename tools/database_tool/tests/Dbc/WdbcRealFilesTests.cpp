#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdbc.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>
#include <map>

TEST_CASE("ReadWdbcFile decodes every real vanilla DBC with a resolved 1.12.1.5875 definition",
          "[Wdbc][vanilla-dbc]")
{
    // Opt-in: DATABASE_TOOL_VANILLA_DBC points at a copy of the 1.12.1.5875 client's dbc
    // directory (phase0-plan.md step 1.3's gate), like DATABASE_TOOL_DBFILESCLIENT.
    char const* dir = std::getenv("DATABASE_TOOL_VANILLA_DBC");
    if (!dir)
    {
        WARN("DATABASE_TOOL_VANILLA_DBC not set, skipping (needs a copy of the 1.12.1.5875 dbc directory)");
        return;
    }

    // data-plan.md 3.4's spot-checked row counts.
    std::map<std::string, uint32_t> expectedRowCounts = {
        { "AreaTable", 1081 },
        { "CharSections", 3671 },
        { "SkillLineAbility", 5072 },
        { "WMOAreaTable", 21115 },
    };

    WdbcLayout layout{ /*locStringSlots*/ 9 };
    uint32_t checked = 0;
    std::vector<std::string> failures;

    for (DbcTableMeta const& table : GetVanillaDbcTables())
    {
        std::filesystem::path filePath = std::filesystem::path(dir) / (std::string(table.fileStem) + ".dbc");
        if (!std::filesystem::exists(filePath))
        {
            failures.push_back(std::string(table.name) + ": file not found");
            continue;
        }

        ++checked;
        WdbcReadResult result = ReadWdbcFile(filePath, table, layout);
        if (!result.error.empty())
        {
            failures.push_back(std::string(table.name) + ": " + result.error);
            continue;
        }

        auto expected = expectedRowCounts.find(table.name);
        if (expected != expectedRowCounts.end() && result.rows.size() != expected->second)
            failures.push_back(std::string(table.name) + ": " + std::to_string(result.rows.size()) +
                                " rows, expected " + std::to_string(expected->second));
    }

    INFO(checked << " of " << GetVanillaDbcTables().size() << " vanilla tables checked");
    for (std::string const& failure : failures)
        UNSCOPED_INFO(failure);
    CHECK(failures.empty());
    CHECK(checked == GetVanillaDbcTables().size());
}
