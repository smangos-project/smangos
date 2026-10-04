#include "WorldArchive.h"

#include "ZipExtract.h"

#include "Extract7z.h"

#include "Crypto/Hash/SHA256.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace
{
    // Shells out to the curl CLI rather than linking libcurl: this project's
    // libcurl is an optional Windows dependency that needs a manual source
    // build (dep/windows/optional_dependencies/curl_download_and_build.bat),
    // while the curl executable ships with Windows itself (since 10 1803)
    // and with essentially every *nix.
    bool DownloadFile(std::string const& url, std::filesystem::path const& destination, std::string& error)
    {
        std::filesystem::path tempPath = destination;
        tempPath += ".part";
        std::filesystem::remove(tempPath);

        std::ostringstream command;
        command << "curl -fsSL -o \"" << tempPath.string() << "\" \"" << url << "\"";
        int exitCode = std::system(command.str().c_str());
        if (exitCode != 0)
        {
            std::filesystem::remove(tempPath);
            error = "curl exited with status " + std::to_string(exitCode);
            return false;
        }

        std::error_code ec;
        std::filesystem::rename(tempPath, destination, ec);
        if (ec)
        {
            error = "cannot finalize download: " + ec.message();
            return false;
        }
        return true;
    }

    std::string Sha256Hex(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        SHA256::Generator generator;
        std::array<char, 1 << 16> buffer;
        while (file)
        {
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0)
                generator.UpdateData(reinterpret_cast<uint8 const*>(buffer.data()), static_cast<size_t>(bytesRead));
        }

        SHA256::Digest digest = generator.GetDigest();
        static char const kHex[] = "0123456789abcdef";
        std::string hex;
        hex.reserve(digest.size() * 2);
        for (uint8 byte : digest)
        {
            hex += kHex[byte >> 4];
            hex += kHex[byte & 0xF];
        }
        return hex;
    }
}

WorldArchiveSpec const& WorldBaseDumpArchiveSpec()
{
    // db_latest is replaced on every upstream push; when the asset below is gone, pin the new
    // one (`database_tool pin-base-dump`) or point --base-dump at a mirrored copy.
    static WorldArchiveSpec const spec{
        "https://github.com/vmangos/core/releases/download/db_latest/db-4641790.zip",
        "http://127.0.0.1:3000/smangos-project/smangos/releases/download/vmangos-db-4641790/db-4641790.zip",
        "db-4641790.zip",
        "mangos.sql",
        "mysql-dump/mangos.sql",
        30511447,
        "9e178f1d70363e92a7b8c259000102dc9739333d79ca00c99ea32dd1fb43e904"
    };
    return spec;
}

WorldArchiveResult FetchAndUnpackWorldArchive(WorldArchiveSpec const& spec, std::filesystem::path const& cacheDirectory)
{
    WorldArchiveResult result;

    std::error_code ec;
    std::filesystem::create_directories(cacheDirectory, ec);

    std::filesystem::path archivePath = cacheDirectory / spec.archiveFileName;
    std::filesystem::path sqlPath = cacheDirectory / spec.unpackedFileName;

    if (!std::filesystem::exists(archivePath))
    {
        if (!DownloadFile(spec.url, archivePath, result.error))
        {
            if (spec.mirrorUrl.empty() || !DownloadFile(spec.mirrorUrl, archivePath, result.error))
                return result;
            result.error.clear();
        }
        result.downloaded = true;
    }

    uintmax_t actualSize = std::filesystem::file_size(archivePath, ec);
    if (ec || actualSize != spec.expectedSize)
    {
        result.error = "archive size mismatch: expected " + std::to_string(spec.expectedSize) +
            ", got " + std::to_string(ec ? 0 : actualSize);
        return result;
    }

    std::string actualHash = Sha256Hex(archivePath);
    if (actualHash != spec.expectedSha256Hex)
    {
        result.error = "archive SHA256 mismatch: expected " + spec.expectedSha256Hex + ", got " + actualHash;
        return result;
    }

    if (!spec.entryName.empty())
    {
        result.error = ExtractZipEntry(archivePath, spec.entryName, sqlPath);
        if (!result.error.empty())
            return result;
        result.sqlPath = sqlPath;
        return result;
    }

    char errorBuffer[256] = {};
    if (Lzmasdk_Extract7z(archivePath.string().c_str(), sqlPath.string().c_str(), errorBuffer, sizeof(errorBuffer)) != 0)
    {
        result.error = errorBuffer[0] ? errorBuffer : "extraction failed";
        return result;
    }

    result.sqlPath = sqlPath;
    return result;
}
