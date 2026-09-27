#include "DbcSchema.h"

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

std::vector<std::string> DbcColumnNames(DbcFieldMeta const& field)
{
    if (field.arrayLength <= 1)
        return { field.name };

    std::vector<std::string> names;
    for (uint16_t i = 1; i <= field.arrayLength; ++i)
        names.push_back(std::string(field.name) + std::to_string(i));
    return names;
}

std::string BuildDbcTableDdl(DbcTableMeta const& table)
{
    std::ostringstream sql;
    sql << "CREATE TABLE \"" << table.sqlTable << "\" (";

    bool first = true;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
    {
        DbcFieldMeta const& field = table.fields[i];
        for (std::string const& columnName : DbcColumnNames(field))
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

std::vector<std::string> BuildDbcTableIndexDdl(DbcTableMeta const& table)
{
    std::vector<std::string> statements;
    for (uint32_t i = 0; i < table.fieldCount; ++i)
    {
        DbcFieldMeta const& field = table.fields[i];
        if (!field.foreignTable)
            continue;

        for (std::string const& columnName : DbcColumnNames(field))
        {
            std::ostringstream sql;
            sql << "CREATE INDEX \"" << table.sqlTable << "__" << columnName << "\" ON \"" << table.sqlTable
                << "\" (\"" << columnName << "\")";
            statements.push_back(sql.str());
        }
    }
    return statements;
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

DbcSchemaResult CreateDbcTables(DoltLiteDatabase& world, std::string const& message)
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

    try
    {
        world.DoltCommit(message);
    }
    catch (std::exception const& e)
    {
        // Recreating byte-identical dbc_* tables (e.g. an unchanged regenerate) is a no-op as
        // far as dolt's working tree is concerned; that's still a successful call.
        std::string_view what = e.what();
        if (what.find("nothing to commit") == std::string_view::npos)
            result.error = e.what();
    }

    return result;
}
