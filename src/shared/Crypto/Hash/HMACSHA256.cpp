#include "HMACSHA256.h"
#include "../BigNumber.h"
#include "Errors.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>

HMACSHA256::Generator::Generator(std::vector<uint8> const& key)
    : Generator(key.data(), key.size())
{
    // use base ctor
}

HMACSHA256::Generator::Generator(uint8 const* key, size_t len)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    // OpenSSL 3.x
    m_mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    MANGOS_ASSERT(m_mac != nullptr);
    m_ctx = EVP_MAC_CTX_new(m_mac);
    MANGOS_ASSERT(m_ctx != nullptr);

    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>("SHA256"), 0);
    params[1] = OSSL_PARAM_construct_end();

    MANGOS_ASSERT(EVP_MAC_init(m_ctx, key, len, params) == 1);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
    // OpenSSL 1.0.x
    m_ctx = new HMAC_CTX;
    HMAC_CTX_init(m_ctx);
    HMAC_Init_ex(m_ctx, key, static_cast<int>(len), EVP_sha256(), nullptr);
#else
    // OpenSSL 1.1.x
    m_ctx = HMAC_CTX_new();
    HMAC_Init_ex(m_ctx, key, static_cast<int>(len), EVP_sha256(), nullptr);
#endif
}

HMACSHA256::Generator::~Generator()
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    // OpenSSL 3.x
    EVP_MAC_CTX_free(m_ctx);
    EVP_MAC_free(m_mac);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
    // OpenSSL 1.0.x
    HMAC_CTX_cleanup(m_ctx);
    delete m_ctx;
#else
    // OpenSSL 1.1.x
    HMAC_CTX_free(m_ctx);
#endif
}

void HMACSHA256::Generator::UpdateData(std::vector<uint8> const& data)
{
    UpdateData(data.data(), data.size());
}

void HMACSHA256::Generator::UpdateData(std::string const& str)
{
    UpdateData(reinterpret_cast<uint8 const*>(str.c_str()), str.length());
}

void HMACSHA256::Generator::UpdateData(BigNumber const& bn)
{
    UpdateData(bn.AsByteArray());
}

void HMACSHA256::Generator::UpdateData(uint8 const* data, size_t length)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    MANGOS_ASSERT(EVP_MAC_update(m_ctx, data, length) == 1);
#else
    HMAC_Update(m_ctx, data, length);
#endif
}

HMACSHA256::Digest HMACSHA256::Generator::GetDigest()
{
    Digest digest;
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    size_t length = 0;
    MANGOS_ASSERT(EVP_MAC_final(m_ctx, digest.data(), &length, digest.size()) == 1);
#else
    uint32 length = 0;
    HMAC_Final(m_ctx, digest.data(), &length);
#endif
    MANGOS_ASSERT(length == digest.size());
    return digest;
}
