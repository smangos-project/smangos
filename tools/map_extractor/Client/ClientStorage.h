#ifndef MAP_EXTRACTOR_CLIENT_CLIENTSTORAGE_H
#define MAP_EXTRACTOR_CLIENT_CLIENTSTORAGE_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct FileStamp
{
    uint64_t size = 0;
    int64_t mtime = 0;
    bool exists = false;
};

// The extracted client tree (.client/Content): lowercase paths plus fdid.index for FileDataID lookups.
class ClientStorage
{
public:
    explicit ClientStorage(std::filesystem::path root);

    // Reads fdid.index. Returns an empty string on success.
    std::string Load();

    std::filesystem::path const& Root() const { return m_root; }

    // Relative path (lowercase, '/') for a FileDataID; files without a name live under _unnamed/.
    std::string PathOf(uint32_t fileDataId) const;

    static std::string Normalize(std::string path);

    bool Exists(std::string const& relPath) const;
    FileStamp Stat(std::string const& relPath) const;
    FileStamp Stat(uint32_t fileDataId) const { return Stat(PathOf(fileDataId)); }

    // Empty on a missing or unreadable file.
    std::vector<uint8_t> Read(std::string const& relPath) const;
    std::vector<uint8_t> Read(uint32_t fileDataId) const { return Read(PathOf(fileDataId)); }

private:
    std::filesystem::path m_root;
    std::unordered_map<uint32_t, std::string> m_paths;
};

#endif
