#ifndef DATABASE_TOOL_VERIFY_ROWCOMPARE_H
#define DATABASE_TOOL_VERIFY_ROWCOMPARE_H

#include <cstdint>
#include <string>
#include <vector>

class DoltLiteDatabase;

// Every row where one column differs between the two sides, with the first such row as an example.
struct ColumnDifference
{
    std::string column;
    uint64_t rows = 0;
    std::string firstKey;
    std::string expected;
    std::string actual;
};

struct RowCompareResult
{
    std::vector<std::string> columns;
    uint64_t expectedRows = 0;
    uint64_t actualRows = 0;
    uint64_t missingRows = 0; // in expected, not in actual
    uint64_t extraRows = 0;   // in actual, not in expected
    std::string firstMissingKey;
    std::string firstExtraKey;
    std::vector<ColumnDifference> differences; // only columns with at least one differing row

    bool Equal() const { return missingRows == 0 && extraRows == 0 && differences.empty(); }
};

// Compares the rows of two SELECTs with the same column count, matched by the columns at
// `keyColumns` (0-based; empty means every column, i.e. a multiset comparison). Rows with an
// equal key are paired in the order of their remaining columns. Floats are compared as float32,
// integers and text exactly; an integer and a float compare numerically. Column names are
// `expectedSql`'s.
RowCompareResult CompareRowSets(DoltLiteDatabase& db, std::string const& expectedSql, std::string const& actualSql,
                                 std::vector<int> const& keyColumns);

#endif
