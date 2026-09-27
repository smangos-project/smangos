#ifndef DATABASE_TOOL_DBC_DBCTABLEMETA_H
#define DATABASE_TOOL_DBC_DBCTABLEMETA_H

#include <cstdint>
#include <span>
#include <string_view>

enum class DbcFieldType : uint8_t { Int, Float, String, LocString };

struct DbcFieldMeta
{
    char const* name;
    DbcFieldType type;
    uint8_t bits;         // Int only: 8/16/32/64
    bool isSigned;
    uint16_t arrayLength; // 1 when scalar
    bool isId;
    bool isRelation;
    bool isNonInline;     // not stored in the record (id list / parent lookup)
    char const* foreignTable;  // nullptr when none
    char const* foreignColumn;
};

struct DbcTableMeta
{
    char const* name;     // WoWDBDefs name
    char const* fileStem;  // lowercase name, as in dbfilesclient
    char const* sqlTable;  // "dbc_" + lowercase name
    uint32_t const* layoutHashes;
    uint32_t layoutHashCount;
    DbcFieldMeta const* fields;
    uint32_t fieldCount;
};

// Defined in the generated generated/DbcTables.g.cpp.
std::span<DbcTableMeta const> GetDbcTables();
DbcTableMeta const* FindDbcTable(std::string_view name); // case-insensitive
char const* GetWowDbDefsCommit();

#endif
