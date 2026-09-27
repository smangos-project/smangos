#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h"
#include "Dbc/Wdc5Writer.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace
{
    uint32_t IdColumnIndex(DbcTableMeta const& table)
    {
        uint32_t column = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            if (table.fields[i].isId)
                return column;
            column += table.fields[i].arrayLength;
        }
        return 0;
    }

    // The writer sorts rows by id; the original file may not have been. Both sides are sorted
    // the same way before comparing so row order differences don't look like data differences.
    void SortById(std::vector<Wdc5Row>& rows, uint32_t idColumn)
    {
        std::sort(rows.begin(), rows.end(),
            [idColumn](Wdc5Row const& a, Wdc5Row const& b) { return a[idColumn].intValue < b[idColumn].intValue; });
    }

    bool RowsEqual(std::vector<Wdc5Row> const& a, std::vector<Wdc5Row> const& b, std::string& why)
    {
        if (a.size() != b.size())
        {
            why = "row count " + std::to_string(a.size()) + " vs " + std::to_string(b.size());
            return false;
        }
        for (size_t r = 0; r < a.size(); ++r)
        {
            if (a[r].size() != b[r].size())
            {
                why = "row " + std::to_string(r) + " column count mismatch";
                return false;
            }
            for (size_t c = 0; c < a[r].size(); ++c)
            {
                Wdc5Value const& x = a[r][c];
                Wdc5Value const& y = b[r][c];
                if (x.intValue != y.intValue || x.floatValue != y.floatValue || x.stringValue != y.stringValue)
                {
                    why = "row " + std::to_string(r) + " column " + std::to_string(c) + " differs";
                    return false;
                }
            }
        }
        return true;
    }
}

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

TEST_CASE("WriteWdc5File round-trips every real dbfilesclient file with a resolved definition",
          "[Wdc5Writer][dbfilesclient]")
{
    char const* dir = std::getenv("DATABASE_TOOL_DBFILESCLIENT");
    if (!dir)
    {
        WARN("DATABASE_TOOL_DBFILESCLIENT not set, skipping (needs a copy of the real dbfilesclient directory)");
        return;
    }

    std::filesystem::path scratch = std::filesystem::temp_directory_path() / "database_tool_test_wdc5_roundtrip.db2";

    uint32_t checked = 0;
    std::vector<std::string> failures;

    for (DbcTableMeta const& table : GetDbcTables())
    {
        std::filesystem::path filePath = std::filesystem::path(dir) / (std::string(table.fileStem) + ".db2");
        if (!std::filesystem::exists(filePath))
            continue;

        Wdc5ReadResult original = ReadWdc5File(filePath, table);
        if (!original.error.empty())
            continue; // covered by the read-only test above

        ++checked;
        Wdc5WriteResult written = WriteWdc5File(table, original.info, original.rows);
        if (!written.error.empty())
        {
            failures.push_back(std::string(table.name) + ": write: " + written.error);
            continue;
        }

        {
            std::ofstream out(scratch, std::ios::binary | std::ios::trunc);
            out.write(reinterpret_cast<char const*>(written.bytes.data()), static_cast<std::streamsize>(written.bytes.size()));
        }

        Wdc5ReadResult reread = ReadWdc5File(scratch, table);
        if (!reread.error.empty())
        {
            failures.push_back(std::string(table.name) + ": reread: " + reread.error);
            continue;
        }

        uint32_t idColumn = IdColumnIndex(table);
        std::vector<Wdc5Row> originalSorted = original.rows;
        std::vector<Wdc5Row> rereadSorted = reread.rows;
        SortById(originalSorted, idColumn);
        SortById(rereadSorted, idColumn);

        std::string why;
        if (!RowsEqual(originalSorted, rereadSorted, why))
            failures.push_back(std::string(table.name) + ": " + why);
    }

    std::filesystem::remove(scratch);

    INFO(checked << " files round-tripped");
    for (std::string const& failure : failures)
        UNSCOPED_INFO(failure);
    CHECK(failures.empty());
    CHECK(checked > 0);
}
