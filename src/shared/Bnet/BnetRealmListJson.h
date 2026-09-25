#ifndef SHARED_BNETREALMLISTJSON_H
#define SHARED_BNETREALMLISTJSON_H

#include "nonstd/optional.hpp"
#include <nlohmann/json.hpp>
#include <string>

// The "name:json" attribute format Battle.net RPC's GameUtilitiesService carries: plain in a
// request (Param_Identity, Param_ClientInfo), zlib-compressed with a length prefix in a
// response (Param_RealmList, Param_ServerAddresses) -- [u32 LE uncompressedLength]
// [zlib_compress(name + ":" + json.dump() + '\0')]. The name prefix isn't validated by either
// side on decode, only used to label the blob for whoever reads a packet log.
class BnetRealmListJson
{
    public:
        static std::string EncodePlain(std::string const& name, nlohmann::json const& json);
        static nonstd::optional<nlohmann::json> DecodePlain(std::string const& text);

        // nullopt only if zlib compression itself fails.
        static nonstd::optional<std::string> EncodeCompressed(std::string const& name, nlohmann::json const& json);
        static nonstd::optional<nlohmann::json> DecodeCompressed(std::string const& blob);
};

#endif
