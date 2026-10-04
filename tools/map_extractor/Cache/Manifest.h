#ifndef MAP_EXTRACTOR_CACHE_MANIFEST_H
#define MAP_EXTRACTOR_CACHE_MANIFEST_H

#include "Client/ClientStorage.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>

// Hash of everything a unit of work depends on (input file stamps, option strings, format version).
class Fingerprint
{
public:
    void Add(std::string const& text);
    void Add(std::string const& name, FileStamp const& stamp);
    std::string Hex() const;

private:
    uint64_t m_hash = 14695981039346656037ull;
};

// key -> fingerprint, persisted as text. Thread safe.
class Manifest
{
public:
    explicit Manifest(std::filesystem::path file);
    ~Manifest();

    std::string Get(std::string const& key) const;
    void Set(std::string const& key, std::string const& value);
    void Erase(std::string const& key);
    void Save();

private:
    std::filesystem::path m_file;
    mutable std::mutex m_mutex;
    std::map<std::string, std::string> m_entries;
    bool m_dirty = false;
};

#endif
