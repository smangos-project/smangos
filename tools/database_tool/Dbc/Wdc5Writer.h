#ifndef DATABASE_TOOL_DBC_WDC5WRITER_H
#define DATABASE_TOOL_DBC_WDC5WRITER_H

#include "Dbc/DbcTableMeta.h"
#include "Dbc/Wdc5Reader.h"

#include <cstdint>
#include <string>
#include <vector>

struct Wdc5WriteResult
{
    std::vector<uint8_t> bytes;
    std::string error; // empty on success
};

// Builds a WDC5 (.db2) file for `table` from `rows` (the same flattened-column shape
// ReadWdc5File produces) and `info` (identity fields sourced from dbc_files), per
// dbc-formats.md's writer rules: one section, every field stored uncompressed at its natural
// width, records sorted by id with MinId/MaxId computed from them, an id list when the id is
// non-inline, a parent lookup for any relation field, no copy table, and sparse layout
// (offset map, inline strings) exactly when info.flags & 0x1, matching how the file was read.
Wdc5WriteResult WriteWdc5File(DbcTableMeta const& table, Wdc5FileInfo const& info, std::vector<Wdc5Row> const& rows);

#endif
