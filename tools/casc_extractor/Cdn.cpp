#include "Cdn.h"

#include <curl/curl.h>
#include <hashes/md5.h>
#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <map>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    std::string Hex(std::uint8_t const* data, std::size_t size)
    {
        static char const digits[] = "0123456789abcdef";
        std::string out(size * 2, '0');
        for (std::size_t i = 0; i < size; ++i)
        {
            out[i * 2] = digits[data[i] >> 4];
            out[i * 2 + 1] = digits[data[i] & 15];
        }
        return out;
    }

    std::uint32_t BE32(std::uint8_t const* p)
    {
        return std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 | std::uint32_t(p[2]) << 8 | p[3];
    }

    std::vector<std::uint8_t> ReadAll(fs::path const& path)
    {
        std::ifstream in(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }

    std::size_t WriteBody(char* data, std::size_t size, std::size_t count, void* user)
    {
        static_cast<std::vector<char>*>(user)->insert(static_cast<std::vector<char>*>(user)->end(), data,
                                                      data + size * count);
        return size * count;
    }

    bool BlteDecode(std::vector<char> const& in, std::vector<char>& out, std::string& error)
    {
        auto const* d = reinterpret_cast<std::uint8_t const*>(in.data());
        if (in.size() < 9 || std::memcmp(d, "BLTE", 4) != 0)
        {
            error = "not a BLTE stream";
            return false;
        }
        std::uint32_t headerSize = BE32(d + 4);
        auto chunk = [&](std::uint8_t const* p, std::size_t size) -> bool
        {
            if (size == 0)
                return true;
            if (p[0] == 'N')
                out.insert(out.end(), p + 1, p + size);
            else if (p[0] == 'Z')
            {
                std::vector<char> buf(1 << 20);
                z_stream z = {};
                inflateInit(&z);
                z.next_in = const_cast<Bytef*>(p + 1);
                z.avail_in = static_cast<uInt>(size - 1);
                int rc;
                do
                {
                    z.next_out = reinterpret_cast<Bytef*>(buf.data());
                    z.avail_out = static_cast<uInt>(buf.size());
                    rc = inflate(&z, Z_NO_FLUSH);
                    out.insert(out.end(), buf.data(), buf.data() + (buf.size() - z.avail_out));
                } while (rc == Z_OK);
                inflateEnd(&z);
                if (rc != Z_STREAM_END)
                {
                    error = "corrupt zlib frame";
                    return false;
                }
            }
            else
            {
                error = p[0] == 'E' ? "encrypted BLTE frame" : "unsupported BLTE frame";
                return false;
            }
            return true;
        };
        if (headerSize == 0)
            return chunk(d + 8, in.size() - 8);
        std::uint32_t count = d[9] << 16 | d[10] << 8 | d[11];
        std::size_t pos = headerSize;
        for (std::uint32_t i = 0; i < count; ++i)
        {
            std::size_t csize = BE32(d + 12 + i * 24);
            if (pos + csize > in.size())
            {
                error = "truncated BLTE stream";
                return false;
            }
            if (!chunk(d + pos, csize))
                return false;
            pos += csize;
        }
        return true;
    }
}

CdnFetcher::CdnFetcher(fs::path const& client, std::vector<std::string> hosts) : _hosts(std::move(hosts))
{
    std::ifstream info(client / ".build.info");
    std::string header, row;
    std::getline(info, header);
    std::getline(info, row);
    auto split = [](std::string const& line)
    {
        std::vector<std::string> cells;
        std::size_t start = 0;
        for (std::size_t end; (end = line.find('|', start)) != std::string::npos; start = end + 1)
            cells.push_back(line.substr(start, end - start));
        cells.push_back(line.substr(start));
        return cells;
    };
    std::vector<std::string> names = split(header), cells = split(row);
    std::string cdnKey;
    for (std::size_t i = 0; i < names.size() && i < cells.size(); ++i)
    {
        std::string name = names[i].substr(0, names[i].find('!'));
        if (name == "CDN Path")
            _cdnPath = cells[i];
        else if (name == "CDN Key")
            cdnKey = cells[i];
    }
    if (_cdnPath.empty() || cdnKey.size() != 32)
        throw std::runtime_error("cannot read the CDN path and key from .build.info");

    std::ifstream config(client / "Data" / "config" / cdnKey.substr(0, 2) / cdnKey.substr(2, 2) / cdnKey);
    for (std::string line; std::getline(config, line);)
    {
        if (line.rfind("archives = ", 0) != 0)
            continue;
        for (std::size_t start = 11; start < line.size();)
        {
            std::size_t end = std::min(line.find(' ', start), line.size());
            if (end > start)
                _archives.push_back(line.substr(start, end - start));
            start = end + 1;
        }
    }
    if (_archives.empty())
        throw std::runtime_error("the CDN config lists no archives");

    LoadIndices(client / "Data" / "indices");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    _curl = curl_easy_init();
}

CdnFetcher::~CdnFetcher()
{
    curl_easy_cleanup(_curl);
}

void CdnFetcher::LoadIndices(fs::path const& dir)
{
    for (std::uint32_t archive = 0; archive < _archives.size(); ++archive)
    {
        std::vector<std::uint8_t> data = ReadAll(dir / (_archives[archive] + ".index"));
        if (data.size() < 28)
            continue;
        // Footer: toc hash, version, 2 unknown, block size (KiB), offset bytes, size bytes, key bytes,
        // hash bytes, element count (LE), footer hash.
        std::uint8_t const* footer = data.data() + data.size() - 20;
        std::uint32_t blockSize = footer[3] * 1024u;
        std::uint32_t count = footer[8] | footer[9] << 8 | footer[10] << 16 | std::uint32_t(footer[11]) << 24;
        if (footer[0] != 1 || blockSize == 0 || footer[4] != 4 || footer[5] != 4 || footer[6] != 16)
            continue;
        std::uint32_t seen = 0;
        for (std::size_t block = 0; seen < count && (block + 1) * blockSize <= data.size(); ++block)
            for (std::uint32_t pos = 0; pos + 24 <= blockSize && seen < count; pos += 24)
            {
                std::uint8_t const* rec = data.data() + block * blockSize + pos;
                Key key;
                std::memcpy(key.data(), rec, 16);
                if (key == Key{})
                    break;
                ++seen;
                _locations.push_back({key, {archive, BE32(rec + 20), BE32(rec + 16)}});
            }
    }
    std::sort(_locations.begin(), _locations.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
}

bool CdnFetcher::Download(std::string const& path, std::int64_t offset, std::int64_t size, std::vector<char>& out,
                          std::string& error)
{
    for (std::string const& host : _hosts)
    {
        out.clear();
        std::string url = host + '/' + _cdnPath + "/data/" + path;
        curl_easy_reset(_curl);
        curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteBody);
        curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(_curl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 15L);
        curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_TIME, 60L);
        if (size >= 0)
        {
            std::string range = std::to_string(offset) + '-' + std::to_string(offset + size - 1);
            curl_easy_setopt(_curl, CURLOPT_RANGE, range.c_str());
        }
        CURLcode rc = curl_easy_perform(_curl);
        if (rc == CURLE_OK && (size < 0 || std::int64_t(out.size()) == size))
            return true;
        error = rc == CURLE_OK ? "short CDN response" : std::string("cdn: ") + curl_easy_strerror(rc);
    }
    return false;
}

bool CdnFetcher::Fetch(Key const& ekey, Key const& ckey, std::vector<char>& out, std::string& error)
{
    std::string name = Hex(ekey.data(), ekey.size());
    std::string dir = name.substr(0, 2) + '/' + name.substr(2, 2) + '/';
    std::vector<char> blob;

    auto it = std::lower_bound(_locations.begin(), _locations.end(), ekey,
                               [](auto const& a, Key const& k) { return a.first < k; });
    bool ok;
    if (it != _locations.end() && it->first == ekey)
    {
        std::string archive = _archives[it->second.archive];
        ok = Download(archive.substr(0, 2) + '/' + archive.substr(2, 2) + '/' + archive, it->second.offset,
                      it->second.size, blob, error);
    }
    else
        ok = Download(dir + name, 0, -1, blob, error);
    if (!ok)
        return false;

    out.clear();
    if (!BlteDecode(blob, out, error))
        return false;
    std::uint8_t digest[16];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, out.data(), out.size());
    MD5_Final(digest, &md5);
    if (std::memcmp(digest, ckey.data(), 16) != 0)
    {
        error = "cdn: content hash mismatch";
        return false;
    }
    return true;
}
