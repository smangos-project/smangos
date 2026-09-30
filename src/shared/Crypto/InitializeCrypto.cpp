#include "InitializeCrypto.h"
#include "Authentication/BnetSRP6.h"
#include "Encoding/Base32.h"
#include "Encryption/RC4.h"
#include "Hash/MD5.h"
#include "Hash/SHA1.h"
#include "IO/Filesystem/FileHandle.h"
#include "IO/Filesystem/FileSystem.h"
#include "Log.h"

#include <openssl/crypto.h>

#include <filesystem>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/provider.h>
#endif

template <typename C1, typename C2>
static bool ArrayEqual(C1 const& a, C2 const& b)
{
    if (a.size() != b.size())
        return false;
    return std::equal(a.begin(), a.end(), b.begin());
}

// Full client/host handshake round trip (registration -> ephemeral exchange -> mutual evidence
// check), the same protocol path bnetserver's REST login exercises -- see CryptoTests.cpp's
// "BnetSRP6 client and server halves agree" for the same shape as a proper unit test.
static bool Check_SRP6()
{
    std::string const username = "VMANGOS";
    std::string const password = "4EVER";

    BnetSRP6 registration;
    registration.CalculateVerifier(username, password);

    BnetSRP6 host;
    host.Initialize(registration.GetSalt(), registration.GetVerifier());
    host.CalculateHostPublicEphemeral();

    BnetSRP6 client;
    client.CalculateClientPublicEphemeral();

    std::vector<uint8> clientM1 = client.CalculateClientEvidence(
        username, password, registration.GetSalt(), host.GetHostPublicEphemeral());
    if (clientM1.empty())
        return false;

    if (!host.VerifyClientEvidence(client.GetClientPublicEphemeral(), clientM1))
        return false;

    std::vector<uint8> serverM2 = host.CalculateServerEvidence();
    return client.VerifyServerEvidence(serverM2);
}

static bool Check_Base32()
{
    std::vector<uint8> testData = { 'H', 'e', 'l', 'l', 'o' };
    std::string encoded = Crypto::Encoding::Base32::Encode(testData);
    std::string expectedEncoded = "JBSWY3DP";
    return encoded == expectedEncoded;
}

static bool Check_RC4()
{
    uint8 constexpr rc4Key[] = { 0xf7, 0xff, 0x9e, 0x8b, 0x7b, 0xb2, 0xe0, 0x9b, 0x70, 0x93, 0x5a, 0x5d, 0x78, 0x5e, 0x0c, 0xc5 };
    RC4 rc4(16);
    rc4.Init(rc4Key);

    uint8 constexpr expectedCipher[] = { 0x1c, 0x9f, 0x73, 0x6b, 0xbc, 0x91, 0xa8, 0xc6, 0x0d };
    uint8 rc4Data[] = { 0x50, 0x6C, 0x61, 0x69, 0x6E, 0x74, 0x65, 0x78, 0x74 }; // "Plaintext"
    rc4.UpdateData(rc4Data, sizeof(rc4Data));
    return std::equal(std::begin(rc4Data), std::end(rc4Data), std::begin(expectedCipher));
}

static bool Check_MD5()
{
    auto md5 = Crypto::Hash::MD5::ComputeFrom("Hello");
    Crypto::Hash::MD5::Digest expectedMd5;
    uint8 constexpr md5Raw[] = { 0x8b, 0x1a, 0x99, 0x53, 0xc4, 0x61, 0x12, 0x96, 0xa8, 0x27, 0xab, 0xf8, 0xc4, 0x78, 0x04, 0xd7 };
    std::copy(std::begin(md5Raw), std::end(md5Raw), expectedMd5.begin());
    return ArrayEqual(md5, expectedMd5);
}

static bool Check_SHA1()
{
    auto sha1 = Crypto::Hash::SHA1::ComputeFrom("Hello");
    Crypto::Hash::SHA1::Digest expectedSha1;
    uint8 constexpr sha1Raw[] = { 0xf7, 0xff, 0x9e, 0x8b, 0x7b, 0xb2, 0xe0, 0x9b, 0x70, 0x93, 0x5a, 0x5d, 0x78, 0x5e, 0x0c, 0xc5, 0xd9, 0xd0, 0xab, 0xf0 };
    std::copy(std::begin(sha1Raw), std::end(sha1Raw), expectedSha1.begin());
    return ArrayEqual(sha1, expectedSha1);
}

static bool CryptoSelfCheck()
{
    sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "Running Crypto Library Selftest...");
    bool isOkay = true;

    // Authentication
    if (!Check_SRP6())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "SelfTest SRP6: Error");
        isOkay = false;
    }

    // Encoding
    if (!Check_Base32())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "SelfTest Base32: Error");
        isOkay = false;
    }

    // Encryption
    if (!Check_RC4()) // This requires "legacy" module
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "SelfTest RC4: Error");
        isOkay = false;
    }

    // Hash
    if (!Check_MD5())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "SelfTest MD5: Error");
        isOkay = false;
    }
    if (!Check_SHA1())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_MINIMAL, "SelfTest SHA1: Error");
        isOkay = false;
    }

    return isOkay;
}

static char const* GetOpenSslRuntimeVersion()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // OpenSSL 1.0.x
    return SSLeay_version(SSLEAY_VERSION);
#else
    // OpenSSL 1.1.x / OpenSSL 3
    return OpenSSL_version(OPENSSL_VERSION);
#endif
}

// The legacy provider sits next to the executable, which is not the working directory when running from .server.
static std::string GetExecutableDirectory()
{
#ifdef WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
        return "";
    return std::filesystem::path(std::string(buffer, length)).parent_path().string();
#else
    std::error_code error;
    std::filesystem::path path = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? "" : path.parent_path().string();
#endif
}

bool Crypto::InitializeCryptoAndPrintVersion()
{
    sLog.Out(LOG_BASIC, LOG_LVL_DETAIL, "OpenSSL Headers: %s | OpenSSL Library: %s", OPENSSL_VERSION_TEXT, GetOpenSslRuntimeVersion());

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    if (!OSSL_PROVIDER_available(nullptr, "legacy"))
    {
        sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "OpenSSL: legacy provider is not available. Trying to load it...");
        if (!OSSL_PROVIDER_load(nullptr, "legacy"))
        {
            sLog.Out(LOG_BASIC, LOG_LVL_DEBUG, "OpenSSL: legacy provider load failed. Retrying with CWD and executable directory...");
            bool loaded = false;
            for (std::string const& dir : { IO::Filesystem::ToAbsolutePath("."), GetExecutableDirectory() })
            {
                if (dir.empty() || !OSSL_PROVIDER_set_default_search_path(nullptr, dir.c_str()))
                    continue;
                if (OSSL_PROVIDER_load(nullptr, "legacy"))
                {
                    loaded = true;
                    break;
                }
            }

            if (!loaded)
            {
                sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "OpenSSL: Failed to load openssl legacy provider.");
                return false;
            }
        }
    }
#endif

    if (!CryptoSelfCheck())
    {
        sLog.Out(LOG_BASIC, LOG_LVL_ERROR, "CryptoSelfCheck failed");
        return false;
    }

    return true;
}
