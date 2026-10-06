#include "DbcSchema.h"

#include "Dbc/Wdbc.h"
#include "World/DoltLiteDatabase.h"

#include <sstream>
#include <string_view>

namespace
{
    char const* SqlColumnType(DbcFieldType type)
    {
        switch (type)
        {
            case DbcFieldType::Int:
                return "INTEGER NOT NULL DEFAULT 0";
            case DbcFieldType::Float:
                return "REAL NOT NULL DEFAULT 0";
            case DbcFieldType::String:
            case DbcFieldType::LocString:
                return "TEXT NOT NULL DEFAULT ''";
        }
        return "TEXT NOT NULL DEFAULT ''";
    }
}

std::vector<std::string> DbcColumnNames(DbcFieldMeta const& field, uint32_t locStringSlots)
{
    if (field.type == DbcFieldType::LocString && locStringSlots > 1)
    {
        // field.name is already "<Name>_lang" (WoWDBDefs convention); slot 0 keeps it as-is.
        std::vector<std::string> names;
        names.push_back(field.name);
        for (uint32_t i = 1; i <= locStringSlots - 2; ++i)
            names.push_back(std::string(field.name) + "_" + std::to_string(i));
        names.push_back(std::string(field.name) + "_mask");
        return names;
    }

    if (field.arrayLength <= 1)
        return { field.name };

    std::vector<std::string> names;
    for (uint16_t i = 1; i <= field.arrayLength; ++i)
        names.push_back(std::string(field.name) + std::to_string(i));
    return names;
}

std::string BuildDbcTableDdl(DbcTableMeta const& table, std::string const& sqlTableName, uint32_t locStringSlots)
{
    std::string name = sqlTableName.empty() ? std::string(table.sqlTable) : sqlTableName;

    std::ostringstream sql;
    sql << "CREATE TABLE \"" << name << "\" (";

    bool first = true;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
    {
        DbcFieldMeta const& field = table.fields[i];
        for (std::string const& columnName : DbcColumnNames(field, locStringSlots))
        {
            sql << (first ? "" : ", ") << "\"" << columnName << "\" " << SqlColumnType(field.type);
            if (field.isId)
                sql << " PRIMARY KEY";
            first = false;
        }
    }

    sql << ")";
    return sql.str();
}

std::vector<std::string> BuildDbcTableIndexDdl(DbcTableMeta const& table, std::string const& sqlTableName,
                                                uint32_t locStringSlots)
{
    std::string name = sqlTableName.empty() ? std::string(table.sqlTable) : sqlTableName;

    std::vector<std::string> statements;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
    {
        DbcFieldMeta const& field = table.fields[i];
        if (!field.foreignTable)
            continue;

        for (std::string const& columnName : DbcColumnNames(field, locStringSlots))
        {
            std::ostringstream sql;
            sql << "CREATE INDEX \"" << name << "__" << columnName << "\" ON \"" << name << "\" (\"" << columnName
                << "\")";
            statements.push_back(sql.str());
        }
    }
    return statements;
}

void BindDbcRow(DoltLiteStatement& statement, DbcTableMeta const& table, Wdc5Row const& row, uint32_t locStringSlots)
{
    WdbcLayout layout{ locStringSlots };
    size_t column = 0;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
    {
        DbcFieldMeta const& field = table.fields[i];
        uint32_t slotCount = FieldSlotCount(field, layout);
        for (uint32_t a = 0; a < slotCount; ++a, ++column)
        {
            int index = static_cast<int>(column) + 1;
            Wdc5Value const& value = row[column];
            if (IsLocStringMaskSlot(field, layout, a))
                statement.BindInt64(index, value.intValue);
            else switch (field.type)
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

std::string BuildDbcFilesTableDdl()
{
    return "CREATE TABLE \"dbc_files\" ("
           "\"table_name\" TEXT PRIMARY KEY, "
           "\"format\" TEXT, "
           "\"layout_hash\" INTEGER, "
           "\"table_hash\" INTEGER, "
           "\"locale\" INTEGER, "
           "\"flags\" INTEGER, "
           "\"schema\" TEXT, "
           "\"row_count\" INTEGER)";
}

DbcSchemaResult CreateDbcTables(DoltLiteDatabase& world)
{
    DbcSchemaResult result;

    try
    {
        world.Begin();

        world.Execute("DROP TABLE IF EXISTS \"dbc_files\"");
        world.Execute(BuildDbcFilesTableDdl());

        for (DbcTableMeta const& table : GetDbcTables())
        {
            world.Execute("DROP TABLE IF EXISTS \"" + std::string(table.sqlTable) + "\"");
            world.Execute(BuildDbcTableDdl(table));
            for (std::string const& indexSql : BuildDbcTableIndexDdl(table))
                world.Execute(indexSql);
            ++result.tablesCreated;
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
