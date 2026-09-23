#include "Crypto/Hash/SHA256.h"
#include "Crypto/Hash/SHA512.h"
#include "Crypto/Hash/HMACSHA256.h"
#include "Crypto/Hash/HMACSHA512.h"
#include "Crypto/Encryption/AES_GCM.h"
#include "Crypto/Encryption/PacketCrypt.h"
#include "Crypto/Authentication/BnetSRP6.h"
#include "Crypto/SessionKeyGenerator512.h"
#include "Crypto/Ed25519.h"
#include "Crypto/RsaSign.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <catch_amalgamated.hpp>
#include <cstring>

// Declared here (not included -- see Ed25519Vendor/ed25519/ed25519.h's own comment: nothing
// outside Ed25519.cpp should include it directly) purely so this test can verify a signature
// independently of Ed25519::SignWithContext, using the same vendored implementation.
extern "C" int ED25519_verify_ctx(uint8_t const* message, size_t message_len,
    uint8_t const signature[64], uint8_t const public_key[32], uint8_t const* context, uint8_t context_len);

namespace
{
std::vector<uint8> HexToBytes(std::string const& hex)
{
    std::vector<uint8> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
        bytes.push_back(static_cast<uint8>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return bytes;
}
}

TEST_CASE("SHA256 matches the known answer for \"abc\"", "[crypto]")
{
    SHA256::Digest digest = SHA256::ComputeFrom(std::string("abc"));
    std::vector<uint8> expected = HexToBytes("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(std::vector<uint8>(digest.begin(), digest.end()) == expected);
}

TEST_CASE("SHA512 matches the known answer for \"abc\"", "[crypto]")
{
    SHA512::Digest digest = SHA512::ComputeFrom(std::string("abc"));
    std::vector<uint8> expected = HexToBytes(
        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39"
        "a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
    CHECK(std::vector<uint8>(digest.begin(), digest.end()) == expected);
}

TEST_CASE("HMACSHA256 matches RFC 4231 test case 1", "[crypto]")
{
    std::vector<uint8> key(20, 0x0b);
    HMACSHA256::Generator gen(key);
    gen.UpdateData(std::string("Hi There"));
    HMACSHA256::Digest digest = gen.GetDigest();

    std::vector<uint8> expected = HexToBytes("b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
    CHECK(std::vector<uint8>(digest.begin(), digest.end()) == expected);
}

TEST_CASE("HMACSHA512 matches RFC 4231 test case 1", "[crypto]")
{
    std::vector<uint8> key(20, 0x0b);
    HMACSHA512::Generator gen(key);
    gen.UpdateData(std::string("Hi There"));
    HMACSHA512::Digest digest = gen.GetDigest();

    std::vector<uint8> expected = HexToBytes(
        "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cde"
        "daa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854");
    CHECK(std::vector<uint8>(digest.begin(), digest.end()) == expected);
}

TEST_CASE("AES_GCM round-trips and rejects a tampered tag", "[crypto]")
{
    AES_GCM::Key key{};
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8>(i);

    AES_GCM::IV iv{};
    for (size_t i = 0; i < iv.size(); ++i)
        iv[i] = static_cast<uint8>(0x40 + i);

    std::vector<uint8> plaintext = { 'h', 'e', 'l', 'l', 'o', ' ', 'b', 'o', 't' };
    std::vector<uint8> buffer = plaintext;

    AES_GCM::Tag tag;
    AES_GCM encryptor(true);
    encryptor.Init(key);
    REQUIRE(encryptor.Process(iv, buffer.data(), buffer.size(), tag));
    CHECK(buffer != plaintext);

    std::vector<uint8> decrypted = buffer;
    AES_GCM decryptor(false);
    decryptor.Init(key);
    REQUIRE(decryptor.Process(iv, decrypted.data(), decrypted.size(), tag));
    CHECK(decrypted == plaintext);

    // Same ciphertext, tampered tag: must fail rather than silently returning garbage.
    std::vector<uint8> stillCiphertext = buffer;
    AES_GCM::Tag badTag;
    memcpy(badTag, tag, sizeof(badTag));
    badTag[0] ^= 0xFF;
    AES_GCM decryptor2(false);
    decryptor2.Init(key);
    CHECK_FALSE(decryptor2.Process(iv, stillCiphertext.data(), stillCiphertext.size(), badTag));
}

TEST_CASE("PacketCrypt client and server halves agree", "[crypto]")
{
    AES_GCM::Key key{};
    for (size_t i = 0; i < key.size(); ++i)
        key[i] = static_cast<uint8>(i * 3 + 1);

    PacketCrypt client(PacketCryptRole::Client, key);
    PacketCrypt server(PacketCryptRole::Server, key);

    std::vector<uint8> payload = { 1, 2, 3, 4, 5 };
    AES_GCM::Tag tag;
    REQUIRE(client.EncryptSend(payload.data(), payload.size(), tag));
    REQUIRE(server.DecryptRecv(payload.data(), payload.size(), tag));
    CHECK(payload == std::vector<uint8>({ 1, 2, 3, 4, 5 }));

    // And the reverse direction, to exercise both magic values.
    std::vector<uint8> reply = { 9, 8, 7 };
    AES_GCM::Tag replyTag;
    REQUIRE(server.EncryptSend(reply.data(), reply.size(), replyTag));
    REQUIRE(client.DecryptRecv(reply.data(), reply.size(), replyTag));
    CHECK(reply == std::vector<uint8>({ 9, 8, 7 }));
}

TEST_CASE("BnetSRP6 client and server halves agree on a full handshake", "[crypto]")
{
    std::string username = "BOT1";
    std::string password = "hunter2";

    BnetSRP6 registration;
    registration.CalculateVerifier(username, password);

    BnetSRP6 host;
    host.Initialize(registration.GetSalt(), registration.GetVerifier());
    host.CalculateHostPublicEphemeral();

    BnetSRP6 client;
    client.CalculateClientPublicEphemeral();

    std::vector<uint8> clientM1 = client.CalculateClientEvidence(
        username, password, registration.GetSalt(), host.GetHostPublicEphemeral());
    REQUIRE_FALSE(clientM1.empty());

    REQUIRE(host.VerifyClientEvidence(client.GetClientPublicEphemeral(), clientM1));

    std::vector<uint8> serverM2 = host.CalculateServerEvidence();
    CHECK(client.VerifyServerEvidence(serverM2));
}

TEST_CASE("BnetSRP6 rejects a wrong password", "[crypto]")
{
    std::string username = "BOT1";

    BnetSRP6 registration;
    registration.CalculateVerifier(username, "correct-password");

    BnetSRP6 host;
    host.Initialize(registration.GetSalt(), registration.GetVerifier());
    host.CalculateHostPublicEphemeral();

    BnetSRP6 client;
    client.CalculateClientPublicEphemeral();

    std::vector<uint8> clientM1 = client.CalculateClientEvidence(
        username, "wrong-password", registration.GetSalt(), host.GetHostPublicEphemeral());
    REQUIRE_FALSE(clientM1.empty());

    CHECK_FALSE(host.VerifyClientEvidence(client.GetClientPublicEphemeral(), clientM1));
}

TEST_CASE("SessionKeyGenerator512 is deterministic and fills the requested length", "[crypto]")
{
    std::vector<uint8> seed(64);
    for (size_t i = 0; i < seed.size(); ++i)
        seed[i] = static_cast<uint8>(i);

    SessionKeyGenerator512 gen1(seed);
    std::vector<uint8> out1(40);
    gen1.Generate(out1.data(), static_cast<uint32>(out1.size()));

    SessionKeyGenerator512 gen2(seed);
    std::vector<uint8> out2(40);
    gen2.Generate(out2.data(), static_cast<uint32>(out2.size()));

    CHECK(out1 == out2);

    bool allZero = true;
    for (uint8 b : out1)
        if (b != 0)
            allZero = false;
    CHECK_FALSE(allZero);
}

TEST_CASE("Ed25519 SignWithContext round-trips through ED25519_verify_ctx", "[crypto]")
{
    std::array<uint8, 32> seed{};
    for (size_t i = 0; i < seed.size(); ++i)
        seed[i] = static_cast<uint8>(i + 1);

    Ed25519 signer;
    REQUIRE(signer.LoadFromByteArray(seed));

    std::vector<uint8> message = { 'p', 'i', 'n', 'g' };
    std::vector<uint8> context = { 'c', 't', 'x' };
    std::vector<uint8> signature = signer.SignWithContext(message.data(), message.size(), context);
    REQUIRE(signature.size() == 64);

    // Derive the public key the same way Ed25519::SignWithContext does, to verify independently.
    EVP_PKEY* key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, seed.data(), seed.size());
    REQUIRE(key != nullptr);
    uint8 publicKey[32] = {};
    size_t publicKeyLength = sizeof(publicKey);
    REQUIRE(EVP_PKEY_get_raw_public_key(key, publicKey, &publicKeyLength) == 1);
    EVP_PKEY_free(key);

    CHECK(ED25519_verify_ctx(message.data(), message.size(), signature.data(), publicKey,
        context.data(), static_cast<uint8_t>(context.size())) == 1);
}

TEST_CASE("Ed25519 VerifyWithContext accepts a matching signature and rejects a tampered one", "[crypto]")
{
    std::array<uint8, 32> seed{};
    for (size_t i = 0; i < seed.size(); ++i)
        seed[i] = static_cast<uint8>(i + 1);

    Ed25519 signer;
    REQUIRE(signer.LoadFromByteArray(seed));

    std::vector<uint8> message = { 'p', 'o', 'n', 'g' };
    std::vector<uint8> context = { 'c', 't', 'x' };
    std::vector<uint8> signature = signer.SignWithContext(message.data(), message.size(), context);
    REQUIRE(signature.size() == 64);

    // The bot side has no private key of its own -- it loads the same known constant purely to
    // recover the public half, exactly as BotWorldHandshake does for CLASSIC_SMSG_ENTER_ENCRYPTED_MODE.
    Ed25519 verifier;
    REQUIRE(verifier.LoadFromByteArray(seed));
    CHECK(verifier.VerifyWithContext(message.data(), message.size(), signature.data(), context));

    std::vector<uint8> tamperedSignature = signature;
    tamperedSignature[0] ^= 0xFF;
    CHECK_FALSE(verifier.VerifyWithContext(message.data(), message.size(), tamperedSignature.data(), context));

    CHECK_FALSE(verifier.VerifyWithContext(message.data(), message.size(), signature.data(), { 'o', 't', 'h', 'e', 'r' }));
}

TEST_CASE("RsaSign produces a fixed-size, deterministic PKCS#1v1.5 signature", "[crypto]")
{
    // RsaSign has no public-key accessor (it only signs -- see RsaSign.h), so this checks what's
    // observable from the wrapper itself: PKCS#1v1.5 signing has no randomness, so the same
    // digest must sign identically every time, and a different digest must sign differently.
    RsaSign signer;
    REQUIRE(signer.LoadBlizzardKey());

    SHA256::Digest digestA = SHA256::ComputeFrom(std::string("test message"));
    std::vector<uint8> signatureA1 = signer.SignDigest(digestA.data(), digestA.size());
    std::vector<uint8> signatureA2 = signer.SignDigest(digestA.data(), digestA.size());
    REQUIRE(signatureA1.size() == 256);
    CHECK(signatureA1 == signatureA2);

    SHA256::Digest digestB = SHA256::ComputeFrom(std::string("a different message"));
    std::vector<uint8> signatureB = signer.SignDigest(digestB.data(), digestB.size());
    REQUIRE(signatureB.size() == 256);
    CHECK(signatureA1 != signatureB);
}
