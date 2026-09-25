#ifndef _MANGOS_CRYPTO_HASH_HMACSHA256_H
#define _MANGOS_CRYPTO_HASH_HMACSHA256_H

#include "Platform/Define.h"

#include <array>
#include <string>
#include <vector>

#include "SHA256.h"

#include <openssl/opensslv.h>

class BigNumber;

typedef struct hmac_ctx_st HMAC_CTX;

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
typedef struct evp_mac_st EVP_MAC;
typedef struct evp_mac_ctx_st EVP_MAC_CTX;
#endif

class HMACSHA256
{
    public:
        typedef SHA256::Digest Digest;

        static constexpr Digest CreateEmpty() { return {}; }

        class Generator
        {
            public:
                Generator(std::vector<uint8> const& key);
                Generator(uint8 const* key, size_t len);
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
                EVP_MAC* m_mac;
                EVP_MAC_CTX* m_ctx;
#else
                HMAC_CTX* m_ctx;
#endif
        };
};

#endif
