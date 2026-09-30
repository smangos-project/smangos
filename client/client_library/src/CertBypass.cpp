// The client refuses TLS handshakes with our bnetserver unless its
// self-signed certificate is manually added to the Windows trusted root
// store. WowClassic.exe resolves almost every Windows API dynamically at
// runtime (its static import table is nearly empty), so there is no fixed
// call site to patch inside the game image itself. Instead we hook the
// system function that actually renders the trust verdict --
// system functions that actually render the trust verdict, which every
// caller in the process goes through regardless of how it resolved the
// address:
//
//  - CertVerifyCertificateChainPolicy: the "did this chain pass policy X"
//    call (e.g. CERT_CHAIN_POLICY_SSL).
//  - CertGetCertificateChain: many callers skip the policy call above and
//    just inspect the returned chain's TrustStatus.dwErrorStatus flags
//    directly, so the chain-trust-only bits get cleared there too (at the
//    top-level chain, each simple chain, and each element, since we don't
//    know which level a given caller looks at).
//
// Only the specific chain-trust conditions that "add to local trusted
// store" would otherwise work around are cleared; anything else (e.g. an
// actual bad signature) still surfaces normally.

#include "CertBypass.hpp"

#include "hook_lib/HookManager.hpp"
#include "hook_lib/Log.hpp"

#include <windows.h>
#include <wincrypt.h>

#include <cstdio>


using CertVerifyCertificateChainPolicyFn = BOOL(WINAPI*)(LPCSTR,
                                                          PCCERT_CHAIN_CONTEXT,
                                                          PCERT_CHAIN_POLICY_PARA,
                                                          PCERT_CHAIN_POLICY_STATUS);
using CertGetCertificateChainFn = BOOL(WINAPI*)(HCERTCHAINENGINE,
                                                 PCCERT_CONTEXT,
                                                 LPFILETIME,
                                                 HCERTSTORE,
                                                 PCERT_CHAIN_PARA,
                                                 DWORD,
                                                 LPVOID,
                                                 PCCERT_CHAIN_CONTEXT*);

CertVerifyCertificateChainPolicyFn OriginalCertVerifyCertificateChainPolicy = nullptr;
CertGetCertificateChainFn OriginalCertGetCertificateChain = nullptr;

bool IsSuppressedChainTrustError(LONG error)
{
    switch (error) {
    case CERT_E_UNTRUSTEDROOT:      // self-signed / untrusted CA (the case this exists for)
    case CERT_E_UNTRUSTEDCA:        // untrusted intermediate CA
    case CERT_E_UNTRUSTEDTESTROOT:  // test root not in trusted store
    case CERT_E_CHAINING:           // generic chain-building failure
    case CERT_E_CN_NO_MATCH:        // cert CN doesn't match the connected hostname
    case CERT_E_EXPIRED:            // self-signed dev cert past its validity window
    case CERT_E_REVOCATION_FAILURE: // our CA has no CRL/OCSP endpoint to check
        return true;
    default:
        return false;
    }
}

// Bits in CERT_TRUST_STATUS::dwErrorStatus that "add to local trusted store"
// would clear: untrusted/partial chain, unknown or unreachable revocation
// info, and validity-period issues (self-signed dev certs are often
// generated with an odd notBefore/notAfter). Deliberately leaves signature
// and usage validity bits alone -- those indicate real corruption.
constexpr DWORD SuppressedTrustStatusMask = CERT_TRUST_IS_UNTRUSTED_ROOT | CERT_TRUST_IS_PARTIAL_CHAIN |
                                             CERT_TRUST_REVOCATION_STATUS_UNKNOWN | CERT_TRUST_IS_OFFLINE_REVOCATION |
                                             CERT_TRUST_IS_NOT_TIME_VALID;

void ClearSuppressedTrustBits(CERT_TRUST_STATUS& status)
{
    status.dwErrorStatus &= ~SuppressedTrustStatusMask;
}

// pszPolicyOID is only sometimes a real string. The common CERT_CHAIN_POLICY_*
// values are small integers cast to a pointer (CERT_CHAIN_POLICY_BASE is
// ((LPCSTR)1), _SSL is ((LPCSTR)4), ...), so formatting one with %s
// dereferences address 0x1 and takes the whole process down -- which is
// exactly what crashed the client twice while this hook was being brought up.
// Anything below the 64K no-access region is a predefined constant, never a
// string.
void FormatPolicyOid(LPCSTR pszPolicyOID, char* buffer, size_t size)
{
    ULONG_PTR value = reinterpret_cast<ULONG_PTR>(pszPolicyOID);
    if (value < 0x10000)
        snprintf(buffer, size, "#%llu", static_cast<unsigned long long>(value));
    else
        snprintf(buffer, size, "%s", pszPolicyOID);
}

BOOL WINAPI DetourCertVerifyCertificateChainPolicy(LPCSTR pszPolicyOID,
                                                    PCCERT_CHAIN_CONTEXT pChainContext,
                                                    PCERT_CHAIN_POLICY_PARA pPolicyPara,
                                                    PCERT_CHAIN_POLICY_STATUS pPolicyStatus)
{
    BOOL result = OriginalCertVerifyCertificateChainPolicy(pszPolicyOID, pChainContext, pPolicyPara, pPolicyStatus);

    char policyText[64] = {};
    FormatPolicyOid(pszPolicyOID, policyText, sizeof(policyText));

    HOOK_LOG_INFO("cert bypass: CertVerifyCertificateChainPolicy(policy=%s) -> result=%d dwError=0x%08lx",
                  policyText,
                  result,
                  pPolicyStatus ? pPolicyStatus->dwError : 0xFFFFFFFF);

    if (result && pPolicyStatus && IsSuppressedChainTrustError(static_cast<LONG>(pPolicyStatus->dwError))) {
        HOOK_LOG_INFO("cert bypass: suppressing chain-trust error 0x%08lx", pPolicyStatus->dwError);
        pPolicyStatus->dwError = 0;
        pPolicyStatus->lChainIndex = -1;
        pPolicyStatus->lElementIndex = -1;
    }

    return result;
}

BOOL WINAPI DetourCertGetCertificateChain(HCERTCHAINENGINE hChainEngine,
                                          PCCERT_CONTEXT pCertContext,
                                          LPFILETIME pTime,
                                          HCERTSTORE hAdditionalStore,
                                          PCERT_CHAIN_PARA pChainPara,
                                          DWORD dwFlags,
                                          LPVOID pvReserved,
                                          PCCERT_CHAIN_CONTEXT* ppChainContext)
{
    BOOL result = OriginalCertGetCertificateChain(
        hChainEngine, pCertContext, pTime, hAdditionalStore, pChainPara, dwFlags, pvReserved, ppChainContext);

    if (!result || !ppChainContext || !*ppChainContext) {
        HOOK_LOG_INFO("cert bypass: CertGetCertificateChain -> result=%d (no chain)", result);
        return result;
    }

    // The API hands back a chain it allocated for us; the top-level pointer is
    // const to discourage callers from mutating it through the public contract,
    // but this process owns that memory, so clearing bits in our own copy of
    // the trust flags before the caller inspects them is safe.
    auto* chainContext = const_cast<CERT_CHAIN_CONTEXT*>(*ppChainContext);

    HOOK_LOG_INFO("cert bypass: CertGetCertificateChain -> dwErrorStatus=0x%08lx (cChain=%lu)",
                  chainContext->TrustStatus.dwErrorStatus,
                  chainContext->cChain);

    ClearSuppressedTrustBits(chainContext->TrustStatus);
    for (DWORD chainIndex = 0; chainIndex < chainContext->cChain; ++chainIndex) {
        CERT_SIMPLE_CHAIN* simpleChain = chainContext->rgpChain[chainIndex];
        ClearSuppressedTrustBits(simpleChain->TrustStatus);
        for (DWORD elementIndex = 0; elementIndex < simpleChain->cElement; ++elementIndex)
            ClearSuppressedTrustBits(simpleChain->rgpElement[elementIndex]->TrustStatus);
    }

    return result;
}


bool InstallCertBypassHook()
{
    HMODULE crypt32 = LoadLibraryA("crypt32.dll");
    if (!crypt32) {
        HOOK_LOG_ERROR("cert bypass: failed to load crypt32.dll");
        return false;
    }

    void* verifyPolicyTarget = reinterpret_cast<void*>(GetProcAddress(crypt32, "CertVerifyCertificateChainPolicy"));
    void* getChainTarget = reinterpret_cast<void*>(GetProcAddress(crypt32, "CertGetCertificateChain"));
    if (!verifyPolicyTarget || !getChainTarget) {
        HOOK_LOG_ERROR("cert bypass: required crypt32.dll export(s) not found");
        return false;
    }

    HookRegistration verifyPolicyHook;
    verifyPolicyHook.Name = "CertVerifyCertificateChainPolicy";
    verifyPolicyHook.Target = reinterpret_cast<Address>(verifyPolicyTarget);
    verifyPolicyHook.Detour = reinterpret_cast<void*>(&DetourCertVerifyCertificateChainPolicy);
    verifyPolicyHook.Original = reinterpret_cast<void**>(&OriginalCertVerifyCertificateChainPolicy);

    HookRegistration getChainHook;
    getChainHook.Name = "CertGetCertificateChain";
    getChainHook.Target = reinterpret_cast<Address>(getChainTarget);
    getChainHook.Detour = reinterpret_cast<void*>(&DetourCertGetCertificateChain);
    getChainHook.Original = reinterpret_cast<void**>(&OriginalCertGetCertificateChain);

    bool ok = InstallHook(verifyPolicyHook);
    ok = InstallHook(getChainHook) && ok;
    return ok;
}
