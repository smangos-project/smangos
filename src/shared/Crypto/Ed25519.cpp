#include "Ed25519.h"
#include "Ed25519Vendor/ed25519/ed25519.h"

#include <openssl/evp.h>

Ed25519::Ed25519() : m_key(nullptr)
{
}

Ed25519::~Ed25519()
{
    if (m_key)
        EVP_PKEY_free(m_key);
}

bool Ed25519::LoadFromByteArray(std::array<uint8, 32> const& keyBytes)
{
    if (m_key)
    {
        EVP_PKEY_free(m_key);
        m_key = nullptr;
    }

    m_key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, keyBytes.data(), keyBytes.size());
    return m_key != nullptr;
}

std::vector<uint8> Ed25519::SignWithContext(uint8 const* message, size_t messageLength, std::vector<uint8> const& context) const
{
    if (!m_key || context.size() > 255)
        return {};

    constexpr size_t KeySize = 32;

    uint8 publicKey[KeySize] = {};
    size_t publicKeyLength = KeySize;
    if (!EVP_PKEY_get_raw_public_key(m_key, publicKey, &publicKeyLength))
        return {};

    uint8 privateKey[KeySize] = {};
    size_t privateKeyLength = KeySize;
    if (!EVP_PKEY_get_raw_private_key(m_key, privateKey, &privateKeyLength))
        return {};

    std::vector<uint8> signature(64);
    int result = ED25519_sign_ctx(signature.data(), message, messageLength, publicKey, privateKey,
        context.data(), uint8(context.size()));
    if (!result)
        return {};

    return signature;
}

bool Ed25519::VerifyWithContext(uint8 const* message, size_t messageLength, uint8 const* signature, std::vector<uint8> const& context) const
{
    if (!m_key || context.size() > 255)
        return false;

    constexpr size_t KeySize = 32;

    uint8 publicKey[KeySize] = {};
    size_t publicKeyLength = KeySize;
    if (!EVP_PKEY_get_raw_public_key(m_key, publicKey, &publicKeyLength))
        return false;

    return ED25519_verify_ctx(message, messageLength, signature, publicKey, context.data(), uint8(context.size())) != 0;
}
