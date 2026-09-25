#include "RsaSign.h"
#include "Errors.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <algorithm>

namespace
{
    // The well-known WoW server RSA-2048 private key. Not a secret in the usual sense: every
    // private WoW server implementation embeds this same key, because the client hardcodes the
    // corresponding public key and there is no way around that.
    char const* const BLIZZARD_RSA_PRIVATE_KEY_PEM =
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "MIIEpAIBAAKCAQEA7rPc1NPDtFRRzmZbyzK48PeSU8YZ8gyFL4omqXpFn2DE683q\n"
        "f41Z2FeyYHsJTJtouMft7x6ADeZrN1tTkOsYEw1/Q2SD2pjmrMIwooKlxsvH+4af\n"
        "n6kCagNJxTj7wMhVzMDOJZG+hc/R0TfOzIPS6jCAB3uAn51EVCIpvoba20jFqfkT\n"
        "NpUjdvEO3IQNlAISqJfzOxTuqm+YBSdOH6Ngpana2BffM8viE1SLGLDKubuIZAbf\n"
        "dabXYQC7sFoOetR3CE0V4hCDsASqnot3qQaJXQhdD7gua8HLZM9uXNtPWGUIUfsN\n"
        "SBpvtj0fC93+Gx3wv7Ana/WOvMdAAf+nC4DWXwIDAQABAoIBACKa5q/gB2Y0Nyvi\n"
        "APrDXrZoXclRVd+WWxSaRaKaPE+vuryovI9DUbwgcpa0H5QAj70CFwdsd4oMVozO\n"
        "6519x56zfTiq8MaXFhIDkQNuR1Q7pMFdMfT2jogJ8/7olO7M3EtzxC8EIwfJKhTX\n"
        "r15M2h3jbBwplmsNZKOB1GVvrXjOm1KtOZ4CTTM0WrPaLVDT9ax8pykjmFw16vGP\n"
        "j/R5Dky9VpabtfZOu/AEW259XDEiQgTrB4Eg+S4GJjHqAzPZBmMy/xhlDK4oMXef\n"
        "qXScfD4w0RxuuCFr6lxLPZz0S35BK1kIWmIkuv+9eQuI4Hr1CyVwch4fkfvrp84x\n"
        "8tvAFnkCgYEA87NZaG9a8/Mob6GgY4BVLHJVOSzzFdNyMA+4LfSbtzgON2RSZyeD\n"
        "0JpDowwXssw5XOyUUctj2cLLdlMCpDfdzk4F/PEakloDJWpason3lmur0/5Oq3T9\n"
        "3+fnNUl4d3UOs1jcJ1yGQ/BfrTyRTcEoZx8Mu9mJ4ituVkKuLeG5vX0CgYEA+r/w\n"
        "QBJS6kDyQPj1k/SMClUhWhyADwDod03hHTQHc9BleJyjXmVy+/pWhN7aELhjgLbf\n"
        "o/Gm3aKJjCxS4qBmqUKwAvGoSVux1Bo2ZjcfF7sX9BXBOlFTG+bPVCZUoaksTyXN\n"
        "g7GsA1frKkWWkgQuOeK3o/p9IZoBl93vEgcTGgsCgYEAv5ucCIjFMllUybCCsrkM\n"
        "Ps4GQ9YbqmV9ulwhq8BPTlc8lkDCqWhgM3uXAnNXjrUTxQQd+dG4yFZoMrhBs2xZ\n"
        "cQPXoXDQO5GaN6jPduETUamGiD/DCvwJQCrNlxAVL5dR36FWN3x/9JriHwsoE8Jz\n"
        "SeEX2frIdpM/RYNX/6sipuECgYEA+rwFRDxOdvm8hGWuQ2WMxyQ7Nn07PEV/LxVM\n"
        "HkSRkyh23vVakyDEqty3uSOSUJfgv6ud07TnU8ac3fLQatdT8LrDgB4fVkN/fYU8\n"
        "kldaGwO1vxgl4OfDQCo7dXzisciViwtVBvQZ+jnm6J0vJBFUHAPt9+WZTIlQQIjm\n"
        "71LtseMCgYBSAhs6lshtz+ujR3fmc4QqJVGqeXvEBPAVm6yYoKYRLwVs/rFv3WLN\n"
        "LOwwBQ6lz7P9RqYYB5wVlaRvEhb9+lCve/xVcxMeZ5GkOBPxVygYV9l/wNdE25Nz\n"
        "OHYtKG3GK3GEcFDwZU2LPHq21EroUAdtRfbrJ4KW2yc8igtXKxTBYw==\n"
        "-----END RSA PRIVATE KEY-----\n";
}

RsaSign::RsaSign() : m_key(nullptr)
{
}

RsaSign::~RsaSign()
{
    if (m_key)
        EVP_PKEY_free(m_key);
}

bool RsaSign::LoadBlizzardKey()
{
    BIO* bio = BIO_new_mem_buf(BLIZZARD_RSA_PRIVATE_KEY_PEM, -1);
    if (!bio)
        return false;

    m_key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    return m_key != nullptr;
}

std::vector<uint8> RsaSign::SignDigest(uint8 const* digest, size_t digestLength) const
{
    std::vector<uint8> signature;
    if (!m_key)
        return signature;

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(m_key, nullptr);
    if (!ctx)
        return signature;

    // "Sign a precomputed digest" mode: EVP_PKEY_sign() treats `digest` as the already-hashed
    // message and wraps it in the PKCS#1 DigestInfo ASN.1 structure for the declared hash
    // algorithm (SHA-256) before the raw RSA private-key operation -- it does NOT hash
    // `digest` again. This is the mode the real client verifies (equivalent to .NET's
    // RSACryptoServiceProvider.SignHash).
    bool ok = EVP_PKEY_sign_init(ctx) == 1 &&
        EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) == 1 &&
        EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) == 1;

    size_t sigLen = 0;
    if (ok)
        ok = EVP_PKEY_sign(ctx, nullptr, &sigLen, digest, digestLength) == 1;

    if (ok)
    {
        signature.resize(sigLen);
        ok = EVP_PKEY_sign(ctx, signature.data(), &sigLen, digest, digestLength) == 1;
    }

    EVP_PKEY_CTX_free(ctx);

    if (!ok)
    {
        signature.clear();
        return signature;
    }

    signature.resize(sigLen);
    std::reverse(signature.begin(), signature.end());
    return signature;
}
