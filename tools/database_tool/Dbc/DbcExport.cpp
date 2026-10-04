#include "DbcExport.h"

#include "Dbc/Wdbc.h"
#include "Dbc/Wdc5Reader.h"
#include "Dbc/Wdc5Writer.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>

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

    bool LoadFileInfo(DoltLiteDatabase& world, DbcTableMeta const& table, Wdc5FileInfo& info)
    {
        DoltLiteStatement statement = world.Prepare(
            "SELECT \"layout_hash\", \"table_hash\", \"locale\", \"flags\", \"schema\" "
            "FROM \"dbc_files\" WHERE \"table_name\" = ?");
        statement.BindText(1, table.name);
        if (!statement.Step())
            return false;

        info.layoutHash = static_cast<uint32_t>(statement.ColumnInt64(0));
        info.tableHash = static_cast<uint32_t>(statement.ColumnInt64(1));
        info.locale = static_cast<uint32_t>(statement.ColumnInt64(2));
        info.flags = static_cast<uint16_t>(statement.ColumnInt64(3));
        info.schema = statement.ColumnText(4);
        return true;
    }

    // dbc_<name>'s own column order matches BuildDbcTableDdl exactly, so "SELECT *" already
    // yields values in the same flattened order Wdc5Row expects.
    std::vector<Wdc5Row> LoadRows(DoltLiteDatabase& world, DbcTableMeta const& table)
    {
        std::vector<Wdc5Row> rows;
        DoltLiteStatement statement = world.Prepare("SELECT * FROM \"" + std::string(table.sqlTable) + "\"");
        while (statement.Step())
        {
            uint32_t totalColumns = 0;
            for (uint32_t i = 0; i < table.fieldCount; ++i)
                totalColumns += table.fields[i].arrayLength;

            Wdc5Row row(totalColumns);
            uint32_t column = 0;
            for (uint32_t i = 0; i < table.fieldCount; ++i)
            {
                DbcFieldMeta const& field = table.fields[i];
                for (uint16_t a = 0; a < field.arrayLength; ++a, ++column)
                {
                    Wdc5Value& value = row[column];
                    switch (field.type)
                    {
                        case DbcFieldType::Float:
                            value.floatValue = statement.ColumnDouble(static_cast<int>(column));
                            break;
                        case DbcFieldType::String:
                        case DbcFieldType::LocString:
                            value.stringValue = statement.ColumnText(static_cast<int>(column));
                            break;
                        case DbcFieldType::Int:
                        default:
                            value.intValue = statement.ColumnInt64(static_cast<int>(column));
                            break;
                    }
                }
            }
            rows.push_back(std::move(row));
        }
        return rows;
    }

    std::string RowKey(Wdc5Row const& row)
    {
        std::string key;
        for (Wdc5Value const& value : row)
        {
            float const f = static_cast<float>(value.floatValue);
            key.append(reinterpret_cast<char const*>(&value.intValue), sizeof(value.intValue));
            key.append(reinterpret_cast<char const*>(&f), sizeof(f));
            key.append(value.stringValue);
            key.push_back('\0');
        }
        return key;
    }

    bool SameRows(std::vector<Wdc5Row> const& a, std::vector<Wdc5Row> const& b)
    {
        if (a.size() != b.size())
            return false;
        std::vector<std::string> keysA, keysB;
        keysA.reserve(a.size());
        keysB.reserve(b.size());
        for (Wdc5Row const& row : a)
            keysA.push_back(RowKey(row));
        for (Wdc5Row const& row : b)
            keysB.push_back(RowKey(row));
        std::sort(keysA.begin(), keysA.end());
        std::sort(keysB.begin(), keysB.end());
        return keysA == keysB;
    }
}

DbcExportResult ExportDb2Files(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables, std::span<DbcTableMeta const> tables,
                                ExportDb2Options const& options)
{
    DbcExportResult result;

    for (DbcTableMeta const& table : tables)
    {
        if (!IsSelected(table, onlyTables))
            continue;

        DbcExportTableResult tableResult;
        tableResult.tableName = table.name;

        Wdc5FileInfo info;
        if (!LoadFileInfo(world, table, info))
        {
            result.tables.push_back(std::move(tableResult));
            continue;
        }
        tableResult.hasFilesRow = true;

        std::vector<Wdc5Row> rows = LoadRows(world, table);
        std::filesystem::path filePath = dbfilesclientDir / (std::string(table.fileStem) + ".db2");

        std::error_code ec;
        bool const exists = std::filesystem::exists(filePath, ec);
        if (options.onlyChanged && exists)
        {
            Wdc5ReadResult existing = ReadWdc5File(filePath, table);
            if (existing.error.empty() && SameRows(existing.rows, rows))
            {
                tableResult.unchanged = true;
                result.tables.push_back(std::move(tableResult));
                continue;
            }
        }

        if (exists && !options.backupDir.empty())
        {
            std::filesystem::path const backup = options.backupDir / filePath.filename();
            std::filesystem::create_directories(options.backupDir, ec);
            if (!std::filesystem::exists(backup, ec))
                std::filesystem::copy_file(filePath, backup, ec);
            if (ec)
            {
                tableResult.error = "failed to back up " + filePath.string() + ": " + ec.message();
                result.tables.push_back(std::move(tableResult));
                continue;
            }
        }

        Wdc5WriteResult written = WriteWdc5File(table, info, rows);
        if (!written.error.empty())
        {
            tableResult.error = written.error;
            result.tables.push_back(std::move(tableResult));
            continue;
        }

        std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            tableResult.error = "failed to open " + filePath.string() + " for writing";
            result.tables.push_back(std::move(tableResult));
            continue;
        }
        out.write(reinterpret_cast<char const*>(written.bytes.data()), static_cast<std::streamsize>(written.bytes.size()));

        tableResult.rowsExported = static_cast<uint32_t>(rows.size());
        result.tables.push_back(std::move(tableResult));
    }

    return result;
}

DbcExportResult ExportDbcFiles(DoltLiteDatabase& world, std::filesystem::path const& dbfilesclientDir,
                                std::vector<std::string> const& onlyTables, std::span<DbcTableMeta const> tables)
{
    DbcExportResult result;

    for (DbcTableMeta const& table : tables)
    {
        if (!IsSelected(table, onlyTables))
            continue;

        DbcExportTableResult tableResult;
        tableResult.tableName = table.name;

        Wdc5FileInfo unused;
        if (!LoadFileInfo(world, table, unused))
        {
            result.tables.push_back(std::move(tableResult));
            continue;
        }
        tableResult.hasFilesRow = true;

        std::vector<Wdc5Row> rows = LoadRows(world, table);

        WdbcWriteResult written = WriteWdbcFile(table, rows);
        if (!written.error.empty())
        {
            tableResult.error = written.error;
            result.tables.push_back(std::move(tableResult));
            continue;
        }

        std::filesystem::path filePath = dbfilesclientDir / (std::string(table.fileStem) + ".dbc");
        std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
        if (!out)
        {
            tableResult.error = "failed to open " + filePath.string() + " for writing";
            result.tables.push_back(std::move(tableResult));
            continue;
        }
        out.write(reinterpret_cast<char const*>(written.bytes.data()), static_cast<std::streamsize>(written.bytes.size()));

        tableResult.rowsExported = static_cast<uint32_t>(rows.size());
        result.tables.push_back(std::move(tableResult));
    }

    return result;
}
