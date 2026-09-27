#ifndef DATABASE_TOOL_DBC_WDBC_H
#define DATABASE_TOOL_DBC_WDBC_H

#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h" // reuses Wdc5Row/Wdc5Value's shape

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct WdbcReadResult
{
    std::vector<Wdc5Row> rows;
    std::string error; // empty on success
};

// Reads a WDBC (.dbc) file at `path` against `table`'s resolved metadata (dbc-formats.md):
// a plain header (record count, flattened field count, record size, string block size), every
// field written inline at its natural width in metadata order (there's no non-inline concept in
// this format), and a string block starting with '\0' holding one NUL-terminated string per
// string/locstring value, addressed by absolute offset.
WdbcReadResult ReadWdbcFile(std::filesystem::path const& path, DbcTableMeta const& table);

struct WdbcWriteResult
{
    std::vector<uint8_t> bytes;
    std::string error; // empty on success
};

// Builds a WDBC (.dbc) file for `table` from `rows` (the same flattened-column shape
// ReadWdbcFile produces).
WdbcWriteResult WriteWdbcFile(DbcTableMeta const& table, std::vector<Wdc5Row> const& rows);

#endif
