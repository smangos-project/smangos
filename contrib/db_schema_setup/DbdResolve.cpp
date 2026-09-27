#include "DbdResolve.h"

#include <algorithm>

namespace
{
    bool VersionMatchesBuild(DbdVersionBlock const& version, DbdBuild const& build)
    {
        if (std::any_of(version.builds.begin(), version.builds.end(),
                         [&](DbdBuild const& candidate) { return candidate == build; }))
            return true;

        return std::any_of(version.buildRanges.begin(), version.buildRanges.end(),
                            [&](DbdBuildRange const& range) { return range.Contains(build); });
    }

    DbdTable BuildResolvedTable(DbdFile const& file, std::string const& tableName, DbdVersionBlock const& version)
    {
        DbdTable table;
        table.name = tableName;
        table.matchedBuilds = version.builds;
        table.matchedBuildRanges = version.buildRanges;
        table.layoutHashes = version.layoutHashes;
        table.comment = version.comment;

        table.fields.reserve(version.fields.size());
        for (DbdFieldDefinition const& raw : version.fields)
        {
            DbdColumn const& column = file.columns.at(raw.name);

            DbdResolvedField field;
            field.name = raw.name;
            field.type = column.type;
            field.bits = raw.bits;
            field.isUnsigned = raw.isUnsigned;
            field.arrayLength = raw.arrayLength == 0 ? 1 : raw.arrayLength;
            field.isId = raw.isId;
            field.isRelation = raw.isRelation;
            field.isNonInline = raw.isNonInline;
            field.foreignTable = column.foreignTable;
            field.foreignColumn = column.foreignColumn;
            field.verified = column.verified;
            field.columnComment = column.comment;
            field.fieldComment = raw.comment;

            table.fields.push_back(std::move(field));
        }

        return table;
    }
}

DbdResolveResult ResolveDbdTable(DbdFile const& file, std::string const& tableName,
                                  std::vector<DbdBuild> const& buildsInPriorityOrder)
{
    for (DbdBuild const& build : buildsInPriorityOrder)
    {
        DbdVersionBlock const* match = nullptr;
        int matchCount = 0;

        for (DbdVersionBlock const& version : file.versions)
        {
            if (VersionMatchesBuild(version, build))
            {
                match = &version;
                ++matchCount;
            }
        }

        if (matchCount > 1)
            return { DbdResolveStatus::Ambiguous, std::nullopt };

        if (matchCount == 1)
            return { DbdResolveStatus::Resolved, BuildResolvedTable(file, tableName, *match) };
    }

    return { DbdResolveStatus::NoDefinition, std::nullopt };
}
