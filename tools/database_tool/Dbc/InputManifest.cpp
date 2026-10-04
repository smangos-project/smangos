#include "InputManifest.h"

#include <openssl/evp.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

std::string Sha256File(std::filesystem::path const& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return {};

    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr);
    std::vector<char> buffer(1 << 20);
    while (in)
    {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        if (in.gcount() > 0)
            EVP_DigestUpdate(ctx.get(), buffer.data(), static_cast<size_t>(in.gcount()));
    }

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    EVP_DigestFinal_ex(ctx.get(), digest, &length);

    std::string hex;
    char byte[3];
    for (unsigned int i = 0; i < length; ++i)
    {
        std::snprintf(byte, sizeof(byte), "%02x", digest[i]);
        hex += byte;
    }
    return hex;
}

std::string WriteInputManifest(std::filesystem::path const& dir, std::string const& extension,
                               std::filesystem::path const& manifestPath)
{
    std::map<std::string, std::string> hashes;
    std::error_code ec;
    for (auto const& entry : std::filesystem::directory_iterator(dir, ec))
    {
        if (!entry.is_regular_file() || entry.path().extension() != extension)
            continue;
        std::string hash = Sha256File(entry.path());
        if (hash.empty())
            return "cannot read " + entry.path().string();
        hashes[entry.path().filename().string()] = std::move(hash);
    }
    if (ec)
        return "cannot list " + dir.string() + ": " + ec.message();

    std::ofstream out(manifestPath, std::ios::binary | std::ios::trunc);
    if (!out)
        return "cannot write " + manifestPath.string();
    for (auto const& [name, hash] : hashes)
        out << hash << "  " << name << '\n';
    return {};
}

std::string VerifyInputManifest(std::filesystem::path const& dir, std::filesystem::path const& manifestPath)
{
    std::ifstream in(manifestPath);
    if (!in)
        return "cannot read input manifest " + manifestPath.string();

    std::string line;
    while (std::getline(in, line))
    {
        size_t const split = line.find("  ");
        if (split == std::string::npos)
            continue;
        std::string const expected = line.substr(0, split);
        std::string const name = line.substr(split + 2);

        std::filesystem::path const file = dir / name;
        if (!std::filesystem::exists(file))
            continue;
        if (Sha256File(file) != expected)
            return "input " + file.string() + " does not match " + manifestPath.filename().string()
                + " (not a stock file)";
    }
    return {};
}

std::filesystem::path BuildStockClientView(std::filesystem::path const& clientDir,
                                           std::filesystem::path const& stockDir,
                                           std::filesystem::path const& viewDir)
{
    std::error_code ec;
    std::filesystem::remove_all(viewDir, ec);
    std::filesystem::create_directories(viewDir, ec);

    auto link = [&](std::filesystem::path const& dir, bool overwrite)
    {
        for (auto const& entry : std::filesystem::directory_iterator(dir, ec))
        {
            if (!entry.is_regular_file() || entry.path().extension() != ".db2")
                continue;
            std::filesystem::path const target = viewDir / entry.path().filename();
            if (overwrite)
                std::filesystem::remove(target, ec);
            if (!std::filesystem::exists(std::filesystem::symlink_status(target)))
                std::filesystem::create_symlink(std::filesystem::absolute(entry.path()), target, ec);
        }
    };
    link(stockDir, false);
    link(clientDir, false);
    return viewDir;
}
