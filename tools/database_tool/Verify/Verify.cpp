#include "Verify.h"

#include "Dbc/DbcSchema.h"
#include "Dbc/DbcTableMeta.h"
#include "Verify/RowCompare.h"
#include "World/DoltLiteDatabase.h"
#include "World/VersionCollapse.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

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
        std::vector<std::string> fields;
        std::istringstream fieldsIn(line);
        std::string field;
        while (std::getline(fieldsIn, field, '\t'))
            fields.push_back(field);
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

static void ReportCompare(VerifyResult& result, std::string const& check, std::string const& table,
                          RowCompareResult const& compare)
{
    if (compare.missingRows)
        result.problems.push_back({ check, table, std::to_string(compare.missingRows) + " of " +
                                                      std::to_string(compare.expectedRows) +
                                                      " rows missing, first " + compare.firstMissingKey });
    if (compare.extraRows)
        result.problems.push_back({ check, table, std::to_string(compare.extraRows) + " extra rows, first " +
                                                      compare.firstExtraKey });
    for (ColumnDifference const& difference : compare.differences)
        result.problems.push_back({ check, table, difference.column + " differs in " +
                                                      std::to_string(difference.rows) + " rows, first " +
                                                      difference.firstKey + ": original " + difference.expected +
                                                      ", rebuilt " + difference.actual });
}

// Checks a mapping's columns against the original's, then compares the rows. `dropped` are the
// original columns the mapping may leave out.
static void VerifyMappedTable(DoltLiteDatabase& db, VerifyResult& result, std::string const& check,
                              std::string const& originalTable, std::filesystem::path const& mappingPath,
                              std::set<std::string> const& dropped)
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
        if (dropped.count(column))
        {
            result.problems.push_back({ check, originalTable, "column " + column + " is both mapped and dropped" });
            columnsOk = false;
        }
    }
    for (VerifyTableColumn const& column : originalColumns)
    {
        if (std::find(mappingColumns.begin(), mappingColumns.end(), column.name) == mappingColumns.end() &&
            !dropped.count(column.name))
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
        ReportCompare(result, check, originalTable, CompareRowSets(db, expected, mapping.select, keyColumns));
        ++result.tablesCompared;
    }
    catch (std::exception const& e)
    {
        result.problems.push_back({ check, originalTable, e.what() });
    }
}

// V1: every srv_ table is rebuilt exactly from the world database by Verify/srv/<table>.sql.
static void VerifyServerTables(DoltLiteDatabase& db, std::filesystem::path const& verifyDir,
                               VerifyOptions const& options, VerifyResult& result)
{
    std::map<std::string, std::set<std::string>> dropped;
    for (std::vector<std::string> const& row : ReadTsv(verifyDir / "srv" / "dropped.tsv", 3))
        dropped[row[0]].insert(row[1]);

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
        VerifyMappedTable(db, result, "V1", originalTable, mappingPath, dropped[table]);
    }
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
            if (!options.updateDelta && delta != expected)
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

    if (options.updateDelta)
    {
        std::filesystem::create_directories(verifyDir);
        std::ofstream out(deltaPath, std::ios::trunc | std::ios::binary);
        out << "table\tinserted\tchanged\tremoved\n";
        for (auto const& [table, delta] : measured)
            if (known.count(table))
                out << table << "\t" << delta[0] << "\t" << delta[1] << "\t" << delta[2] << "\n";
    }
}

VerifyResult RunVerify(std::filesystem::path const& worldPath, std::filesystem::path const& referencePath,
                       std::filesystem::path const& verifyDir, VerifyOptions const& options)
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

        if (Wants(options, "V1"))
            VerifyServerTables(db, verifyDir, options, result);
        if (Wants(options, "V3"))
            VerifyClientDelta(db, verifyDir, options, result);
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}

VerifyInitResult InitVerifyMappings(std::filesystem::path const& referencePath, std::filesystem::path const& verifyDir)
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
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
    }
    return result;
}
