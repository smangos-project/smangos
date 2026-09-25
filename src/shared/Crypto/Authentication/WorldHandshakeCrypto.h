#ifndef SHARED_WORLDHANDSHAKECRYPTO_H
#define SHARED_WORLDHANDSHAKECRYPTO_H

#include "Common.h"
#include <array>
#include <vector>

// The key-derivation math behind CMSG_AUTH_SESSION and CLASSIC_SMSG_ENTER_ENCRYPTED_MODE. Every
// function here is the same formula either side of the handshake runs: a client computes a value
// this way to send it, a server computes it this way to check what it received. The fixed 32-byte
// seeds are the protocol's own constants, not a secret either side derives.
class WorldHandshakeCrypto
{
    public:
        static std::array<uint8, 16> const& AuthCheckClientSeed();
        static std::array<uint8, 32> const& AuthCheckSeed();
        static std::array<uint8, 32> const& SessionKeySeed();
        static std::array<uint8, 32> const& EncryptionKeySeed();
        static std::array<uint8, 32> const& EnableEncryptionSeed();
        static std::array<uint8, 32> const& ContinuedSessionSeed();

        // The fixed Ed25519 keypair CLASSIC_SMSG_ENTER_ENCRYPTED_MODE is signed under -- not a
        // real secret (both sides of every reference implementation hardcode it), just the
        // shape the modern protocol expects. The bot loads this same "private" key purely to
        // recover its public half and verify the server's signature with it.
        static std::array<uint8, 32> const& EnterEncryptedModePrivateKey();
        static std::array<uint8, 16> const& EnableEncryptionContext();

        // digestKey = SHA512(joinSessionKey || AuthCheckClientSeed);
        // digest = HMAC-SHA512(digestKey, localChallenge || serverChallenge || AuthCheckSeed)[0..24]
        // -- the CMSG_AUTH_SESSION Digest field. Same on both sides: a client computes it this
        // way to send, a server computes it this way to check what it received.
        static std::array<uint8, 24> ComputeAuthCheckDigest(
            std::vector<uint8> const& joinSessionKey,
            std::array<uint8, 32> const& localChallenge,
            std::array<uint8, 32> const& serverChallenge);

        // sessionKey(40) = SessionKeyGenerator512(HMAC-SHA512(SHA512(joinSessionKey),
        // serverChallenge || localChallenge || SessionKeySeed))[0..32].Generate(40)
        // -- note the swapped challenge order relative to ComputeAuthCheckDigest above; not a
        // typo, matches WorldSocket.cpp exactly.
        static std::array<uint8, 40> ComputeSessionKey(
            std::vector<uint8> const& joinSessionKey,
            std::array<uint8, 32> const& localChallenge,
            std::array<uint8, 32> const& serverChallenge);

        // encryptKey(32) = HMAC-SHA512(sessionKey, localChallenge || serverChallenge ||
        // EncryptionKeySeed)[0..32]
        static std::array<uint8, 32> ComputeEncryptKey(
            std::array<uint8, 40> const& sessionKey,
            std::array<uint8, 32> const& localChallenge,
            std::array<uint8, 32> const& serverChallenge);

        // toSign = HMAC-SHA512(encryptKey, [enabled byte] || EnableEncryptionSeed), the digest
        // CLASSIC_SMSG_ENTER_ENCRYPTED_MODE's signature covers.
        static std::array<uint8, 64> ComputeEnterEncryptedModeDigest(
            std::array<uint8, 32> const& encryptKey, bool enabled);

        // digest = HMAC-SHA512(sessionKey, packedKey(8 bytes, native/little-endian) ||
        // localChallenge || serverChallenge || ContinuedSessionSeed)[0..24] -- the
        // CMSG_AUTH_CONTINUED_SESSION Digest field the Instance connection sends in place of
        // CMSG_AUTH_SESSION's auth-check digest (see AuthContinuedSession). `sessionKey`
        // here is the *existing* 40-byte session key the Realm connection's CMSG_AUTH_SESSION
        // already established, not a fresh derivation.
        static std::array<uint8, 24> ComputeContinuedSessionDigest(
            std::array<uint8, 40> const& sessionKey,
            uint64 packedKey,
            std::array<uint8, 32> const& localChallenge,
            std::array<uint8, 32> const& serverChallenge);
};

#endif
