#ifndef DATABASE_TOOL_WORLDCOMPARE_H
#define DATABASE_TOOL_WORLDCOMPARE_H

#include <cstdint>
#include <string>
#include <vector>

class DoltLiteDatabase;
class Database;

// One mismatching cell in the first row (in primary-key order, or every
// column's order if a table has none) where .world and the MySQL world
// disagree.
struct WorldRowDifference
{
    uint64_t rowIndex = 0; // 0-based position in that order
    std::string column;
    std::string worldValue;
    std::string mysqlValue;
};

struct WorldTableComparison
{
    std::string table;

    bool columnsMatch = true;
    std::vector<std::string> worldColumns;
    std::vector<std::string> mysqlColumns;

    // Left at 0/true when columnsMatch is false: row content depends on
    // knowing which columns line up, so it's never compared in that case.
    uint64_t worldRowCount = 0;
    uint64_t mysqlRowCount = 0;
    bool rowCountMatches = true;

    // Every mismatching column of the first differing row, empty if none
    // found (whether or not every row was actually compared).
    std::vector<WorldRowDifference> rowDifferences;
};

struct WorldCompareResult
{
    std::vector<std::string> tablesOnlyInWorld;
    std::vector<std::string> tablesOnlyInMysql;
    std::vector<WorldTableComparison> tableComparisons; // tables present on both sides, name order

    bool Identical() const;
};

// Compares every ordinary table `.world` and a MySQL world database have in
// common, table by table: DoltLite's own dolt_*/sqlite_* tables are skipped,
// since they have no MySQL counterpart. For each common table: column names
// in order, row count, and rows compared in primary-key order (or by every
// column if the table has none). Floats are compared as float32 (MySQL FLOAT
// is single precision; SQLite REAL is a double) and everything else as text.
WorldCompareResult CompareWorldWithMysql(DoltLiteDatabase& world, Database& mysql);

#endif
