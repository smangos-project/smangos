#include "AES_GCM.h"
#include "Errors.h"

#include <openssl/evp.h>
#include <limits>

AES_GCM::AES_GCM(bool encrypting) : m_ctx(EVP_CIPHER_CTX_new()), m_encrypting(encrypting)
{
    // EVP_CIPHER_CTX_new() already returns a freshly-initialized context -- the explicit
    // EVP_CIPHER_CTX_init() call TrinityCore's AES.cpp makes here is a pre-1.1 OpenSSL relic
    // (needed only for stack-allocated, not heap-allocated-via-_new(), contexts) and the
    // symbol is gone entirely in OpenSSL 3.x.
    int status = EVP_CipherInit_ex(m_ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr, m_encrypting ? 1 : 0);
    MANGOS_ASSERT(status);
}

AES_GCM::~AES_GCM()
{
    EVP_CIPHER_CTX_free(m_ctx);
}

void AES_GCM::Init(Key const& key)
{
    int status = EVP_CipherInit_ex(m_ctx, nullptr, nullptr, key.data(), nullptr, -1);
    MANGOS_ASSERT(status);
}

bool AES_GCM::Process(IV const& iv, uint8* data, size_t length, Tag& tag)
{
    MANGOS_ASSERT(length <= static_cast<size_t>(std::numeric_limits<int>::max()));
    int len = static_cast<int>(length);
    if (!EVP_CipherInit_ex(m_ctx, nullptr, nullptr, nullptr, iv.data(), -1))
        return false;

    int outLen;
    if (!EVP_CipherUpdate(m_ctx, data, &outLen, data, len))
        return false;

    len -= outLen;

    if (!m_encrypting && !EVP_CIPHER_CTX_ctrl(m_ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE_BYTES, tag))
        return false;

    if (!EVP_CipherFinal_ex(m_ctx, data + outLen, &outLen))
        return false;

    MANGOS_ASSERT(len == outLen);

    if (m_encrypting && !EVP_CIPHER_CTX_ctrl(m_ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE_BYTES, tag))
        return false;

    return true;
}

bool AES_GCM::ProcessNoIntegrityCheck(IV const& iv, uint8* data, size_t partialLength)
{
    MANGOS_ASSERT(!m_encrypting);
    MANGOS_ASSERT(partialLength <= static_cast<size_t>(std::numeric_limits<int>::max()));
    int len = static_cast<int>(partialLength);
    if (!EVP_CipherInit_ex(m_ctx, nullptr, nullptr, nullptr, iv.data(), -1))
        return false;

    int outLen;
    if (!EVP_CipherUpdate(m_ctx, data, &outLen, data, len))
        return false;

    len -= outLen;

    if (!EVP_CipherFinal_ex(m_ctx, data + outLen, &outLen))
        return false;

    MANGOS_ASSERT(len == outLen);

    return true;
}
