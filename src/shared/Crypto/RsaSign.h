/*
 * RSA-2048 PKCS#1v1.5/SHA-256 signing for the 1.14.2 world-socket handshake
 * (SMSG_ENTER_ENCRYPTED_MODE, SMSG_CONNECT_TO). The private key is the well-known
 * Blizzard-issued key embedded in every real WoW server implementation (TrinityCore's
 * AuthenticationPackets.cpp, and confirmed identical -- same RSA modulus -- in jimsproxy's
 * Framework/Cryptography/RsaCrypt.cs, which is confirmed working against real 1.14.2 clients
 * build 41858/42597). There is no way around embedding it: the client hardcodes the matching
 * public key and refuses to proceed without a valid signature.
 *
 * IMPORTANT: this signs an *already-computed* digest (SHA-256 of the message for
 * SMSG_CONNECT_TO, HMAC-SHA256 of the message for SMSG_ENTER_ENCRYPTED_MODE) -- it does not
 * hash the message itself. This matches jimsproxy's `RsaCrypt.RSA.SignHash(digest, SHA256,
 * Pkcs1)` exactly (.NET's SignHash signs a precomputed digest under a declared hash OID, it
 * does not re-hash). The output is then **byte-reversed** before being written to the wire --
 * also confirmed against jimsproxy, and load-bearing: skipping the reversal produces a
 * signature the client will reject.
 */

#ifndef _MANGOS_CRYPTO_RSASIGN_H
#define _MANGOS_CRYPTO_RSASIGN_H

#include "Common.h"
#include <vector>

typedef struct evp_pkey_st EVP_PKEY;

class RsaSign
{
    public:
        RsaSign();
        ~RsaSign();

        // Loads the embedded Blizzard private key (see RsaSign.cpp). Returns false if OpenSSL
        // fails to parse it, which should never happen with a build-time-constant key.
        bool LoadBlizzardKey();

        // `digest` must already be the SHA-256 (or HMAC-SHA256) digest of the signed message --
        // see the file-level comment. Returns the byte-reversed PKCS#1v1.5 signature (256
        // bytes for a 2048-bit key), or an empty vector on failure.
        std::vector<uint8> SignDigest(uint8 const* digest, size_t digestLength) const;

    private:
        EVP_PKEY* m_key;
};

#endif
