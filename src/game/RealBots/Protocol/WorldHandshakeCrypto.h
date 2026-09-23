#ifndef REALBOTS_WORLDHANDSHAKECRYPTO_H
#define REALBOTS_WORLDHANDSHAKECRYPTO_H

#include "Common.h"
#include <array>
#include <vector>

// The key-derivation math behind CMSG_AUTH_SESSION and CLASSIC_SMSG_ENTER_ENCRYPTED_MODE.
// Ported from vibemangos's src/game/Server/WorldSocket.cpp (_HandleAuthSession,
// SendEnterEncryptedMode) -- that file only has the server side (it verifies a client's
// values); the client-side functions here (ComputeAuthCheckDigest, ComputeEncryptKey from the
// bot's own perspective) are new, built from the exact same primitives so both sides are
// provably the same protocol (see tests-realbots's half-against-half test).
//
// All of localChallenge/serverChallenge/joinSessionKey/sessionKey below are exactly what
// WorldSocket.cpp names them; the fixed 32-byte seeds are vibemangos's own constants (copied
// verbatim from TrinityCore's cata classic WorldSocket.cpp/AuthenticationPackets.cpp).
class WorldHandshakeCrypto
{
    public:
        static std::array<uint8, 16> const& AuthCheckClientSeed();
        static std::array<uint8, 32> const& AuthCheckSeed();
        static std::array<uint8, 32> const& SessionKeySeed();
        static std::array<uint8, 32> const& EncryptionKeySeed();
        static std::array<uint8, 32> const& EnableEncryptionSeed();

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
};

#endif
