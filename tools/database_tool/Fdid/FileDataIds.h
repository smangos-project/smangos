#ifndef DATABASE_TOOL_FILEDATAIDS_H
#define DATABASE_TOOL_FILEDATAIDS_H

#include "World/DoltLiteDatabase.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// The file_data_id table in .world: every FileDataID the client's name tables know, stock rows
// from the listfile plus ids the editors allocate from kFirstCustomId up. Ids are never deleted or
// reused; releasing one sets `retired`.
constexpr int64_t kFirstCustomId = 2000000000;

// Lowercase, forward slashes.
std::string NormalizeClientPath(std::string const& path);

struct FdidImportResult
{
    uint32_t added = 0;
    uint32_t unchanged = 0;
    std::vector<std::string> conflicts;
    std::string error;
};

// Reads `id;path` lines into file_data_id with `source`. A row whose id or path is already
// present under a different path or id is reported as a conflict and skipped; a `listfile` import
// updates the path of an existing `listfile` row.
FdidImportResult ImportFdidFile(DoltLiteDatabase& db, std::filesystem::path const& file, std::string const& source);

// Returns the id of each path (new ids come from the custom range); one transaction.
std::vector<int64_t> AllocateFdids(DoltLiteDatabase& db, std::vector<std::string> const& paths, std::string const& source);

// Sets `retired` on the given paths; returns how many rows changed.
uint32_t RetireFdids(DoltLiteDatabase& db, std::vector<std::string> const& paths);

struct FdidExportResult
{
    uint32_t rows = 0;
    bool written = false;
    std::string error;
};

// Writes every non-retired row as `id;path` lines sorted by id. Leaves the file alone when the
// content is already identical.
FdidExportResult ExportFdidIndex(DoltLiteDatabase& db, std::filesystem::path const& indexFile);

#endif
