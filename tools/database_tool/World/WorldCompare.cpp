#include "WorldCompare.h"

#include "World/DoltLiteDatabase.h"

#include "Database/DatabaseEnv.h"

#include <algorithm>
#include <iterator>

namespace
{
    struct WorldColumnInfo
    {
        std::string name;
        std::string type;
        int primaryKeyPosition = 0; // 0 = not part of the primary key
    };

    bool HasPrefix(std::string const& value, std::string const& prefix)
    {
        return value.compare(0, prefix.size(), prefix) == 0;
    }

    bool EqualsCaseInsensitive(std::string const& a, std::string const& b)
    {
        return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(),
            [](char x, char y) { return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y)); });
    }

    std::string JoinBackticked(std::vector<std::string> const& names)
    {
        std::string joined;
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (i > 0)
                joined += ", ";
            joined += "`" + names[i] + "`";
        }
        return joined;
    }

    // MySQL's default text-protocol serialization for a FLOAT column shows
    // only ~6 significant digits (e.g. -10350.8 for the true, full-precision
    // -10350.7587890625), so Field::GetFloat()'s atof() of that already-lossy
    // string reconstructs a different float32 bit pattern than what's
    // actually stored. Wrapping in a DECIMAL cast forces MySQL to serialize
    // it in full; SQLite ignores the (M,N) and just keeps REAL affinity, so
    // this is a no-op there.
    std::string JoinSelectList(std::vector<std::string> const& names, std::vector<bool> const& isFloatColumn)
    {
        std::string joined;
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (i > 0)
                joined += ", ";
            // Scale 30 (MySQL's max) rather than something more modest: game
            // data includes near-zero values (e.g. a rotation quaternion
            // component around 1e-8) where a smaller scale would truncate
            // away the only significant digits a small float32 has.
            joined += isFloatColumn[i] ? "CAST(`" + names[i] + "` AS DECIMAL(65,30))" : "`" + names[i] + "`";
        }
        return joined;
    }

    std::vector<std::string> GetWorldTables(DoltLiteDatabase& world)
    {
        std::vector<std::string> tables;
        DoltLiteStatement stmt = world.Prepare("SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name");
        while (stmt.Step())
        {
            std::string name = stmt.ColumnText(0);
            // DoltLite's own housekeeping tables (dolt_*) and SQLite's
            // internal ones (sqlite_*) have no MySQL counterpart.
            if (HasPrefix(name, "dolt_") || HasPrefix(name, "sqlite_"))
                continue;
            tables.push_back(std::move(name));
        }
        return tables;
    }

    std::vector<std::string> GetMysqlTables(Database& mysql)
    {
        std::vector<std::string> tables;
        if (auto result = mysql.Query("SHOW TABLES"))
        {
            do
            {
                tables.push_back((*result)[0].GetCppString());
            } while (result->NextRow());
        }
        std::sort(tables.begin(), tables.end());
        return tables;
    }

    std::vector<WorldColumnInfo> GetWorldColumns(DoltLiteDatabase& world, std::string const& table)
    {
        std::vector<WorldColumnInfo> columns;
        DoltLiteStatement stmt = world.Prepare("PRAGMA table_info(`" + table + "`)");
        while (stmt.Step())
        {
            WorldColumnInfo column;
            column.name = stmt.ColumnText(1);
            column.type = stmt.ColumnText(2);
            column.primaryKeyPosition = static_cast<int>(stmt.ColumnInt64(5));
            columns.push_back(std::move(column));
        }
        return columns;
    }

    std::vector<std::string> GetMysqlColumns(Database& mysql, std::string const& table)
    {
        std::vector<std::string> columns;
        if (auto result = mysql.Query("SHOW COLUMNS FROM `" + table + "`"))
        {
            do
            {
                columns.push_back((*result)[0].GetCppString());
            } while (result->NextRow());
        }
        return columns;
    }

    uint64_t CountWorldRows(DoltLiteDatabase& world, std::string const& table)
    {
        DoltLiteStatement stmt = world.Prepare("SELECT COUNT(*) FROM `" + table + "`");
        stmt.Step();
        return static_cast<uint64_t>(stmt.ColumnInt64(0));
    }

    uint64_t CountMysqlRows(Database& mysql, std::string const& table)
    {
        auto result = mysql.Query("SELECT COUNT(*) FROM `" + table + "`");
        return result ? (*result)[0].GetUInt64() : 0;
    }

    // Columns that uniquely determine a row, in a fixed order both queries
    // below use identically -- their order doesn't need to match the real
    // PRIMARY KEY clause's order, it only needs to be the same on both
    // sides, which a shared ORDER BY column list guarantees regardless.
    std::vector<std::string> RowOrderColumns(std::vector<WorldColumnInfo> const& worldColumns, std::vector<std::string> const& allColumns)
    {
        std::vector<std::pair<int, std::string>> primaryKey;
        for (WorldColumnInfo const& column : worldColumns)
            if (column.primaryKeyPosition > 0)
                primaryKey.emplace_back(column.primaryKeyPosition, column.name);
        std::sort(primaryKey.begin(), primaryKey.end());

        if (primaryKey.empty())
            return allColumns;

        std::vector<std::string> order;
        order.reserve(primaryKey.size());
        for (auto const& entry : primaryKey)
            order.push_back(entry.second);
        return order;
    }

    // Appends every mismatching column of the row the two cursors currently
    // point at to comparison.rowDifferences. Returns true if any were found.
    bool CompareCurrentRow(DoltLiteStatement& worldRow, QueryResult& mysqlRow, std::vector<std::string> const& columns,
        std::vector<bool> const& isFloatColumn, uint64_t rowIndex, WorldTableComparison& comparison)
    {
        bool anyDifference = false;
        for (int i = 0; i < static_cast<int>(columns.size()); ++i)
        {
            bool worldNull = worldRow.ColumnIsNull(i);
            bool mysqlNull = mysqlRow[i].IsNULL();

            std::string worldText, mysqlText;
            bool mismatch = false;

            if (worldNull != mysqlNull)
            {
                mismatch = true;
                worldText = worldNull ? "NULL" : worldRow.ColumnText(i);
                mysqlText = mysqlNull ? "NULL" : mysqlRow[i].GetCppString();
            }
            else if (!worldNull)
            {
                if (isFloatColumn[i])
                {
                    float worldValue = static_cast<float>(worldRow.ColumnDouble(i));
                    float mysqlValue = mysqlRow[i].GetFloat();
                    if (worldValue != mysqlValue)
                    {
                        mismatch = true;
                        worldText = std::to_string(worldValue);
                        mysqlText = std::to_string(mysqlValue);
                    }
                }
                else
                {
                    worldText = worldRow.ColumnText(i);
                    mysqlText = mysqlRow[i].GetCppString();
                    mismatch = worldText != mysqlText;
                }
            }

            if (mismatch)
            {
                comparison.rowDifferences.push_back({ rowIndex, columns[i], worldText, mysqlText });
                anyDifference = true;
            }
        }
        return anyDifference;
    }

    void CompareRows(DoltLiteDatabase& world, Database& mysql, WorldTableComparison& comparison,
        std::vector<std::string> const& columns, std::vector<bool> const& isFloatColumn, std::vector<std::string> const& orderColumns)
    {
        std::string selectSql = "SELECT " + JoinSelectList(columns, isFloatColumn) + " FROM `" + comparison.table + "` ORDER BY " + JoinBackticked(orderColumns);

        DoltLiteStatement worldRow = world.Prepare(selectSql);
        auto mysqlResult = mysql.Query(selectSql);

        bool worldHasRow = worldRow.Step();
        bool mysqlHasRow = mysqlResult != nullptr; // Query() already fetched the first row

        uint64_t rowIndex = 0;
        while (worldHasRow && mysqlHasRow)
        {
            if (CompareCurrentRow(worldRow, *mysqlResult, columns, isFloatColumn, rowIndex, comparison))
                break; // the first differing row is enough

            worldHasRow = worldRow.Step();
            mysqlHasRow = mysqlResult->NextRow();
            ++rowIndex;
        }
    }

    WorldTableComparison CompareTable(DoltLiteDatabase& world, Database& mysql, std::string const& table)
    {
        WorldTableComparison comparison;
        comparison.table = table;

        std::vector<WorldColumnInfo> worldColumns = GetWorldColumns(world, table);
        for (WorldColumnInfo const& column : worldColumns)
            comparison.worldColumns.push_back(column.name);
        comparison.mysqlColumns = GetMysqlColumns(mysql, table);

        comparison.columnsMatch = comparison.worldColumns == comparison.mysqlColumns;
        if (!comparison.columnsMatch)
            return comparison;

        comparison.worldRowCount = CountWorldRows(world, table);
        comparison.mysqlRowCount = CountMysqlRows(mysql, table);
        comparison.rowCountMatches = comparison.worldRowCount == comparison.mysqlRowCount;

        std::vector<bool> isFloatColumn;
        isFloatColumn.reserve(worldColumns.size());
        for (WorldColumnInfo const& column : worldColumns)
            isFloatColumn.push_back(EqualsCaseInsensitive(column.type, "REAL"));

        CompareRows(world, mysql, comparison, comparison.worldColumns, isFloatColumn, RowOrderColumns(worldColumns, comparison.worldColumns));

        return comparison;
    }
}

bool WorldCompareResult::Identical() const
{
    if (!tablesOnlyInWorld.empty() || !tablesOnlyInMysql.empty())
        return false;
    for (WorldTableComparison const& comparison : tableComparisons)
        if (!comparison.columnsMatch || !comparison.rowCountMatches || !comparison.rowDifferences.empty())
            return false;
    return true;
}

WorldCompareResult CompareWorldWithMysql(DoltLiteDatabase& world, Database& mysql)
{
    WorldCompareResult result;

    std::vector<std::string> worldTables = GetWorldTables(world);
    std::vector<std::string> mysqlTables = GetMysqlTables(mysql);

    std::set_difference(worldTables.begin(), worldTables.end(), mysqlTables.begin(), mysqlTables.end(),
        std::back_inserter(result.tablesOnlyInWorld));
    std::set_difference(mysqlTables.begin(), mysqlTables.end(), worldTables.begin(), worldTables.end(),
        std::back_inserter(result.tablesOnlyInMysql));

    std::vector<std::string> commonTables;
    std::set_intersection(worldTables.begin(), worldTables.end(), mysqlTables.begin(), mysqlTables.end(),
        std::back_inserter(commonTables));

    for (std::string const& table : commonTables)
        result.tableComparisons.push_back(CompareTable(world, mysql, table));

    return result;
}
