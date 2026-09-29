#include "Counterpart.h"

#include "ColumnMatch.h"
#include "World/DoltLiteDatabase.h"

#include <map>

static bool TableExists(DoltLiteDatabase& db, std::string const& table)
{
    DoltLiteStatement stmt = db.Prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?");
    stmt.BindText(1, table);
    return stmt.Step();
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

    ValueEncoder encoder;
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
        LoadedTable sourceData = LoadTable(reference, sourceTable, encoder);

        std::vector<PreparedJoin> joins;
        for (SpecPairRow const* pair : groupPairs)
        {
            std::string cdbcTable = "cdbc_" + pair->cdbcTable;
            if (!TableExists(reference, cdbcTable))
            {
                ++result.pairsSkipped;
                continue;
            }
            LoadedTable cdbcData = LoadTable(reference, cdbcTable, encoder);
            int sourceKey = sourceData.ColumnIndex(pair->sourceKey);
            int cdbcKey = cdbcData.ColumnIndex(pair->cdbcKey);
            if (sourceKey < 0 || cdbcKey < 0)
            {
                ++result.pairsSkipped;
                continue;
            }

            joins.push_back(PrepareJoin(sourceData, sourceKey, pair->cdbcTable, cdbcData, cdbcKey));
            ++result.pairsJoined;
        }

        size_t const columnCount = sourceData.columns.size();
        std::vector<int> allColumns(columnCount);
        for (size_t i = 0; i < columnCount; ++i)
            allColumns[i] = static_cast<int>(i);
        std::vector<std::vector<RuleVariant>> variants = ApplyRulesToTable(reference, sourceTable, sourceData, allColumns, valueRules, encoder);

        std::vector<BestColumnMatch> best(columnCount);
        ParallelFor(columnCount, [&](size_t i)
        {
            std::string const category = TypeCategory(sourceData.columns[i].type);
            for (PreparedJoin const& join : joins)
                ScoreJoin(join, variants[i], category, best[i]);
        });

        for (size_t i = 0; i < columnCount; ++i)
        {
            if (best[i].joined == 0)
                continue;
            std::string matchKey = source + "	" + table + "	" + sourceData.columns[i].name;
            result.matches[matchKey] = { "cdbc_" + best[i].targetTable + "." + best[i].targetColumn, best[i].ruleName,
                std::to_string(best[i].matched) + "/" + std::to_string(best[i].joined) };
        }
    }

    return result;
}
