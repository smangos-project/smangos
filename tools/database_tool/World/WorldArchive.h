#ifndef DATABASE_TOOL_WORLDARCHIVE_H
#define DATABASE_TOOL_WORLDARCHIVE_H

#include <cstdint>
#include <filesystem>
#include <string>

// One pinned download: a URL, the archive/unpacked file names inside the
// cache directory, and the archive's expected size and SHA256 (world-sql.md).
struct WorldArchiveSpec
{
    std::string url;
    std::string archiveFileName;
    std::string unpackedFileName;
    std::string entryName; // zip entry to extract; empty for a single-file 7z
    uintmax_t expectedSize;
    std::string expectedSha256Hex; // lowercase hex
};

// The pinned vmangos release dump (db_latest): mysql-dump/mangos.sql from db-<sha>.zip.
WorldArchiveSpec const& WorldBaseDumpArchiveSpec();

struct WorldArchiveResult
{
    std::filesystem::path sqlPath; // valid only when error is empty
    bool downloaded = false;       // false if the archive was already cached
    std::string error;             // empty on success
};

// Downloads spec.url into cacheDirectory/spec.archiveFileName (creating
// cacheDirectory if needed) unless it's already there, checks its size and
// SHA256 against spec either way, and unpacks the single file inside it (spec.entryName for a zip)
// to cacheDirectory/spec.unpackedFileName.
WorldArchiveResult FetchAndUnpackWorldArchive(WorldArchiveSpec const& spec, std::filesystem::path const& cacheDirectory);

#endif
