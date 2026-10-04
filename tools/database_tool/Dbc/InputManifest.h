#pragma once

#include <filesystem>
#include <string>

// Lowercase hex sha256 of a file's contents; empty if it can't be read.
std::string Sha256File(std::filesystem::path const& path);

// Writes "<sha256>  <name>" for every regular file in `dir` with extension `extension`, sorted by name.
// Returns an error message, empty on success.
std::string WriteInputManifest(std::filesystem::path const& dir, std::string const& extension,
                               std::filesystem::path const& manifestPath);

// Hashes every file listed in `manifestPath` that exists in `dir` and returns the first
// mismatch as an error message naming the file; empty if all match. Listed files that are
// absent are skipped (a client may ship fewer files than the manifest lists).
std::string VerifyInputManifest(std::filesystem::path const& dir, std::filesystem::path const& manifestPath);

// Directory holding the stock client db2s: for each file, `stockDir/<name>` when it exists
// (a backup of a file `export --client` overwrote), else `clientDir/<name>`. Built as a
// directory of symlinks under `viewDir`, which is recreated on every call.
std::filesystem::path BuildStockClientView(std::filesystem::path const& clientDir,
                                           std::filesystem::path const& stockDir,
                                           std::filesystem::path const& viewDir);
