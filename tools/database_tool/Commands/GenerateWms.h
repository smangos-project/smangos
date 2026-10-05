#ifndef DATABASE_TOOL_GENERATEWMS_H
#define DATABASE_TOOL_GENERATEWMS_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class DoltLiteDatabase;

struct WmsColumn
{
    std::string name;
    std::string sqlType;
    bool notNull = false;
    int primaryKeyOrder = 0; // 1-based position in the primary key, 0 when not part of it
    std::string clrType;     // without the nullable marker
    std::string property;
};

struct WmsTable
{
    std::string name;
    std::string record; // C# class name
    std::vector<WmsColumn> columns;
};

// "dbc_areatable" -> "DbcAreatable"; "spawn_flags" -> "SpawnFlags".
std::string WmsPascalCase(std::string const& name);

// The CLR type for an INTEGER column holding values in [minValue, maxValue].
std::string WmsIntegerType(int64_t minValue, int64_t maxValue);

// Every user table of the open world database (no sqlite_/dolt_/wms_ tables), with CLR types
// chosen from the declared column types and, for INTEGER columns, the stored value range.
std::vector<WmsTable> ReadWmsTables(DoltLiteDatabase& world);

// The C# source of one table: its record, its DbSet on WorldDbContext and its table configuration.
std::string RenderWmsTable(WmsTable const& table);

struct WmsGenerateResult
{
    size_t tables = 0;
    std::string error;
};

// Rewrites outputDirectory with one <Record>.cs per table.
WmsGenerateResult GenerateWmsSources(DoltLiteDatabase& world, std::filesystem::path const& outputDirectory);

#endif
