#include "ColumnMatch.h"

#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_set>

struct EncodedValueHash
{
    size_t operator()(EncodedValue v) const { return std::hash<int64_t>()(v.value) * 31 + v.kind; }
};

struct EncodedValueSame
{
    bool operator()(EncodedValue a, EncodedValue b) const { return a.kind == b.kind && a.value == b.value; }
};

EncodedValue ValueEncoder::Text(std::string const& text)
{
    auto [it, added] = m_textIds.try_emplace(text, static_cast<int64_t>(m_texts.size()));
    if (added)
        m_texts.push_back(text);
    return { it->second, EncodedValue::Text };
}

void ValueEncoder::Bind(DoltLiteStatement& stmt, int index, EncodedValue value) const
{
    switch (value.kind)
    {
        case EncodedValue::Integer: stmt.BindInt64(index, value.value); break;
        case EncodedValue::Real:
        {
            double number;
            std::memcpy(&number, &value.value, sizeof(number));
            stmt.BindDouble(index, number);
            break;
        }
        case EncodedValue::Text: stmt.BindText(index, m_texts[static_cast<size_t>(value.value)]); break;
        default: stmt.BindNull(index); break;
    }
}

EncodedValue ValueEncoder::Number(double number)
{
    if (std::isfinite(number) && number == std::floor(number) && std::fabs(number) < 9.2e18)
        return { static_cast<int64_t>(number), EncodedValue::Integer };
    int64_t bits;
    std::memcpy(&bits, &number, sizeof(bits));
    return { bits, EncodedValue::Real };
}

EncodedValue ValueEncoder::Read(DoltLiteStatement const& stmt, int index)
{
    switch (stmt.ColumnType(index))
    {
        case 1: return { stmt.ColumnInt64(index), EncodedValue::Integer };
        case 2: return Number(stmt.ColumnDouble(index));
        case 5: return {};
        default: return Text(stmt.ColumnText(index));
    }
}

int LoadedTable::ColumnIndex(std::string const& name) const
{
    for (size_t i = 0; i < columns.size(); ++i)
        if (columns[i].name == name)
            return static_cast<int>(i);
    return -1;
}

std::string TypeCategory(std::string type)
{
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (type.find("CHAR") != std::string::npos || type.find("TEXT") != std::string::npos || type.find("CLOB") != std::string::npos)
        return "text";
    return "numeric";
}

LoadedTable LoadTable(DoltLiteDatabase& db, std::string const& table, ValueEncoder& encoder)
{
    LoadedTable loaded;
    {
        DoltLiteStatement info = db.Prepare("PRAGMA table_info(\"" + table + "\")");
        while (info.Step())
            loaded.columns.push_back({ info.ColumnText(1), info.ColumnText(2) });
    }
    loaded.values.resize(loaded.columns.size());

    DoltLiteStatement rows = db.Prepare("SELECT * FROM \"" + table + "\"");
    while (rows.Step())
    {
        for (size_t i = 0; i < loaded.columns.size(); ++i)
            loaded.values[i].push_back(encoder.Read(rows, static_cast<int>(i)));
        ++loaded.rowCount;
    }
    return loaded;
}

PreparedJoin PrepareJoin(LoadedTable const& source, int sourceKey, std::string const& targetTable, LoadedTable const& target,
    int targetKey)
{
    PreparedJoin join;
    join.targetTable = targetTable;
    join.targetColumns = target.columns;
    join.targetValues.resize(target.columns.size());

    std::unordered_multimap<EncodedValue, uint32_t, EncodedValueHash, EncodedValueSame> targetRowsByKey;
    targetRowsByKey.reserve(target.rowCount);
    for (uint32_t row = 0; row < target.rowCount; ++row)
    {
        EncodedValue key = target.values[targetKey][row];
        if (key.kind != EncodedValue::Null)
            targetRowsByKey.emplace(key, row);
    }

    std::vector<uint32_t> targetRows;
    for (uint32_t row = 0; row < source.rowCount; ++row)
    {
        EncodedValue key = source.values[sourceKey][row];
        if (key.kind == EncodedValue::Null)
            continue;
        auto [begin, end] = targetRowsByKey.equal_range(key);
        for (auto it = begin; it != end; ++it)
        {
            join.sourceRows.push_back(row);
            targetRows.push_back(it->second);
        }
    }

    for (size_t column = 0; column < target.columns.size(); ++column)
    {
        std::vector<EncodedValue>& gathered = join.targetValues[column];
        gathered.reserve(targetRows.size());
        for (uint32_t row : targetRows)
            gathered.push_back(target.values[column][row]);
    }
    return join;
}

// The value SQLite compares after applying a `category` column's affinity to a rule result.
static EncodedValue ReadRuleResult(DoltLiteStatement const& stmt, int index, std::string const& category, ValueEncoder& encoder)
{
    int type = stmt.ColumnType(index);
    if (type == 5)
        return {};
    if (category == "text")
        return encoder.Text(stmt.ColumnText(index));
    if (type == 3)
    {
        // Numeric affinity converts text that is a plain decimal literal (no hex, inf or nan).
        std::string const text = stmt.ColumnText(index);
        size_t const first = text.find_first_not_of(" \t\r\n");
        size_t const last = text.find_last_not_of(" \t\r\n");
        if (first != std::string::npos && text.find_first_of("xXnNiI") == std::string::npos)
        {
            std::string const trimmed = text.substr(first, last - first + 1);
            char* end = nullptr;
            double const number = std::strtod(trimmed.c_str(), &end);
            if (*end == '\0')
                return encoder.Number(number);
        }
        return encoder.Text(text);
    }
    return encoder.Read(stmt, index);
}

static std::string ApplyRuleExpr(std::string const& expr, std::string const& columnRef)
{
    std::string out;
    for (size_t i = 0; i < expr.size();)
    {
        if (expr.compare(i, 3, "{v}") == 0)
        {
            out += columnRef;
            i += 3;
        }
        else
            out += expr[i++];
    }
    return out;
}

using ValueMap = std::unordered_map<EncodedValue, EncodedValue, EncodedValueHash, EncodedValueSame>;

// Per rule, the column's values mapped through `resultByValue`, minus rules identical to an earlier one.
static std::vector<RuleVariant> BuildVariants(std::vector<EncodedValue> const& sourceValues, std::vector<SpecRuleRow> const& rules,
    std::vector<ValueMap>& resultByValue)
{
    std::vector<RuleVariant> variants;
    for (size_t r = 0; r < rules.size(); ++r)
    {
        RuleVariant variant{ &rules[r], {} };
        variant.values.reserve(sourceValues.size());
        for (EncodedValue value : sourceValues)
            variant.values.push_back(resultByValue[r][value]);

        bool duplicate = std::any_of(variants.begin(), variants.end(), [&](RuleVariant const& earlier)
        {
            return std::equal(earlier.values.begin(), earlier.values.end(), variant.values.begin(), EncodedValueSame());
        });
        if (!duplicate)
            variants.push_back(std::move(variant));
    }
    return variants;
}

std::vector<RuleVariant> ApplyRules(DoltLiteDatabase& db, std::string const& sourceTable, LoadedTable const& source, int column,
    std::vector<SpecRuleRow> const& rules, ValueEncoder& encoder)
{
    if (rules.empty())
        return {};

    std::string const category = TypeCategory(source.columns[column].type);
    std::string const columnRef = "s.\"" + source.columns[column].name + "\"";

    std::string sql = "SELECT DISTINCT " + columnRef;
    for (SpecRuleRow const& rule : rules)
        sql += ", (" + ApplyRuleExpr(rule.expr.empty() ? "{v}" : rule.expr, columnRef) + ")";
    sql += " FROM \"" + sourceTable + "\" s";

    std::vector<ValueMap> resultByValue(rules.size());
    try
    {
        DoltLiteStatement stmt = db.Prepare(sql);
        while (stmt.Step())
        {
            EncodedValue value = encoder.Read(stmt, 0);
            for (size_t r = 0; r < rules.size(); ++r)
                resultByValue[r][value] = ReadRuleResult(stmt, static_cast<int>(r) + 1, category, encoder);
        }
    }
    catch (DoltLiteError const&)
    {
        // A rule expression that doesn't type-check against this column: the column gets no
        // counterpart, as when the whole scoring query failed.
        return {};
    }
    return BuildVariants(source.values[column], rules, resultByValue);
}

std::vector<std::vector<RuleVariant>> ApplyRulesToTable(DoltLiteDatabase& db, std::string const& sourceTable, LoadedTable const& source,
    std::vector<int> const& columns, std::vector<SpecRuleRow> const& rules, ValueEncoder& encoder)
{
    std::vector<std::vector<RuleVariant>> variants(columns.size());
    if (rules.empty() || columns.empty())
        return variants;

    // Columns grouped by declared type: each group's distinct values go into one scratch table
    // whose column has that type, so the rules see the same affinity as on the source table.
    std::map<std::string, std::vector<size_t>> positionsByType;
    for (size_t i = 0; i < columns.size(); ++i)
        positionsByType[source.columns[columns[i]].type].push_back(i);

    DoltLiteDatabase scratch(":memory:");
    for (auto const& [type, positions] : positionsByType)
    {
        scratch.Execute("DROP TABLE IF EXISTS t");
        scratch.Execute("CREATE TABLE t (position INTEGER, v " + type + ")");
        scratch.Begin();
        {
            DoltLiteStatement insert = scratch.Prepare("INSERT INTO t VALUES (?, ?)");
            for (size_t position : positions)
            {
                std::unordered_set<EncodedValue, EncodedValueHash, EncodedValueSame> seen;
                for (EncodedValue value : source.values[columns[position]])
                {
                    if (!seen.insert(value).second)
                        continue;
                    insert.BindInt64(1, static_cast<int64_t>(position));
                    encoder.Bind(insert, 2, value);
                    insert.Step();
                    insert.Reset();
                }
            }
        }
        scratch.Commit();

        std::string sql = "SELECT position, t.v";
        for (SpecRuleRow const& rule : rules)
            sql += ", (" + ApplyRuleExpr(rule.expr.empty() ? "{v}" : rule.expr, "t.v") + ")";
        sql += " FROM t";

        std::string const category = TypeCategory(type);
        std::map<size_t, std::vector<ValueMap>> resultByPosition;
        bool failed = false;
        try
        {
            DoltLiteStatement stmt = scratch.Prepare(sql);
            while (stmt.Step())
            {
                std::vector<ValueMap>& results = resultByPosition[static_cast<size_t>(stmt.ColumnInt64(0))];
                results.resize(rules.size());
                EncodedValue value = encoder.Read(stmt, 1);
                for (size_t r = 0; r < rules.size(); ++r)
                    results[r][value] = ReadRuleResult(stmt, static_cast<int>(r) + 2, category, encoder);
            }
        }
        catch (DoltLiteError const&)
        {
            failed = true; // find the failing columns one by one
        }

        for (size_t position : positions)
        {
            if (failed)
                variants[position] = ApplyRules(db, sourceTable, source, columns[position], rules, encoder);
            else
            {
                std::vector<ValueMap>& results = resultByPosition[position];
                results.resize(rules.size());
                variants[position] = BuildVariants(source.values[columns[position]], rules, results);
            }
        }
    }
    return variants;
}

void ScoreJoin(PreparedJoin const& join, std::vector<RuleVariant> const& variants, std::string const& sourceCategory,
    BestColumnMatch& best)
{
    std::vector<size_t> compatible;
    for (size_t i = 0; i < join.targetColumns.size(); ++i)
        if (TypeCategory(join.targetColumns[i].type) == sourceCategory)
            compatible.push_back(i);
    int64_t const joined = static_cast<int64_t>(join.sourceRows.size());
    if (compatible.empty() || joined == 0)
        return;

    std::vector<EncodedValue> sourceJoined(join.sourceRows.size());
    for (RuleVariant const& variant : variants)
    {
        for (size_t k = 0; k < join.sourceRows.size(); ++k)
            sourceJoined[k] = variant.values[join.sourceRows[k]];

        for (size_t column : compatible)
        {
            std::vector<EncodedValue> const& target = join.targetValues[column];
            int64_t matched = 0;
            for (size_t k = 0; k < sourceJoined.size(); ++k)
                matched += SqlEqual(sourceJoined[k], target[k]);
            if (best.BeatsRate(matched, joined))
                best = { join.targetTable, join.targetColumns[column].name, variant.rule->name, matched, joined };
        }
    }
}

void ParallelFor(size_t count, std::function<void(size_t)> const& fn)
{
    size_t const threads = std::min<size_t>(count, std::max(1u, std::thread::hardware_concurrency()));
    std::atomic<size_t> next{ 0 };
    std::exception_ptr failure;
    std::mutex failureMutex;

    auto worker = [&]()
    {
        for (size_t i = next++; i < count; i = next++)
        {
            try
            {
                fn(i);
            }
            catch (...)
            {
                std::lock_guard<std::mutex> lock(failureMutex);
                if (!failure)
                    failure = std::current_exception();
            }
        }
    };

    std::vector<std::thread> pool;
    for (size_t i = 1; i < threads; ++i)
        pool.emplace_back(worker);
    worker();
    for (std::thread& thread : pool)
        thread.join();
    if (failure)
        std::rethrow_exception(failure);
}
