#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// Fetches encoded blobs that are missing from the local CASC storage: EKey ->
// archive index (Data/indices) -> HTTP range request, or the loose blob path.
class CdnFetcher
{
public:
    using Key = std::array<std::uint8_t, 16>;

    // Throws std::runtime_error when the install's CDN config or indices are unusable.
    CdnFetcher(std::filesystem::path const& client, std::vector<std::string> hosts);
    ~CdnFetcher();

    // Returns the decoded file contents, or an empty optional-style failure with `error` set.
    bool Fetch(Key const& ekey, Key const& ckey, std::vector<char>& out, std::string& error);

private:
    struct Location
    {
        std::uint32_t archive;
        std::uint32_t offset;
        std::uint32_t size;
    };

    bool Download(std::string const& path, std::int64_t offset, std::int64_t size, std::vector<char>& out,
                  std::string& error);
    void LoadIndices(std::filesystem::path const& dir);

    std::string _cdnPath;
    std::vector<std::string> _hosts;
    std::vector<std::string> _archives;
    std::vector<std::pair<Key, Location>> _locations;
    void* _curl = nullptr;
};
