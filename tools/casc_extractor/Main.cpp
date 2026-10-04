// Extracts the files named in a listfile from a client's CASC storage into
// the loose-file Content/ tree.

#include <CascLib.h>

extern "C"
{
#include "jenkins/lookup.h"
}

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    struct Options
    {
        fs::path client = fs::path(CASC_EXTRACTOR_SOURCE_DIR) / ".client";
        fs::path listfile;
        fs::path output;
        fs::path tactKeys;
        std::string product;
        DWORD localeMask = CASC_LOCALE_ENUS;
        bool force = false;
    };

    struct ListEntry
    {
        DWORD fileDataId = 0;
        std::string path;
    };

    char const* ErrorName(DWORD error)
    {
        switch (error)
        {
            case ERROR_FILE_NOT_FOUND: return "not found";
            case ERROR_FILE_ENCRYPTED: return "encrypted";
            case ERROR_FILE_CORRUPT: return "corrupt";
            case ERROR_FILE_OFFLINE: return "not in local storage";
            case ERROR_BAD_FORMAT: return "bad format";
            case ERROR_HANDLE_EOF: return "unexpected end of file";
            case ERROR_ACCESS_DENIED: return "access denied";
            case ERROR_NOT_SUPPORTED: return "not supported";
            default: return "unknown error";
        }
    }

    std::basic_string<TCHAR> ToTchar(std::string const& s)
    {
        return std::basic_string<TCHAR>(s.begin(), s.end());
    }

    std::string NormalizePath(std::string path)
    {
        std::replace(path.begin(), path.end(), '\\', '/');
        std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) { return std::tolower(c); });
        return path;
    }

    std::uint64_t NameHash(std::string const& path)
    {
        std::string key = path;
        for (char& c : key)
            c = c == '/' ? '\\' : static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        std::uint32_t c = 0, b = 0;
        hashlittle2(key.data(), key.size(), &c, &b);
        return (std::uint64_t(c) << 32) | b;
    }

    std::string Hex(unsigned char const* data, std::size_t size)
    {
        char const* digits = "0123456789abcdef";
        std::string out;
        for (std::size_t i = 0; i < size; ++i)
        {
            out += digits[data[i] >> 4];
            out += digits[data[i] & 15];
        }
        return out;
    }

    std::vector<ListEntry> ReadListfile(fs::path const& path)
    {
        std::ifstream in(path);
        std::vector<ListEntry> entries;
        std::string line;
        while (std::getline(in, line))
        {
            while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
                line.pop_back();
            if (line.empty() || line[0] == '#')
                continue;

            ListEntry entry;
            std::size_t semicolon = line.find(';');
            DWORD id = 0;
            if (semicolon != std::string::npos
                && std::from_chars(line.data(), line.data() + semicolon, id).ec == std::errc()
                && semicolon > 0)
            {
                entry.fileDataId = id;
                entry.path = NormalizePath(line.substr(semicolon + 1));
            }
            else
                entry.path = NormalizePath(line);
            if (!entry.path.empty())
                entries.push_back(std::move(entry));
        }
        return entries;
    }

    // Lines of "<key>;<rest>" keyed by the leading number, so reruns merge.
    std::map<DWORD, std::string> ReadKeyedLines(fs::path const& path)
    {
        std::map<DWORD, std::string> lines;
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line))
        {
            std::size_t semicolon = line.find(';');
            DWORD id = 0;
            if (semicolon != std::string::npos && semicolon > 0
                && std::from_chars(line.data(), line.data() + semicolon, id).ec == std::errc())
                lines[id] = line.substr(semicolon + 1);
        }
        return lines;
    }

    void WriteKeyedLines(fs::path const& path, std::string const& header, std::map<DWORD, std::string> const& lines)
    {
        std::ofstream out(path, std::ios::trunc);
        out << header;
        for (auto const& [id, rest] : lines)
            out << id << ';' << rest << '\n';
    }

    bool ReadWhole(HANDLE file, std::vector<char>& data)
    {
        ULONGLONG size = 0;
        if (!CascGetFileSize64(file, &size))
            return false;
        data.resize(size);
        std::size_t done = 0;
        while (done < size)
        {
            DWORD read = 0;
            DWORD chunk = static_cast<DWORD>(std::min<ULONGLONG>(size - done, 1u << 24));
            if (!CascReadFile(file, data.data() + done, chunk, &read) || read == 0)
                return false;
            done += read;
        }
        return true;
    }

    void Usage()
    {
        std::puts("usage: casc_extractor --listfile FILE [--client DIR] [--output DIR] [--locale enUS]\n"
                  "                      [--product CODE] [--tact-keys FILE] [--force]\n"
                  "\n"
                  "Extracts every file in the listfile (<fileDataId>;<path> or <path> per line) from the\n"
                  "CASC storage in --client (default .client) to --output (default <client>/Content).");
    }

    bool ParseLocale(std::string_view name, DWORD& mask)
    {
        static constexpr std::pair<std::string_view, DWORD> locales[] = {
            {"enUS", CASC_LOCALE_ENUS}, {"koKR", CASC_LOCALE_KOKR}, {"frFR", CASC_LOCALE_FRFR},
            {"deDE", CASC_LOCALE_DEDE}, {"zhCN", CASC_LOCALE_ZHCN}, {"esES", CASC_LOCALE_ESES},
            {"zhTW", CASC_LOCALE_ZHTW}, {"enGB", CASC_LOCALE_ENGB}, {"esMX", CASC_LOCALE_ESMX},
            {"ruRU", CASC_LOCALE_RURU}, {"ptBR", CASC_LOCALE_PTBR}, {"itIT", CASC_LOCALE_ITIT},
            {"ptPT", CASC_LOCALE_PTPT}};
        for (auto const& [n, m] : locales)
            if (n == name)
            {
                mask = m;
                return true;
            }
        return false;
    }

    bool ParseArgs(int argc, char** argv, Options& options)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];
            auto value = [&]() -> char const* { return i + 1 < argc ? argv[++i] : nullptr; };
            char const* v = nullptr;
            if (arg == "--force")
                options.force = true;
            else if (arg == "--listfile" && (v = value()))
                options.listfile = v;
            else if (arg == "--client" && (v = value()))
                options.client = v;
            else if (arg == "--output" && (v = value()))
                options.output = v;
            else if (arg == "--tact-keys" && (v = value()))
                options.tactKeys = v;
            else if (arg == "--product" && (v = value()))
                options.product = v;
            else if (arg == "--locale" && (v = value()))
            {
                if (!ParseLocale(v, options.localeMask))
                {
                    std::fprintf(stderr, "unknown locale '%s'\n", v);
                    return false;
                }
            }
            else
                return false;
        }
        if (options.listfile.empty())
            return false;
        if (options.output.empty())
            options.output = options.client / "Content";
        return true;
    }
}

int main(int argc, char** argv)
{
    Options options;
    if (!ParseArgs(argc, argv, options))
    {
        Usage();
        return 2;
    }

    std::vector<ListEntry> entries = ReadListfile(options.listfile);
    if (entries.empty())
    {
        std::fprintf(stderr, "no entries in %s\n", options.listfile.string().c_str());
        return 1;
    }

    std::basic_string<TCHAR> clientPath = ToTchar(options.client.string());
    std::basic_string<TCHAR> product = ToTchar(options.product);
    CASC_OPEN_STORAGE_ARGS args = {};
    args.Size = sizeof(args);
    args.szLocalPath = clientPath.c_str();
    args.szCodeName = product.empty() ? nullptr : product.c_str();
    args.dwLocaleMask = options.localeMask;

    HANDLE storage = nullptr;
    if (!CascOpenStorageEx(nullptr, &args, false, &storage))
    {
        std::fprintf(stderr, "cannot open CASC storage in %s: %s\n", options.client.string().c_str(),
                     ErrorName(GetCascError()));
        return 1;
    }
    if (!options.tactKeys.empty())
    {
        std::basic_string<TCHAR> keys = ToTchar(options.tactKeys.string());
        if (!CascImportKeysFromFile(storage, keys.c_str()))
            std::fprintf(stderr, "warning: could not import tact keys from %s\n", options.tactKeys.string().c_str());
    }

    fs::create_directories(options.output);
    fs::path indexPath = options.output / "fdid.index";
    fs::path hashesPath = options.output / "root.hashes";
    std::map<DWORD, std::string> index = ReadKeyedLines(indexPath);
    std::map<DWORD, std::string> hashes = ReadKeyedLines(hashesPath);

    std::vector<std::string> failures;
    std::size_t extracted = 0, upToDate = 0;
    std::vector<char> data;
    for (std::size_t n = 0; n < entries.size(); ++n)
    {
        ListEntry const& entry = entries[n];
        if (n && n % 5000 == 0)
            std::printf("  %zu/%zu\n", n, entries.size());

        auto fail = [&](DWORD error)
        {
            failures.push_back((entry.fileDataId ? std::to_string(entry.fileDataId) : std::string()) + ';' + entry.path
                               + ';' + ErrorName(error));
        };

        HANDLE file = nullptr;
        bool opened = entry.fileDataId
            ? CascOpenFile(storage, CASC_FILE_DATA_ID(entry.fileDataId), options.localeMask, CASC_OPEN_BY_FILEID, &file)
            : CascOpenFile(storage, entry.path.c_str(), options.localeMask, CASC_OPEN_BY_NAME, &file);
        if (!opened)
        {
            fail(GetCascError());
            continue;
        }

        fs::path dest = options.output / fs::path(entry.path);
        ULONGLONG size = 0;
        CascGetFileSize64(file, &size);

        CASC_FILE_FULL_INFO info = {};
        size_t needed = 0;
        bool haveInfo = CascGetFileInfo(file, CascFileFullInfo, &info, sizeof(info), &needed);
        DWORD fileDataId = entry.fileDataId ? entry.fileDataId : haveInfo ? info.FileDataId : CASC_INVALID_ID;

        std::error_code ec;
        if (!options.force && fs::is_regular_file(dest, ec) && fs::file_size(dest, ec) == size)
            ++upToDate;
        else if (!ReadWhole(file, data))
        {
            fail(GetCascError());
            CascCloseFile(file);
            continue;
        }
        else
        {
            fs::create_directories(dest.parent_path(), ec);
            fs::path part = dest;
            part += ".part";
            {
                std::ofstream out(part, std::ios::binary | std::ios::trunc);
                out.write(data.data(), static_cast<std::streamsize>(data.size()));
                if (!out)
                {
                    failures.push_back(std::to_string(fileDataId) + ';' + entry.path + ";write failed");
                    CascCloseFile(file);
                    continue;
                }
            }
            fs::rename(part, dest);
            ++extracted;
        }

        if (fileDataId != CASC_INVALID_ID && fileDataId != 0)
        {
            index[fileDataId] = entry.path;
            if (haveInfo)
            {
                char line[128];
                std::snprintf(line, sizeof(line), "%016llx;%s;%llu", static_cast<unsigned long long>(NameHash(entry.path)),
                              Hex(info.CKey, sizeof(info.CKey)).c_str(), static_cast<unsigned long long>(size));
                hashes[fileDataId] = line;
            }
        }
        CascCloseFile(file);
    }
    CascCloseStorage(storage);

    WriteKeyedLines(indexPath, "", index);
    WriteKeyedLines(hashesPath, "# fileDataId;nameHash;ckey;size -- from the build's root and encoding\n", hashes);

    fs::path failLog = options.output / "_dump_failed.log";
    if (!failures.empty())
    {
        std::ofstream out(failLog, std::ios::trunc);
        for (std::string const& f : failures)
            out << f << '\n';
    }
    else
    {
        std::error_code ec;
        fs::remove(failLog, ec);
    }

    std::printf("extracted %zu, up to date %zu, failed %zu%s\n", extracted, upToDate, failures.size(),
                failures.empty() ? "" : " (see _dump_failed.log)");
    return failures.size() == entries.size() ? 1 : 0;
}
