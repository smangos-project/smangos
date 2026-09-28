#include "SpecInventory.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <map>
#include <set>
#include <tuple>

namespace
{
    bool HasPrefix(std::string const& value, std::string const& prefix)
    {
        return value.compare(0, prefix.size(), prefix) == 0;
    }

    std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& reference, std::string const& prefix)
    {
        std::vector<std::string> names;
        DoltLiteStatement stmt = reference.Prepare(
            "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
        while (stmt.Step())
            names.push_back(stmt.ColumnText(0));
        return names;
    }

    // Ordinal-ordered column names of `table`, as PRAGMA table_info reports them.
    std::vector<std::string> TableColumns(DoltLiteDatabase& reference, std::string const& table)
    {
        std::vector<std::string> columns;
        DoltLiteStatement stmt = reference.Prepare("PRAGMA table_info(\"" + table + "\")");
        while (stmt.Step())
            columns.push_back(stmt.ColumnText(1));
        return columns;
    }

    int SourceRank(std::string const& source)
    {
        if (source == "srv") return 0;
        if (source == "vdbc") return 1;
        return 2; // "new", or anything else hand-added
    }

    // Sorts by (source rank, table, column ordinal in `liveColumns`); a row whose column no
    // longer exists live sorts after its table's live columns, keeping its prior relative order.
    void SortColumnRows(std::vector<SpecColumnRow>& rows, std::map<std::pair<std::string, std::string>, std::vector<std::string>> const& liveColumns)
    {
        auto ordinal = [&](SpecColumnRow const& row) -> size_t
        {
            auto it = liveColumns.find({ row.source, row.table });
            if (it == liveColumns.end())
                return SIZE_MAX;
            auto colIt = std::find(it->second.begin(), it->second.end(), row.column);
            return colIt == it->second.end() ? SIZE_MAX : static_cast<size_t>(colIt - it->second.begin());
        };

        std::stable_sort(rows.begin(), rows.end(), [&](SpecColumnRow const& a, SpecColumnRow const& b)
        {
            if (SourceRank(a.source) != SourceRank(b.source))
                return SourceRank(a.source) < SourceRank(b.source);
            if (a.table != b.table)
                return a.table < b.table;
            return ordinal(a) < ordinal(b);
        });
    }

    void SortDbcColumnRows(std::vector<SpecDbcColumnRow>& rows, std::map<std::string, std::vector<std::string>> const& liveColumns)
    {
        auto ordinal = [&](SpecDbcColumnRow const& row) -> size_t
        {
            auto it = liveColumns.find(row.table);
            if (it == liveColumns.end())
                return SIZE_MAX;
            auto colIt = std::find(it->second.begin(), it->second.end(), row.column);
            return colIt == it->second.end() ? SIZE_MAX : static_cast<size_t>(colIt - it->second.begin());
        };

        std::stable_sort(rows.begin(), rows.end(), [&](SpecDbcColumnRow const& a, SpecDbcColumnRow const& b)
        {
            if (a.table != b.table)
                return a.table < b.table;
            return ordinal(a) < ordinal(b);
        });
    }
}

SpecInitResult UpdateSpecInventory(std::filesystem::path const& referencePath, std::filesystem::path const& specDir)
{
    SpecInitResult result;
    if (!std::filesystem::exists(referencePath))
    {
        result.error = referencePath.string() + " does not exist (run reference first)";
        return result;
    }

    DoltLiteDatabase reference(referencePath.string());

    // --- columns.tsv: every srv_* and vdbc_* column ---

    std::filesystem::path columnsPath = specDir / "columns.tsv";
    std::vector<SpecColumnRow> columns = ReadColumnsTsv(columnsPath);

    std::set<std::tuple<std::string, std::string, std::string>> existingKeys;
    for (SpecColumnRow const& row : columns)
        existingKeys.insert({ row.source, row.table, row.column });

    std::map<std::pair<std::string, std::string>, std::vector<std::string>> liveColumns;
    for (std::string const& source : { std::string("srv"), std::string("vdbc") })
    {
        for (std::string const& qualified : TablesWithPrefix(reference, source))
        {
            std::string table = qualified.substr(source.size() + 1); // strip "<source>_"
            std::vector<std::string> tableColumns = TableColumns(reference, qualified);
            liveColumns[{ source, table }] = tableColumns;

            for (std::string const& column : tableColumns)
            {
                if (existingKeys.count({ source, table, column }))
                    continue;
                columns.push_back({ source, table, column, "", "", "", "", "" });
                ++result.columnsAdded;
            }
        }
    }

    for (SpecColumnRow const& row : columns)
    {
        if (row.source == "new")
            continue; // hand-added, never checked against the reference database
        auto it = liveColumns.find({ row.source, row.table });
        bool stillLive = it != liveColumns.end() &&
            std::find(it->second.begin(), it->second.end(), row.column) != it->second.end();
        if (!stillLive)
            result.missingColumns.push_back(row.source + ":" + row.table + "." + row.column);
    }

    SortColumnRows(columns, liveColumns);
    WriteColumnsTsv(columnsPath, columns);

    // --- dbc-columns.tsv: every cdbc_* column of a table with a same-named vdbc_* table ---

    std::filesystem::path dbcColumnsPath = specDir / "dbc-columns.tsv";
    std::vector<SpecDbcColumnRow> dbcColumns = ReadDbcColumnsTsv(dbcColumnsPath);

    std::set<std::pair<std::string, std::string>> existingDbcKeys;
    for (SpecDbcColumnRow const& row : dbcColumns)
        existingDbcKeys.insert({ row.table, row.column });

    std::set<std::string> vdbcFileStems;
    for (std::string const& qualified : TablesWithPrefix(reference, "vdbc"))
        vdbcFileStems.insert(qualified.substr(5)); // strip "vdbc_"

    std::map<std::string, std::vector<std::string>> liveDbcColumns;
    for (std::string const& qualified : TablesWithPrefix(reference, "cdbc"))
    {
        std::string fileStem = qualified.substr(5); // strip "cdbc_"
        if (!vdbcFileStems.count(fileStem))
            continue; // no vanilla counterpart: can never receive a vanilla row

        std::string dbcTable = "dbc_" + fileStem;
        std::vector<std::string> tableColumns = TableColumns(reference, qualified);
        liveDbcColumns[dbcTable] = tableColumns;

        for (std::string const& column : tableColumns)
        {
            if (existingDbcKeys.count({ dbcTable, column }))
                continue;
            dbcColumns.push_back({ dbcTable, column, "", "" });
            ++result.dbcColumnsAdded;
        }
    }

    for (SpecDbcColumnRow const& row : dbcColumns)
    {
        auto it = liveDbcColumns.find(row.table);
        bool stillLive = it != liveDbcColumns.end() &&
            std::find(it->second.begin(), it->second.end(), row.column) != it->second.end();
        if (!stillLive)
            result.missingDbcColumns.push_back(row.table + "." + row.column);
    }

    SortDbcColumnRows(dbcColumns, liveDbcColumns);
    WriteDbcColumnsTsv(dbcColumnsPath, dbcColumns);

    return result;
}

SpecSetResult SetSpecColumns(std::filesystem::path const& specDir, SpecSetFilter const& filter, SpecSetValues const& values, bool overwrite)
{
    SpecSetResult result;
    std::filesystem::path columnsPath = specDir / "columns.tsv";
    std::vector<SpecColumnRow> columns = ReadColumnsTsv(columnsPath);

    for (SpecColumnRow& row : columns)
    {
        if (row.source != filter.source || row.table != filter.table)
            continue;
        if (!filter.column.empty() && row.column != filter.column)
            continue;
        ++result.matched;

        if (!row.disposition.empty() && !overwrite)
            continue;

        if (values.disposition) row.disposition = *values.disposition;
        if (values.target) row.target = *values.target;
        if (values.rule) row.rule = *values.rule;
        if (values.reason) row.reason = *values.reason;
        if (values.reviewed) row.reviewed = *values.reviewed;
        ++result.updated;
    }

    if (result.matched == 0)
    {
        result.error = "no rows matched source=" + filter.source + " table=" + filter.table +
            (filter.column.empty() ? "" : " column=" + filter.column);
        return result;
    }

    WriteColumnsTsv(columnsPath, columns);
    return result;
}
