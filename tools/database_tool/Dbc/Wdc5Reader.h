#ifndef DATABASE_TOOL_DBC_WDC5READER_H
#define DATABASE_TOOL_DBC_WDC5READER_H

#include "Dbc/DbcTableMeta.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// One decoded column value. Which member is meaningful is decided by the corresponding
// DbcFieldMeta::type (Int -> intValue, Float -> floatValue, String/LocString -> stringValue).
struct Wdc5Value
{
    int64_t intValue = 0;
    double floatValue = 0;
    std::string stringValue;
};

// One row, values in the same flattened order as DbcColumnNames() over table.fields (an array
// field contributes one value per element).
using Wdc5Row = std::vector<Wdc5Value>;

struct Wdc5FileInfo
{
    uint32_t tableHash = 0;
    uint32_t layoutHash = 0;
    uint32_t locale = 0;
    uint16_t flags = 0;
    std::string schema;
};

struct Wdc5ReadResult
{
    Wdc5FileInfo info;
    std::vector<Wdc5Row> rows;
    std::vector<std::string> warnings; // e.g. one per skipped encrypted section
    std::string error;                 // empty on success
};

// Reads a WDC5 (.db2) file at `path` against `table`'s resolved metadata (dbc-formats.md):
// header/section parsing, every column compression type, regular and sparse record layout, the
// id list, the copy table and the parent lookup. A section with an unrecognized TactId (data
// zero-filled/absent) is skipped with a warning; its rows are left out.
Wdc5ReadResult ReadWdc5File(std::filesystem::path const& path, DbcTableMeta const& table);

#endif
