#include "DbcImport.h"

#include "Dbc/DbcSchema.h"
#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

namespace
{
    std::string ToLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    bool IsSelected(DbcTableMeta const& table, std::vector<std::string> const& onlyTables)
    {
        if (onlyTables.empty())
            return true;
        std::string name = ToLower(table.name);
        return std::any_of(onlyTables.begin(), onlyTables.end(),
            [&](std::string const& t) { return ToLower(t) == name; });
    }

    std::string BuildInsertSql(DbcTableMeta const& table, std::vector<std::string> const& columnNames)
    {
        std::ostringstream sql;
        sql << "INSERT INTO \"" << table.sqlTable << "\" (";
        for (size_t i = 0; i < columnNames.size(); ++i)
            sql << (i ? ", \"" : "\"") << columnNames[i] << "\"";
        sql << ") VALUES (";
        for (size_t i = 0; i < columnNames.size(); ++i)
            sql << (i ? ", ?" : "?");
        sql << ")";
        return sql.str();
    }

    void BindRow(DoltLiteStatement& statement, DbcTableMeta const& table, Wdc5Row const& row)
    {
        size_t column = 0;
        for (uint32_t i = 0; i < table.fieldCount; ++i)
        {
            DbcFieldMeta const& field = table.fields[i];
            for (uint16_t a = 0; a < field.arrayLength; ++a, ++column)
            {
                int index = static_cast<int>(column) + 1;
                Wdc5Value const& value = row[column];
                switch (field.type)
                {
                    case DbcFieldType::Float:
                        statement.BindDouble(index, value.floatValue);
                        break;
                    case DbcFieldType::String:
                    case DbcFieldType::LocString:
                        statement.BindText(index, value.stringValue);
                        break;
                    case DbcFieldType::Int:
                    default:
                        statement.BindInt64(index, value.intValue);
                        break;
                }
            }
        }
        statement.Step();
        statement.Reset();
    }

    void UpsertDbcFilesRow(DoltLiteDatabase& world, DbcTableMeta const& table, Wdc5FileInfo const& info,
                            uint32_t rowCount)
    {
        DoltLiteStatement statement = world.Prepare(
            "INSERT OR REPLACE INTO \"dbc_files\" "
            "(\"table_name\", \"format\", \"layout_hash\", \"table_hash\", \"locale\", \"flags\", \"schema\", \"row_count\") "
            "VALUES (?, 'db2', ?, ?, ?, ?, ?, ?)");
        statement.BindText(1, table.name);
        statement.BindInt64(2, static_cast<int64_t>(info.layoutHash));
        statement.BindInt64(3, static_cast<int64_t>(info.tableHash));
        statement.BindInt64(4, static_cast<int64_t>(info.locale));
        statement.BindInt64(5, static_cast<int64_t>(info.flags));
        statement.BindText(6, info.schema);
        statement.BindInt64(7, static_cast<int64_t>(rowCount));
        statement.Step();
    }
}

DbcImportResult ImportDb2Files(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables, std::span<DbcTableMeta const> tables)
{
    DbcImportResult result;
    uint32_t importedTableCount = 0;

    try
    {
        world.Begin();

        for (DbcTableMeta const& table : tables)
        {
            if (!IsSelected(table, onlyTables))
                continue;

            DbcImportTableResult tableResult;
            tableResult.tableName = table.name;

            std::filesystem::path filePath = dbfilesclientDir / (std::string(table.fileStem) + ".db2");
            if (!std::filesystem::exists(filePath))
            {
                result.tables.push_back(std::move(tableResult));
                continue;
            }

            tableResult.fileFound = true;

            Wdc5ReadResult read = ReadWdc5File(filePath, table);
            if (!read.error.empty())
            {
                tableResult.error = read.error;
                result.tables.push_back(std::move(tableResult));
                continue;
            }

            world.Execute("DROP TABLE IF EXISTS \"" + std::string(table.sqlTable) + "\"");
            world.Execute(BuildDbcTableDdl(table));
            for (std::string const& indexSql : BuildDbcTableIndexDdl(table))
                world.Execute(indexSql);

            std::vector<std::string> columnNames;
            for (uint32_t i = 0; i < table.fieldCount; ++i)
                for (std::string const& name : DbcColumnNames(table.fields[i]))
                    columnNames.push_back(name);

            if (!read.rows.empty())
            {
                DoltLiteStatement insert = world.Prepare(BuildInsertSql(table, columnNames));
                for (Wdc5Row const& row : read.rows)
                    BindRow(insert, table, row);
            }

            UpsertDbcFilesRow(world, table, read.info, static_cast<uint32_t>(read.rows.size()));

            tableResult.rowsImported = static_cast<uint32_t>(read.rows.size());
            result.tables.push_back(std::move(tableResult));
            ++importedTableCount;
        }

        world.Commit();
    }
    catch (std::exception const& e)
    {
        result.error = e.what();
        try { world.Rollback(); } catch (std::exception const&) {}
        return result;
    }

    if (importedTableCount)
    {
        try
        {
            world.DoltCommit("import db2 (" + std::to_string(importedTableCount) + " tables)");
        }
        catch (std::exception const& e)
        {
            std::string_view what = e.what();
            if (what.find("nothing to commit") == std::string_view::npos)
                result.error = e.what();
        }
    }

    return result;
}
