/*
 * Ed25519 signing for CLASSIC_SMSG_ENTER_ENCRYPTED_MODE (the cata classic client build). Real
 * TrinityCore signs this packet with Ed25519ctx (RFC 8032's context variant), not the older
 * 1.14.2 protocol's RSA/HMAC-SHA256 -- see AuthenticationPackets.cpp EnterEncryptedMode::Write()
 * in the cata classic reference tree. Key management (LoadFromByteArray/public key extraction)
 * goes through OpenSSL's normal EVP_PKEY raw-key API, since that part is standard Ed25519 with
 * no context involved; only the actual signature needs the vendored Ed25519Vendor/curve25519.c
 * (OpenSSL's own code, kept out-of-tree because its ED25519_sign_ctx isn't exposed through the
 * public EVP API).
 */

#ifndef _MANGOS_CRYPTO_ED25519_H
#define _MANGOS_CRYPTO_ED25519_H

#include "Common.h"
#include <array>
#include <vector>

typedef struct evp_pkey_st EVP_PKEY;

class Ed25519
{
    public:
        Ed25519();
        ~Ed25519();

        Ed25519(Ed25519 const&) = delete;
        Ed25519& operator=(Ed25519 const&) = delete;

        // `keyBytes` is the 32-byte raw Ed25519 private key seed.
        bool LoadFromByteArray(std::array<uint8, 32> const& keyBytes);

        // Signs `message` under RFC 8032 Ed25519ctx with `context` (max 255 bytes -- wire format
        // stores the length in a single byte). Returns a 64-byte signature, or an empty vector
        // on failure (message/context malformed, or LoadFromByteArray never called/failed).
        std::vector<uint8> SignWithContext(uint8 const* message, size_t messageLength, std::vector<uint8> const& context) const;

        template <std::size_t N>
        std::vector<uint8> SignWithContext(std::array<uint8, N> const& message, std::vector<uint8> const& context) const
        {
            return SignWithContext(message.data(), message.size(), context);
        }

    private:
        EVP_PKEY* m_key;
};

#endif
