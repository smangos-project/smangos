#include "GapAnalysis.h"

#include "Spec/ColumnMatch.h"
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

            // A LEFT JOIN, not NOT EXISTS: SQLite indexes the join but scans the subquery per row.
            std::string missingClause = "\"" + sourceTable + "\" s LEFT JOIN \"" + cdbcTable + "\" c ON c.\"" + pair.cdbcKey +
                "\" = s.\"" + pair.sourceKey + "\" WHERE c.\"" + pair.cdbcKey + "\" IS NULL";

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

    // One source table's search: the columns 4.3 left uncovered and their rule variants.
    struct GapSearch
    {
        std::string source;
        std::string table;
        KeyGroup const* group = nullptr;
        LoadedTable data;
        int sourceKey = -1;
        std::vector<int> columns;
        std::vector<std::vector<RuleVariant>> variants;
        std::vector<BestColumnMatch> best;
    };

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
            // A constant (or all-null) column "matches" any other same-valued constant column at
            // 100% by coincidence; there is nothing this pass can usefully find for it.
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

        ValueEncoder encoder;
        std::vector<GapSearch> searches;
        for (auto const& [key, group] : groups)
        {
            auto columnsIt = uncoveredColumns.find(key);
            if (columnsIt == uncoveredColumns.end())
                continue;

            std::string sourceTable = (key.first == "vdbc" ? "vdbc_" : "srv_") + key.second;
            if (!TableExists(reference, sourceTable))
                continue;

            GapSearch search;
            search.source = key.first;
            search.table = key.second;
            search.group = &group;
            search.data = LoadTable(reference, sourceTable, encoder);
            search.sourceKey = search.data.ColumnIndex(group.sourceKey);
            if (search.data.rowCount == 0 || search.sourceKey < 0)
                continue;

            for (std::string const& columnName : columnsIt->second)
            {
                int column = search.data.ColumnIndex(columnName);
                if (column < 0)
                    continue;
                search.columns.push_back(column);
            }
            search.variants = ApplyRulesToTable(reference, sourceTable, search.data, search.columns, valueRules, encoder);
            search.best.resize(search.columns.size());
            searches.push_back(std::move(search));
        }

        // Candidates in name order, each loaded once. Every column still meets its candidates in
        // the same order, so ties resolve as they would table by table.
        for (std::string const& qualified : TablesWithPrefix(reference, "cdbc"))
        {
            std::string cdbcTable = qualified.substr(5); // strip "cdbc_"
            std::vector<std::pair<GapSearch*, std::string>> joinsByKey;
            std::vector<ColumnInfo> cdbcColumns;
            for (GapSearch& search : searches)
            {
                if (search.group->pairedCdbcTables.count(cdbcTable))
                    continue; // already a declared pair -- 4.3's job, not this one's
                if (cdbcColumns.empty())
                    cdbcColumns = TableColumnInfos(reference, qualified);
                // One join per candidate table is enough, even if several columns share a key name.
                auto keyIt = std::find_if(cdbcColumns.begin(), cdbcColumns.end(),
                    [&](ColumnInfo const& c) { return search.group->keyNames.count(c.name) != 0; });
                if (keyIt != cdbcColumns.end())
                    joinsByKey.push_back({ &search, keyIt->name });
            }
            if (joinsByKey.empty() || TableRowCount(reference, qualified) == 0)
                continue;

            LoadedTable cdbcData = LoadTable(reference, qualified, encoder);
            for (auto const& [search, keyName] : joinsByKey)
            {
                PreparedJoin join = PrepareJoin(search->data, search->sourceKey, cdbcTable, cdbcData, cdbcData.ColumnIndex(keyName));
                ParallelFor(search->columns.size(), [&](size_t i)
                {
                    ScoreJoin(join, search->variants[i], TypeCategory(search->data.columns[search->columns[i]].type), search->best[i]);
                });
            }
        }

        for (GapSearch const& search : searches)
        {
            int64_t const sourceRowCount = static_cast<int64_t>(search.data.rowCount);
            for (size_t i = 0; i < search.columns.size(); ++i)
            {
                BestColumnMatch const& best = search.best[i];
                // Two gates beyond "found something": most of the source table's ids have to
                // show up in the candidate at all (else it's a coincidental small-integer
                // overlap), and the best rule has to actually agree on most of the joined rows.
                if (best.joined * 2 < sourceRowCount || best.matched * 2 < best.joined)
                    continue;

                std::string detail = "cdbc_" + best.targetTable + "." + best.targetColumn + " via " + best.ruleName + " " +
                    std::to_string(best.matched) + "/" + std::to_string(best.joined) + " (not in pairs.tsv)";
                gaps.push_back({ "counterpart_candidate", search.source + "_" + search.table,
                    search.data.columns[search.columns[i]].name, detail });
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
