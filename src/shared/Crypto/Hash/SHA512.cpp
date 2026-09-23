#include "SHA512.h"

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/evp.h>
#else
#include <openssl/sha.h>
#endif

#include "Crypto/BigNumber.h"
#include "Errors.h"

SHA512::Digest SHA512::ComputeFrom(std::vector<uint8> const& data)
{
    Generator generator;
    generator.UpdateData(data);
    return generator.GetDigest();
}

SHA512::Digest SHA512::ComputeFrom(std::string const& data)
{
    Generator generator;
    generator.UpdateData(data);
    return generator.GetDigest();
}

SHA512::Digest SHA512::ComputeFrom(BigNumber const& data)
{
    Generator generator;
    generator.UpdateData(data);
    return generator.GetDigest();
}

SHA512::Digest SHA512::ComputeFrom(uint8 const* data, size_t length)
{
    Generator generator;
    generator.UpdateData(data, length);
    return generator.GetDigest();
}

SHA512::Generator::Generator()
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    m_ctx = EVP_MD_CTX_new();
    MANGOS_ASSERT(m_ctx != nullptr);
    MANGOS_ASSERT(EVP_DigestInit_ex(m_ctx, EVP_sha512(), nullptr) == 1);
#else
    m_ctx = new SHA512_CTX;
    SHA512_Init(m_ctx);
#endif
}

SHA512::Generator::~Generator()
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_MD_CTX_free(m_ctx);
#else
    delete m_ctx;
#endif
}

void SHA512::Generator::UpdateData(std::vector<uint8> const& data)
{
    UpdateData(data.data(), data.size());
}

void SHA512::Generator::UpdateData(std::string const& data)
{
    UpdateData(reinterpret_cast<uint8 const*>(data.c_str()), data.size());
}

void SHA512::Generator::UpdateData(BigNumber const& data)
{
    std::vector<uint8> bytes = data.AsByteArray();
    UpdateData(bytes);
}

void SHA512::Generator::UpdateData(uint8 const* data, size_t length)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    MANGOS_ASSERT(EVP_DigestUpdate(m_ctx, data, length) == 1);
#else
    SHA512_Update(m_ctx, data, length);
#endif
}

SHA512::Digest SHA512::Generator::GetDigest()
{
    Digest digest;
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    uint32 length = 0;
    MANGOS_ASSERT(EVP_DigestFinal_ex(m_ctx, digest.data(), &length) == 1);
    MANGOS_ASSERT(length == digest.size());
#else
    SHA512_Final(digest.data(), m_ctx);
#endif
    return digest;
}
