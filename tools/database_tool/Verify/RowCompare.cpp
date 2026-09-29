#include "RowCompare.h"

#include "World/DoltLiteDatabase.h"

#include <cmath>
#include <cstdio>
#include <optional>

struct VerifyValue
{
    int type = 5; // sqlite3 fundamental type: 1 integer, 2 float, 3 text, 4 blob, 5 null
    int64_t integer = 0;
    double real = 0.0;
    std::string text;
};

typedef std::vector<VerifyValue> VerifyRow;

static VerifyValue ReadValue(DoltLiteStatement const& stmt, int index)
{
    VerifyValue value;
    value.type = stmt.ColumnType(index);
    if (value.type == 1)
        value.integer = stmt.ColumnInt64(index);
    else if (value.type == 2)
        value.real = stmt.ColumnDouble(index);
    else if (value.type == 3 || value.type == 4)
        value.text = stmt.ColumnText(index);
    return value;
}

static double NumericValue(VerifyValue const& value)
{
    return value.type == 1 ? static_cast<double>(value.integer) : value.real;
}

static int TypeClass(int type)
{
    switch (type)
    {
        case 5: return 0;
        case 1:
        case 2: return 1;
        case 3: return 2;
        default: return 3;
    }
}

// SQLite's ORDER BY order under the BINARY collation.
static int OrderValues(VerifyValue const& a, VerifyValue const& b)
{
    int classA = TypeClass(a.type), classB = TypeClass(b.type);
    if (classA != classB)
        return classA < classB ? -1 : 1;
    if (classA == 0)
        return 0;
    if (classA == 1)
    {
        if (a.type == 1 && b.type == 1)
            return a.integer < b.integer ? -1 : a.integer > b.integer ? 1 : 0;
        double x = NumericValue(a), y = NumericValue(b);
        return x < y ? -1 : x > y ? 1 : 0;
    }
    int c = a.text.compare(b.text);
    return c < 0 ? -1 : c > 0 ? 1 : 0;
}

static bool EqualValues(VerifyValue const& a, VerifyValue const& b)
{
    int classA = TypeClass(a.type), classB = TypeClass(b.type);
    if (classA != classB)
        return false;
    if (classA == 1)
    {
        if (a.type == 1 && b.type == 1)
            return a.integer == b.integer;
        float x = static_cast<float>(NumericValue(a)), y = static_cast<float>(NumericValue(b));
        return x == y || (std::isnan(x) && std::isnan(y));
    }
    return classA == 0 || a.text == b.text;
}

static std::string FormatValue(VerifyValue const& value)
{
    switch (value.type)
    {
        case 1: return std::to_string(value.integer);
        case 2:
        {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%.9g", value.real);
            return buffer;
        }
        case 3:
        case 4: return "'" + value.text + "'";
        default: return "NULL";
    }
}

static int OrderKeys(VerifyRow const& a, VerifyRow const& b, std::vector<int> const& keyColumns)
{
    for (int column : keyColumns)
        if (int c = OrderValues(a[column], b[column]))
            return c;
    return 0;
}

static std::string FormatKey(VerifyRow const& row, std::vector<int> const& keyColumns, std::vector<std::string> const& names)
{
    std::string key;
    for (size_t i = 0; i < keyColumns.size() && i < 8; ++i)
        key += (i ? ", " : "") + names[keyColumns[i]] + "=" + FormatValue(row[keyColumns[i]]);
    if (keyColumns.size() > 8)
        key += ", ...";
    return key;
}

// Reads one statement in runs of rows with an equal key.
class VerifyRunReader
{
public:
    VerifyRunReader(DoltLiteStatement& stmt, std::vector<int> const& keyColumns, uint64_t& rowCount)
        : m_stmt(stmt), m_keyColumns(keyColumns), m_rowCount(rowCount)
    {
        Advance();
    }

    bool Done() const { return !m_pending; }
    VerifyRow const& Front() const { return *m_pending; }

    std::vector<VerifyRow> TakeRun()
    {
        std::vector<VerifyRow> run;
        run.push_back(std::move(*m_pending));
        Advance();
        while (m_pending && OrderKeys(*m_pending, run.front(), m_keyColumns) == 0)
        {
            run.push_back(std::move(*m_pending));
            Advance();
        }
        return run;
    }

private:
    void Advance()
    {
        if (!m_stmt.Step())
        {
            m_pending.reset();
            return;
        }
        VerifyRow row(static_cast<size_t>(m_stmt.ColumnCount()));
        for (int i = 0; i < m_stmt.ColumnCount(); ++i)
            row[i] = ReadValue(m_stmt, i);
        m_pending = std::move(row);
        ++m_rowCount;
    }

    DoltLiteStatement& m_stmt;
    std::vector<int> const& m_keyColumns;
    uint64_t& m_rowCount;
    std::optional<VerifyRow> m_pending;
};

static std::string OrderedSelect(std::string const& sql, std::vector<int> const& keyColumns, int columnCount)
{
    std::string order;
    std::vector<bool> used(static_cast<size_t>(columnCount), false);
    for (int column : keyColumns)
    {
        order += (order.empty() ? "" : ", ") + std::to_string(column + 1);
        used[column] = true;
    }
    for (int column = 0; column < columnCount; ++column)
        if (!used[column])
            order += (order.empty() ? "" : ", ") + std::to_string(column + 1);
    return "SELECT * FROM (" + sql + ") ORDER BY " + order;
}

RowCompareResult CompareRowSets(DoltLiteDatabase& db, std::string const& expectedSql, std::string const& actualSql,
                                 std::vector<int> const& keyColumns)
{
    RowCompareResult result;

    int columnCount = 0;
    {
        DoltLiteStatement probe = db.Prepare(expectedSql);
        columnCount = probe.ColumnCount();
        for (int i = 0; i < columnCount; ++i)
            result.columns.push_back(probe.ColumnName(i));
        DoltLiteStatement probeActual = db.Prepare(actualSql);
        if (probeActual.ColumnCount() != columnCount)
            throw DoltLiteError("column count differs: " + std::to_string(columnCount) + " expected, " +
                                std::to_string(probeActual.ColumnCount()) + " actual");
    }

    std::vector<int> keys = keyColumns;
    if (keys.empty())
        for (int i = 0; i < columnCount; ++i)
            keys.push_back(i);

    DoltLiteStatement expectedStmt = db.Prepare(OrderedSelect(expectedSql, keys, columnCount));
    DoltLiteStatement actualStmt = db.Prepare(OrderedSelect(actualSql, keys, columnCount));
    VerifyRunReader expected(expectedStmt, keys, result.expectedRows);
    VerifyRunReader actual(actualStmt, keys, result.actualRows);

    std::vector<ColumnDifference> perColumn(static_cast<size_t>(columnCount));

    while (!expected.Done() || !actual.Done())
    {
        int order = expected.Done() ? 1 : actual.Done() ? -1 : OrderKeys(expected.Front(), actual.Front(), keys);
        if (order < 0)
        {
            std::vector<VerifyRow> run = expected.TakeRun();
            if (result.firstMissingKey.empty())
                result.firstMissingKey = FormatKey(run.front(), keys, result.columns);
            result.missingRows += run.size();
            continue;
        }
        if (order > 0)
        {
            std::vector<VerifyRow> run = actual.TakeRun();
            if (result.firstExtraKey.empty())
                result.firstExtraKey = FormatKey(run.front(), keys, result.columns);
            result.extraRows += run.size();
            continue;
        }

        std::vector<VerifyRow> expectedRun = expected.TakeRun();
        std::vector<VerifyRow> actualRun = actual.TakeRun();
        size_t paired = std::min(expectedRun.size(), actualRun.size());
        for (size_t r = 0; r < paired; ++r)
        {
            bool changed = false;
            for (int column = 0; column < columnCount; ++column)
            {
                VerifyValue const& e = expectedRun[r][column];
                VerifyValue const& a = actualRun[r][column];
                if (EqualValues(e, a))
                    continue;
                changed = true;
                ColumnDifference& difference = perColumn[column];
                if (difference.rows++ == 0)
                {
                    difference.firstKey = FormatKey(expectedRun[r], keys, result.columns);
                    difference.expected = FormatValue(e);
                    difference.actual = FormatValue(a);
                }
            }
            if (changed)
                ++result.changedRows;
        }
        if (expectedRun.size() > paired)
        {
            if (result.firstMissingKey.empty())
                result.firstMissingKey = FormatKey(expectedRun.front(), keys, result.columns);
            result.missingRows += expectedRun.size() - paired;
        }
        if (actualRun.size() > paired)
        {
            if (result.firstExtraKey.empty())
                result.firstExtraKey = FormatKey(actualRun.front(), keys, result.columns);
            result.extraRows += actualRun.size() - paired;
        }
    }

    for (int column = 0; column < columnCount; ++column)
    {
        if (!perColumn[column].rows)
            continue;
        perColumn[column].column = result.columns[column];
        result.differences.push_back(std::move(perColumn[column]));
    }
    return result;
}
