#include "Client/ClientStorage.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <system_error>

ClientStorage::ClientStorage(std::filesystem::path root) : m_root(std::move(root)) { }

std::string ClientStorage::Normalize(std::string path)
{
    for (char& c : path)
    {
        if (c == '\\')
            c = '/';
        else if (c >= 'A' && c <= 'Z')
            c = char(c - 'A' + 'a');
    }
    return path;
}

std::string ClientStorage::Load()
{
    std::ifstream in(m_root / "fdid.index");
    if (!in)
        return "cannot open " + (m_root / "fdid.index").string();

    std::string line;
    while (std::getline(in, line))
    {
        size_t sep = line.find(';');
        if (sep == std::string::npos)
            continue;
        uint32_t id = uint32_t(std::strtoul(line.c_str(), nullptr, 10));
        std::string path = line.substr(sep + 1);
        size_t end = path.find(';');
        if (end != std::string::npos)
            path.resize(end);
        path = Normalize(std::move(path));
        m_ids.emplace(path, id);
        m_paths[id] = std::move(path);
    }
    return {};
}

std::string ClientStorage::PathOf(uint32_t fileDataId) const
{
    auto itr = m_paths.find(fileDataId);
    if (itr != m_paths.end())
        return itr->second;
    return "_unnamed/" + std::to_string(fileDataId);
}

uint32_t ClientStorage::FileDataIdOf(std::string const& path) const
{
    auto itr = m_ids.find(Normalize(path));
    return itr == m_ids.end() ? 0 : itr->second;
}

bool ClientStorage::Exists(std::string const& relPath) const
{
    std::error_code ec;
    return std::filesystem::is_regular_file(m_root / relPath, ec);
}

FileStamp ClientStorage::Stat(std::string const& relPath) const
{
    FileStamp stamp;
    std::error_code ec;
    auto path = m_root / relPath;
    auto size = std::filesystem::file_size(path, ec);
    if (ec)
        return stamp;
    auto time = std::filesystem::last_write_time(path, ec);
    stamp.exists = true;
    stamp.size = size;
    stamp.mtime = ec ? 0 : std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
    return stamp;
}

std::vector<uint8_t> ClientStorage::Read(std::string const& relPath) const
{
    std::ifstream in(m_root / relPath, std::ios::binary | std::ios::ate);
    if (!in)
        return {};
    std::vector<uint8_t> data(size_t(in.tellg()));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(data.data()), std::streamsize(data.size()));
    return data;
}
