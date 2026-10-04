#include "Cache/Manifest.h"

#include <cstdio>
#include <fstream>

void Fingerprint::Add(std::string const& text)
{
    for (unsigned char c : text)
    {
        m_hash ^= c;
        m_hash *= 1099511628211ull;
    }
    m_hash ^= 0xff;
    m_hash *= 1099511628211ull;
}

void Fingerprint::Add(std::string const& name, FileStamp const& stamp)
{
    Add(name + (stamp.exists ? ":" + std::to_string(stamp.size) + ":" + std::to_string(stamp.mtime) : ":missing"));
}

std::string Fingerprint::Hex() const
{
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(m_hash));
    return buf;
}

Manifest::Manifest(std::filesystem::path file) : m_file(std::move(file))
{
    std::ifstream in(m_file);
    std::string line;
    while (std::getline(in, line))
    {
        size_t tab = line.find('\t');
        if (tab != std::string::npos)
            m_entries[line.substr(0, tab)] = line.substr(tab + 1);
    }
}

Manifest::~Manifest()
{
    Save();
}

std::string Manifest::Get(std::string const& key) const
{
    std::lock_guard lock(m_mutex);
    auto itr = m_entries.find(key);
    return itr == m_entries.end() ? std::string() : itr->second;
}

void Manifest::Set(std::string const& key, std::string const& value)
{
    std::lock_guard lock(m_mutex);
    m_entries[key] = value;
    m_dirty = true;
}

void Manifest::Erase(std::string const& key)
{
    std::lock_guard lock(m_mutex);
    m_dirty |= m_entries.erase(key) > 0;
}

void Manifest::Save()
{
    std::lock_guard lock(m_mutex);
    if (!m_dirty)
        return;
    std::error_code ec;
    std::filesystem::create_directories(m_file.parent_path(), ec);
    std::filesystem::path tmp = m_file;
    tmp += ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        for (auto const& [k, v] : m_entries)
            out << k << '\t' << v << '\n';
    }
    std::filesystem::rename(tmp, m_file, ec);
    m_dirty = false;
}
