#include "Crypto/Encoding/Base64.h"

#include <catch_amalgamated.hpp>

TEST_CASE("Base64 encodes the RFC 4648 test vectors", "[base64]")
{
    CHECK(Base64::Encode(std::string()) == "");
    CHECK(Base64::Encode(std::string("f")) == "Zg==");
    CHECK(Base64::Encode(std::string("fo")) == "Zm8=");
    CHECK(Base64::Encode(std::string("foo")) == "Zm9v");
    CHECK(Base64::Encode(std::string("foob")) == "Zm9vYg==");
    CHECK(Base64::Encode(std::string("fooba")) == "Zm9vYmE=");
    CHECK(Base64::Encode(std::string("foobar")) == "Zm9vYmFy");
}

TEST_CASE("Base64 decodes what it encodes, for every length", "[base64]")
{
    std::vector<uint8> data;
    for (size_t size = 0; size < 40; ++size)
    {
        std::string const encoded = Base64::Encode(data.data(), data.size());
        nonstd::optional<std::vector<uint8>> decoded = Base64::Decode(encoded);
        REQUIRE(decoded.has_value());
        CHECK(*decoded == data);
        data.push_back(uint8(size * 37 + 200));
    }
}

TEST_CASE("Base64 rejects malformed input", "[base64]")
{
    CHECK_FALSE(Base64::Decode("Zg=").has_value());
    CHECK_FALSE(Base64::Decode("Zg=a").has_value());
    CHECK_FALSE(Base64::Decode("=Zg=").has_value());
    CHECK_FALSE(Base64::Decode("Zm9v!A==").has_value());
    CHECK_FALSE(Base64::Decode("Zg==Zg==").has_value());
}
