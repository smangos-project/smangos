#include "BnetRealmListJson.h"
#include "Common.h"
#include "Compression/ZLib.h"
#include <cstring>

namespace
{
nonstd::optional<nlohmann::json> ParseNamedJson(std::string const& text)
{
    size_t const colon = text.find(':');
    if (colon == std::string::npos)
        return nonstd::nullopt;

    std::string jsonPart = text.substr(colon + 1);
    if (!jsonPart.empty() && jsonPart.back() == '\0')
        jsonPart.pop_back();

    try
    {
        return nlohmann::json::parse(jsonPart);
    }
    catch (nlohmann::json::exception const&)
    {
        return nonstd::nullopt;
    }
}
}

std::string BnetRealmListJson::EncodePlain(std::string const& name, nlohmann::json const& json)
{
    return name + ":" + json.dump();
}

nonstd::optional<nlohmann::json> BnetRealmListJson::DecodePlain(std::string const& text)
{
    return ParseNamedJson(text);
}

nonstd::optional<std::string> BnetRealmListJson::EncodeCompressed(std::string const& name, nlohmann::json const& json)
{
    std::string const plain = EncodePlain(name, json) + '\0';
    std::vector<uint8> const plainBytes(plain.begin(), plain.end());

    nonstd::optional<std::vector<uint8>> compressed = Compression::ZLib::Compress(plainBytes);
    if (!compressed)
        return nonstd::nullopt;

    uint32 const uncompressedLen = uint32(plainBytes.size());
    std::string out(sizeof(uncompressedLen), '\0');
    std::memcpy(&out[0], &uncompressedLen, sizeof(uncompressedLen));
    out.append(reinterpret_cast<char const*>(compressed->data()), compressed->size());
    return out;
}

nonstd::optional<nlohmann::json> BnetRealmListJson::DecodeCompressed(std::string const& blob)
{
    if (blob.size() < 4)
        return nonstd::nullopt;

    uint32 uncompressedLen;
    std::memcpy(&uncompressedLen, blob.data(), sizeof(uncompressedLen));

    std::vector<uint8> const compressed(blob.begin() + 4, blob.end());
    nonstd::optional<std::vector<uint8>> decompressed = Compression::ZLib::Decompress(compressed, uncompressedLen);
    if (!decompressed)
        return nonstd::nullopt;

    std::string const plain(reinterpret_cast<char const*>(decompressed->data()), decompressed->size());
    return ParseNamedJson(plain);
}
