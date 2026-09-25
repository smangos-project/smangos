#ifndef SHARED_CRYPTO_ENCODING_BASE64_H
#define SHARED_CRYPTO_ENCODING_BASE64_H

#include "Platform/Define.h"
#include "nonstd/optional.hpp"

#include <string>
#include <vector>

// RFC 4648 base64 with the standard alphabet and '=' padding.
class Base64
{
    public:
        static std::string Encode(uint8 const* data, size_t size);
        static std::string Encode(std::string const& data);

        // nullopt on any character outside the alphabet or bad padding.
        static nonstd::optional<std::vector<uint8>> Decode(std::string const& text);
};

#endif
