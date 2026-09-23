/*
 * AES-256-GCM wrapper for the cata classic world-socket packet cipher (see PacketCrypt.h). Ported
 * from TrinityCore's Trinity::Crypto::AES (src/common/Cryptography/AES.h) -- ProcessNoIntegrityCheck
 * exists solely for WorldSocket::ReadHeaderHandler's opcode peek, which needs to decrypt just the
 * 2-byte opcode before the rest of the payload has even arrived, without checking/consuming the GCM tag.
 *
 * Was AES-128-GCM (16-byte key) for the older 1.14.2 protocol; cata classic's _encryptKey is
 * 32 bytes (see WorldSocket::HandleAuthSessionCallback in the cata classic reference), so this
 * is now hardcoded to 256-bit -- there is no other caller left needing 128-bit.
 */

#ifndef _MANGOS_CRYPTO_ENCRYPTION_AES_GCM_H
#define _MANGOS_CRYPTO_ENCRYPTION_AES_GCM_H

#include "Common.h"
#include <array>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

class AES_GCM
{
    public:
        static size_t const IV_SIZE_BYTES = 12;
        static size_t const KEY_SIZE_BYTES = 32;
        static size_t const TAG_SIZE_BYTES = 12;

        using IV = std::array<uint8, IV_SIZE_BYTES>;
        using Key = std::array<uint8, KEY_SIZE_BYTES>;
        using Tag = uint8[TAG_SIZE_BYTES];

        explicit AES_GCM(bool encrypting);
        ~AES_GCM();

        void Init(Key const& key);

        bool Process(IV const& iv, uint8* data, size_t length, Tag& tag);
        bool ProcessNoIntegrityCheck(IV const& iv, uint8* data, size_t partialLength);

    private:
        EVP_CIPHER_CTX* m_ctx;
        bool m_encrypting;
};

#endif
