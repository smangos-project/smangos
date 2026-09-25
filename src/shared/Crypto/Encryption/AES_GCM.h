/*
 * AES-256-GCM wrapper for the cata classic world-socket packet cipher (see PacketCrypt.h).
 * ProcessNoIntegrityCheck exists solely to peek at a packet's opcode before the rest of the
 * payload has arrived, without checking or consuming the GCM tag.
 *
 * The world session's encrypt key is 32 bytes, so this is hardcoded to 256-bit.
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
