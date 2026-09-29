#include "SpecCheck.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

static std::map<std::string, std::set<std::string>> LoadTableColumns(DoltLiteDatabase& reference, std::string const& prefix)
{
    std::map<std::string, std::set<std::string>> tables;
    std::vector<std::string> names;
    DoltLiteStatement list = reference.Prepare(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
    while (list.Step())
        names.push_back(list.ColumnText(0));

    for (std::string const& name : names)
    {
        std::set<std::string>& columns = tables[name.substr(prefix.size() + 1)];
        DoltLiteStatement info = reference.Prepare("PRAGMA table_info(\"" + name + "\")");
        while (info.Step())
            columns.insert(info.ColumnText(1));
    }
    return tables;
}

static bool IsDisposition(std::string const& value)
{
    return value == "dbc" || value == "server" || value == "derived" || value == "constant" || value == "drop";
}

static std::string RowName(SpecColumnRow const& row)
{
    return row.source + ":" + row.table + "." + row.column;
}

// Columns of one source table that differ only by a trailing slot number (spellid_1..5): several
// rows of one target table, written from one row of the source.
static bool IsArraySlots(std::vector<SpecColumnRow const*> const& writers)
{
    auto stem = [](std::string const& column)
    {
        size_t end = column.size();
        while (end > 0 && std::isdigit(static_cast<unsigned char>(column[end - 1])))
            --end;
        if (end > 0 && column[end - 1] == '_')
            --end;
        return column.substr(0, end);
    };

    for (SpecColumnRow const* writer : writers)
    {
        if (writer->source != writers[0]->source || writer->table != writers[0]->table ||
            stem(writer->column) == writer->column || stem(writer->column) != stem(writers[0]->column))
            return false;
    }
    return true;
}

// A tables.tsv list field: "a, b, c".
static std::vector<std::string> SplitList(std::string const& value)
{
    std::vector<std::string> items;
    std::istringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ','))
    {
        size_t const first = item.find_first_not_of(' ');
        if (first != std::string::npos)
            items.push_back(item.substr(first, item.find_last_not_of(' ') - first + 1));
    }
    return items;
}

// `pattern` is a table name, or a prefix ending in '*' (dbc_spell*).
static bool MatchesTable(std::string const& pattern, std::string const& table)
{
    if (!pattern.empty() && pattern.back() == '*')
        return table.compare(0, pattern.size() - 1, pattern, 0, pattern.size() - 1) == 0;
    return pattern == table;
}

static std::string JoinSet(std::set<std::string> const& items)
{
    std::string joined;
    for (std::string const& item : items)
        joined += (joined.empty() ? "" : ", ") + item;
    return joined;
}

SpecCheckResult CheckSpec(std::filesystem::path const& referencePath, std::filesystem::path const& specDir, bool final)
{
    SpecCheckResult result;
    if (!std::filesystem::exists(referencePath))
    {
        result.error = referencePath.string() + " does not exist (run reference first)";
        return result;
    }

    DoltLiteDatabase reference(referencePath.string());
    std::map<std::string, std::map<std::string, std::set<std::string>>> live;
    for (std::string const source : { "srv", "vdbc", "cdbc" })
        live[source] = LoadTableColumns(reference, source);

    std::vector<SpecColumnRow> const columns = ReadColumnsTsv(specDir / "columns.tsv");
    std::vector<SpecDbcColumnRow> const dbcColumns = ReadDbcColumnsTsv(specDir / "dbc-columns.tsv");
    std::vector<SpecEvidenceRow> const evidence = ReadEvidenceTsv(specDir / "evidence.tsv");
    std::vector<SpecRuleRow> const rules = ReadRulesTsv(specDir / "rules.tsv");
    std::vector<SpecGapRow> const gaps = ReadGapsTsv(specDir / "gaps.tsv");
    std::vector<SpecTableRow> const tableRows = ReadTablesTsv(specDir / "tables.tsv");
    std::map<std::string, std::string> overrides;
    for (SpecTableRow const& row : tableRows)
    {
        if (!row.overrides.empty())
            overrides[row.table] = row.overrides;
    }
    result.columnsChecked = static_cast<uint32_t>(columns.size());

    auto report = [&](uint32_t check, std::string message)
    {
        result.problems.push_back({ check, std::move(message) });
    };

    std::set<std::string> ruleNames;
    for (SpecRuleRow const& rule : rules)
        ruleNames.insert(rule.name);

    std::map<std::tuple<std::string, std::string, std::string>, SpecEvidenceRow const*> evidenceByKey;
    for (SpecEvidenceRow const& row : evidence)
        evidenceByKey[{ row.source, row.table, row.column }] = &row;

    // 1. inventory
    std::map<std::tuple<std::string, std::string, std::string>, uint32_t> rowCounts;
    for (SpecColumnRow const& row : columns)
    {
        ++rowCounts[{ row.source, row.table, row.column }];

        if (row.source == "new")
            continue;
        bool real = false;
        if (row.source == "srv" || row.source == "vdbc")
        {
            auto const& tables = live[row.source];
            auto tableIt = tables.find(row.table);
            real = tableIt != tables.end() && tableIt->second.count(row.column) != 0;
        }
        if (!real)
            report(1, RowName(row) + " names no column in the reference database");
    }

    for (auto const& [key, count] : rowCounts)
    {
        if (count > 1)
            report(1, std::get<0>(key) + ":" + std::get<1>(key) + "." + std::get<2>(key) + " has " + std::to_string(count) + " rows");
    }

    for (std::string const source : { "srv", "vdbc" })
    {
        for (auto const& [table, tableColumns] : live[source])
        {
            for (std::string const& column : tableColumns)
            {
                if (!rowCounts.count({ source, table, column }))
                    report(1, source + ":" + table + "." + column + " has no row");
            }
        }
    }

    std::map<std::string, std::vector<SpecColumnRow const*>> dbcWriters; // target -> rows
    std::set<std::string> writtenTables;
    for (SpecColumnRow const& row : columns)
    {
        if (!IsDisposition(row.disposition))
        {
            report(1, RowName(row) + " has no valid disposition (\"" + row.disposition + "\")");
            continue;
        }

        if (final && row.reviewed != "yes")
            report(9, RowName(row) + " is not reviewed");

        if (row.disposition == "dbc")
        {
            // 2. dbc
            size_t const dot = row.target.find('.');
            std::string const targetTable = row.target.substr(0, dot);
            std::string const targetColumn = dot == std::string::npos ? std::string() : row.target.substr(dot + 1);
            if (targetTable.compare(0, 4, "dbc_") != 0 || targetColumn.empty())
            {
                report(2, RowName(row) + " has target \"" + row.target + "\", expected dbc_<table>.<column>");
            }
            else
            {
                auto tableIt = live["cdbc"].find(targetTable.substr(4));
                if (tableIt == live["cdbc"].end() || !tableIt->second.count(targetColumn))
                    report(2, RowName(row) + " targets " + row.target + ", which the client metadata doesn't have");
                dbcWriters[row.target].push_back(&row);
                writtenTables.insert(targetTable);
            }

            if (!ruleNames.count(row.rule))
                report(2, RowName(row) + " uses rule \"" + row.rule + "\", which rules.tsv doesn't define");

            auto evidenceIt = evidenceByKey.find({ row.source, row.table, row.column });
            bool const hasMatch = evidenceIt != evidenceByKey.end() && !evidenceIt->second->matchRate.empty();
            if (!hasMatch && row.reason.empty())
                report(2, RowName(row) + " has no recorded match rate and no reason");
        }
        else if (row.disposition == "server")
        {
            // 3. server
            auto evidenceIt = evidenceByKey.find({ row.source, row.table, row.column });
            bool const hasReader = evidenceIt != evidenceByKey.end() && !evidenceIt->second->readers.empty() &&
                evidenceIt->second->readers != "none";
            if (!hasReader && row.reason.empty())
                report(3, RowName(row) + " has no reader in evidence.tsv and no reason");
        }
        else if (row.disposition == "derived")
        {
            // 4. derived/constant/drop
            if (row.rule.empty() && row.target.empty())
                report(4, RowName(row) + " is derived without a formula");
        }
        else if (row.disposition == "constant")
        {
            if (row.target.empty())
                report(4, RowName(row) + " is constant without a value");
        }
        else if (row.reason.empty())
        {
            report(4, RowName(row) + " is dropped without a reason");
        }
    }

    // 5. one writer per target column; an override table's writer shares a column only with the
    //    overridden table's
    for (auto const& [target, allWriters] : dbcWriters)
    {
        std::vector<SpecColumnRow const*> writers;
        for (SpecColumnRow const* writer : allWriters)
        {
            auto overrideIt = overrides.find(writer->table);
            if (overrideIt == overrides.end())
            {
                writers.push_back(writer);
                continue;
            }
            bool const hasBase = std::any_of(allWriters.begin(), allWriters.end(), [&](SpecColumnRow const* base)
            { return base->source == writer->source && base->table == overrideIt->second && base->rule == writer->rule; });
            if (allWriters.size() > 1 && !hasBase)
                report(5, RowName(*writer) + " overrides " + overrideIt->second + ", which doesn't fill " + target +
                    " with rule " + writer->rule);
        }
        if (writers.size() < 2)
            continue;
        bool priority = writers.size() == 2 && writers[0]->source != writers[1]->source &&
            writers[0]->rule == writers[1]->rule;
        if (priority || IsArraySlots(writers))
            continue;

        std::string names;
        for (SpecColumnRow const* writer : writers)
            names += (names.empty() ? "" : ", ") + RowName(*writer);
        report(5, target + " is filled by " + names);
    }

    // 6. tables.tsv: a row for every srv_ table and every table the spec writes, agreeing with
    //    columns.tsv on kind, key, sources and into
    std::map<std::string, std::set<std::string>> sourcesOf;      // written table -> "srv:x", "vdbc:x"
    std::map<std::string, std::set<std::string>> destinationsOf; // srv table -> other tables it writes
    std::map<std::string, std::set<std::string>> serverColumns;  // server table -> its columns
    std::set<std::string> keptInPlace;
    std::set<std::string> newTables;
    for (SpecColumnRow const& row : columns)
    {
        if (!IsDisposition(row.disposition))
            continue;
        if (row.source == "new")
        {
            newTables.insert(row.table);
            serverColumns[row.table].insert(row.column);
            continue;
        }
        if (row.source == "srv" && row.disposition != "drop")
            serverColumns[row.table].insert(row.column);

        size_t const dot = row.target.find('.');
        std::string destination;
        if (row.disposition == "dbc" || (row.disposition == "server" && !row.target.empty()))
            destination = row.target.substr(0, dot);
        if (row.disposition == "server" && dot != std::string::npos)
            serverColumns[destination].insert(row.target.substr(dot + 1));

        if (row.disposition == "server" && row.source == "srv" && (destination.empty() || destination == row.table))
        {
            keptInPlace.insert(row.table);
            sourcesOf[row.table].insert("srv:" + row.table);
            continue;
        }
        if (row.disposition == "server" && destination.empty())
            report(6, RowName(row) + " is server with no target table");
        if (destination.empty())
            continue;
        sourcesOf[destination].insert(row.source + ":" + row.table);
        if (row.source == "srv")
            destinationsOf[row.table].insert(destination);
    }

    std::map<std::string, SpecTableRow const*> tableByName;
    for (SpecTableRow const& row : tableRows)
    {
        if (!tableByName.emplace(row.table, &row).second)
            report(6, row.table + " has more than one tables.tsv row");
    }

    std::set<std::string> const noTables;
    for (SpecTableRow const& row : tableRows)
    {
        std::string const& name = row.table;
        bool const dbcKind = row.kind == "dbc" || row.kind == "placeholder";
        if (!dbcKind && row.kind != "server" && row.kind != "gone")
        {
            report(6, name + " has kind \"" + row.kind + "\", expected dbc, placeholder, server or gone");
            continue;
        }
        auto const clientIt = name.compare(0, 4, "dbc_") == 0 ? live["cdbc"].find(name.substr(4)) : live["cdbc"].end();
        if (dbcKind != (clientIt != live["cdbc"].end()))
        {
            report(6, name + " is kind " + row.kind + (dbcKind ? " but names no client table" : " but names a client table"));
            continue;
        }

        if (live["srv"].count(name))
        {
            bool const kept = keptInPlace.count(name) != 0;
            if (kept && row.kind == "gone")
                report(6, name + " is gone but keeps columns in place");
            if (!kept && row.kind == "server")
                report(6, name + " is server but keeps no column in place");
        }
        else if (row.kind == "gone")
        {
            report(6, name + " is gone but isn't a srv_ table");
        }

        if (row.kind != "gone")
        {
            std::vector<std::string> const key = SplitList(row.key);
            if (key.empty())
                report(6, name + " has no key");
            auto const columnsIt = serverColumns.find(name);
            std::set<std::string> const& tableColumns = dbcKind ? clientIt->second :
                columnsIt != serverColumns.end() ? columnsIt->second : noTables;
            for (std::string const& column : key)
            {
                if (!tableColumns.count(column))
                    report(6, name + " has key column " + column + ", which the table doesn't have");
            }
        }

        auto const sourcesIt = sourcesOf.find(name);
        std::set<std::string> const& actualSources = sourcesIt != sourcesOf.end() ? sourcesIt->second : noTables;
        std::vector<std::string> const listed = SplitList(row.sources);
        if (row.kind == "placeholder")
        {
            if (!actualSources.empty())
                report(6, "placeholder " + name + " is written by " + JoinSet(actualSources));
            if (listed.empty())
                report(6, "placeholder " + name + " names no server table holding its data");
            for (std::string const& holder : listed)
            {
                auto const holderIt = tableByName.find(holder);
                if (holderIt == tableByName.end() || holderIt->second->kind != "server")
                    report(6, "placeholder " + name + " names " + holder + ", which isn't a server table");
            }
        }
        else
        {
            std::set<std::string> const listedSet(listed.begin(), listed.end());
            std::set<std::string> missing, extra;
            std::set_difference(actualSources.begin(), actualSources.end(), listedSet.begin(), listedSet.end(),
                std::inserter(missing, missing.end()));
            std::set_difference(listedSet.begin(), listedSet.end(), actualSources.begin(), actualSources.end(),
                std::inserter(extra, extra.end()));
            if (!missing.empty())
                report(6, name + " sources lack " + JoinSet(missing));
            if (!extra.empty())
                report(6, name + " sources list " + JoinSet(extra) + ", which write nothing into it");
        }

        auto const destinationsIt = destinationsOf.find(name);
        std::set<std::string> const& destinations = destinationsIt != destinationsOf.end() ? destinationsIt->second : noTables;
        std::vector<std::string> const into = SplitList(row.into);
        for (std::string const& pattern : into)
        {
            if (std::none_of(destinations.begin(), destinations.end(), [&](std::string const& table) { return MatchesTable(pattern, table); }))
                report(6, name + " has into " + pattern + ", which none of its columns write");
        }
        for (std::string const& destination : destinations)
        {
            if (std::none_of(into.begin(), into.end(), [&](std::string const& pattern) { return MatchesTable(pattern, destination); }))
                report(6, name + " writes " + destination + ", which its into doesn't name");
        }
        if (row.kind == "gone" && into.empty() && row.reason.empty())
            report(6, name + " is gone with no into and no reason");

        if (!row.overrides.empty() && !tableByName.count(row.overrides))
            report(6, name + " overrides " + row.overrides + ", which has no tables.tsv row");
    }

    for (auto const& [table, tableColumns] : live["srv"])
    {
        if (!tableByName.count(table))
            report(6, "srv_" + table + " has no tables.tsv row");
    }
    for (auto const& [table, sources] : sourcesOf)
    {
        if (!tableByName.count(table))
            report(6, table + " is written by " + JoinSet(sources) + " and has no tables.tsv row");
    }
    for (std::string const& table : newTables)
    {
        auto const it = tableByName.find(table);
        if (it == tableByName.end() || it->second->kind != "server")
            report(6, table + " has new columns and no server row in tables.tsv");
    }

    // 7. 1.15-only columns of written tables
    for (SpecDbcColumnRow const& row : dbcColumns)
    {
        if (!writtenTables.count(row.table) || dbcWriters.count(row.table + "." + row.column))
            continue;
        if (row.vanillaValue.empty())
            report(7, row.table + "." + row.column + " is 1.15-only and has no vanilla_value");
    }

    // 8. removed vanilla tables and columns
    std::map<std::string, std::vector<SpecColumnRow const*>> vdbcRowsByTable;
    for (SpecColumnRow const& row : columns)
    {
        if (row.source == "vdbc")
            vdbcRowsByTable[row.table].push_back(&row);
    }
    for (SpecGapRow const& gap : gaps)
    {
        if (gap.kind != "removed_table" && gap.kind != "removed_column")
            continue;

        auto tableIt = vdbcRowsByTable.find(gap.table);
        bool covered = tableIt != vdbcRowsByTable.end();
        if (covered && gap.kind == "removed_column")
        {
            covered = std::any_of(tableIt->second.begin(), tableIt->second.end(),
                [&](SpecColumnRow const* row) { return row->column == gap.column && IsDisposition(row->disposition); });
        }
        if (!covered)
            report(8, gap.kind + " " + gap.table + (gap.column.empty() ? "" : "." + gap.column) + " has no disposition");
    }

    return result;
}
