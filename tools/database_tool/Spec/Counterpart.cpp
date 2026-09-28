#include "Counterpart.h"

#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <map>

struct ColumnInfo
{
    std::string name;
    std::string type;
};

static bool TableExists(DoltLiteDatabase& db, std::string const& table)
{
    DoltLiteStatement stmt = db.Prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?");
    stmt.BindText(1, table);
    return stmt.Step();
}

static std::vector<ColumnInfo> TableColumns(DoltLiteDatabase& db, std::string const& table)
{
    std::vector<ColumnInfo> columns;
    DoltLiteStatement stmt = db.Prepare("PRAGMA table_info(\"" + table + "\")");
    while (stmt.Step())
        columns.push_back({ stmt.ColumnText(1), stmt.ColumnText(2) });
    return columns;
}

// SQLite's declared-type affinity is loose enough (INTEGER and REAL already compare correctly
// against each other) that only text needs to be kept apart from everything else.
static std::string TypeCategory(std::string type)
{
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (type.find("CHAR") != std::string::npos || type.find("TEXT") != std::string::npos || type.find("CLOB") != std::string::npos)
        return "text";
    return "numeric";
}

// Replaces every "{v}" in `expr` with `columnRef` (already a quoted, table-qualified reference).
static std::string ApplyRuleExpr(std::string const& expr, std::string const& columnRef)
{
    std::string out;
    out.reserve(expr.size());
    for (size_t i = 0; i < expr.size();)
    {
        if (expr.compare(i, 3, "{v}") == 0)
        {
            out += columnRef;
            i += 3;
        }
        else
        {
            out += expr[i];
            ++i;
        }
    }
    return out;
}

struct BestMatch
{
    std::string cdbcTable;
    std::string cdbcColumn;
    std::string ruleName;
    int64_t matched = 0;
    int64_t joined = 0;

    bool BeatsRate(int64_t candidateMatched, int64_t candidateJoined) const
    {
        return joined == 0 || candidateMatched * joined > matched * candidateJoined;
    }
};

struct JoinCandidate
{
    std::string cdbcTable; // unqualified, e.g. "areatable"
    std::string joinSql;   // `"srv_area_template" s JOIN "cdbc_areatable" c ON ...`
    std::vector<ColumnInfo> cdbcColumns;
};

struct ScoreTerm
{
    SpecRuleRow const* rule;
    ColumnInfo const* column;
};

// Every (rule, type-compatible cdbc column) combination against `join`, in a single query -- see
// Counterpart.h: this is what keeps a wide table's search from costing (source columns * cdbc
// columns * rules) round trips instead of one per source column per join. Updates `best` in place
// with whichever combination has the strictly highest match rate.
static void ScoreColumnAgainstJoin(DoltLiteDatabase& db, JoinCandidate const& join, std::string const& srcColumnRef,
    std::vector<SpecRuleRow> const& valueRules, std::string const& sourceCategory, BestMatch& best)
{
    std::vector<ColumnInfo> compatible;
    for (ColumnInfo const& cdbcColumn : join.cdbcColumns)
        if (TypeCategory(cdbcColumn.type) == sourceCategory)
            compatible.push_back(cdbcColumn);
    if (compatible.empty())
        return;

    std::vector<ScoreTerm> terms;
    std::string sql = "SELECT COUNT(*)";
    for (SpecRuleRow const& rule : valueRules)
    {
        std::string ruleValue = ApplyRuleExpr(rule.expr.empty() ? "{v}" : rule.expr, srcColumnRef);
        for (ColumnInfo const& candidate : compatible)
        {
            sql += ", SUM(CASE WHEN (" + ruleValue + ") = c.\"" + candidate.name + "\" THEN 1 ELSE 0 END)";
            terms.push_back({ &rule, &candidate });
        }
    }
    sql += " FROM " + join.joinSql;

    DoltLiteStatement stmt = db.Prepare(sql);
    if (!stmt.Step())
        return;

    int64_t joined = stmt.ColumnInt64(0);
    if (joined == 0)
        return;

    for (size_t i = 0; i < terms.size(); ++i)
    {
        if (stmt.ColumnIsNull(static_cast<int>(i) + 1))
            continue;
        int64_t matched = stmt.ColumnInt64(static_cast<int>(i) + 1);
        if (best.BeatsRate(matched, joined))
            best = { join.cdbcTable, terms[i].column->name, terms[i].rule->name, matched, joined };
    }
}

ComputeCounterpartsResult ComputeCounterparts(DoltLiteDatabase& reference, std::vector<SpecPairRow> const& pairs,
    std::vector<SpecRuleRow> const& rules)
{
    ComputeCounterpartsResult result;

    std::vector<SpecRuleRow> valueRules;
    for (SpecRuleRow const& rule : rules)
        if (rule.kind == "value")
            valueRules.push_back(rule);

    // Group pairs by (source, table): a source table may line up against more than one cdbc
    // table (e.g. item_template against both Item and ItemSparse), and every group is searched
    // together so the best match can come from any of them.
    std::map<std::pair<std::string, std::string>, std::vector<SpecPairRow const*>> grouped;
    for (SpecPairRow const& pair : pairs)
    {
        if (pair.joinKind != "id")
        {
            ++result.pairsSkipped;
            continue;
        }
        grouped[{ pair.source, pair.table }].push_back(&pair);
    }

    for (auto const& [key, groupPairs] : grouped)
    {
        std::string const& source = key.first;
        std::string const& table = key.second;
        std::string sourceTable = (source == "vdbc" ? "vdbc_" : "srv_") + table;

        if (!TableExists(reference, sourceTable))
        {
            result.pairsSkipped += static_cast<uint32_t>(groupPairs.size());
            continue;
        }
        std::vector<ColumnInfo> sourceColumns = TableColumns(reference, sourceTable);

        std::vector<JoinCandidate> joins;
        for (SpecPairRow const* pair : groupPairs)
        {
            std::string cdbcTable = "cdbc_" + pair->cdbcTable;
            if (!TableExists(reference, cdbcTable))
            {
                ++result.pairsSkipped;
                continue;
            }
            std::vector<ColumnInfo> cdbcColumns = TableColumns(reference, cdbcTable);
            bool hasSourceKey = std::any_of(sourceColumns.begin(), sourceColumns.end(),
                [&](ColumnInfo const& c) { return c.name == pair->sourceKey; });
            bool hasCdbcKey = std::any_of(cdbcColumns.begin(), cdbcColumns.end(),
                [&](ColumnInfo const& c) { return c.name == pair->cdbcKey; });
            if (!hasSourceKey || !hasCdbcKey)
            {
                ++result.pairsSkipped;
                continue;
            }

            std::string joinSql = "\"" + sourceTable + "\" s JOIN \"" + cdbcTable + "\" c ON s.\"" + pair->sourceKey +
                                   "\" = c.\"" + pair->cdbcKey + "\"";
            joins.push_back({ pair->cdbcTable, std::move(joinSql), std::move(cdbcColumns) });
            ++result.pairsJoined;
        }

        for (ColumnInfo const& sourceColumn : sourceColumns)
        {
            std::string sourceCategory = TypeCategory(sourceColumn.type);
            std::string srcColumnRef = "s.\"" + sourceColumn.name + "\"";

            BestMatch best;
            for (JoinCandidate const& join : joins)
            {
                try
                {
                    ScoreColumnAgainstJoin(reference, join, srcColumnRef, valueRules, sourceCategory, best);
                }
                catch (DoltLiteError const&)
                {
                    // A rule expression that doesn't type-check against this column (e.g. a
                    // bit-shift landing on a column SQLite stores as text) -- move on.
                }
            }

            if (best.joined == 0)
                continue;

            std::string key = source + "\t" + table + "\t" + sourceColumn.name;
            result.matches[key] = { "cdbc_" + best.cdbcTable + "." + best.cdbcColumn, best.ruleName,
                std::to_string(best.matched) + "/" + std::to_string(best.joined) };
        }
    }

    return result;
}
