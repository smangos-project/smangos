#include "VersionCollapse.h"

#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace
{
    bool EqualsIgnoreCase(std::string const& a, char const* b)
    {
        return a.size() == std::strlen(b) &&
               std::equal(a.begin(), a.end(), b,
                          [](char x, char y) { return std::tolower(static_cast<unsigned char>(x)) == y; });
    }

    std::string FindIgnoreCase(std::vector<std::string> const& columns, char const* name)
    {
        for (std::string const& column : columns)
            if (EqualsIgnoreCase(column, name))
                return column;
        return {};
    }

    std::string JoinQuoted(std::vector<std::string> const& names, std::string const& separator)
    {
        std::string joined;
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (i)
                joined += separator;
            joined += "\"" + names[i] + "\"";
        }
        return joined;
    }
}

VersionColumnInfo DetectVersionColumn(std::vector<std::string> const& columns)
{
    VersionColumnInfo info;

    std::string patchMin = FindIgnoreCase(columns, "patch_min");
    std::string patchMax = FindIgnoreCase(columns, "patch_max");
    if (!patchMin.empty() && !patchMax.empty())
    {
        info.kind = VersionColumnKind::PatchRange;
        info.minColumn = patchMin;
        info.maxColumn = patchMax;
        return info;
    }

    std::string buildMin = FindIgnoreCase(columns, "build_min");
    std::string buildMax = FindIgnoreCase(columns, "build_max");
    if (!buildMin.empty() && !buildMax.empty())
    {
        info.kind = VersionColumnKind::BuildRange;
        info.minColumn = buildMin;
        info.maxColumn = buildMax;
        return info;
    }

    if (std::string build = FindIgnoreCase(columns, "build"); !build.empty())
    {
        info.kind = VersionColumnKind::Build;
        info.column = build;
        return info;
    }

    if (std::string patch = FindIgnoreCase(columns, "patch"); !patch.empty())
    {
        info.kind = VersionColumnKind::Patch;
        info.column = patch;
        return info;
    }

    return info;
}

std::string BuildCollapseSelect(std::string const& table, std::vector<std::string> const& columns,
                                 std::vector<std::string> const& keyColumns, VersionColumnInfo const& info,
                                 CollapseTarget const& target)
{
    std::string columnList = JoinQuoted(columns, ", ");

    if (info.kind == VersionColumnKind::None)
        return "SELECT " + columnList + " FROM \"" + table + "\"";

    if (info.kind == VersionColumnKind::PatchRange || info.kind == VersionColumnKind::BuildRange)
    {
        std::string version = std::to_string(info.kind == VersionColumnKind::PatchRange ? target.patch : target.build);
        return "SELECT " + columnList + " FROM \"" + table + "\" WHERE \"" + info.minColumn + "\" <= " + version +
               " AND " + version + " <= \"" + info.maxColumn + "\"";
    }

    // Build/Patch: one row per logical key, the greatest qualifying version, ties broken by
    // rowid (SQLStorageImpl.h's LoadProgressive rule -- `t1.col = (SELECT MAX(...) ...)`,
    // expressed here as a window function so a genuine tie still yields exactly one row).
    uint32_t targetValue = info.kind == VersionColumnKind::Build ? target.build : target.patch;
    std::string partitionClause = keyColumns.empty() ? "" : ("PARTITION BY " + JoinQuoted(keyColumns, ", ") + " ");

    return "SELECT " + columnList + " FROM (SELECT " + columnList + ", ROW_NUMBER() OVER (" + partitionClause +
           "ORDER BY \"" + info.column + "\" DESC, rowid DESC) AS __rn FROM \"" + table + "\" WHERE \"" +
           info.column + "\" <= " + std::to_string(targetValue) + ") WHERE __rn = 1";
}

CollapseReport CollapseTableInto(DoltLiteDatabase& db, std::string const& sourceTable, std::string const& destTable,
                                  std::vector<std::string> const& columns, std::vector<std::string> const& keyColumns,
                                  VersionColumnInfo const& info, CollapseTarget const& target)
{
    CollapseReport report;
    report.table = sourceTable;

    DoltLiteStatement rowsBefore = db.Prepare("SELECT COUNT(*) FROM \"" + sourceTable + "\"");
    rowsBefore.Step();
    report.rowsBefore = static_cast<uint64_t>(rowsBefore.ColumnInt64(0));

    if (keyColumns.empty())
    {
        report.distinctKeysBefore = report.rowsBefore;
    }
    else
    {
        DoltLiteStatement distinctKeys = db.Prepare(
            "SELECT COUNT(*) FROM (SELECT DISTINCT " + JoinQuoted(keyColumns, ", ") + " FROM \"" + sourceTable + "\")");
        distinctKeys.Step();
        report.distinctKeysBefore = static_cast<uint64_t>(distinctKeys.ColumnInt64(0));
    }

    std::string select = BuildCollapseSelect(sourceTable, columns, keyColumns, info, target);

    DoltLiteStatement rowsAfter = db.Prepare("SELECT COUNT(*) FROM (" + select + ")");
    rowsAfter.Step();
    report.rowsAfter = static_cast<uint64_t>(rowsAfter.ColumnInt64(0));

    db.Execute("INSERT INTO \"" + destTable + "\" (" + JoinQuoted(columns, ", ") + ") " + select);

    report.keysWithNoRow =
        report.distinctKeysBefore > report.rowsAfter ? report.distinctKeysBefore - report.rowsAfter : 0;

    return report;
}
