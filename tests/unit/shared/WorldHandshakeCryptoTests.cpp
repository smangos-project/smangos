#include "Crypto/Authentication/WorldHandshakeCrypto.h"
#include <catch_amalgamated.hpp>

// These check the implementation is self-consistent (deterministic, correct output sizes) and
// guard against accidental regressions. They cannot prove wire-compatibility with a real server --
// that needs a real handshake against a live target, which the conformance suite's
// world_socket: scenarios cover.

namespace
{
std::vector<uint8> TestJoinSessionKey()
{
    std::vector<uint8> key(64);
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8>(i * 7 + 3);
    return key;
}

std::array<uint8, 32> TestChallenge(uint8 seed)
{
    std::array<uint8, 32> challenge{};
    for (size_t i = 0; i < challenge.size(); ++i)
        challenge[i] = static_cast<uint8>(seed + i);
    return challenge;
}
}

TEST_CASE("WorldHandshakeCrypto: ComputeAuthCheckDigest is deterministic", "[worldhandshake]")
{
    std::vector<uint8> key = TestJoinSessionKey();
    std::array<uint8, 32> local = TestChallenge(1);
    std::array<uint8, 32> server = TestChallenge(100);

    auto digest1 = WorldHandshakeCrypto::ComputeAuthCheckDigest(key, local, server);
    auto digest2 = WorldHandshakeCrypto::ComputeAuthCheckDigest(key, local, server);
    CHECK(digest1 == digest2);

    // Order matters: local-then-server must differ from server-then-local.
    auto swapped = WorldHandshakeCrypto::ComputeAuthCheckDigest(key, server, local);
    CHECK(digest1 != swapped);
}

TEST_CASE("WorldHandshakeCrypto: session key and encrypt key derivation chain is deterministic", "[worldhandshake]")
{
    std::vector<uint8> key = TestJoinSessionKey();
    std::array<uint8, 32> local = TestChallenge(5);
    std::array<uint8, 32> server = TestChallenge(200);

    auto sessionKey1 = WorldHandshakeCrypto::ComputeSessionKey(key, local, server);
    auto sessionKey2 = WorldHandshakeCrypto::ComputeSessionKey(key, local, server);
    CHECK(sessionKey1 == sessionKey2);

    auto encryptKey1 = WorldHandshakeCrypto::ComputeEncryptKey(sessionKey1, local, server);
    auto encryptKey2 = WorldHandshakeCrypto::ComputeEncryptKey(sessionKey1, local, server);
    CHECK(encryptKey1 == encryptKey2);

    // A different join session key must produce a different chain end-to-end.
    std::vector<uint8> otherKey = key;
    otherKey[0] ^= 0xFF;
    auto otherSessionKey = WorldHandshakeCrypto::ComputeSessionKey(otherKey, local, server);
    CHECK(sessionKey1 != otherSessionKey);
}

TEST_CASE("WorldHandshakeCrypto: ComputeEnterEncryptedModeDigest differs by the enabled flag", "[worldhandshake]")
{
    std::array<uint8, 32> encryptKey = TestChallenge(9);

    auto enabledDigest = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(encryptKey, true);
    auto disabledDigest = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(encryptKey, false);
    CHECK(enabledDigest != disabledDigest);

    auto again = WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(encryptKey, true);
    CHECK(enabledDigest == again);
}

TEST_CASE("WorldHandshakeCrypto: ComputeContinuedSessionDigest is deterministic", "[worldhandshake]")
{
    std::array<uint8, 40> sessionKey{};
    for (size_t i = 0; i < sessionKey.size(); ++i)
        sessionKey[i] = static_cast<uint8>(i * 11 + 1);
    std::array<uint8, 32> local = TestChallenge(3);
    std::array<uint8, 32> server = TestChallenge(150);
    uint64 packedKey = 0x0123456789ABCDEFull;

    auto digest1 = WorldHandshakeCrypto::ComputeContinuedSessionDigest(sessionKey, packedKey, local, server);
    auto digest2 = WorldHandshakeCrypto::ComputeContinuedSessionDigest(sessionKey, packedKey, local, server);
    CHECK(digest1 == digest2);

    // A different packed key (a different Instance connection's identity) must change the digest.
    auto otherKeyDigest = WorldHandshakeCrypto::ComputeContinuedSessionDigest(sessionKey, packedKey ^ 1, local, server);
    CHECK(digest1 != otherKeyDigest);

    // Order matters: local-then-server must differ from server-then-local.
    auto swapped = WorldHandshakeCrypto::ComputeContinuedSessionDigest(sessionKey, packedKey, server, local);
    CHECK(digest1 != swapped);
}
