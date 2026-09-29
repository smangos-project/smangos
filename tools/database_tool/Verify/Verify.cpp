#include "Verify.h"

#include "Dbc/DbcSchema.h"
#include "Dbc/DbcTableMeta.h"
#include "Spec/SpecFile.h"
#include "Verify/RowCompare.h"
#include "World/DoltLiteDatabase.h"
#include "World/VersionCollapse.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

static std::string Trim(std::string const& value)
{
    size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};
    size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

static std::string Quote(std::string const& name)
{
    return "\"" + name + "\"";
}

static std::vector<std::string> SplitTrimmed(std::string const& value, char separator)
{
    std::vector<std::string> parts;
    std::string current;
    std::istringstream in(value);
    while (std::getline(in, current, separator))
        if (std::string part = Trim(current); !part.empty())
            parts.push_back(part);
    return parts;
}

VerifyMapping ReadVerifyMapping(std::filesystem::path const& path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("cannot read " + path.string());

    VerifyMapping mapping;
    std::string line;
    while (std::getline(in, line))
    {
        std::string trimmed = Trim(line);
        if (trimmed.rfind("--", 0) == 0)
        {
            std::string comment = Trim(trimmed.substr(2));
            if (comment.rfind("key:", 0) == 0)
                mapping.key = SplitTrimmed(comment.substr(4), ',');
            continue;
        }
        if (!trimmed.empty())
            mapping.select += line + "\n";
    }

    mapping.select = Trim(mapping.select);
    if (!mapping.select.empty() && mapping.select.back() == ';')
        mapping.select.pop_back();
    return mapping;
}

void WriteVerifyMapping(std::filesystem::path const& path, std::string const& header, VerifyMapping const& mapping)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::trunc | std::ios::binary);
    if (!header.empty())
        out << "-- " << header << "\n";
    out << "-- key:";
    for (size_t i = 0; i < mapping.key.size(); ++i)
        out << (i ? ", " : " ") << mapping.key[i];
    out << "\n" << mapping.select << "\n";
}

struct VerifyTableColumn
{
    std::string name;
    int primaryKeyIndex = 0;
};

static std::vector<VerifyTableColumn> ReadTableColumns(DoltLiteDatabase& db, std::string const& schema,
                                                       std::string const& table)
{
    std::vector<VerifyTableColumn> columns;
    DoltLiteStatement info = db.Prepare("PRAGMA " + schema + ".table_info(" + Quote(table) + ")");
    while (info.Step())
        columns.push_back({ info.ColumnText(1), static_cast<int>(info.ColumnInt64(5)) });
    return columns;
}

static std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& db, std::string const& schema,
                                                 std::string const& prefix)
{
    std::vector<std::string> tables;
    DoltLiteStatement stmt = db.Prepare("SELECT name FROM " + schema +
                                        ".sqlite_master WHERE type = 'table' AND substr(name, 1, " +
                                        std::to_string(prefix.size()) + ") = ? ORDER BY name");
    stmt.BindText(1, prefix);
    while (stmt.Step())
        tables.push_back(stmt.ColumnText(0));
    return tables;
}

// Tab-separated, one header line, rows of at least `columns` fields.
static std::vector<std::vector<std::string>> ReadTsv(std::filesystem::path const& path, size_t columns)
{
    std::vector<std::vector<std::string>> rows;
    std::ifstream in(path);
    std::string line;
    if (!in || !std::getline(in, line))
        return rows;
    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;
        std::vector<std::string> fields(1);
        for (char c : line)
        {
            if (c == '\t')
                fields.emplace_back();
            else
                fields.back() += c;
        }
        if (fields.size() < columns)
            throw std::runtime_error(path.string() + ": row with " + std::to_string(fields.size()) + " fields");
        rows.push_back(std::move(fields));
    }
    return rows;
}

static bool Wants(VerifyOptions const& options, std::string const& check)
{
    return options.checks.empty() ||
           std::find(options.checks.begin(), options.checks.end(), check) != options.checks.end();
}

static bool WantsTable(VerifyOptions const& options, std::string const& table)
{
    return options.tables.empty() ||
           std::find(options.tables.begin(), options.tables.end(), table) != options.tables.end();
}

// What a table's rebuild may differ in: original columns the mapping may leave out (`drop` in
// columns.tsv), exact differing-row counts per column ("(missing)" for missing rows) from
// Verify/vdbc/allow.tsv, and whether rows only the world has are fine.
struct VerifyTableRules
{
    std::set<std::string> dropped;
    std::map<std::string, uint64_t> allowed;
    bool allowExtraRows = false;
};

static void ReportDifference(VerifyResult& result, std::string const& check, std::string const& table,
                             VerifyTableRules const& rules, std::set<std::string>& allowedSeen,
                             std::string const& column, uint64_t rows, std::string const& message)
{
    auto allowed = rules.allowed.find(column);
    if (allowed != rules.allowed.end())
    {
        allowedSeen.insert(column);
        if (allowed->second == rows)
            return;
        result.problems.push_back({ check, table, message + " (allow.tsv allows " + std::to_string(allowed->second) + ")" });
        return;
    }
    result.problems.push_back({ check, table, message });
}

static void ReportCompare(VerifyResult& result, std::string const& check, std::string const& table,
                          VerifyTableRules const& rules, RowCompareResult const& compare)
{
    std::set<std::string> allowedSeen;
    if (compare.missingRows)
        ReportDifference(result, check, table, rules, allowedSeen, "(missing)", compare.missingRows,
                         std::to_string(compare.missingRows) + " of " + std::to_string(compare.expectedRows) +
                             " rows missing, first " + compare.firstMissingKey);
    if (compare.extraRows && !rules.allowExtraRows)
        result.problems.push_back({ check, table, std::to_string(compare.extraRows) + " extra rows, first " +
                                                      compare.firstExtraKey });
    for (ColumnDifference const& difference : compare.differences)
        ReportDifference(result, check, table, rules, allowedSeen, difference.column, difference.rows,
                         difference.column + " differs in " + std::to_string(difference.rows) + " rows, first " +
                             difference.firstKey + ": original " + difference.expected + ", rebuilt " +
                             difference.actual);
    for (auto const& [column, rows] : rules.allowed)
        if (!allowedSeen.count(column))
            result.problems.push_back({ check, table, "allow.tsv allows " + std::to_string(rows) + " rows in " + column +
                                                          ", none differ" });
}

// Checks a mapping's columns against the original's, then compares the rows.
static void VerifyMappedTable(DoltLiteDatabase& db, VerifyResult& result, std::string const& check,
                              std::string const& originalTable, std::filesystem::path const& mappingPath,
                              VerifyTableRules const& rules)
{
    VerifyMapping mapping;
    std::vector<std::string> mappingColumns;
    try
    {
        mapping = ReadVerifyMapping(mappingPath);
        DoltLiteStatement probe = db.Prepare(mapping.select);
        for (int i = 0; i < probe.ColumnCount(); ++i)
            mappingColumns.push_back(probe.ColumnName(i));
    }
    catch (std::exception const& e)
    {
        result.problems.push_back({ check, originalTable, mappingPath.filename().string() + ": " + e.what() });
        return;
    }

    std::vector<VerifyTableColumn> originalColumns = ReadTableColumns(db, "ref", originalTable);
    size_t next = 0;
    bool columnsOk = true;
    for (std::string const& column : mappingColumns)
    {
        auto it = std::find_if(originalColumns.begin(), originalColumns.end(),
                               [&](VerifyTableColumn const& c) { return c.name == column; });
        if (it == originalColumns.end())
        {
            result.problems.push_back({ check, originalTable, "mapping column " + column + " is not in the original" });
            columnsOk = false;
            continue;
        }
        size_t index = static_cast<size_t>(it - originalColumns.begin());
        if (index < next)
        {
            result.problems.push_back({ check, originalTable, "mapping column " + column + " is out of order" });
            columnsOk = false;
        }
        next = index + 1;
    }
    for (VerifyTableColumn const& column : originalColumns)
    {
        if (std::find(mappingColumns.begin(), mappingColumns.end(), column.name) == mappingColumns.end() &&
            !rules.dropped.count(column.name))
        {
            result.problems.push_back({ check, originalTable, "column " + column.name + " is neither mapped nor dropped" });
            columnsOk = false;
        }
    }

    std::vector<int> keyColumns;
    for (std::string const& key : mapping.key)
    {
        auto it = std::find(mappingColumns.begin(), mappingColumns.end(), key);
        if (it == mappingColumns.end())
        {
            result.problems.push_back({ check, originalTable, "key column " + key + " is not in the mapping" });
            columnsOk = false;
            continue;
        }
        keyColumns.push_back(static_cast<int>(it - mappingColumns.begin()));
    }
    if (!columnsOk)
        return;

    std::string expected = "SELECT ";
    for (size_t i = 0; i < mappingColumns.size(); ++i)
        expected += (i ? ", " : "") + Quote(mappingColumns[i]);
    expected += " FROM ref." + Quote(originalTable);

    try
    {
        ReportCompare(result, check, originalTable, rules, CompareRowSets(db, expected, mapping.select, keyColumns));
        ++result.tablesCompared;
    }
    catch (std::exception const& e)
    {
        result.problems.push_back({ check, originalTable, e.what() });
    }
}

// Per (source, table): the columns columns.tsv marks `drop`, and every column it lists.
struct VerifySpecColumns
{
    std::map<std::string, std::set<std::string>> dropped;
    std::map<std::string, std::vector<SpecColumnRow>> rows;
};

static VerifySpecColumns ReadSpecColumns(std::filesystem::path const& specDir)
{
    VerifySpecColumns spec;
    for (SpecColumnRow& row : ReadColumnsTsv(specDir / "columns.tsv"))
    {
        std::string key = row.source + ":" + row.table;
        if (row.disposition == "drop")
            spec.dropped[key].insert(row.column);
        spec.rows[key].push_back(std::move(row));
    }
    return spec;
}

// V1: every srv_ table is rebuilt exactly from the world database by Verify/srv/<table>.sql.
static void VerifyServerTables(DoltLiteDatabase& db, std::filesystem::path const& verifyDir,
                               VerifySpecColumns const& spec, VerifyOptions const& options, VerifyResult& result)
{
    for (std::string const& originalTable : TablesWithPrefix(db, "ref", "srv_"))
    {
        std::string table = originalTable.substr(4);
        if (!WantsTable(options, originalTable) && !WantsTable(options, table))
            continue;

        std::filesystem::path mappingPath = verifyDir / "srv" / (table + ".sql");
        if (!std::filesystem::exists(mappingPath))
        {
            result.problems.push_back({ "V1", originalTable, "no mapping file srv/" + table + ".sql" });
            continue;
        }

        VerifyTableRules rules;
        if (auto it = spec.dropped.find("srv:" + table); it != spec.dropped.end())
            rules.dropped = it->second;
        VerifyMappedTable(db, result, "V1", originalTable, mappingPath, rules);
    }
}

// V2: every vdbc_ table with data the finished database keeps is rebuilt by
// Verify/vdbc/<Name>.sql, up to the differences in Verify/vdbc/allow.tsv. The world may have rows
// the 1.12 files don't. A table whose columns are all `drop` needs no mapping.
static void VerifyVanillaTables(DoltLiteDatabase& db, std::filesystem::path const& verifyDir,
                                VerifySpecColumns const& spec, VerifyOptions const& options, VerifyResult& result)
{
    std::map<std::string, std::map<std::string, uint64_t>> allowed;
    for (std::vector<std::string> const& row : ReadTsv(verifyDir / "vdbc" / "allow.tsv", 4))
        allowed[row[0]][row[1]] = std::stoull(row[2]);

    std::vector<std::string> unmapped;
    for (std::string const& originalTable : TablesWithPrefix(db, "ref", "vdbc_"))
    {
        std::string table = originalTable.substr(5);
        if (!WantsTable(options, originalTable) && !WantsTable(options, table))
            continue;

        DbcTableMeta const* meta = FindVanillaDbcTable(table);
        std::string name = meta ? meta->name : table;
        std::filesystem::path mappingPath = verifyDir / "vdbc" / (name + ".sql");

        VerifyTableRules rules;
        rules.allowExtraRows = true;
        if (auto it = spec.dropped.find("vdbc:" + table); it != spec.dropped.end())
            rules.dropped = it->second;
        if (auto it = allowed.find(name); it != allowed.end())
            rules.allowed = it->second;

        if (std::filesystem::exists(mappingPath))
        {
            VerifyMappedTable(db, result, "V2", originalTable, mappingPath, rules);
            continue;
        }

        auto columns = spec.rows.find("vdbc:" + table);
        bool keepsData = columns == spec.rows.end() ||
                         std::any_of(columns->second.begin(), columns->second.end(),
                                     [](SpecColumnRow const& row) { return row.disposition != "drop"; });
        if (!keepsData)
            continue;
        if (options.final)
            result.problems.push_back({ "V2", originalTable, "no mapping file vdbc/" + name + ".sql" });
        else
            unmapped.push_back(name);
    }

    if (!unmapped.empty())
    {
        std::string note = "V2: " + std::to_string(unmapped.size()) + " vanilla tables not mapped yet:";
        for (std::string const& name : unmapped)
            note += " " + name;
        result.notes.push_back(note);
    }
}

// The mapping columns.tsv implies for a vanilla table whose kept columns all come from one dbc_
// table through a rule with a scalar inverse, or are constants. Empty when there is none.
static std::string GenerateVanillaMapping(std::string const& table, std::vector<SpecColumnRow> const& columns,
                                          std::map<std::string, SpecRuleRow> const& rules, std::string& reason)
{
    std::string dbcTable;
    std::string select;
    for (SpecColumnRow const& column : columns)
    {
        std::string expression;
        if (column.disposition == "drop")
            continue;
        if (column.disposition == "constant")
            expression = column.target;
        else if (column.disposition == "dbc")
        {
            size_t dot = column.target.find('.');
            auto rule = rules.find(column.rule.empty() ? "identity" : column.rule);
            if (dot == std::string::npos || rule == rules.end() || rule->second.kind != "value" ||
                rule->second.inverse.empty())
            {
                reason = column.column + ": no scalar inverse for rule " + column.rule;
                return {};
            }
            std::string target = column.target.substr(0, dot);
            if (!dbcTable.empty() && target != dbcTable)
            {
                reason = column.column + ": a second target table " + target;
                return {};
            }
            dbcTable = target;

            expression = rule->second.inverse;
            std::string value = "d." + Quote(column.target.substr(dot + 1));
            for (size_t at = expression.find("{v}"); at != std::string::npos; at = expression.find("{v}", at + value.size()))
                expression.replace(at, 3, value);
        }
        else
        {
            reason = column.column + ": disposition " + column.disposition;
            return {};
        }

        select += (select.empty() ? "SELECT " : ",\n       ") + expression + " AS " + Quote(column.column);
    }

    if (dbcTable.empty())
    {
        reason = "no dbc column";
        return {};
    }
    return select + "\nFROM " + Quote(dbcTable) + " d";
}

typedef std::array<uint64_t, 3> VerifyDelta; // inserted, changed, removed

// V3: every row of a dbc_ table that differs from the stock client (cdbc_) is counted in
// Verify/client-delta.tsv. That file is the hotfix manifest.
static void VerifyClientDelta(DoltLiteDatabase& db, std::filesystem::path const& verifyDir,
                              VerifyOptions const& options, VerifyResult& result)
{
    std::filesystem::path deltaPath = verifyDir / "client-delta.tsv";
    std::map<std::string, VerifyDelta> recorded;
    for (std::vector<std::string> const& row : ReadTsv(deltaPath, 4))
        recorded[row[0]] = { std::stoull(row[1]), std::stoull(row[2]), std::stoull(row[3]) };

    std::vector<std::string> clientTables = TablesWithPrefix(db, "ref", "cdbc_");
    std::map<std::string, VerifyDelta> measured = recorded;
    std::set<std::string> known;
    uint64_t clientRows = 0;

    for (DbcTableMeta const& table : GetDbcTables())
    {
        std::string dbcTable = table.sqlTable;
        known.insert(dbcTable);
        if (!WantsTable(options, dbcTable))
            continue;

        std::string clientTable = "cdbc_" + std::string(table.fileStem);
        bool hasClientRows = std::binary_search(clientTables.begin(), clientTables.end(), clientTable);

        std::vector<int> keyColumns;
        int column = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            if (table.fields[i].isId)
                keyColumns.push_back(column);
            column += static_cast<int>(DbcColumnNames(table.fields[i]).size());
        }

        try
        {
            std::string original = hasClientRows ? "SELECT * FROM ref." + Quote(clientTable)
                                                 : "SELECT * FROM main." + Quote(dbcTable) + " WHERE 0";
            RowCompareResult compare =
                CompareRowSets(db, original, "SELECT * FROM main." + Quote(dbcTable), keyColumns);
            ++result.tablesCompared;
            clientRows += compare.expectedRows;

            VerifyDelta delta = { compare.extraRows, compare.changedRows, compare.missingRows };
            if (delta == VerifyDelta{})
                measured.erase(dbcTable);
            else
                measured[dbcTable] = delta;

            VerifyDelta expected = recorded.count(dbcTable) ? recorded[dbcTable] : VerifyDelta{};
            if (!options.update && delta != expected)
                result.problems.push_back({ "V3", dbcTable,
                    std::to_string(delta[0]) + " inserted, " + std::to_string(delta[1]) + " changed, " +
                    std::to_string(delta[2]) + " removed; client-delta.tsv says " + std::to_string(expected[0]) +
                    ", " + std::to_string(expected[1]) + ", " + std::to_string(expected[2]) });
        }
        catch (std::exception const& e)
        {
            result.problems.push_back({ "V3", dbcTable, e.what() });
        }
    }

    result.notes.push_back("V3: " + std::to_string(clientRows) + " client rows compared");

    for (auto const& [table, delta] : recorded)
        if (!known.count(table))
            result.problems.push_back({ "V3", table, "client-delta.tsv names a table the client doesn't have" });

    if (options.update)
    {
        std::filesystem::create_directories(verifyDir);
        std::ofstream out(deltaPath, std::ios::trunc | std::ios::binary);
        out << "table\tinserted\tchanged\tremoved\n";
        for (auto const& [table, delta] : measured)
            if (known.count(table))
                out << table << "\t" << delta[0] << "\t" << delta[1] << "\t" << delta[2] << "\n";
    }
}

// One row of Verify/references.tsv: a column that points into a dbc_ table, and how many of its
// non-zero values don't resolve. A dbc_ foreign key from the client metadata needs a row only
// when something dangles; a server reference is declared by its row.
struct VerifyReference
{
    std::string table;
    std::string column;
    std::string target; // "dbc_x.Column"
    std::string where;  // optional condition on `table`'s rows
    uint64_t dangling = 0;
    std::string reason;
};

static uint64_t CountDangling(DoltLiteDatabase& db, VerifyReference const& reference)
{
    size_t dot = reference.target.find('.');
    if (dot == std::string::npos)
        throw std::runtime_error("target " + reference.target + " is not table.column");
    std::string sql = "SELECT COUNT(*) FROM main." + Quote(reference.table) + " WHERE " + Quote(reference.column) +
                      " <> 0 AND " + Quote(reference.column) + " NOT IN (SELECT " +
                      Quote(reference.target.substr(dot + 1)) + " FROM main." +
                      Quote(reference.target.substr(0, dot)) + ")";
    if (!reference.where.empty())
        sql += " AND (" + reference.where + ")";
    DoltLiteStatement count = db.Prepare(sql);
    count.Step();
    return static_cast<uint64_t>(count.ColumnInt64(0));
}

// V4: every client-metadata foreign key and every declared server reference resolves, except for
// the dangling counts Verify/references.tsv lists.
static void VerifyReferences(DoltLiteDatabase& db, std::filesystem::path const& verifyDir,
                             VerifyOptions const& options, VerifyResult& result)
{
    std::filesystem::path referencesPath = verifyDir / "references.tsv";
    std::map<std::pair<std::string, std::string>, VerifyReference> listed;
    for (std::vector<std::string> const& row : ReadTsv(referencesPath, 6))
        listed[{ row[0], row[1] }] = { row[0], row[1], row[2], row[3], std::stoull(row[4]), row[5] };

    std::vector<VerifyReference> references;
    std::set<std::pair<std::string, std::string>> metadataKeys;
    uint32_t unresolvedTargets = 0;
    for (DbcTableMeta const& table : GetDbcTables())
    {
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            DbcFieldMeta const& field = table.fields[i];
            if (!field.foreignTable || field.isNonInline)
                continue;
            DbcTableMeta const* target = FindDbcTable(field.foreignTable);
            if (!target)
            {
                ++unresolvedTargets;
                continue;
            }
            for (std::string const& column : DbcColumnNames(field))
            {
                VerifyReference reference{ table.sqlTable, column,
                                           std::string(target->sqlTable) + "." + field.foreignColumn };
                metadataKeys.insert({ reference.table, reference.column });
                references.push_back(std::move(reference));
            }
        }
    }
    for (auto const& [key, reference] : listed)
        if (!metadataKeys.count(key))
            references.push_back(reference);

    std::vector<VerifyReference> measured;
    uint32_t checked = 0;
    for (VerifyReference& reference : references)
    {
        auto it = listed.find({ reference.table, reference.column });
        if (it != listed.end())
        {
            reference.where = it->second.where;
            reference.reason = it->second.reason;
        }
        bool declared = !metadataKeys.count({ reference.table, reference.column });

        if (!WantsTable(options, reference.table))
        {
            if (it != listed.end())
                measured.push_back(it->second);
            continue;
        }

        try
        {
            reference.dangling = CountDangling(db, reference);
            ++checked;
        }
        catch (std::exception const& e)
        {
            result.problems.push_back({ "V4", reference.table, reference.column + ": " + e.what() });
            if (it != listed.end())
                measured.push_back(it->second);
            continue;
        }

        uint64_t expected = it != listed.end() ? it->second.dangling : 0;
        if (!options.update && reference.dangling != expected)
            result.problems.push_back({ "V4", reference.table, reference.column + " -> " + reference.target + ": " +
                                                                   std::to_string(reference.dangling) +
                                                                   " values don't resolve; references.tsv says " +
                                                                   std::to_string(expected) });
        if (reference.dangling || declared)
            measured.push_back(reference);
    }

    result.notes.push_back("V4: " + std::to_string(checked) + " references checked, " +
                           std::to_string(unresolvedTargets) + " metadata foreign keys name a table the client lacks");

    if (options.update)
    {
        std::sort(measured.begin(), measured.end(), [](VerifyReference const& a, VerifyReference const& b)
                  { return std::tie(a.table, a.column) < std::tie(b.table, b.column); });
        std::filesystem::create_directories(verifyDir);
        std::ofstream out(referencesPath, std::ios::trunc | std::ios::binary);
        out << "table\tcolumn\ttarget\twhere\tdangling\treason\n";
        for (VerifyReference const& reference : measured)
            out << reference.table << "\t" << reference.column << "\t" << reference.target << "\t" << reference.where
                << "\t" << reference.dangling << "\t" << reference.reason << "\n";
    }
}

VerifyResult RunVerify(std::filesystem::path const& worldPath, std::filesystem::path const& referencePath,
                       std::filesystem::path const& specDir, std::filesystem::path const& verifyDir,
                       VerifyOptions const& options)
{
    VerifyResult result;
    for (std::filesystem::path const& path : { worldPath, referencePath })
    {
        if (!std::filesystem::exists(path))
        {
            result.error = path.string() + " does not exist";
            return result;
        }
    }

    try
    {
        DoltLiteDatabase db(worldPath.string());
        db.Execute("ATTACH DATABASE '" + referencePath.string() + "' AS ref");

        VerifySpecColumns spec = ReadSpecColumns(specDir);
        if (Wants(options, "V1"))
            VerifyServerTables(db, verifyDir, spec, options, result);
        if (Wants(options, "V2"))
            VerifyVanillaTables(db, verifyDir, spec, options, result);
        if (Wants(options, "V3"))
            VerifyClientDelta(db, verifyDir, options, result);
        if (Wants(options, "V4"))
            VerifyReferences(db, verifyDir, options, result);
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}

static void InitVanillaMappings(DoltLiteDatabase& db, std::filesystem::path const& specDir,
                                std::filesystem::path const& verifyDir, std::vector<std::string> const& vanillaTables,
                                VerifyInitResult& result)
{
    VerifySpecColumns spec = ReadSpecColumns(specDir);
    std::map<std::string, SpecRuleRow> rules;
    for (SpecRuleRow& rule : ReadRulesTsv(specDir / "rules.tsv"))
        rules[rule.name] = std::move(rule);

    bool all = std::find(vanillaTables.begin(), vanillaTables.end(), "all") != vanillaTables.end();
    for (std::string const& originalTable : TablesWithPrefix(db, "main", "vdbc_"))
    {
        DbcTableMeta const* meta = FindVanillaDbcTable(originalTable.substr(5));
        auto columns = spec.rows.find("vdbc:" + originalTable.substr(5));
        if (!meta || columns == spec.rows.end())
            continue;
        std::string name = meta->name;
        if (!all && std::find(vanillaTables.begin(), vanillaTables.end(), name) == vanillaTables.end())
            continue;
        std::filesystem::path path = verifyDir / "vdbc" / (name + ".sql");
        if (std::filesystem::exists(path))
            continue;

        std::string reason;
        VerifyMapping mapping;
        mapping.select = GenerateVanillaMapping(originalTable, columns->second, rules, reason);
        if (mapping.select.empty())
        {
            if (!all || reason != "no dbc column")
                result.skipped.push_back(name + ": " + reason);
            continue;
        }
        for (uint32_t i = 0; i < meta->fieldCount; ++i)
            if (meta->fields[i].isId)
                mapping.key.push_back(meta->fields[i].name);

        WriteVerifyMapping(path, {}, mapping);
        ++result.written;
    }
}

VerifyInitResult InitVerifyMappings(std::filesystem::path const& referencePath, std::filesystem::path const& specDir,
                                    std::filesystem::path const& verifyDir,
                                    std::vector<std::string> const& vanillaTables)
{
    VerifyInitResult result;
    try
    {
        DoltLiteDatabase db(referencePath.string());
        for (std::string const& originalTable : TablesWithPrefix(db, "main", "srv_"))
        {
            std::string table = originalTable.substr(4);
            std::filesystem::path path = verifyDir / "srv" / (table + ".sql");
            if (std::filesystem::exists(path))
                continue;

            std::vector<VerifyTableColumn> columns = ReadTableColumns(db, "main", originalTable);
            std::vector<std::string> names;
            std::vector<std::pair<int, std::string>> primaryKey;
            for (VerifyTableColumn const& column : columns)
            {
                names.push_back(column.name);
                if (column.primaryKeyIndex > 0)
                    primaryKey.emplace_back(column.primaryKeyIndex, column.name);
            }
            std::sort(primaryKey.begin(), primaryKey.end());

            VersionColumnInfo version = DetectVersionColumn(names);
            VerifyMapping mapping;
            for (auto const& [index, name] : primaryKey)
                if (name != version.column && name != version.minColumn && name != version.maxColumn)
                    mapping.key.push_back(name);

            if (version.kind == VersionColumnKind::None)
            {
                mapping.select = "SELECT ";
                for (size_t i = 0; i < names.size(); ++i)
                    mapping.select += (i ? ",\n       " : "") + Quote(names[i]);
                mapping.select += "\nFROM " + Quote(table);
            }
            else
            {
                mapping.select = BuildCollapseSelect(table, names, mapping.key, version);
            }

            WriteVerifyMapping(path, {}, mapping);
            ++result.written;
        }

        if (!vanillaTables.empty())
            InitVanillaMappings(db, specDir, verifyDir, vanillaTables, result);
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}
