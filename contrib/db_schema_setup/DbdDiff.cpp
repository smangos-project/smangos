#include "DbdDiff.h"
#include "DbdResolve.h"

#include <algorithm>
#include <map>

namespace
{
    std::map<std::string, DbdResolvedField const*> FieldsByName(DbdTable const& table)
    {
        std::map<std::string, DbdResolvedField const*> byName;
        for (DbdResolvedField const& field : table.fields)
            byName.emplace(field.name, &field);
        return byName;
    }
}

std::vector<DbdTableDiff> DiffDbdTables(DbdDefinitions const& definitions,
    std::vector<DbdBuild> const& fromBuilds, std::vector<DbdBuild> const& toBuilds)
{
    std::vector<DbdTableDiff> diffs;

    for (auto const& [name, file] : definitions.tables)
    {
        DbdResolveResult from = ResolveDbdTable(file, name, fromBuilds);
        if (from.status != DbdResolveStatus::Resolved)
            continue;

        DbdResolveResult to = ResolveDbdTable(file, name, toBuilds);
        if (to.status != DbdResolveStatus::Resolved)
        {
            diffs.push_back({ name, true, {} });
            continue;
        }

        std::map<std::string, DbdResolvedField const*> toFields = FieldsByName(*to.table);
        DbdTableDiff diff;
        diff.table = name;

        for (DbdResolvedField const& field : from.table->fields)
        {
            auto it = toFields.find(field.name);
            if (it == toFields.end())
            {
                diff.fields.push_back({ field.name, "removed", "" });
                continue;
            }
            if (it->second->arrayLength != field.arrayLength)
            {
                diff.fields.push_back({ field.name, "array_length",
                    std::to_string(field.arrayLength) + " -> " + std::to_string(it->second->arrayLength) });
            }
            else if (it->second->type != field.type)
            {
                diff.fields.push_back({ field.name, "type", field.type + " -> " + it->second->type });
            }
        }

        if (!diff.fields.empty())
            diffs.push_back(std::move(diff));
    }

    std::sort(diffs.begin(), diffs.end(), [](DbdTableDiff const& a, DbdTableDiff const& b) { return a.table < b.table; });
    return diffs;
}
