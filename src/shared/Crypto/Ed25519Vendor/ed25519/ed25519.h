/*
 * Vendored from TrinityCore's dep/openssl_ed25519 (itself OpenSSL's own crypto/ec/curve25519.c,
 * Apache-2.0/OpenSSL-licensed), which in turn vendors it because OpenSSL's public EVP_PKEY API
 * for Ed25519 does not expose the RFC 8032 "context" parameter (Ed25519ctx) needed to sign
 * CLASSIC_SMSG_ENTER_ENCRYPTED_MODE the way a real client verifies it -- only pure Ed25519 (no
 * context) is reachable through EVP_DigestSign. See Crypto/Ed25519.h for the C++ wrapper that
 * actually gets used; nothing outside Ed25519.cpp should include this header directly.
 *
 * Unlike the upstream copy, the dllexport/dllimport macro dance is dropped: this compiles
 * straight into the same static "shared" library as its caller, never a separate DLL/SO, so
 * there is nothing to export across a boundary.
 */

#ifndef CUSTOM_OPENSSL_ED25519_H
#define CUSTOM_OPENSSL_ED25519_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int ED25519_sign(uint8_t* out_sig, const uint8_t* message, size_t message_len,
    const uint8_t public_key[32], const uint8_t private_key[32]);
int ED25519_sign_ctx(uint8_t* out_sig, const uint8_t* message, size_t message_len,
    const uint8_t public_key[32], const uint8_t private_key[32], const uint8_t* context, uint8_t context_len);
int ED25519_sign_ph(uint8_t* out_sig, const uint8_t* message, size_t message_len,
    const uint8_t public_key[32], const uint8_t private_key[32], const uint8_t* hash, uint8_t hash_len);

int ED25519_verify(const uint8_t* message, size_t message_len,
    const uint8_t signature[64], const uint8_t public_key[32]);
int ED25519_verify_ctx(const uint8_t* message, size_t message_len,
    const uint8_t signature[64], const uint8_t public_key[32], const uint8_t* context, uint8_t context_len);
int ED25519_verify_ph(const uint8_t* message, size_t message_len,
    const uint8_t signature[64], const uint8_t public_key[32], const uint8_t* hash, uint8_t hash_len);

void ED25519_public_from_private(uint8_t out_public_key[32],
    const uint8_t private_key[32]);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CUSTOM_OPENSSL_ED25519_H
