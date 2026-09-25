#include "Base64.h"

namespace
{
constexpr char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int DecodeChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    return -1;
}
}

std::string Base64::Encode(uint8 const* data, size_t size)
{
    std::string out;
    out.reserve((size + 2) / 3 * 4);

    size_t i = 0;
    for (; i + 2 < size; i += 3)
    {
        uint32 const block = (uint32(data[i]) << 16) | (uint32(data[i + 1]) << 8) | data[i + 2];
        out += ALPHABET[(block >> 18) & 0x3F];
        out += ALPHABET[(block >> 12) & 0x3F];
        out += ALPHABET[(block >> 6) & 0x3F];
        out += ALPHABET[block & 0x3F];
    }

    if (size - i == 1)
    {
        uint32 const block = uint32(data[i]) << 16;
        out += ALPHABET[(block >> 18) & 0x3F];
        out += ALPHABET[(block >> 12) & 0x3F];
        out += "==";
    }
    else if (size - i == 2)
    {
        uint32 const block = (uint32(data[i]) << 16) | (uint32(data[i + 1]) << 8);
        out += ALPHABET[(block >> 18) & 0x3F];
        out += ALPHABET[(block >> 12) & 0x3F];
        out += ALPHABET[(block >> 6) & 0x3F];
        out += '=';
    }
    return out;
}

std::string Base64::Encode(std::string const& data)
{
    return Encode(reinterpret_cast<uint8 const*>(data.data()), data.size());
}

nonstd::optional<std::vector<uint8>> Base64::Decode(std::string const& text)
{
    if (text.size() % 4 != 0)
        return nonstd::nullopt;

    std::vector<uint8> out;
    out.reserve(text.size() / 4 * 3);

    for (size_t i = 0; i < text.size(); i += 4)
    {
        bool const last = i + 4 == text.size();
        int values[4];
        size_t padding = 0;
        for (size_t j = 0; j < 4; ++j)
        {
            char const c = text[i + j];
            if (c == '=')
            {
                if (!last || j < 2)
                    return nonstd::nullopt;
                ++padding;
                values[j] = 0;
                continue;
            }
            if (padding)
                return nonstd::nullopt;
            values[j] = DecodeChar(c);
            if (values[j] < 0)
                return nonstd::nullopt;
        }

        uint32 const block = (uint32(values[0]) << 18) | (uint32(values[1]) << 12) | (uint32(values[2]) << 6) | uint32(values[3]);
        out.push_back(uint8(block >> 16));
        if (padding < 2)
            out.push_back(uint8(block >> 8));
        if (padding < 1)
            out.push_back(uint8(block));
    }
    return out;
}
