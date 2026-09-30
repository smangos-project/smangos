// Feeds the extracted Content/ tree into the client's own loose-file override
// map, so every asset is read from disk as a plain OS file and CASC is never
// consulted for it.
//
// This is not a bolted-on file system: the CAS layer already carries a second
// file source next to CASC (the "local manifest"), and the shipping client uses
// it for the Battle.net avatar cache. CAS_OpenFile checks that map first, and
// on a hit hands back a plain OS file handle -- no encoding table, no indices,
// no data.NNN, no BLTE decode. An override also does not need a matching build
// manifest entry, so it can add files, not just replace them.
// docs/CascBypass.md records how all of that was established.
//
// Two client functions are needed, both located by byte pattern rather than
// RVA so a rebuilt client either gets hooked correctly or logs a clear error:
//
//   CAS_AddLocalOverride(path, basePath, fileDataId, addToFileIdMap) -> fileId
//       Registers one override. With a basePath, the entry is keyed by the
//       name hash of the relative `path` and opens basePath\path, so it answers
//       lookups by name as well as by id. A non-zero fileDataId registers that
//       exact id; the return value is the id the entry ended up with (0 on
//       failure).
//
//   CAS_SetLooseFileDirAndScan(dir)
//       Called once by CAS init, after the manifests are loaded and before the
//       first asset is read -- exactly the window we need. The client uses it
//       to scan its working directory; that scan runs first and Content/ is
//       registered on top of it (see DetourSetLooseFileDir for why).
//
// By default CAS runs in its own loose-file-only mode with a build manifest
// generated from Content/, so the client never opens Data/ or goes to a CDN.

#include "CascBypass.hpp"

#include "custom/LaunchProtocol.hpp"
#include "hook_lib/HookManager.hpp"
#include "hook_lib/Log.hpp"
#include "hook_lib/Memory.hpp"
#include "hook_lib/Pattern.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <windows.h>
#include <intrin.h>
#include <shellapi.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <mutex>
#include <span>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>


using CasAddLocalOverrideFn = int(*)(char const* path, char const* basePath, int fileDataId, bool addToFileIdMap);
using CasSetLooseFileDirFn = void(*)(char const* directory);
using FindOverrideByFileIdFn = char const*(*)(void* container, int fileDataId);
using FindOverrideByNameHashFn = char const*(*)(void* container, std::uint64_t nameHash, std::uint32_t* fileDataId);
using LoadBuildManifestFn = bool(*)(void** buffer, unsigned long long size, void* config, void* log);
using ReadFromStorageFn = bool(*)(void** storage, void const* key, void** buffer, std::uint32_t* size,
                                  std::uint32_t flags, std::uint32_t tag);
using QueryStorageAvailabilityFn = std::uint64_t(*)(void** storage, void const* key, std::uint64_t offset,
                                                    std::uint64_t size);
using OpenFromStorageAsyncFn = bool(*)(void** storage, void const* key, void const* ckey, int fileDataId,
                                       void** handle);
using FormatStorageKeyFn = void*(*)(void const* storage, void* outString);
using QueryResidencyFn = std::uint64_t(*)(void const* entryKey, std::uint32_t a, std::uint32_t b, std::uint32_t c);
using MakeResidentFn = std::uint64_t(*)(void** storage, void const* key, void* callback, std::uint32_t a,
                                        std::uint32_t b);
using GetFileProgressFn = std::uint64_t(*)(std::int32_t fileDataId, std::uint64_t* residentBytes,
                                           std::uint64_t* totalBytes);
using CasShutdownFn = void*(*)();

CasAddLocalOverrideFn CasAddLocalOverride = nullptr;
CasSetLooseFileDirFn OriginalSetLooseFileDir = nullptr;
FindOverrideByFileIdFn FindOverrideByFileId = nullptr;
FindOverrideByNameHashFn FindOverrideByNameHash = nullptr;
LoadBuildManifestFn OriginalLoadBuildManifest = nullptr;
ReadFromStorageFn OriginalReadFromStorage = nullptr;
QueryStorageAvailabilityFn OriginalQueryStorageAvailability = nullptr;
OpenFromStorageAsyncFn OriginalOpenFromStorageAsync = nullptr;
FormatStorageKeyFn OriginalFormatStorageKey = nullptr;
QueryResidencyFn OriginalQueryResidency = nullptr;
MakeResidentFn OriginalMakeResident = nullptr;
GetFileProgressFn OriginalGetFileProgress = nullptr;
CasShutdownFn OriginalCasShutdown = nullptr;

// DAT_143638890, the CASC storage object; resolved from CAS_Shutdown.
void** StorageSlot = nullptr;

bool LooseFileOnly = false;

// The client's global holding the container; null until CAS init creates it,
// so it is read at use time rather than captured.
void** CasContainerSlot = nullptr;

// Kept from registration so the map can be read back once the client's own
// directory scan has also run.
std::vector<std::pair<std::uint32_t, std::string>> VerifiedSamples;

std::string ContentDirectory;

bool HasCommandLineFlag(wchar_t const* flag)
{
    for (std::wstring const& argument : DllArguments()) {
        if (_wcsicmp(argument.c_str(), flag) == 0)
            return true;
    }
    return false;
}

// --contentdir <path>, else <exe dir>\..\Content.
std::string ResolveContentDirectory()
{
    std::vector<std::wstring> const arguments = DllArguments();
    for (std::size_t i = 0; i + 1 < arguments.size(); ++i) {
        if (_wcsicmp(arguments[i].c_str(), L"--contentdir") != 0 && _wcsicmp(arguments[i].c_str(), L"-contentdir") != 0)
            continue;

        char narrow[MAX_PATH * 4] = {};
        int written = WideCharToMultiByte(CP_ACP, 0, arguments[i + 1].c_str(), -1, narrow, sizeof(narrow), nullptr, nullptr);
        if (written > 0)
            return narrow;
        return {};
    }

    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) == 0)
        return {};

    char* lastSlash = std::strrchr(exePath, '\\');
    if (!lastSlash)
        return {};
    *lastSlash = '\0';

    std::string candidate = std::string(exePath) + "\\..\\Content";

    // Collapse the "..", so the registered paths and the log read cleanly.
    char full[MAX_PATH] = {};
    DWORD written = GetFullPathNameA(candidate.c_str(), MAX_PATH, full, nullptr);
    if (written > 0 && written < MAX_PATH)
        return full;

    return candidate;
}

// A few ids whose use is easy to see on the login screen, always sampled so the
// readback says something about assets we can check by eye.
bool IsDiagnosticFileDataId(std::uint32_t fileDataId)
{
    switch (fileDataId) {
    case 615960:  // fonts/frizqt__.ttf  -- every glyph of UI text
    case 615962:  // fonts/morpheus.ttf  -- headings
    case 131197:  // interface/glues/common/glues-wow-logo.blp
    case 5216544: // buildinfo.json      -- read right after CAS init
    case 204352:  // world/minimaps/azeroth/map29_27.blp -- opened by name
        return true;
    default:
        return false;
    }
}

bool ReadWholeFile(char const* path, std::vector<char>& out)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    LARGE_INTEGER size = {};
    if (!GetFileSizeEx(file, &size) || size.QuadPart <= 0 || size.QuadPart > (256ll << 20)) {
        CloseHandle(file);
        return false;
    }

    out.resize(static_cast<std::size_t>(size.QuadPart));
    std::size_t offset = 0;
    while (offset < out.size()) {
        std::size_t remaining = out.size() - offset;
        DWORD chunk = static_cast<DWORD>(remaining < (1u << 20) ? remaining : (1u << 20));
        DWORD read = 0;
        if (!ReadFile(file, out.data() + offset, chunk, &read, nullptr) || read == 0) {
            CloseHandle(file);
            return false;
        }
        offset += read;
    }

    CloseHandle(file);
    return true;
}

// ---------------------------------------------------------------------------
// Content/ as a file system
//
// What exists is whatever is in Content/ when the client starts: the directory
// is walked, not a list of what the dumper once wrote. Adding a file serves it,
// deleting one removes it from the game.
//
// The client addresses files by FileDataId and by name, so each file needs an
// id. It comes from a name table -- Content/fdid.index (written by
// client_dumper) and any other Content/*.index, lines of "<id>;<path>", later
// files winning -- or from the name itself for _unnamed/<id>. A file no table
// knows is new content; it gets a stable synthetic id (see SyntheticFileDataId)
// and is still reachable by name.

// The client's name hash (CAS_HashPathKey, 0x14034ce80): uppercase ASCII
// letters, '/' -> '\', then Bob Jenkins' lookup3 hashlittle2 with both seeds
// zero, returned as (c << 32) | b. It keys both the build manifest's name
// lookup and the local manifest's path map, so the two agree only if we hash
// exactly like this.
std::uint64_t HashLittle2(std::string_view key)
{
    auto rot = [](std::uint32_t x, int k) { return (x << k) | (x >> (32 - k)); };

    auto const* k = reinterpret_cast<std::uint8_t const*>(key.data());
    std::size_t length = key.size();
    std::uint32_t a = 0xDEADBEEF + static_cast<std::uint32_t>(length);
    std::uint32_t b = a;
    std::uint32_t c = a;

    auto word = [&](std::size_t at) {
        return std::uint32_t(k[at]) | (std::uint32_t(k[at + 1]) << 8) | (std::uint32_t(k[at + 2]) << 16) |
               (std::uint32_t(k[at + 3]) << 24);
    };

    while (length > 12) {
        a += word(0);
        b += word(4);
        c += word(8);
        a -= c; a ^= rot(c, 4);  c += b;
        b -= a; b ^= rot(a, 6);  a += c;
        c -= b; c ^= rot(b, 8);  b += a;
        a -= c; a ^= rot(c, 16); c += b;
        b -= a; b ^= rot(a, 19); a += c;
        c -= b; c ^= rot(b, 4);  b += a;
        length -= 12;
        k += 12;
    }

    switch (length) {
    case 12: c += std::uint32_t(k[11]) << 24; [[fallthrough]];
    case 11: c += std::uint32_t(k[10]) << 16; [[fallthrough]];
    case 10: c += std::uint32_t(k[9]) << 8;   [[fallthrough]];
    case 9:  c += k[8];                       [[fallthrough]];
    case 8:  b += std::uint32_t(k[7]) << 24;  [[fallthrough]];
    case 7:  b += std::uint32_t(k[6]) << 16;  [[fallthrough]];
    case 6:  b += std::uint32_t(k[5]) << 8;   [[fallthrough]];
    case 5:  b += k[4];                       [[fallthrough]];
    case 4:  a += std::uint32_t(k[3]) << 24;  [[fallthrough]];
    case 3:  a += std::uint32_t(k[2]) << 16;  [[fallthrough]];
    case 2:  a += std::uint32_t(k[1]) << 8;   [[fallthrough]];
    case 1:
        a += k[0];
        c ^= b; c -= rot(b, 14);
        a ^= c; a -= rot(c, 11);
        b ^= a; b -= rot(a, 25);
        c ^= b; c -= rot(b, 16);
        a ^= c; a -= rot(c, 4);
        b ^= a; b -= rot(a, 14);
        c ^= b; c -= rot(b, 24);
        break;
    case 0:
        break;
    }

    return (std::uint64_t(c) << 32) | b;
}

std::uint64_t ClientNameHash(std::string_view path)
{
    std::string key(path);
    for (char& ch : key) {
        if (ch >= 'a' && ch <= 'z')
            ch = static_cast<char>(ch - 'a' + 'A');
        else if (ch == '/')
            ch = '\\';
    }
    return HashLittle2(key);
}

// lookup3's own test vectors (driver5: hashlittle2 with zero seeds prints
// c, b), checked once so a porting slip fails loudly instead of making every
// name lookup miss.
bool ClientNameHashSelfTest()
{
    return HashLittle2("") == 0xDEADBEEFDEADBEEFull &&
           HashLittle2("Four score and seven years ago") == 0x17770551CE7226E6ull;
}

struct ContentFile {
    std::string path;               // relative to Content/, '\' separated, as found on disk
    std::uint32_t fileDataId;
    std::uint64_t nameHash;         // hash of `path`: the local-manifest key
    std::uint64_t manifestNameHash; // what the build manifest lists: the real name when known
    std::array<std::uint8_t, 16> ckey;
    std::uint64_t size;
    std::uint64_t writeTime;
};

struct ContentCatalog {
    std::vector<ContentFile> files;   // sorted by fileDataId, ids unique
    std::size_t fromTable = 0;
    std::size_t fromUnnamed = 0;
    std::size_t synthetic = 0;
    std::size_t duplicates = 0;
    std::size_t tableEntries = 0;
    std::size_t realNames = 0;
    std::size_t realCKeys = 0;
    ULONGLONG milliseconds = 0;
};

// Content/root.hashes, from the build's own root and encoding: for each id,
// its real name hash, CKey and size. The listfile's names are the community's
// best guess and a few are wrong (e.g. shaders/preload_fdid_list.blob), and
// the client looks files up by the real name, so the manifest must carry the
// real hash. The CKey is kept when the file still has the original size.
struct RealRootEntry {
    std::uint64_t nameHash;
    std::array<std::uint8_t, 16> ckey;
    std::int64_t size;
};

bool ParseHexBytes(std::string_view text, std::uint8_t* out, std::size_t count)
{
    if (text.size() != count * 2)
        return false;
    auto nibble = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < count; ++i) {
        int hi = nibble(text[i * 2]);
        int lo = nibble(text[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out[i] = static_cast<std::uint8_t>(hi << 4 | lo);
    }
    return true;
}

std::unordered_map<std::uint32_t, RealRootEntry> LoadRealRoot()
{
    std::unordered_map<std::uint32_t, RealRootEntry> real;
    std::vector<char> text;
    if (!ReadWholeFile((ContentDirectory + "\\root.hashes").c_str(), text)) {
        HOOK_LOG_WARN("casc bypass: Content/root.hashes not found -- names come from the listfile only, and the "
                      "few it gets wrong will not resolve by name");
        return real;
    }
    real.reserve(160000);

    char const* cursor = text.data();
    char const* end = cursor + text.size();
    while (cursor < end) {
        char const* lineEnd = static_cast<char const*>(std::memchr(cursor, '\n', static_cast<std::size_t>(end - cursor)));
        if (!lineEnd)
            lineEnd = end;
        std::string_view line(cursor, static_cast<std::size_t>(lineEnd - cursor));
        cursor = lineEnd + 1;
        if (line.empty() || line[0] == '#')
            continue;

        // id;nameHash;ckey;size
        std::string_view fields[4];
        for (std::size_t f = 0; f < 4; ++f) {
            std::size_t at = line.find(';');
            fields[f] = line.substr(0, at);
            line = at == std::string_view::npos ? std::string_view() : line.substr(at + 1);
        }

        std::uint32_t id = 0;
        for (char ch : fields[0])
            id = id * 10 + static_cast<std::uint32_t>(ch - '0');

        RealRootEntry entry = {};
        std::uint8_t hash[8] = {};
        if (id == 0 || !ParseHexBytes(fields[1], hash, 8) || !ParseHexBytes(fields[2], entry.ckey.data(), 16))
            continue;
        for (std::uint8_t byte : hash)
            entry.nameHash = entry.nameHash << 8 | byte;
        entry.size = fields[3].empty() || fields[3][0] == '-' ? -1 : std::stoll(std::string(fields[3]));
        real.emplace(id, entry);
    }

    HOOK_LOG_INFO("casc bypass: Content/root.hashes: %zu real name hash(es)", real.size());
    return real;
}

// No archive stands behind a synthetic CKey; it only needs to be stable and
// distinct per version of a file, so it is made from id, size and mtime.
std::array<std::uint8_t, 16> SyntheticCKey(std::uint32_t fileDataId, std::uint64_t size, std::uint64_t writeTime)
{
    std::array<std::uint8_t, 16> key = {};
    std::uint32_t size32 = static_cast<std::uint32_t>(size);
    std::memcpy(key.data(), &fileDataId, 4);
    std::memcpy(key.data() + 4, &size32, 4);
    std::memcpy(key.data() + 8, &writeTime, 8);
    return key;
}

std::string NormalizeKey(std::string_view path)
{
    std::string key(path);
    for (char& ch : key) {
        if (ch >= 'A' && ch <= 'Z')
            ch = static_cast<char>(ch - 'A' + 'a');
        else if (ch == '\\')
            ch = '/';
    }
    return key;
}

// Name tables: every Content/*.index, fdid.index first so the others override.
std::unordered_map<std::string, std::uint32_t> LoadNameTables(std::size_t& entries)
{
    std::vector<std::string> tables;
    WIN32_FIND_DATAA found = {};
    HANDLE find = FindFirstFileA((ContentDirectory + "\\*.index").c_str(), &found);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                tables.emplace_back(found.cFileName);
        } while (FindNextFileA(find, &found));
        FindClose(find);
    }
    std::sort(tables.begin(), tables.end(), [](std::string const& a, std::string const& b) {
        bool aMain = _stricmp(a.c_str(), "fdid.index") == 0;
        bool bMain = _stricmp(b.c_str(), "fdid.index") == 0;
        return aMain != bMain ? aMain : _stricmp(a.c_str(), b.c_str()) < 0;
    });

    std::unordered_map<std::string, std::uint32_t> names;
    names.reserve(200000);
    entries = 0;

    for (std::string const& table : tables) {
        std::vector<char> text;
        if (!ReadWholeFile((ContentDirectory + "\\" + table).c_str(), text)) {
            HOOK_LOG_WARN("casc bypass: name table '%s' is unreadable, skipped", table.c_str());
            continue;
        }

        std::size_t added = 0;
        char const* cursor = text.data();
        char const* end = cursor + text.size();
        while (cursor < end) {
            char const* lineEnd = static_cast<char const*>(std::memchr(cursor, '\n', static_cast<std::size_t>(end - cursor)));
            if (!lineEnd)
                lineEnd = end;
            std::string_view line(cursor, static_cast<std::size_t>(lineEnd - cursor));
            cursor = lineEnd + 1;
            if (!line.empty() && line.back() == '\r')
                line.remove_suffix(1);

            std::size_t separator = line.find(';');
            if (separator == 0 || separator == std::string_view::npos || separator + 1 >= line.size())
                continue;

            std::uint32_t fileDataId = 0;
            bool digits = true;
            for (char ch : line.substr(0, separator)) {
                digits = digits && ch >= '0' && ch <= '9';
                fileDataId = fileDataId * 10 + static_cast<std::uint32_t>(ch - '0');
            }
            if (!digits || fileDataId == 0)
                continue;

            names[NormalizeKey(line.substr(separator + 1))] = fileDataId;
            ++added;
        }

        entries += added;
        HOOK_LOG_INFO("casc bypass: name table '%s': %zu name(s)", table.c_str(), added);
    }

    return names;
}

// For a file no name table knows. Real ids stay well below 0x40000000, so the
// range above it is ours; derived from the name so it doesn't change between
// runs as other files come and go.
std::uint32_t SyntheticFileDataId(std::uint64_t nameHash)
{
    return 0x40000000u | static_cast<std::uint32_t>(nameHash & 0x3FFFFFFFu);
}

// Files at the top of Content/ that describe it rather than belong to it.
bool IsCatalogMetadata(std::string_view relative)
{
    if (relative.find('\\') != std::string_view::npos)
        return false;
    std::string key = NormalizeKey(relative);
    return key.ends_with(".index") || key.ends_with(".hashes") || key == "_dump_failed.log";
}

ContentCatalog BuildContentCatalog()
{
    ContentCatalog catalog;
    ULONGLONG const start = GetTickCount64();

    std::unordered_map<std::string, std::uint32_t> const names = LoadNameTables(catalog.tableEntries);
    std::unordered_map<std::uint32_t, RealRootEntry> const real = LoadRealRoot();

    std::vector<std::string> pending = { std::string() }; // relative directories
    while (!pending.empty()) {
        std::string directory = std::move(pending.back());
        pending.pop_back();

        std::string pattern = ContentDirectory + "\\" + directory + (directory.empty() ? "*" : "\\*");
        WIN32_FIND_DATAA found = {};
        HANDLE find = FindFirstFileExA(pattern.c_str(), FindExInfoBasic, &found, FindExSearchNameMatch, nullptr,
                                       FIND_FIRST_EX_LARGE_FETCH);
        if (find == INVALID_HANDLE_VALUE)
            continue;

        do {
            if (found.cFileName[0] == '.')
                continue;
            std::string relative = directory.empty() ? found.cFileName : directory + "\\" + found.cFileName;
            if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                pending.push_back(std::move(relative));
                continue;
            }
            if (IsCatalogMetadata(relative))
                continue;

            ContentFile file;
            file.nameHash = ClientNameHash(relative);
            file.size = (std::uint64_t(found.nFileSizeHigh) << 32) | found.nFileSizeLow;
            file.writeTime = (std::uint64_t(found.ftLastWriteTime.dwHighDateTime) << 32) | found.ftLastWriteTime.dwLowDateTime;
            file.fileDataId = 0;

            std::string key = NormalizeKey(relative);
            if (auto it = names.find(key); it != names.end()) {
                file.fileDataId = it->second;
                ++catalog.fromTable;
            } else if (key.starts_with("_unnamed/")) {
                std::uint32_t id = 0;
                bool digits = key.size() > 9;
                for (char ch : std::string_view(key).substr(9)) {
                    if (ch == '.')
                        break;
                    digits = digits && ch >= '0' && ch <= '9';
                    id = id * 10 + static_cast<std::uint32_t>(ch - '0');
                }
                if (digits && id != 0) {
                    file.fileDataId = id;
                    ++catalog.fromUnnamed;
                }
            }
            if (file.fileDataId == 0) {
                file.fileDataId = SyntheticFileDataId(file.nameHash);
                ++catalog.synthetic;
                if (catalog.synthetic <= 20)
                    HOOK_LOG_INFO("casc bypass: '%s' is in no name table, serving it as FileDataId %u",
                                  relative.c_str(),
                                  file.fileDataId);
            }

            file.manifestNameHash = file.nameHash;
            file.ckey = SyntheticCKey(file.fileDataId, file.size, file.writeTime);
            if (auto it = real.find(file.fileDataId); it != real.end()) {
                file.manifestNameHash = it->second.nameHash;
                ++catalog.realNames;
                if (it->second.size >= 0 && static_cast<std::uint64_t>(it->second.size) == file.size) {
                    file.ckey = it->second.ckey;
                    ++catalog.realCKeys;
                }
            }

            file.path = std::move(relative);
            catalog.files.push_back(std::move(file));
        } while (FindNextFileA(find, &found));
        FindClose(find);
    }

    // One file per id. Stable sort keeps directory order for the tie, so which
    // one wins is at least consistent; a synthetic id that collides moves up.
    std::stable_sort(catalog.files.begin(), catalog.files.end(),
                     [](ContentFile const& a, ContentFile const& b) { return a.fileDataId < b.fileDataId; });
    std::vector<ContentFile> unique;
    unique.reserve(catalog.files.size());
    for (ContentFile& file : catalog.files) {
        if (!unique.empty() && unique.back().fileDataId == file.fileDataId) {
            ++catalog.duplicates;
            HOOK_LOG_WARN("casc bypass: '%s' and '%s' both claim FileDataId %u; using the first",
                          unique.back().path.c_str(),
                          file.path.c_str(),
                          file.fileDataId);
            continue;
        }
        unique.push_back(std::move(file));
    }
    catalog.files = std::move(unique);

    catalog.milliseconds = GetTickCount64() - start;
    return catalog;
}

// The walk starts on a thread at DLL load, so it overlaps the client's own
// startup; everything that needs it waits here.
std::jthread CatalogThread;
ContentCatalog Catalog;
std::once_flag CatalogJoined;
std::atomic<bool> CatalogReady{ false }; // Catalog may be read without waiting

ContentCatalog const& WaitForCatalog()
{
    std::call_once(CatalogJoined, [] {
        if (CatalogThread.joinable())
            CatalogThread.join();
        CatalogReady = true;
        HOOK_LOG_INFO("casc bypass: Content/ holds %zu file(s): %zu named by table, %zu _unnamed, %zu new "
                      "(synthetic id), %zu duplicate id(s) dropped; %zu table name(s); %zu real name hash(es), "
                      "%zu real CKey(s); scanned in %llu ms",
                      Catalog.files.size(),
                      Catalog.fromTable,
                      Catalog.fromUnnamed,
                      Catalog.synthetic,
                      Catalog.duplicates,
                      Catalog.tableEntries,
                      Catalog.realNames,
                      Catalog.realCKeys,
                      Catalog.milliseconds);
    });
    return Catalog;
}

constexpr std::uint32_t BuildInfoFileDataId = 5216544; // buildinfo.json

// LocalManifestEntry::path is char[0x104] and holds Content dir + '\' + path.
bool FitsLocalManifestPath(ContentFile const& file)
{
    return ContentDirectory.size() + 1 + file.path.size() < 0x104;
}

// Registers every catalogued file as (relative path, Content dir). Given a
// base, LocalManifest_AddOverride keys the entry by the hash of the relative
// path -- the same hash the build manifest carries for that name -- and opens
// base\path.
//
// Keying by name is not enough to be *found* by name, though. The by-name
// lookup (LocalManifest_FindOverrideByNameHash, 0x1403508f0) only returns
// entries whose "named" flag (entry +0x108) is set, and AddOverride sets it
// only when called with FileDataId 0 -- the way the client's own directory
// scan registers files. Registered with an explicit id, an entry is reachable
// by id alone, and every open by name (UI files, minimap tiles, ...) quietly
// went to CASC instead; for a file this streaming install never downloaded,
// that means a failed CDN request and a placeholder (the green minimap tiles).
//
// So each file is registered twice: first with its id, then again with id 0.
// The second call finds the existing entry, sets the flag and re-inserts it
// under the id it already has, leaving the id mapping as the first call made it.
void RegisterContentOverrides()
{
    if (ContentDirectory.empty()) {
        HOOK_LOG_WARN("casc bypass: could not work out the Content directory, no overrides registered");
        return;
    }

    ContentCatalog const& catalog = WaitForCatalog();
    ULONGLONG const startTicks = GetTickCount64();

    std::size_t registered = 0;
    std::size_t rejected = 0;
    std::size_t tooLong = 0;

    // A spread of ids kept back so the registration can be read out of the
    // client's own map afterwards, rather than trusted because the call
    // returned non-zero.
    std::vector<std::pair<std::uint32_t, std::string>> samples;

    for (ContentFile const& file : catalog.files) {
        if (!FitsLocalManifestPath(file)) {
            if (++tooLong <= 10)
                HOOK_LOG_WARN("casc bypass: path too long for the client, skipped: %s", file.path.c_str());
            continue;
        }

        if (CasAddLocalOverride(file.path.c_str(), ContentDirectory.c_str(), static_cast<int>(file.fileDataId), true) != 0) {
            CasAddLocalOverride(file.path.c_str(), ContentDirectory.c_str(), 0, true); // named: see above
            bool spread = registered % 20000 == 0 && samples.size() < 8;
            if (spread || IsDiagnosticFileDataId(file.fileDataId))
                samples.emplace_back(file.fileDataId, ContentDirectory + "\\" + file.path);
            ++registered;
        } else {
            ++rejected;
        }
    }

    HOOK_LOG_INFO("casc bypass: registered %zu override(s) in %llu ms (%zu rejected, %zu path(s) too long)",
                  registered,
                  GetTickCount64() - startTicks,
                  rejected,
                  tooLong);

    VerifiedSamples = std::move(samples);
}

// A root manifest listing exactly the catalogued files, in the format the
// client's parser reads (FUN_140351270 / FUN_14034d9a0): header "TSFM", size
// 20, version 1, total and named counts, then one block
//
//     uint32 count, contentFlags, localeFlags
//     int32  fileDataIdDelta[count]   // id = previous id + 1 + delta
//     uint8  ckey[count][16]
//     uint64 nameHash[count]
//
// The parser keeps a block when content flags have bit 31 clear (text, not
// speech), none of 0x191 (don't-load, low-violence, Mac, ...), and the locale
// mask includes the client's locale -- so flags 0 and every locale bit.
//
// Names and CKeys are the build's real ones where Content/root.hashes has them
// (see LoadRealRoot), otherwise derived from the path and SyntheticCKey.
//
// Every entry must have a registered override. When an open misses the local
// manifest but finds a build-manifest entry, the client reads the entry
// straight from CASC storage (FUN_14034a6e0), and in loose-only mode there is
// none -- see DetourReadFromStorage. For the same reason buildinfo.json is left
// out: CAS init reads it from storage whenever the manifest lists it, with no
// local-manifest check at all. Left out, CAS init takes the patched
// missing-build-info path and ApplyBuildInfoFromContent supplies the values.
std::vector<unsigned char> BuildRootManifest(ContentCatalog const& catalog)
{
    std::vector<ContentFile const*> listed;
    listed.reserve(catalog.files.size());
    for (ContentFile const& file : catalog.files) {
        if (file.fileDataId != BuildInfoFileDataId && FitsLocalManifestPath(file))
            listed.push_back(&file);
    }

    std::size_t const count = listed.size();
    std::vector<unsigned char> root;
    root.reserve(32 + count * (4 + 16 + 8));

    auto put32 = [&](std::uint32_t value) {
        for (int i = 0; i < 4; ++i)
            root.push_back(static_cast<unsigned char>(value >> (8 * i)));
    };
    auto put64 = [&](std::uint64_t value) {
        put32(static_cast<std::uint32_t>(value));
        put32(static_cast<std::uint32_t>(value >> 32));
    };

    root.insert(root.end(), { 'T', 'S', 'F', 'M' });
    put32(20);
    put32(1);
    put32(static_cast<std::uint32_t>(count));
    put32(static_cast<std::uint32_t>(count));

    put32(static_cast<std::uint32_t>(count));
    put32(0);          // content flags
    put32(0xFFFFFFFF); // every locale

    std::uint32_t previous = 0;
    for (std::size_t i = 0; i < count; ++i) {
        std::uint32_t id = listed[i]->fileDataId;
        put32(i == 0 ? id : id - previous - 1);
        previous = id;
    }
    for (ContentFile const* file : listed)
        root.insert(root.end(), file->ckey.begin(), file->ckey.end());
    for (ContentFile const* file : listed)
        put64(file->manifestNameHash);

    return root;
}

// Read a sample back out of the client's own FileDataId map. A hit means an
// asset request for that id resolves to our file. Run after the client's own
// working-directory scan so it reflects the map the game will actually see,
// not the map as we left it.
void VerifyRegisteredOverrides()
{
    auto const& samples = VerifiedSamples;
    if (samples.empty())
        return;

    void* container = CasContainerSlot ? *CasContainerSlot : nullptr;
    if (!FindOverrideByFileId || !container)
        return;

    auto normalized = [](std::string path) {
        std::replace(path.begin(), path.end(), '/', '\\');
        return path;
    };

    std::size_t hits = 0;
    std::size_t nameHits = 0;
    for (auto const& [fileDataId, expected] : samples) {
        char const* resolved = FindOverrideByFileId(container, static_cast<int>(fileDataId));
        bool match = resolved && _stricmp(normalized(resolved).c_str(), normalized(expected).c_str()) == 0;
        if (match)
            ++hits;
        else
            HOOK_LOG_WARN("casc bypass: readback for FileDataId %u returned '%s', expected '%s'",
                          fileDataId,
                          resolved ? resolved : "<null>",
                          expected.c_str());

        // The same file by name, as open-by-name looks it up.
        if (!FindOverrideByNameHash)
            continue;
        std::uint32_t namedId = 0;
        std::string relative = expected.substr(ContentDirectory.size() + 1);
        char const* byName = FindOverrideByNameHash(container, ClientNameHash(relative), &namedId);
        if (byName && namedId == fileDataId)
            ++nameHits;
        else
            HOOK_LOG_WARN("casc bypass: readback by name '%s' returned '%s' (FileDataId %u), expected FileDataId %u",
                          relative.c_str(),
                          byName ? byName : "<null>",
                          namedId,
                          fileDataId);
    }

    HOOK_LOG_INFO("casc bypass: readback verified %zu/%zu sampled override(s) in the client's FileDataId map, "
                  "%zu/%zu by name",
                  hits,
                  samples.size(),
                  nameHits,
                  FindOverrideByNameHash ? samples.size() : 0);
}

std::uint8_t Byte(std::span<std::byte> text, std::size_t index)
{
    return static_cast<std::uint8_t>(text[index]);
}

// Target of an E8 rel32 at `index`, or nullptr if that isn't a call.
std::byte const* CallTarget(std::span<std::byte> text, std::size_t index)
{
    if (Byte(text, index) != 0xE8)
        return nullptr;
    std::int32_t rel = ReadValue<std::int32_t>(text.data() + index + 1);
    return text.data() + index + 5 + rel;
}

// CAS has a loose-file-only mode of its own. CAS_LoadManifests takes a flag as
// its fifth argument, and when it is set the whole CASC side is skipped: no
// storage is opened, no .root is read, the build manifest is built from an
// empty buffer, and the flag is latched into a global so that a lookup missing
// the local manifest reports "Loose file not found and CAS is in loose file
// only mode" (CAS error 3) instead of going to the archives.
//
// Both callers hard-code it off, as `mov byte ptr [rsp+0x20], 0` immediately
// before calling CAS_InitializeContainer. Flipping those two immediates to 1 is
// the whole change -- the client then never touches Data/data/data.NNN.
//
// The immediate on its own appears ~1300 times in .text, so a site only counts
// when a call to CAS_InitializeContainer follows within a few bytes. That
// function is identified without a pattern of its own: it is the only caller of
// CAS_SetLooseFileDirAndScan, which we have already located, so a candidate
// call target is the right one if it calls that within its first few hundred
// bytes.
std::size_t EnableLooseFileOnlyMode(std::span<std::byte> text, std::byte const* setLooseFileDir)
{
    constexpr std::uint8_t FlagStore[] = { 0xC6, 0x44, 0x24, 0x20, 0x00 }; // mov byte [rsp+0x20], 0
    constexpr std::size_t CallSearch = 20;   // flag store -> call
    constexpr std::size_t BodySearch = 0x300; // scanned inside the callee

    std::size_t patched = 0;

    for (std::size_t i = 0; i + CallSearch + 8 < text.size(); ++i) {
        bool matches = true;
        for (std::size_t k = 0; k < sizeof(FlagStore) && matches; ++k)
            matches = Byte(text, i + k) == FlagStore[k];
        if (!matches)
            continue;

        std::byte const* callee = nullptr;
        for (std::size_t k = sizeof(FlagStore); k < CallSearch; ++k) {
            if (Byte(text, i + k) == 0xE8) {
                callee = CallTarget(text, i + k);
                break;
            }
        }

        if (!callee || callee < text.data() || callee + BodySearch >= text.data() + text.size())
            continue;

        std::size_t calleeIndex = static_cast<std::size_t>(callee - text.data());
        bool isInitializeContainer = false;
        for (std::size_t k = 0; k < BodySearch && !isInitializeContainer; ++k)
            isInitializeContainer = CallTarget(text, calleeIndex + k) == setLooseFileDir;

        if (!isInitializeContainer)
            continue;

        static constexpr std::byte One[1] = { std::byte{ 0x01 } };
        if (WriteBytes(text.data() + i + 4, One)) {
            ++patched;
            HOOK_LOG_INFO("casc bypass: loose-file-only mode enabled at WowClassic.exe+0x%llx",
                          static_cast<unsigned long long>(text.data() + i - GetProcessImage().BaseAddress));
        }
    }

    return patched;
}

// CAS_AddLocalOverride loads the container global into rcx before tail-calling
// the local-manifest insert, so its first instructions carry the only address
// we need to recognise a local-manifest lookup anywhere else in .text.
std::byte* ResolveCasContainerGlobal(std::byte* addLocalOverride)
{
    for (std::size_t i = 0; i < 0x40; ++i) {
        auto at = [&](std::size_t k) { return static_cast<std::uint8_t>(addLocalOverride[i + k]); };
        if (at(0) == 0x48 && at(1) == 0x8B && at(2) == 0x0D) { // mov rcx, [rip+rel32]
            std::int32_t rel = ReadValue<std::int32_t>(addLocalOverride + i + 3);
            return addLocalOverride + i + 7 + rel;
        }
    }
    return nullptr;
}

// Registering overrides is only half the job. Every CAS read path gates its
// local-manifest lookup on bit 0 of a flags argument, and most call sites leave
// that bit clear -- which is why registered overrides are otherwise ignored by
// ordinary asset loads. There are seven such gates, and they all compile to the
// same shape:
//
//     test  <reg>b, 1                      ; flags & 1
//     jz    <casc path>                    ; short (74 xx) or near (0F 84 xx xx xx xx)
//     [optionally: test <reg>,<reg> / jz]
//     mov   rcx, [g_CasContainer]          ; <- the local-manifest lookup
//
// Rather than hard-code six patterns -- an earlier attempt did, and silently
// missed the async path, which is the one that actually streams game assets --
// this recognises the shape: a test of bit 0 whose jz is followed within a few
// bytes by a load of the container global. Replacing the jz with NOPs makes the
// lookup unconditional. That is safe rather than a redirect, because each
// lookup block already falls through to the original CASC read when no override
// matches, so anything absent from Content/ still loads exactly as before.
std::size_t ForceLocalOverrideLookups(std::span<std::byte> text, std::byte const* container)
{
    constexpr std::size_t Lookahead = 8; // bytes between the jz and the container load
    constexpr std::size_t Tail = 32;     // keep every index below in range

    std::size_t patched = 0;

    for (std::size_t i = 0; i + Tail < text.size(); ++i) {
        std::size_t cursor = i;

        std::uint8_t first = Byte(text, cursor);
        if (first >= 0x40 && first <= 0x45) // optional REX prefix (bpl/dil/r13b/...)
            ++cursor;

        if (Byte(text, cursor) != 0xF6) // test r/m8, imm8
            continue;

        std::uint8_t modrm = Byte(text, cursor + 1);
        if (modrm < 0xC0 || modrm > 0xC7) // register form only
            continue;

        if (Byte(text, cursor + 2) != 0x01) // ... against bit 0
            continue;

        std::size_t jump = cursor + 3;
        std::size_t jumpLength = 0;
        if (Byte(text, jump) == 0x74)
            jumpLength = 2;
        else if (Byte(text, jump) == 0x0F && Byte(text, jump + 1) == 0x84)
            jumpLength = 6;
        else
            continue;

        // Only a jz that guards a container load is one of ours.
        bool guardsLookup = false;
        for (std::size_t k = jump + jumpLength; k <= jump + jumpLength + Lookahead; ++k) {
            if (Byte(text, k) != 0x48 || Byte(text, k + 1) != 0x8B || Byte(text, k + 2) != 0x0D)
                continue;

            std::int32_t rel = ReadValue<std::int32_t>(text.data() + k + 3);
            guardsLookup = (text.data() + k + 7 + rel) == container;
            break;
        }

        if (!guardsLookup)
            continue;

        std::byte nops[6];
        for (std::size_t n = 0; n < jumpLength; ++n)
            nops[n] = std::byte{ 0x90 };

        if (WriteBytes(text.data() + jump, std::span<std::byte const>(nops, jumpLength))) {
            ++patched;
            HOOK_LOG_INFO("casc bypass: forced local-manifest lookup at WowClassic.exe+0x%llx (%zu-byte jz)",
                          static_cast<unsigned long long>(text.data() + jump - GetProcessImage().BaseAddress),
                          jumpLength);
        }

        i = jump + jumpLength - 1; // ++i steps past what we just rewrote
    }

    return patched;
}

// Right after the manifests load, CAS init reads buildinfo.json (FileDataId
// 5216544) to pick up dataBuild and clientBuild -- and it reads it through the
// build manifest only, with no local-manifest check. With an empty manifest the
// read yields nothing and the client dies on
// "ERROR #134 (0x85100086) Failed to download required build info asset".
//
// The shape is:
//
//     test rbx, rbx                 ; rbx = the buffer that was never filled
//     jnz  parse                    ; got one -> parse it
//     <fatal: "Failed to download required build info asset">
//   parse:
//     <json context init, parse, read dataBuild/clientBuild, context teardown>
//     <free the buffer>
//   done:
//     mov  al, 1                    ; success
//
// Simply skipping the fatal is not enough, and was worth getting wrong once:
// control then falls into the JSON block with a null buffer and the parser
// dereferences it (ACCESS_VIOLATION at WowClassic.exe+0x710E29). Nor can the
// null path jump to the teardown, which would destroy a context that was never
// constructed. It has to jump past the whole block to `done`, so the fatal is
// replaced with a jmp there.
//
// That leaves dataBuild/clientBuild unset, which is why ApplyBuildInfoFromContent
// writes them from Content/buildinfo.json. They are not cosmetic: dataBuild is
// what the client sends when asking the server for hotfixes, and the built-in
// default (58797) is not this build's data build (58725).
std::size_t SkipMissingBuildInfoFatal(ProcessImage const& image)
{
    // WowClassic.exe+0x345992 in 1.15.6.58797 (docs/CascBypass.md).
    std::byte* guard = VerifyPatternAt(image, 0x345992,
                                       ParsePattern("48 85 DB 75 ? 4C 8D 0D ? ? ? ? 45 33 C0 33 D2 "
                                                    "B9 86 00 10 85 E8 ? ? ? ?"));
    if (!guard)
        return 0;

    // `mov al,1 / jmp / xor al,al` just past the buffer free, +0x345a48. The
    // same shape also occurs elsewhere in .text (WowClassic.exe+0x1ee66d2),
    // which is why this is pinned to the one inside this function rather than
    // scanned for.
    std::byte* done =
        VerifyPatternAt(image, 0x345a48, ParsePattern("B0 01 EB 02 32 C0 4C 8D 9C 24 90 00 00 00"));
    if (!done)
        return 0;

    std::byte* fatal = guard + 5; // past `test rbx,rbx / jnz`
    std::int32_t rel = static_cast<std::int32_t>(done - (fatal + 5));

    std::byte jump[5];
    jump[0] = std::byte{ 0xE9 };
    std::memcpy(&jump[1], &rel, sizeof(rel));
    return WriteBytes(fatal, jump) ? 1 : 0;
}

// 14,517 of the build's 156,282 FileDataIds are listed in the root manifest but
// have no data in the local storage -- Battle.net never downloaded them. The
// stock client does not read them either; it just fails softly, because the
// manifest entry still exists, so the async open succeeds and the consumer ends
// up with an empty file. With no manifest at all the open fails outright, and
// the DB2 loader turns that into a fatal:
//
//     ERROR #134 (0x85100086) Unable to open FileData ID 1375805:
//     Can't find file in build manifest        (dbfilesclient/startup_strings.db2)
//
// Every one of these loaders is shaped `if (required) fatal(...); mark_empty();`
// where mark_empty is the same state stock ends in, so removing the fatal call
// leaves the soft path behind. Three loaders raise it, found by their shared
// message rather than one pattern each: the id is loaded with
// `lea r9, [rip -> "Unable to open %s: %s"]` and the reporter is called a few
// bytes later.
//
// Nothing we could have served is hidden by this: an id present in Content/ is
// answered from the override map long before reaching here.
std::size_t SoftenMissingFileFatals(std::span<std::byte> text, std::span<std::byte> rdata)
{
    std::byte* message = FindPattern(rdata, LiteralPattern(std::string_view("Unable to open %s: %s\0", 22)));
    if (!message)
        return 0;

    constexpr std::size_t CallSearch = 30;
    std::byte const* reporter = nullptr;
    std::size_t patched = 0;

    for (std::size_t i = 0; i + CallSearch + 8 < text.size(); ++i) {
        // lea r9, [rip+disp32] -> the message
        if (Byte(text, i) != 0x4C || Byte(text, i + 1) != 0x8D || Byte(text, i + 2) != 0x0D)
            continue;
        if (text.data() + i + 7 + ReadValue<std::int32_t>(text.data() + i + 3) != message)
            continue;

        for (std::size_t k = i + 7; k < i + CallSearch; ++k) {
            std::byte const* target = CallTarget(text, k);
            if (!target)
                continue;

            // All three sites share one reporter; anything else is not ours.
            if (!reporter)
                reporter = target;
            if (target != reporter)
                break;

            static constexpr std::byte Nops[5] = { std::byte{ 0x90 }, std::byte{ 0x90 }, std::byte{ 0x90 },
                                                   std::byte{ 0x90 }, std::byte{ 0x90 } };
            if (WriteBytes(text.data() + k, Nops)) {
                ++patched;
                HOOK_LOG_INFO("casc bypass: softened missing-file fatal at WowClassic.exe+0x%llx",
                              static_cast<unsigned long long>(text.data() + k - GetProcessImage().BaseAddress));
            }
            break;
        }
    }

    return patched;
}

// The store into dataBuild sits a few bytes after the only reference to the
// "dataBuild" JSON key, as `mov [rip+disp32], eax`. clientBuild is the dword
// after it.
std::uint32_t* ResolveDataBuildGlobal(ProcessImage const& image, std::span<std::byte> text)
{
    std::span<std::byte> rdata = FindSection(image, ".rdata");
    if (rdata.empty())
        return nullptr;

    std::byte* key = FindPattern(rdata, LiteralPattern(std::string_view("dataBuild\0", 10)));
    if (!key)
        return nullptr;

    for (std::size_t i = 0; i + 64 < text.size(); ++i) {
        // lea r8, [rip+disp32] -> the key
        if (Byte(text, i) != 0x4C || Byte(text, i + 1) != 0x8D || Byte(text, i + 2) != 0x05)
            continue;
        if (text.data() + i + 7 + ReadValue<std::int32_t>(text.data() + i + 3) != key)
            continue;

        for (std::size_t k = i + 7; k < i + 56; ++k) {
            if (Byte(text, k) == 0x89 && Byte(text, k + 1) == 0x05) { // mov [rip+disp32], eax
                std::byte* global = text.data() + k + 6 + ReadValue<std::int32_t>(text.data() + k + 2);
                return reinterpret_cast<std::uint32_t*>(global);
            }
        }
    }

    return nullptr;
}

std::uint32_t* DataBuildGlobal = nullptr;

// Minimal scan for "key":<digits> -- buildinfo.json is a flat object of two
// integers, so this does not need a JSON parser.
bool ReadJsonUInt(std::string const& text, char const* key, std::uint32_t& out)
{
    std::string needle = std::string("\"") + key + "\"";
    std::size_t at = text.find(needle);
    if (at == std::string::npos)
        return false;

    at = text.find(':', at + needle.size());
    if (at == std::string::npos)
        return false;

    ++at;
    while (at < text.size() && (text[at] == ' ' || text[at] == '\t'))
        ++at;

    std::uint32_t value = 0;
    std::size_t digits = 0;
    while (at < text.size() && text[at] >= '0' && text[at] <= '9') {
        value = value * 10 + static_cast<std::uint32_t>(text[at] - '0');
        ++at;
        ++digits;
    }

    if (digits == 0)
        return false;

    out = value;
    return true;
}

void ApplyBuildInfoFromContent()
{
    if (!DataBuildGlobal || ContentDirectory.empty())
        return;

    std::vector<char> raw;
    if (!ReadWholeFile((ContentDirectory + "\\buildinfo.json").c_str(), raw))
        return;

    std::string text(raw.begin(), raw.end());
    std::uint32_t dataBuild = 0;
    std::uint32_t clientBuild = 0;

    if (ReadJsonUInt(text, "dataBuild", dataBuild))
        WriteValue(DataBuildGlobal, dataBuild);
    if (ReadJsonUInt(text, "clientBuild", clientBuild))
        WriteValue(DataBuildGlobal + 1, clientBuild);

    HOOK_LOG_INFO("casc bypass: build info from Content/buildinfo.json -- dataBuild=%u clientBuild=%u",
                  dataBuild,
                  clientBuild);
}

// Loose-file-only mode is half-finished in this build. It skips opening CASC
// and skips reading .root, but still hands the buffer it never filled to the
// build-manifest parser -- which rejects it with "build manifest is truncated"
// and takes CAS init down with a message box.
//
// So we supply the manifest it is missing, built from Content/ (see
// BuildRootManifest). It matters that it isn't empty: the build manifest is
// where the client turns a *name* into a FileDataId -- CAS_GetFileIdByName,
// open-by-name, and the working-directory scan all go through it -- so an
// empty one leaves everything that is looked up by name unresolved.
std::vector<unsigned char> RootManifest;

bool DetourLoadBuildManifest(void** buffer, unsigned long long size, void* config, void* log)
{
    bool missing = !buffer || *buffer == nullptr || size == 0;
    if (!LooseFileOnly || !missing)
        return OriginalLoadBuildManifest(buffer, size, config, log);

    RootManifest = BuildRootManifest(WaitForCatalog());
    void* substitute = RootManifest.data();
    bool ok = OriginalLoadBuildManifest(&substitute, RootManifest.size(), config, log);
    std::uint32_t listed = 0;
    std::memcpy(&listed, RootManifest.data() + 12, sizeof(listed));
    HOOK_LOG_INFO("casc bypass: supplied a build manifest of %u file(s) from Content/ (%s)",
                  listed,
                  ok ? "accepted" : "rejected");
    return ok;
}

// ---------------------------------------------------------------------------
// No CASC storage
//
// In loose-only mode CAS never creates the storage object (DAT_143638890).
// Much of CAS assumes it exists: a raw scan of .text finds 44 loads of it
// (Ghidra's xrefs list only 26 -- the rest sit in code it never analysed),
// and ten dereference it unchecked. What *is* checked everywhere is the TACT
// instance inside it (storage[0]): every TACT entry point rejects a null
// instance with "Invalid tact instance" (error 3).
//
// So loose mode gets an empty storage object: zeroed, TACT instance null,
// storage[1] pointing at a zeroed sub-object (read for a streaming-throughput
// float, 0x140345150). Every unchecked dereference then lands in our block and
// every TACT call fails cleanly. CAS_Shutdown would free it with the client's
// allocator, so the slot is cleared first.
//
// A few paths still need an answer rather than a clean failure, because
// failing means something visible:
//
//   FUN_1403499b0 / 0x140344980   "is this file downloaded?" -- the streaming
//        check. A no makes the client show a placeholder, which is what left
//        character-select backgrounds and loading-screen art blank. In loose
//        mode every manifest entry is a file on disk: always yes.
//   FUN_14034a6e0 / FUN_14034b2e0 sync read / async open, reached only when a
//        manifest entry's file can't be opened: refused and logged.
//   FUN_1403497a0  storage key as text, for WoW.mfil: formats a zero key.
alignas(16) unsigned char EmptyStorageObject[0x1000] = {};
alignas(16) unsigned char EmptyStorageSubObject[0x200] = {};

bool HasTactInstance(void const* storage)
{
    return storage && *static_cast<void* const*>(storage);
}

void InstallEmptyStorage()
{
    if (!StorageSlot || *StorageSlot)
        return;
    void* sub = EmptyStorageSubObject;
    std::memcpy(EmptyStorageObject + 8, &sub, sizeof(sub));
    *StorageSlot = EmptyStorageObject;
    HOOK_LOG_INFO("casc bypass: installed an empty CASC storage object (no TACT instance)");
}

void* DetourCasShutdown()
{
    if (StorageSlot && *StorageSlot == EmptyStorageObject)
        *StorageSlot = nullptr;
    return OriginalCasShutdown();
}

// ---------------------------------------------------------------------------
// Residency: a file in Content/ is on disk, so it is never "not downloaded"
//
// On this streaming install the client asks, before using an asset, whether
// the file is resident in local CASC storage, and asks TACT to download it if
// not. It asks about the *storage*, not about the file it will actually read:
// a file served from Content/ still counted as missing if CASC didn't have it
// locally. The client then showed a placeholder (the blank character-select
// backgrounds) and downloaded a copy it would never read (4,617 downloads on
// the login and character screens alone, and the slow world entry).
//
// So each of these questions is answered from Content/ first, in either mode,
// and only a file Content/ lacks goes to TACT. With no TACT at all (loose-only
// mode) every manifest entry is a Content/ file, so the answer is always yes.

bool InContent(std::uint32_t fileDataId)
{
    if (!CatalogReady || fileDataId == 0)
        return false;
    std::vector<ContentFile> const& files = Catalog.files;
    auto it = std::lower_bound(files.begin(), files.end(), fileDataId,
                               [](ContentFile const& file, std::uint32_t id) { return file.fileDataId < id; });
    return it != files.end() && it->fileDataId == fileDataId;
}

// The storage key is TVFS-style text from FUN_140341bb0,
// "%08X%04X:%08X<32 hex CKey>" -- locale, content flags, ':', FileDataId.
std::uint32_t KeyFileDataId(void const* key)
{
    auto const* text = static_cast<char const*>(key);
    if (!text || text[12] != ':')
        return 0;
    std::uint32_t id = 0;
    for (int i = 13; i < 21; ++i) {
        char ch = text[i];
        int nibble = (ch >= '0' && ch <= '9') ? ch - '0' : (ch >= 'A' && ch <= 'F') ? ch - 'A' + 10 : -1;
        if (nibble < 0)
            return 0;
        id = id << 4 | static_cast<std::uint32_t>(nibble);
    }
    return id;
}

bool NoTact()
{
    return StorageSlot && !HasTactInstance(*StorageSlot);
}

// 0x140344980(ckey, fileDataId, locale, contentFlags): 2 = resident.
std::uint64_t DetourQueryResidency(void const* entryKey, std::uint32_t fileDataId, std::uint32_t b, std::uint32_t c)
{
    if (InContent(fileDataId) || NoTact())
        return 2;
    return OriginalQueryResidency(entryKey, fileDataId, b, c);
}

// FUN_140349a60: "make this file resident" -- on a streaming install, start
// downloading it. Reached for manifest entries by id (0x140344c60) and by name
// (0x140344d90). A Content/ file is already resident.
std::uint64_t DetourMakeResident(void** storage, void const* key, void* callback, std::uint32_t a, std::uint32_t b)
{
    if (InContent(KeyFileDataId(key)) || !HasTactInstance(storage))
        return 1;
    return OriginalMakeResident(storage, key, callback, a, b);
}

// FUN_1403446b0(id, &residentBytes, &totalBytes): download progress of one
// file, used to track how much of a model and its dependencies has arrived.
// A Content/ file is all there, at its size on disk.
std::uint64_t DetourGetFileProgress(std::int32_t fileDataId, std::uint64_t* residentBytes, std::uint64_t* totalBytes)
{
    if (fileDataId < 0 || (!InContent(static_cast<std::uint32_t>(fileDataId)) && !NoTact()))
        return OriginalGetFileProgress(fileDataId, residentBytes, totalBytes);

    std::vector<ContentFile> const& files = Catalog.files;
    auto it = std::lower_bound(files.begin(), files.end(), static_cast<std::uint32_t>(fileDataId),
                               [](ContentFile const& file, std::uint32_t id) { return file.fileDataId < id; });
    bool present = it != files.end() && it->fileDataId == static_cast<std::uint32_t>(fileDataId);
    std::uint64_t size = present ? it->size : 0;
    if (totalBytes)
        *totalBytes = size;
    if (residentBytes)
        *residentBytes = size;
    return present ? 2 : 0;
}
std::atomic<unsigned> RefusedStorageReads{ 0 };

void LogRefusedStorageAccess(char const* what, void const* key)
{
    unsigned refused = ++RefusedStorageReads;
    if (refused > 20 && (refused & (refused - 1)) != 0)
        return;

    // The key starts with the entry's CKey, and ours begin with the FileDataId.
    std::uint32_t fileDataId = 0;
    if (key)
        std::memcpy(&fileDataId, key, sizeof(fileDataId));
    HOOK_LOG_WARN("casc bypass: no CASC storage in loose-file-only mode, refused %s of FileDataId %u (#%u) -- "
                  "in the manifest but no longer on disk?",
                  what,
                  fileDataId,
                  refused);
}

// A Content/ file being read from CASC means some path skipped the override
// lookup, and on this install it becomes a CDN request. Logged once per id,
// with the caller, so the path can be found.
std::mutex BypassMutex;
std::unordered_map<std::uint32_t, unsigned> BypassCounts;

void LogContentReadFromCasc(char const* what, std::uint32_t fileDataId, void* caller)
{
    if (!InContent(fileDataId))
        return;
    std::lock_guard<std::mutex> lock(BypassMutex);
    if (BypassCounts[fileDataId]++ != 0 || BypassCounts.size() > 200)
        return;

    // The async open only falls through to CASC when the override lookup
    // missed or the file failed to open, so say which: can we open it now?
    std::vector<ContentFile> const& files = Catalog.files;
    auto it = std::lower_bound(files.begin(), files.end(), fileDataId,
                               [](ContentFile const& file, std::uint32_t id) { return file.fileDataId < id; });
    std::string path = ContentDirectory + "\\" + it->path;
    HANDLE probe = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    DWORD probeError = probe == INVALID_HANDLE_VALUE ? GetLastError() : 0;
    if (probe != INVALID_HANDLE_VALUE)
        CloseHandle(probe);

    HOOK_LOG_WARN("casc bypass: %s of FileDataId %u (%s) went to CASC although Content/ has it "
                  "(caller WowClassic.exe+0x%llx, our open: %s, error %lu)",
                  what,
                  fileDataId,
                  it->path.c_str(),
                  static_cast<unsigned long long>(static_cast<std::byte*>(caller) - GetProcessImage().BaseAddress),
                  probeError == 0 ? "ok" : "failed",
                  static_cast<unsigned long>(probeError));
}

bool DetourReadFromStorage(void** storage, void const* key, void** buffer, std::uint32_t* size, std::uint32_t flags,
                           std::uint32_t tag)
{
    if (HasTactInstance(storage)) {
        LogContentReadFromCasc("a read", KeyFileDataId(key), _ReturnAddress());
        return OriginalReadFromStorage(storage, key, buffer, size, flags, tag);
    }
    LogRefusedStorageAccess("a read", key);
    return false;
}

bool DetourOpenFromStorageAsync(void** storage, void const* key, void const* ckey, int fileDataId, void** handle)
{
    if (HasTactInstance(storage)) {
        LogContentReadFromCasc("an async open", static_cast<std::uint32_t>(fileDataId), _ReturnAddress());
        return OriginalOpenFromStorageAsync(storage, key, ckey, fileDataId, handle);
    }
    LogRefusedStorageAccess("an async open", key);
    return false;
}

// 2 = present locally, 1 = needs a missing decryption key, 0 = not downloaded.
std::uint64_t DetourQueryStorageAvailability(void** storage, void const* key, std::uint64_t offset, std::uint64_t size)
{
    if (InContent(KeyFileDataId(key)) || !HasTactInstance(storage))
        return 2;
    return OriginalQueryStorageAvailability(storage, key, offset, size);
}

// Formats the 16-byte key at storage+0x38 as hex. With no storage, format a
// zero key instead, so WoW.mfil gets a well-formed placeholder.
void* DetourFormatStorageKey(void const* storage, void* outString)
{
    alignas(16) static unsigned char const NoStorage[0x100] = {};
    return OriginalFormatStorageKey(storage ? storage : NoStorage, outString);
}

// ---------------------------------------------------------------------------
// What the client asks for that Content/ doesn't have
//
// In loose-only mode a file is either in Content/ or it does not exist, and
// the client rarely says which file it gave up on. So every build-manifest
// lookup that misses is logged once: by FileDataId (BuildManifest_FindEntry)
// and by name (the hash lookup, 0x1403506d0). The name lookup only gets the
// hash, so CAS_HashPathKey is hooked to remember the last path each thread
// hashed.
using HashPathKeyFn = std::uint64_t(*)(char const* path);
using FindEntryByIdFn = void*(*)(void* manifest, std::uint32_t fileDataId);
using FindEntryByNameHashFn = void*(*)(void* manifest, std::uint64_t nameHash);

HashPathKeyFn OriginalHashPathKey = nullptr;
FindEntryByIdFn OriginalFindEntryById = nullptr;
FindEntryByNameHashFn OriginalFindEntryByNameHash = nullptr;

thread_local std::uint64_t LastHashedKey = 0;
thread_local char LastHashedPath[MAX_PATH] = {};

std::mutex MissMutex;
std::unordered_map<std::uint64_t, unsigned> MissCounts; // id or name hash -> times asked
constexpr std::size_t MaxLoggedMisses = 400;

// Off while the client scans its own working directory (WTF\, Logs\, ...):
// every file there is looked up by name, and none of them is game data.
std::atomic<bool> RecordMisses{ false };

void RecordMiss(std::uint64_t key, char const* what)
{
    if (!RecordMisses)
        return;
    std::lock_guard<std::mutex> lock(MissMutex);
    if (auto it = MissCounts.find(key); it != MissCounts.end()) {
        ++it->second;
        return;
    }
    if (MissCounts.size() >= MaxLoggedMisses)
        return;
    MissCounts.emplace(key, 1);
    HOOK_LOG_INFO("casc bypass: not in Content/: %s", what);
    if (MissCounts.size() == MaxLoggedMisses)
        HOOK_LOG_INFO("casc bypass: (further misses not logged)");
}

std::uint64_t DetourHashPathKey(char const* path)
{
    std::uint64_t hash = OriginalHashPathKey(path);
    LastHashedKey = hash;
    strncpy_s(LastHashedPath, path ? path : "", _TRUNCATE);
    return hash;
}

void* DetourFindEntryById(void* manifest, std::uint32_t fileDataId)
{
    void* entry = OriginalFindEntryById(manifest, fileDataId);
    // Negative ids are the client's own loose files; buildinfo.json is left
    // out of the manifest on purpose.
    if (!entry && static_cast<std::int32_t>(fileDataId) > 0 && fileDataId != BuildInfoFileDataId) {
        char text[64];
        snprintf(text, sizeof(text), "FileDataId %u", fileDataId);
        RecordMiss(fileDataId, text);
    }
    return entry;
}

void* DetourFindEntryByNameHash(void* manifest, std::uint64_t nameHash)
{
    void* entry = OriginalFindEntryByNameHash(manifest, nameHash);
    if (!entry) {
        char text[MAX_PATH + 64];
        snprintf(text,
                 sizeof(text),
                 "name '%s' (hash %016llx)",
                 nameHash == LastHashedKey ? LastHashedPath : "?",
                 static_cast<unsigned long long>(nameHash));
        RecordMiss(nameHash | (1ull << 63), text);
    }
    return entry;
}

struct StorageGuard {
    char const* name;
    std::size_t rva; // WowClassic.exe+rva in 1.15.6.58797, see docs/CascBypass.md
    char const* pattern;
    void* detour;
    void** original;
};

bool InstallStorageGuards(ProcessImage const& image)
{
    StorageGuard const guards[] = {
        { "CAS_ReadFromStorage",
          0x34a6e0,
          "40 55 57 41 54 41 56 41 57 48 81 EC D0 00 00 00 8B AC 24 20 01 00 00 "
          "4D 8B F9 4D 8B F0 4C 8B E2 48 8B F9 40 F6 C5 08",
          reinterpret_cast<void*>(&DetourReadFromStorage),
          reinterpret_cast<void**>(&OriginalReadFromStorage) },
        { "CAS_QueryStorageAvailability",
          0x3499b0,
          "48 89 5C 24 08 57 48 83 EC 20 B8 FF FF FF FF 48 8B F9 4C 3B C0 0F 87 81 00 00 00 "
          "4C 3B C8 77 7C 48 8B 09",
          reinterpret_cast<void*>(&DetourQueryStorageAvailability),
          reinterpret_cast<void**>(&OriginalQueryStorageAvailability) },
        { "CAS_OpenFromStorageAsync",
          0x34b2e0,
          "48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 81 EC B0 00 00 00 "
          "45 8B F1 4D 8B E0 48 8B F9 4C 8D 44 24 30",
          reinterpret_cast<void*>(&DetourOpenFromStorageAsync),
          reinterpret_cast<void**>(&OriginalOpenFromStorageAsync) },
        { "CAS_FormatStorageKey",
          0x3497a0,
          "48 89 5C 24 08 57 48 83 EC 50 48 8B FA 4C 8D 41 38 BA 21 00 00 00 48 8D 4C 24 20 E8",
          reinterpret_cast<void*>(&DetourFormatStorageKey),
          reinterpret_cast<void**>(&OriginalFormatStorageKey) },
        { "CAS_QueryResidency",
          0x344980,
          "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 70 44 8B 15 ? ? ? ? "
          "65 48 8B 04 25 58 00 00 00 44 89 4C 24 20",
          reinterpret_cast<void*>(&DetourQueryResidency),
          reinterpret_cast<void**>(&OriginalQueryResidency) },
        { "CAS_MakeResident",
          0x349a60,
          "48 89 74 24 20 57 48 81 EC E0 00 00 00 48 8B 09 41 8B F1 49 8B F8 48 85 C9 75 38",
          reinterpret_cast<void*>(&DetourMakeResident),
          reinterpret_cast<void**>(&OriginalMakeResident) },
        { "CAS_GetFileProgress",
          0x3446b0,
          "48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 81 EC F0 00 00 00 8B E9 4C 8B F2 48 8B 0D",
          reinterpret_cast<void*>(&DetourGetFileProgress),
          reinterpret_cast<void**>(&OriginalGetFileProgress) },
        { "CAS_Shutdown",
          0x3480a0,
          "4C 8B DC 55 48 81 EC D0 00 00 00 48 8B 05 ? ? ? ? 33 ED 49 89 43 08 48 8B 05 ? ? ? ? 90 "
          "49 89 5B 10 49 89 7B 20",
          reinterpret_cast<void*>(&DetourCasShutdown),
          reinterpret_cast<void**>(&OriginalCasShutdown) },
        { "CAS_HashPathKey",
          0x34ce80,
          "40 55 48 83 EC 20 48 8D 6C 24 20 48 89 5D 10 4C 8B C9 48 C7 C2 FF FF FF FF",
          reinterpret_cast<void*>(&DetourHashPathKey),
          reinterpret_cast<void**>(&OriginalHashPathKey) },
        { "BuildManifest_FindEntry",
          0x350750,
          "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 4C 8B 11 48 B8 25 23 22 84 E4 9C F2 CB 44 8B DA",
          reinterpret_cast<void*>(&DetourFindEntryById),
          reinterpret_cast<void**>(&OriginalFindEntryById) },
        { "BuildManifest_FindEntryByNameHash",
          0x3506d0,
          "48 89 5C 24 08 48 89 7C 24 10 4C 8B 01 4C 8B DA 90 45 33 C9 49 8B 78 28 4D 8B 50 30",
          reinterpret_cast<void*>(&DetourFindEntryByNameHash),
          reinterpret_cast<void**>(&OriginalFindEntryByNameHash) },
    };

    for (StorageGuard const& guard : guards) {
        std::byte* target = VerifyPatternAt(image, guard.rva, ParsePattern(guard.pattern));
        if (!target) {
            HOOK_LOG_ERROR("casc bypass: %s bytes at +0x%zx don't match", guard.name, guard.rva);
            return false;
        }

        // CAS_Shutdown's second rip-relative load (mov rax, [rip+disp32] at
        // +0x18) is the storage object.
        if (guard.detour == reinterpret_cast<void*>(&DetourCasShutdown))
            StorageSlot = reinterpret_cast<void**>(target + 0x1F + ReadValue<std::int32_t>(target + 0x1B));

        HookRegistration hook;
        hook.Name = guard.name;
        hook.Target = reinterpret_cast<Address>(target);
        hook.Detour = guard.detour;
        hook.Original = guard.original;
        if (!InstallHook(hook))
            return false;
    }
    return true;
}

// The client calls this once with its working directory, and it is the only
// point in CAS init that runs after the container exists and before the first
// asset is read.
//
// Order matters, and not in the obvious way: LocalManifest_ScanDirIntoManifest
// *frees every node in both maps* and resets their counters before it walks the
// directory. Registering first therefore registers into a map that is about to
// be thrown away -- which it was, silently, until a readback taken after the
// scan came back 0/9. So the client's own scan runs first and Content/ goes in
// on top of it.
//
// The consequence is that Content/ shadows a hand-placed loose file of the same
// path or FileDataId rather than the other way round. That is the right way
// round here: Content/ is the game's file system, and editing a file there is
// simpler than shadowing it from the working directory.
void DetourSetLooseFileDir(char const* directory)
{
    HOOK_LOG_INFO("casc bypass: CAS init reached the loose-file stage (client scan dir '%s')",
                  directory ? directory : "<none>");

    // Before anything after CAS init can reach for the storage.
    if (LooseFileOnly)
        InstallEmptyStorage();

    if (OriginalSetLooseFileDir)
        OriginalSetLooseFileDir(directory);

    RegisterContentOverrides();
    VerifyRegisteredOverrides();
    RecordMisses = LooseFileOnly;

    // Before CAS init goes on to read buildinfo.json, which it cannot do with
    // an empty build manifest.
    if (LooseFileOnly)
        ApplyBuildInfoFromContent();
}


bool InstallCascBypassHook()
{
    ProcessImage image = GetProcessImage();
    std::span<std::byte> text = FindSection(image, ".text");
    if (text.empty()) {
        HOOK_LOG_ERROR("casc bypass: .text section not found");
        return false;
    }

    // CAS_AddLocalOverride, WowClassic.exe+0x3443e0 in 1.15.6.58797 (docs/CascBypass.md).
    std::byte* addOverride =
        VerifyPatternAt(image, 0x3443e0,
                        ParsePattern("48 89 5C 24 08 57 48 83 EC 50 41 8B C0 48 C7 44 24 30 00 00 00 00 "
                                     "4C 8D 44 24 30 4C 89 44 24 28 48 8D 3D ? ? ? ?"));
    if (!addOverride) {
        HOOK_LOG_ERROR("casc bypass: CAS_AddLocalOverride bytes at +0x3443e0 don't match");
        return false;
    }

    // CAS_SetLooseFileDirAndScan, WowClassic.exe+0x34aa10 in 1.15.6.58797.
    std::byte* setLooseFileDir =
        VerifyPatternAt(image, 0x34aa10,
                        ParsePattern("48 81 EC 38 01 00 00 48 85 C9 74 ? 80 39 00 74 ? 4C 8B C9 45 33 C0 "
                                     "48 8D 4C 24 20 BA 04 01 00 00 E8 ? ? ? ? 85 C0 75 ?"));
    if (!setLooseFileDir) {
        HOOK_LOG_ERROR("casc bypass: CAS_SetLooseFileDirAndScan bytes at +0x34aa10 don't match");
        return false;
    }

    CasAddLocalOverride = reinterpret_cast<CasAddLocalOverrideFn>(addOverride);
    ContentDirectory = ResolveContentDirectory();

    {
        std::string arguments;
        for (std::wstring const& argument : DllArguments()) {
            char narrow[MAX_PATH * 2] = {};
            WideCharToMultiByte(CP_ACP, 0, argument.c_str(), -1, narrow, sizeof(narrow), nullptr, nullptr);
            arguments += arguments.empty() ? "" : " ";
            arguments += narrow;
        }
        HOOK_LOG_INFO("casc bypass: options seen: %s", arguments.empty() ? "<none>" : arguments.c_str());
    }

    if (!ClientNameHashSelfTest()) {
        HOOK_LOG_ERROR("casc bypass: name hash self-test failed, lookups by name would all miss");
        return false;
    }

    // Walks Content/ while the client starts up; joined when first needed.
    if (!ContentDirectory.empty())
        CatalogThread = std::jthread([] { Catalog = BuildContentCatalog(); });

    std::byte* container = ResolveCasContainerGlobal(addOverride);
    if (!container) {
        HOOK_LOG_ERROR("casc bypass: could not resolve the CAS container global");
        return false;
    }
    CasContainerSlot = reinterpret_cast<void**>(container);

    // LocalManifest_FindOverrideByFileId, WowClassic.exe+0x350830. Only used to
    // read registrations back; not fatal if the shape changed.
    if (std::byte* findById = VerifyPatternAt(image, 0x350830,
                                              ParsePattern("48 89 5C 24 08 57 48 83 EC 20 48 8B F9 48 63 DA "
                                                           "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 97 30 01 00 00")))
        FindOverrideByFileId = reinterpret_cast<FindOverrideByFileIdFn>(findById);
    else
        HOOK_LOG_WARN("casc bypass: override readback unavailable (bytes at +0x350830 don't match)");

    // LocalManifest_FindOverrideByNameHash, WowClassic.exe+0x3508f0 in 1.15.6.58797 (docs/CascBypass.md).
    if (std::byte* findByName = VerifyPatternAt(image, 0x3508f0,
                                            ParsePattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F1 "
                                                         "49 8B F8 48 8D 0D ? ? ? ? 48 8B DA E8 ? ? ? ? "
                                                         "4C 8B 86 10 01 00 00")))
        FindOverrideByNameHash = reinterpret_cast<FindOverrideByNameHashFn>(findByName);
    else
        HOOK_LOG_WARN("casc bypass: by-name readback unavailable (bytes at +0x3508f0 don't match)");

    // Seven in 1.15.6.58797: the synchronous open-by-id, open-by-id-bool and
    // open-by-name paths, the async streaming open, name->FileDataId lookup and
    // two smaller helpers. Treated as a hard requirement: if the client changes
    // shape, failing loudly beats silently reading half the game out of CASC.
    constexpr std::size_t ExpectedGates = 7;
    std::size_t gates = ForceLocalOverrideLookups(text, container);
    if (gates != ExpectedGates) {
        HOOK_LOG_ERROR("casc bypass: unlocked %zu local-manifest lookup(s), expected %zu -- "
                       "some asset reads would still come from CASC",
                       gates,
                       ExpectedGates);
        return false;
    }

    HOOK_LOG_INFO("casc bypass: CAS_AddLocalOverride at WowClassic.exe+0x%llx, "
                  "CAS_SetLooseFileDirAndScan at WowClassic.exe+0x%llx, content dir '%s'",
                  static_cast<unsigned long long>(addOverride - image.BaseAddress),
                  static_cast<unsigned long long>(setLooseFileDir - image.BaseAddress),
                  ContentDirectory.empty() ? "<unresolved>" : ContentDirectory.c_str());

    // The residency answers (see "Residency" above) apply in both modes, and
    // the storage guards are harmless while a real storage exists.
    if (!InstallStorageGuards(image))
        return false;

    // Default: CAS's own loose-file-only mode. No storage, no TACT, a build
    // manifest generated from Content/; Data/ is never opened and may be
    // absent. (The black login screen this mode once had went away with the
    // by-name registration fix, see RegisterContentOverrides.)
    //
    // --casc keeps a real CASC storage and TACT instance underneath Content/
    // instead: every file in Content/ is still read from disk and reported as
    // resident, and TACT only fetches what Content/ lacks. Needs Data/.
    // (--looseonly is accepted as a no-op.)
    if (HasCommandLineFlag(L"--casc")) {
        HOOK_LOG_INFO("casc bypass: --casc: CASC underneath Content/");
    } else {
        // Build-manifest loader, WowClassic.exe+0x34a460 in 1.15.6.58797.
        std::byte* loadManifest =
            VerifyPatternAt(image, 0x34a460,
                            ParsePattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 "
                                         "49 8B E9 C7 44 24 28 00 00 00 00"));
        if (!loadManifest) {
            HOOK_LOG_ERROR("casc bypass: build-manifest loader bytes at +0x34a460 don't match");
            return false;
        }

        constexpr std::size_t ExpectedSites = 2;
        std::size_t sites = EnableLooseFileOnlyMode(text, setLooseFileDir);
        if (sites != ExpectedSites) {
            HOOK_LOG_ERROR("casc bypass: enabled loose-file-only mode at %zu site(s), expected %zu",
                           sites,
                           ExpectedSites);
            return false;
        }

        if (SkipMissingBuildInfoFatal(image) != 1) {
            HOOK_LOG_ERROR("casc bypass: could not neutralise the missing-build-info fatal");
            return false;
        }

        constexpr std::size_t ExpectedFatals = 3;
        std::size_t fatals = SoftenMissingFileFatals(text, FindSection(image, ".rdata"));
        if (fatals != ExpectedFatals) {
            HOOK_LOG_ERROR("casc bypass: softened %zu missing-file fatal(s), expected %zu",
                           fatals,
                           ExpectedFatals);
            return false;
        }

        DataBuildGlobal = ResolveDataBuildGlobal(image, text);
        if (!DataBuildGlobal)
            HOOK_LOG_WARN("casc bypass: dataBuild global not found, the client will use its built-in "
                          "default build numbers");

        HookRegistration manifestHook;
        manifestHook.Name = "CAS_LoadBuildManifest";
        manifestHook.Target = reinterpret_cast<Address>(loadManifest);
        manifestHook.Detour = reinterpret_cast<void*>(&DetourLoadBuildManifest);
        manifestHook.Original = reinterpret_cast<void**>(&OriginalLoadBuildManifest);
        if (!InstallHook(manifestHook))
            return false;

        LooseFileOnly = true;
    }

    HookRegistration hook;
    hook.Name = "CAS_SetLooseFileDirAndScan";
    hook.Target = reinterpret_cast<Address>(setLooseFileDir);
    hook.Detour = reinterpret_cast<void*>(&DetourSetLooseFileDir);
    hook.Original = reinterpret_cast<void**>(&OriginalSetLooseFileDir);
    return InstallHook(hook);
}
