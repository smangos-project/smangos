#ifndef _MANGOS_CRYPTO_HASH_SHA512_H
#define _MANGOS_CRYPTO_HASH_SHA512_H

#include "Platform/Define.h"

#include <array>
#include <string>
#include <vector>

#include <openssl/opensslv.h>

class BigNumber;

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
typedef struct evp_md_ctx_st EVP_MD_CTX;
#else
typedef struct SHA512state_st SHA512_CTX;
#endif

class SHA512
{
    public:
        struct Digest : std::array<uint8, 64>
        {
            static constexpr size_t size() { return 64; }
        };

        static constexpr Digest CreateZero() { return {}; }

        template <std::size_t N>
        static Digest ComputeFrom(std::array<uint8, N> const& data) { return ComputeFrom(data.data(), data.size()); }
        static Digest ComputeFrom(std::vector<uint8> const& data);
        static Digest ComputeFrom(std::string const& data);
        static Digest ComputeFrom(BigNumber const& data);
        static Digest ComputeFrom(uint8 const* data, size_t length);

        class Generator
        {
            public:
                Generator();
                ~Generator();

                template <std::size_t N>
                void UpdateData(std::array<uint8, N> const& data) { UpdateData(data.data(), data.size()); }
                void UpdateData(std::vector<uint8> const& data);
                void UpdateData(std::string const& data);
                void UpdateData(BigNumber const& data);
                void UpdateData(uint8 const* data, size_t length);

                Digest GetDigest();

            private:
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
                EVP_MD_CTX* m_ctx;
#else
                SHA512_CTX* m_ctx;
#endif
        };
};

#endif
