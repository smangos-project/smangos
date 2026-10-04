#ifndef DATABASE_TOOL_ZIPEXTRACT_H
#define DATABASE_TOOL_ZIPEXTRACT_H

#include <filesystem>
#include <string>

// Extracts the entry `entryName` (stored or deflate) of the zip at `zipPath` to `destination`.
// Returns an error message, empty on success.
std::string ExtractZipEntry(std::filesystem::path const& zipPath, std::string const& entryName,
                            std::filesystem::path const& destination);

#endif
