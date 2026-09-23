#include "WorldHandshakeCrypto.h"
#include "Crypto/Hash/SHA512.h"
#include "Crypto/Hash/HMACSHA512.h"
#include "Crypto/SessionKeyGenerator512.h"

std::array<uint8, 16> const& WorldHandshakeCrypto::AuthCheckClientSeed()
{
    static std::array<uint8, 16> const seed = { 0x64, 0x11, 0x84, 0x4E, 0x2D, 0xE7, 0x01, 0x4E, 0xEC, 0x55, 0xD0, 0xC9, 0x6D, 0xAF, 0x11, 0x90 };
    return seed;
}

std::array<uint8, 32> const& WorldHandshakeCrypto::AuthCheckSeed()
{
    static std::array<uint8, 32> const seed = { 0xDE, 0x3A, 0x2A, 0x8E, 0x6B, 0x89, 0x52, 0x66, 0x88, 0x9D, 0x7E, 0x7A, 0x77, 0x1D, 0x5D, 0x1F,
        0x4E, 0xD9, 0x0C, 0x23, 0x9B, 0xCD, 0x0E, 0xDC, 0xD2, 0xE8, 0x04, 0x3A, 0x68, 0x64, 0xC7, 0xB0 };
    return seed;
}

std::array<uint8, 32> const& WorldHandshakeCrypto::SessionKeySeed()
{
    static std::array<uint8, 32> const seed = { 0xE8, 0x1E, 0x8B, 0x59, 0x27, 0x62, 0x1E, 0xAA, 0x86, 0x15, 0x18, 0xEA, 0xC0, 0xBF, 0x66, 0x8C,
        0x6D, 0xBF, 0x83, 0x93, 0xBC, 0xAA, 0x80, 0x52, 0x5B, 0x1E, 0xDC, 0x23, 0xA0, 0x12, 0xB7, 0x50 };
    return seed;
}

std::array<uint8, 32> const& WorldHandshakeCrypto::EncryptionKeySeed()
{
    static std::array<uint8, 32> const seed = { 0x71, 0xC9, 0xED, 0x5A, 0xA7, 0x0E, 0x4D, 0xFF, 0x4C, 0x36, 0xA6, 0x5A, 0x3E, 0x46, 0x8A, 0x4A,
        0x5D, 0xA1, 0x48, 0xC8, 0x30, 0x47, 0x4A, 0xDE, 0xF6, 0x0D, 0x6C, 0xBE, 0x6F, 0xE4, 0x55, 0x73 };
    return seed;
}

std::array<uint8, 32> const& WorldHandshakeCrypto::EnableEncryptionSeed()
{
    static std::array<uint8, 32> const seed = { 0x66, 0xBE, 0x29, 0x79, 0xEF, 0xF2, 0xD5, 0xB5, 0x61, 0x53, 0xF6, 0x5F, 0x45, 0xAE, 0x81, 0xCB,
        0x32, 0xEC, 0x94, 0xEC, 0x75, 0xB3, 0x5F, 0x44, 0x6A, 0x63, 0x43, 0x67, 0x17, 0x20, 0x44, 0x34 };
    return seed;
}

std::array<uint8, 24> WorldHandshakeCrypto::ComputeAuthCheckDigest(
    std::vector<uint8> const& joinSessionKey,
    std::array<uint8, 32> const& localChallenge,
    std::array<uint8, 32> const& serverChallenge)
{
    SHA512::Generator digestKeyHash;
    digestKeyHash.UpdateData(joinSessionKey);
    digestKeyHash.UpdateData(AuthCheckClientSeed());
    SHA512::Digest digestKey = digestKeyHash.GetDigest();

    HMACSHA512::Generator hmac(digestKey.data(), digestKey.size());
    hmac.UpdateData(localChallenge);
    hmac.UpdateData(serverChallenge);
    hmac.UpdateData(AuthCheckSeed());
    HMACSHA512::Digest full = hmac.GetDigest();

    std::array<uint8, 24> truncated{};
    std::copy(full.begin(), full.begin() + truncated.size(), truncated.begin());
    return truncated;
}

std::array<uint8, 40> WorldHandshakeCrypto::ComputeSessionKey(
    std::vector<uint8> const& joinSessionKey,
    std::array<uint8, 32> const& localChallenge,
    std::array<uint8, 32> const& serverChallenge)
{
    SHA512::Digest joinSessionKeyDigest = SHA512::ComputeFrom(joinSessionKey);

    HMACSHA512::Generator hmac(joinSessionKeyDigest.data(), joinSessionKeyDigest.size());
    hmac.UpdateData(serverChallenge);
    hmac.UpdateData(localChallenge);
    hmac.UpdateData(SessionKeySeed());
    HMACSHA512::Digest sessionKeySeed = hmac.GetDigest();

    std::array<uint8, 40> sessionKey{};
    SessionKeyGenerator512 keyGen(sessionKeySeed);
    keyGen.Generate(sessionKey.data(), uint32(sessionKey.size()));
    return sessionKey;
}

std::array<uint8, 32> WorldHandshakeCrypto::ComputeEncryptKey(
    std::array<uint8, 40> const& sessionKey,
    std::array<uint8, 32> const& localChallenge,
    std::array<uint8, 32> const& serverChallenge)
{
    HMACSHA512::Generator hmac(sessionKey.data(), sessionKey.size());
    hmac.UpdateData(localChallenge);
    hmac.UpdateData(serverChallenge);
    hmac.UpdateData(EncryptionKeySeed());
    HMACSHA512::Digest full = hmac.GetDigest();

    std::array<uint8, 32> encryptKey{};
    std::copy(full.begin(), full.begin() + encryptKey.size(), encryptKey.begin());
    return encryptKey;
}

std::array<uint8, 64> WorldHandshakeCrypto::ComputeEnterEncryptedModeDigest(
    std::array<uint8, 32> const& encryptKey, bool enabled)
{
    uint8 enabledByte = enabled ? 1 : 0;

    HMACSHA512::Generator hmac(encryptKey.data(), encryptKey.size());
    hmac.UpdateData(&enabledByte, 1);
    hmac.UpdateData(EnableEncryptionSeed());
    return hmac.GetDigest();
}
