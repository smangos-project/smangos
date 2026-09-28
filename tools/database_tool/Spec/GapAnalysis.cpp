#include "GapAnalysis.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include "DbdDefinitions.h"
#include "DbdDiff.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>

namespace
{
    struct ColumnInfo
    {
        std::string name;
        std::string type;
    };

    bool TableExists(DoltLiteDatabase& db, std::string const& table)
    {
        DoltLiteStatement stmt = db.Prepare("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?");
        stmt.BindText(1, table);
        return stmt.Step();
    }

    std::vector<ColumnInfo> TableColumnInfos(DoltLiteDatabase& db, std::string const& table)
    {
        std::vector<ColumnInfo> columns;
        DoltLiteStatement stmt = db.Prepare("PRAGMA table_info(\"" + table + "\")");
        while (stmt.Step())
            columns.push_back({ stmt.ColumnText(1), stmt.ColumnText(2) });
        return columns;
    }

    int64_t TableRowCount(DoltLiteDatabase& db, std::string const& table)
    {
        DoltLiteStatement stmt = db.Prepare("SELECT COUNT(*) FROM \"" + table + "\"");
        return stmt.Step() ? stmt.ColumnInt64(0) : 0;
    }

    std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& db, std::string const& prefix)
    {
        std::vector<std::string> names;
        DoltLiteStatement stmt = db.Prepare(
            "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
        while (stmt.Step())
            names.push_back(stmt.ColumnText(0));
        return names;
    }

    std::string ToLowerCopy(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    std::string Join(std::vector<std::string> const& parts, std::string const& sep)
    {
        std::string out;
        for (size_t i = 0; i < parts.size(); ++i)
            out += (i ? sep : "") + parts[i];
        return out;
    }

    // SQLite's declared-type affinity is loose enough (INTEGER and REAL already compare
    // correctly against each other) that only text needs to be kept apart from everything else --
    // the same rule Counterpart.cpp uses.
    std::string TypeCategory(std::string type)
    {
        std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        if (type.find("CHAR") != std::string::npos || type.find("TEXT") != std::string::npos || type.find("CLOB") != std::string::npos)
            return "text";
        return "numeric";
    }

    std::string ApplyRuleExpr(std::string const& expr, std::string const& columnRef)
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

    // --- 4.5.1: schema diff ---

    void ComputeSchemaDiffGaps(DoltLiteDatabase& reference, std::filesystem::path const& definitionsDir, std::vector<SpecGapRow>& gaps)
    {
        if (definitionsDir.empty() || !std::filesystem::exists(definitionsDir))
            return; // no WoWDBDefs checkout available; leave the schema diff out rather than fail the whole pass

        DbdDefinitions definitions = LoadDbdDefinitions(definitionsDir.string());
        std::vector<DbdBuild> const vanillaBuild = { DbdBuild::Parse("1.12.1.5875") };
        std::vector<DbdBuild> const clientBuilds = { DbdBuild::Parse("1.15.6.58797"), DbdBuild::Parse("1.15.6.58725") };

        for (DbdTableDiff const& diff : DiffDbdTables(definitions, vanillaBuild, clientBuilds))
        {
            std::string table = ToLowerCopy(diff.table);
            std::string qualified = "vdbc_" + table;
            if (!TableExists(reference, qualified))
                continue; // not imported at all -- already "not imported" per stuff-plan.md 8.3/F0, not this pass's concern

            if (diff.removedEntirely)
            {
                gaps.push_back({ "removed_table", table, "", "no 1.15.6.58797/58725 definition" });
                continue;
            }

            std::vector<ColumnInfo> liveColumns = TableColumnInfos(reference, qualified);
            for (DbdFieldDiff const& field : diff.fields)
            {
                bool isLive = std::any_of(liveColumns.begin(), liveColumns.end(),
                    [&](ColumnInfo const& c) { return c.name == field.field; });
                if (!isLive)
                    continue; // the .dbd field name doesn't line up with a real column (e.g. locstring expansion)

                std::string kind = field.kind == "removed" ? "removed_column"
                    : field.kind == "array_length"          ? "array_length"
                                                             : "type_change";
                std::string detail = field.detail.empty() ? "removed in 1.15.6.58797" : field.detail;
                gaps.push_back({ kind, table, field.field, detail });
            }
        }
    }

    // --- 4.5.2: row coverage per pair ---

    void ComputeRowCoverageGaps(DoltLiteDatabase& reference, std::vector<SpecPairRow> const& pairs, std::vector<SpecGapRow>& gaps)
    {
        for (SpecPairRow const& pair : pairs)
        {
            if (pair.joinKind != "id")
                continue;

            std::string sourceTable = (pair.source == "vdbc" ? "vdbc_" : "srv_") + pair.table;
            std::string cdbcTable = "cdbc_" + pair.cdbcTable;
            if (!TableExists(reference, sourceTable) || !TableExists(reference, cdbcTable))
                continue;

            std::string missingClause = "\"" + sourceTable + "\" s WHERE NOT EXISTS (SELECT 1 FROM \"" + cdbcTable +
                "\" c WHERE c.\"" + pair.cdbcKey + "\" = s.\"" + pair.sourceKey + "\")";

            DoltLiteStatement countStmt = reference.Prepare("SELECT COUNT(*) FROM " + missingClause);
            int64_t total = countStmt.Step() ? countStmt.ColumnInt64(0) : 0;
            if (total == 0)
                continue;

            std::vector<std::string> sample;
            DoltLiteStatement sampleStmt =
                reference.Prepare("SELECT s.\"" + pair.sourceKey + "\" FROM " + missingClause + " ORDER BY s.\"" + pair.sourceKey + "\" LIMIT 20");
            while (sampleStmt.Step())
                sample.push_back(sampleStmt.ColumnText(0));

            std::string detail = "vanilla-only ids: " + std::to_string(total) + " (" + Join(sample, ", ") +
                (total > static_cast<int64_t>(sample.size()) ? ", ..." : "") + ")";
            gaps.push_back({ "row_coverage", pair.source + "_" + pair.table, "", detail });
        }
    }

    // --- 4.5.3: counterpart discovery outside the pairs ---

    struct KeyGroup
    {
        std::string sourceKey;
        std::set<std::string> pairedCdbcTables; // unqualified, e.g. "areatable"
        std::set<std::string> keyNames;         // e.g. "ID", "SpellID"
    };

    struct BestOutsideMatch
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

    struct OutsideJoinCandidate
    {
        std::string cdbcTable;
        std::string joinSql;
        std::vector<ColumnInfo> cdbcColumns;
    };

    void ScoreAgainstOutsideJoin(DoltLiteDatabase& db, OutsideJoinCandidate const& join, std::string const& srcColumnRef,
        std::vector<SpecRuleRow> const& valueRules, std::string const& sourceCategory, BestOutsideMatch& best)
    {
        std::vector<ColumnInfo> compatible;
        for (ColumnInfo const& column : join.cdbcColumns)
            if (TypeCategory(column.type) == sourceCategory)
                compatible.push_back(column);
        if (compatible.empty())
            return;

        struct Term
        {
            SpecRuleRow const* rule;
            ColumnInfo const* column;
        };
        std::vector<Term> terms;
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

        try
        {
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
        catch (DoltLiteError const&)
        {
            // a rule expression that doesn't type-check against this candidate column -- move on
        }
    }

    void ComputeCounterpartCandidateGaps(DoltLiteDatabase& reference, std::vector<SpecPairRow> const& pairs,
        std::vector<SpecRuleRow> const& rules, std::vector<SpecEvidenceRow> const& evidence, std::vector<SpecGapRow>& gaps)
    {
        std::vector<SpecRuleRow> valueRules;
        for (SpecRuleRow const& rule : rules)
            if (rule.kind == "value")
                valueRules.push_back(rule);

        std::map<std::pair<std::string, std::string>, KeyGroup> groups;
        for (SpecPairRow const& pair : pairs)
        {
            if (pair.joinKind != "id")
                continue;
            KeyGroup& group = groups[{ pair.source, pair.table }];
            group.sourceKey = pair.sourceKey;
            group.pairedCdbcTables.insert(pair.cdbcTable);
            group.keyNames.insert(pair.cdbcKey);
        }

        // Every non-90%-or-better column with a reader, grouped by (source, table): what 4.5.3
        // has to find a home for.
        std::map<std::pair<std::string, std::string>, std::vector<std::string>> uncoveredColumns;
        for (SpecEvidenceRow const& row : evidence)
        {
            if (row.readers == "none")
                continue;
            // A constant (or all-null) column, like the empty vanilla locale slots 4.4 already
            // flags, "matches" any other same-valued constant column at 100% by coincidence --
            // noise from the search having nothing real to compare against, the same trap 4.4's
            // Progress notes call out. There is nothing this pass can usefully find for it.
            if (row.shape.find("distinct=0 ") != std::string::npos || row.shape.find("distinct=1 ") != std::string::npos)
                continue;
            if (!row.counterpart.empty())
            {
                size_t slash = row.matchRate.find('/');
                if (slash != std::string::npos)
                {
                    int64_t matched = std::atoll(row.matchRate.substr(0, slash).c_str());
                    int64_t total = std::atoll(row.matchRate.substr(slash + 1).c_str());
                    if (total > 0 && matched * 100 >= total * 90)
                        continue; // already covered at 90%+
                }
            }
            uncoveredColumns[{ row.source, row.table }].push_back(row.column);
        }

        std::vector<std::string> allCdbcTables = TablesWithPrefix(reference, "cdbc");

        for (auto const& [key, group] : groups)
        {
            auto columnsIt = uncoveredColumns.find(key);
            if (columnsIt == uncoveredColumns.end())
                continue;

            std::string const& source = key.first;
            std::string const& table = key.second;
            std::string sourceTable = (source == "vdbc" ? "vdbc_" : "srv_") + table;
            if (!TableExists(reference, sourceTable))
                continue;

            int64_t sourceRowCount = TableRowCount(reference, sourceTable);
            if (sourceRowCount == 0)
                continue;

            std::vector<ColumnInfo> sourceColumns = TableColumnInfos(reference, sourceTable);
            bool hasSourceKey = std::any_of(sourceColumns.begin(), sourceColumns.end(),
                [&](ColumnInfo const& c) { return c.name == group.sourceKey; });
            if (!hasSourceKey)
                continue;

            std::vector<OutsideJoinCandidate> candidates;
            for (std::string const& qualified : allCdbcTables)
            {
                std::string cdbcTable = qualified.substr(5); // strip "cdbc_"
                if (group.pairedCdbcTables.count(cdbcTable))
                    continue; // already a declared pair -- 4.3's job, not this one's
                if (TableRowCount(reference, qualified) == 0)
                    continue;

                std::vector<ColumnInfo> cdbcColumns = TableColumnInfos(reference, qualified);
                for (ColumnInfo const& column : cdbcColumns)
                {
                    if (!group.keyNames.count(column.name))
                        continue;
                    std::string joinSql = "\"" + sourceTable + "\" s JOIN \"" + qualified + "\" c ON s.\"" + group.sourceKey +
                                           "\" = c.\"" + column.name + "\"";
                    candidates.push_back({ cdbcTable, std::move(joinSql), cdbcColumns });
                    break; // one join per candidate table is enough, even if several columns share a key name
                }
            }
            if (candidates.empty())
                continue;

            for (std::string const& columnName : columnsIt->second)
            {
                auto columnIt = std::find_if(sourceColumns.begin(), sourceColumns.end(),
                    [&](ColumnInfo const& c) { return c.name == columnName; });
                if (columnIt == sourceColumns.end())
                    continue;

                std::string sourceCategory = TypeCategory(columnIt->type);
                std::string srcColumnRef = "s.\"" + columnName + "\"";

                BestOutsideMatch best;
                for (OutsideJoinCandidate const& candidate : candidates)
                    ScoreAgainstOutsideJoin(reference, candidate, srcColumnRef, valueRules, sourceCategory, best);

                // Two gates beyond "found something": most of the source table's ids have to
                // show up in the candidate at all (else it's a coincidental small-integer
                // overlap, the same trap 4.3's Progress notes call out for the `inverse` rule),
                // and the best rule has to actually agree on most of the joined rows.
                if (best.joined * 2 < sourceRowCount || best.matched * 2 < best.joined)
                    continue;

                std::string detail = "cdbc_" + best.cdbcTable + "." + best.cdbcColumn + " via " + best.ruleName + " " +
                    std::to_string(best.matched) + "/" + std::to_string(best.joined) + " (not in pairs.tsv)";
                gaps.push_back({ "counterpart_candidate", source + "_" + table, columnName, detail });
            }
        }
    }

    // --- 4.5.4: empty client tables ---

    void ComputeEmptyClientTableGaps(DoltLiteDatabase& reference, std::vector<SpecPairRow> const& pairs, std::vector<SpecGapRow>& gaps)
    {
        std::map<std::string, std::vector<std::string>> srvHoldersByCdbcTable;
        for (SpecPairRow const& pair : pairs)
            if (pair.source == "srv")
                srvHoldersByCdbcTable[pair.cdbcTable].push_back(pair.table);

        for (std::string const& qualified : TablesWithPrefix(reference, "cdbc"))
        {
            if (TableRowCount(reference, qualified) != 0)
                continue;

            std::string fileStem = qualified.substr(5);
            std::vector<std::string> detailParts;

            std::string vdbcTable = "vdbc_" + fileStem;
            if (TableExists(reference, vdbcTable))
                detailParts.push_back("vanilla twin: " + vdbcTable);

            auto srvIt = srvHoldersByCdbcTable.find(fileStem);
            if (srvIt != srvHoldersByCdbcTable.end())
                for (std::string const& srvTable : srvIt->second)
                    detailParts.push_back("srv holds: srv_" + srvTable);

            std::string detail = detailParts.empty() ? "no known srv/vdbc twin -- server-only data" : Join(detailParts, "; ");
            gaps.push_back({ "empty_client_table", "dbc_" + fileStem, "", detail });
        }
    }

    // --- 4.5.5: client-only fields mangosd will need ---

    struct KnownClientOnlyItem
    {
        std::string tablePrefix;  // matched against the cdbc_ file stem
        std::string columnPrefix; // empty means "the whole table"
        std::string note;
    };

    std::vector<KnownClientOnlyItem> const& KnownClientOnlyItems()
    {
        // data-plan.md section 4: "DB2-only data the server will need" -- fixed list, not
        // something derivable from the reference database, so the reviewer knows up front which
        // dbc-columns.tsv rows to mark `mangosd_reads` even before a loader exists for them.
        static std::vector<KnownClientOnlyItem> const items = {
            { "questv2", "UniqueBitFlag", "completed-quest bit (QuestCompleted update field)" },
            { "itemmodifiedappearance", "", "item -> display id, feeds ItemAppearance.DefaultIconFileDataID" },
            { "spellxspellvisual", "", "wire visual id" },
            { "summonproperties", "", "summon effect dispatch after the enum renumbering" },
            { "spellmisc", "Attributes", "aura flags such as points-on-client (indices 5-15)" },
            { "chrcustomization", "", "character create/barber validation" },
            { "chrmodel", "", "character create/barber validation" },
            { "chrracexchrmodel", "", "character create/barber validation" },
            { "uimap", "", "replaces WorldMapArea for zone lookups" },
            { "soundkit", "", "sound id space for PlaySound/PlayMusic" },
        };
        return items;
    }

    void ComputeClientOnlyFieldGaps(DoltLiteDatabase& reference, std::vector<SpecGapRow>& gaps)
    {
        for (std::string const& qualified : TablesWithPrefix(reference, "cdbc"))
        {
            std::string fileStem = qualified.substr(5);
            for (KnownClientOnlyItem const& item : KnownClientOnlyItems())
            {
                if (fileStem.compare(0, item.tablePrefix.size(), item.tablePrefix) != 0)
                    continue;

                std::string dbcTable = "dbc_" + fileStem;
                std::string note = item.note + " (data-plan.md section 4)";
                if (item.columnPrefix.empty())
                {
                    gaps.push_back({ "client_only_field", dbcTable, "", note });
                    continue;
                }
                for (ColumnInfo const& column : TableColumnInfos(reference, qualified))
                    if (column.name.compare(0, item.columnPrefix.size(), item.columnPrefix) == 0)
                        gaps.push_back({ "client_only_field", dbcTable, column.name, note });
            }
        }
    }
}

SpecGapResult BuildGaps(std::filesystem::path const& referencePath, std::filesystem::path const& definitionsDir,
    std::filesystem::path const& specDir)
{
    SpecGapResult result;
    if (!std::filesystem::exists(referencePath))
    {
        result.error = referencePath.string() + " does not exist (run reference first)";
        return result;
    }

    DoltLiteDatabase reference(referencePath.string());
    std::vector<SpecPairRow> pairs = ReadPairsTsv(specDir / "pairs.tsv");
    std::vector<SpecRuleRow> rules = ReadRulesTsv(specDir / "rules.tsv");
    std::vector<SpecEvidenceRow> evidence = ReadEvidenceTsv(specDir / "evidence.tsv");

    std::vector<SpecGapRow> gaps;
    ComputeSchemaDiffGaps(reference, definitionsDir, gaps);
    ComputeRowCoverageGaps(reference, pairs, gaps);
    ComputeCounterpartCandidateGaps(reference, pairs, rules, evidence, gaps);
    ComputeEmptyClientTableGaps(reference, pairs, gaps);
    ComputeClientOnlyFieldGaps(reference, gaps);

    std::sort(gaps.begin(), gaps.end(), [](SpecGapRow const& a, SpecGapRow const& b)
    {
        if (a.kind != b.kind)
            return a.kind < b.kind;
        if (a.table != b.table)
            return a.table < b.table;
        return a.column < b.column;
    });

    result.rowsWritten = static_cast<uint32_t>(gaps.size());
    WriteGapsTsv(specDir / "gaps.tsv", gaps);
    return result;
}
