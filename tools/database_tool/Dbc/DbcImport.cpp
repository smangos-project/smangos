#include "DbcImport.h"

#include "Dbc/DbcSchema.h"
#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdbc.h"
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

    void UpsertDbcFilesRow(DoltLiteDatabase& world, DbcTableMeta const& table, char const* format,
                            Wdc5FileInfo const& info, uint32_t rowCount)
    {
        DoltLiteStatement statement = world.Prepare(
            "INSERT OR REPLACE INTO \"dbc_files\" "
            "(\"table_name\", \"format\", \"layout_hash\", \"table_hash\", \"locale\", \"flags\", \"schema\", \"row_count\") "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        statement.BindText(1, table.name);
        statement.BindText(2, format);
        statement.BindInt64(3, static_cast<int64_t>(info.layoutHash));
        statement.BindInt64(4, static_cast<int64_t>(info.tableHash));
        statement.BindInt64(5, static_cast<int64_t>(info.locale));
        statement.BindInt64(6, static_cast<int64_t>(info.flags));
        statement.BindText(7, info.schema);
        statement.BindInt64(8, static_cast<int64_t>(rowCount));
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
                    BindDbcRow(insert, table, row);
            }

            UpsertDbcFilesRow(world, table, "db2", read.info, static_cast<uint32_t>(read.rows.size()));

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

    return result;
}

DbcImportResult ImportDbcFiles(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
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

            std::filesystem::path filePath = dbfilesclientDir / (std::string(table.fileStem) + ".dbc");
            if (!std::filesystem::exists(filePath))
            {
                result.tables.push_back(std::move(tableResult));
                continue;
            }

            tableResult.fileFound = true;

            WdbcReadResult read = ReadWdbcFile(filePath, table);
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
                    BindDbcRow(insert, table, row);
            }

            UpsertDbcFilesRow(world, table, "dbc", Wdc5FileInfo{}, static_cast<uint32_t>(read.rows.size()));

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

    return result;
}
