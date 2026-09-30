// Diagnostic-only instrumentation for the "client opens TLS to bnetserver's
// REST login (127.0.0.1:8081), completes the handshake, then closes without
// sending a single byte" bug. Ghidra's static analysis was a dead end (this
// binary resolves nearly all its Windows API calls dynamically, so there's
// no fixed call site, and only ~34 boilerplate strings exist anywhere in the
// image); live debugger attach also failed identically both unelevated and
// elevated, consistent with Blizzard's clients actively resisting debugger
// attachment. This reuses the same in-process hooking approach that found
// the real certificate-trust bug earlier in this investigation.
//
// The key hook is EncryptMessage (SSPI): it receives the *plaintext* buffer
// right before SChannel encrypts it into an opaque TLS record, so it proves
// or disproves "did the client ever build an HTTP request" regardless of
// whether the resulting ciphertext goes out via send(), WSASend(), or
// something else entirely.

#include "NetDiagnostics.hpp"

#include "hook_lib/HookManager.hpp"
#include "hook_lib/Log.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#define SECURITY_WIN32
#include <security.h>
#include <schannel.h>
#include <wincrypt.h>

#include <mutex>
#include <unordered_set>


// Ghidra can't find these call sites statically (WowClassic.exe resolves imports by hash, not
// by name), but every hook here fires from inside the game's own code, so we can just ask the
// stack for it: walk the return addresses and report the first one that lands inside
// WowClassic.exe's own module range. That RVA can be fed straight into Ghidra's
// decompile_function, skipping the need to find the call site by any static means at all.
using RtlCaptureStackBackTraceFn = USHORT(WINAPI*)(ULONG, ULONG, PVOID*, PULONG);
RtlCaptureStackBackTraceFn PfnCaptureStackBackTrace = nullptr;
std::byte* ImageBase = nullptr;
std::size_t ImageSize = 0;

void LogCallerInModule(char const* label)
{
    if (!PfnCaptureStackBackTrace || !ImageBase)
        return;

    void* frames[24] = {};
    USHORT count = PfnCaptureStackBackTrace(0, 24, frames, nullptr);
    for (USHORT i = 0; i < count; ++i) {
        auto* addr = static_cast<std::byte*>(frames[i]);
        if (addr >= ImageBase && addr < ImageBase + ImageSize) {
            HOOK_LOG_INFO("net diag: %s caller -> WowClassic.exe+0x%llx (frame %u)",
                          label,
                          static_cast<unsigned long long>(addr - ImageBase),
                          i);
        }
    }
}

using ConnectFn = int(WSAAPI*)(SOCKET, const sockaddr*, int);
using SendFn = int(WSAAPI*)(SOCKET, const char*, int, int);
using WSASendFn = int(WSAAPI*)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
using CloseSocketFn = int(WSAAPI*)(SOCKET);
using EncryptMessageFn = SECURITY_STATUS(SEC_ENTRY*)(PCtxtHandle, ULONG, PSecBufferDesc, ULONG);
using QueryContextAttributesAFn = SECURITY_STATUS(SEC_ENTRY*)(PCtxtHandle, ULONG, PVOID);
using QueryContextAttributesWFn = SECURITY_STATUS(SEC_ENTRY*)(PCtxtHandle, ULONG, PVOID);

ConnectFn OriginalConnect = nullptr;
SendFn OriginalSend = nullptr;
WSASendFn OriginalWSASend = nullptr;
CloseSocketFn OriginalCloseSocket = nullptr;
EncryptMessageFn OriginalEncryptMessage = nullptr;
QueryContextAttributesAFn OriginalQueryContextAttributesA = nullptr;
QueryContextAttributesWFn OriginalQueryContextAttributesW = nullptr;

std::mutex TrackedSocketsMutex;
std::unordered_set<SOCKET> TrackedSockets; // sockets connect()ed to port 8081

void LogHexPreview(const char* label, const void* data, int len)
{
    if (!data || len <= 0)
        return;

    // Large enough to capture a full ClientHello (~400-600 bytes) in one line --
    // we need the real wire bytes to decode SNI/ALPN/supported_versions by hand.
    constexpr int MaxPreview = 1024;
    int previewLen = len < MaxPreview ? len : MaxPreview;
    char preview[MaxPreview * 2 + 1] = {};
    auto const* bytes = static_cast<unsigned char const*>(data);
    for (int i = 0; i < previewLen; ++i)
        snprintf(preview + i * 2, 3, "%02x", bytes[i]);

    HOOK_LOG_INFO("net diag: %s len=%d bytes=%s%s", label, len, preview, len > previewLen ? "..." : "");
}

bool IsTracked(SOCKET s)
{
    std::lock_guard<std::mutex> lock(TrackedSocketsMutex);
    return TrackedSockets.count(s) != 0;
}

int WSAAPI DetourConnect(SOCKET s, const sockaddr* name, int namelen)
{
    int result = OriginalConnect(s, name, namelen);
    int lastError = result == 0 ? 0 : WSAGetLastError();

    if (name && name->sa_family == AF_INET) {
        auto const* in = reinterpret_cast<sockaddr_in const*>(name);
        char ipBuf[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &in->sin_addr, ipBuf, sizeof(ipBuf));
        unsigned short port = ntohs(in->sin_port);

        HOOK_LOG_INFO("net diag: connect(sock=%p, %s:%u) -> %d (lastError=%d)",
                      reinterpret_cast<void*>(s),
                      ipBuf,
                      port,
                      result,
                      lastError);

        if (port == 8081) {
            std::lock_guard<std::mutex> lock(TrackedSocketsMutex);
            TrackedSockets.insert(s);
            LogCallerInModule("connect(:8081)");
        }
    }

    // WSAEWOULDBLOCK is normal for a non-blocking connect still in progress.
    if (result != 0 && lastError != WSAEWOULDBLOCK)
        WSASetLastError(lastError);

    return result;
}

int WSAAPI DetourSend(SOCKET s, const char* buf, int len, int flags)
{
    int result = OriginalSend(s, buf, len, flags);
    if (IsTracked(s)) {
        HOOK_LOG_INFO("net diag: send(sock=%p, len=%d) -> %d", reinterpret_cast<void*>(s), len, result);
        LogHexPreview("send payload", buf, len);
    }
    return result;
}

int WSAAPI DetourWSASend(SOCKET s,
                          LPWSABUF lpBuffers,
                          DWORD dwBufferCount,
                          LPDWORD lpNumberOfBytesSent,
                          DWORD dwFlags,
                          LPWSAOVERLAPPED lpOverlapped,
                          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int result = OriginalWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
    if (IsTracked(s)) {
        DWORD totalLen = 0;
        for (DWORD i = 0; i < dwBufferCount; ++i)
            totalLen += lpBuffers[i].len;
        HOOK_LOG_INFO("net diag: WSASend(sock=%p, buffers=%lu, totalLen=%lu) -> %d",
                      reinterpret_cast<void*>(s),
                      dwBufferCount,
                      totalLen,
                      result);
        if (dwBufferCount > 0)
            LogHexPreview("WSASend payload", lpBuffers[0].buf, static_cast<int>(lpBuffers[0].len));
    }
    return result;
}

int WSAAPI DetourCloseSocket(SOCKET s)
{
    if (IsTracked(s)) {
        HOOK_LOG_INFO("net diag: closesocket(sock=%p)", reinterpret_cast<void*>(s));
        LogCallerInModule("closesocket(:8081)");
        std::lock_guard<std::mutex> lock(TrackedSocketsMutex);
        TrackedSockets.erase(s);
    }
    return OriginalCloseSocket(s);
}

SECURITY_STATUS SEC_ENTRY DetourEncryptMessage(PCtxtHandle phContext, ULONG fQOP, PSecBufferDesc pMessage, ULONG MessageSeqNo)
{
    if (pMessage) {
        for (ULONG i = 0; i < pMessage->cBuffers; ++i) {
            SecBuffer const& buffer = pMessage->pBuffers[i];
            if (buffer.BufferType == SECBUFFER_DATA && buffer.cbBuffer > 0 && buffer.pvBuffer)
                LogHexPreview("EncryptMessage plaintext", buffer.pvBuffer, static_cast<int>(buffer.cbBuffer));
        }
    }

    return OriginalEncryptMessage(phContext, fQOP, pMessage, MessageSeqNo);
}

void LogAttributeQuery(char const* apiName, ULONG ulAttribute, SECURITY_STATUS result, PVOID pBuffer)
{
    HOOK_LOG_INFO("net diag: %s(attribute=0x%08lx) -> 0x%08lx", apiName, ulAttribute, static_cast<unsigned long>(result));

    // SECPKG_ATTR_REMOTE_CERT_CONTEXT: pBuffer is filled with a PCCERT_CONTEXT pointing at
    // the peer's certificate. If the client queries this and then makes its own trust
    // decision from the raw cert fields (subject/issuer/pinned key), that decision happens
    // entirely inside the client and never touches crypt32's chain-verification APIs -- which
    // is exactly the gap our CertBypass hooks can't see into.
    if (ulAttribute == SECPKG_ATTR_REMOTE_CERT_CONTEXT && result == SEC_E_OK && pBuffer) {
        auto certContext = *reinterpret_cast<PCCERT_CONTEXT*>(pBuffer);
        if (certContext) {
            char subjectBuf[256] = {};
            char issuerBuf[256] = {};
            CertNameToStrA(X509_ASN_ENCODING, &certContext->pCertInfo->Subject, CERT_X500_NAME_STR, subjectBuf, sizeof(subjectBuf));
            CertNameToStrA(X509_ASN_ENCODING, &certContext->pCertInfo->Issuer, CERT_X500_NAME_STR, issuerBuf, sizeof(issuerBuf));
            HOOK_LOG_INFO("net diag: %s SECPKG_ATTR_REMOTE_CERT_CONTEXT -> subject='%s' issuer='%s'", apiName, subjectBuf, issuerBuf);
        }
        LogCallerInModule(apiName);
    }
}

SECURITY_STATUS SEC_ENTRY DetourQueryContextAttributesA(PCtxtHandle phContext, ULONG ulAttribute, PVOID pBuffer)
{
    SECURITY_STATUS result = OriginalQueryContextAttributesA(phContext, ulAttribute, pBuffer);
    LogAttributeQuery("QueryContextAttributesA", ulAttribute, result, pBuffer);
    return result;
}

SECURITY_STATUS SEC_ENTRY DetourQueryContextAttributesW(PCtxtHandle phContext, ULONG ulAttribute, PVOID pBuffer)
{
    SECURITY_STATUS result = OriginalQueryContextAttributesW(phContext, ulAttribute, pBuffer);
    LogAttributeQuery("QueryContextAttributesW", ulAttribute, result, pBuffer);
    return result;
}


bool InstallNetDiagnosticsHooks()
{
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    PfnCaptureStackBackTrace = kernel32 ? reinterpret_cast<RtlCaptureStackBackTraceFn>(
                                           GetProcAddress(kernel32, "RtlCaptureStackBackTrace"))
                                      : nullptr;
    if (!PfnCaptureStackBackTrace)
        HOOK_LOG_ERROR("net diag: RtlCaptureStackBackTrace not found -- caller logging disabled");

    ProcessImage image = GetProcessImage();
    ImageBase = image.BaseAddress;
    ImageSize = image.ImageSize;

    HMODULE ws2_32 = LoadLibraryA("ws2_32.dll");
    HMODULE secur32 = LoadLibraryA("secur32.dll");
    if (!ws2_32 || !secur32) {
        HOOK_LOG_ERROR("net diag: failed to load ws2_32.dll/secur32.dll");
        return false;
    }

    void* connectTarget = reinterpret_cast<void*>(GetProcAddress(ws2_32, "connect"));
    void* sendTarget = reinterpret_cast<void*>(GetProcAddress(ws2_32, "send"));
    void* wsaSendTarget = reinterpret_cast<void*>(GetProcAddress(ws2_32, "WSASend"));
    void* closeSocketTarget = reinterpret_cast<void*>(GetProcAddress(ws2_32, "closesocket"));
    void* encryptMessageTarget = reinterpret_cast<void*>(GetProcAddress(secur32, "EncryptMessage"));
    void* queryContextAttributesATarget = reinterpret_cast<void*>(GetProcAddress(secur32, "QueryContextAttributesA"));
    void* queryContextAttributesWTarget = reinterpret_cast<void*>(GetProcAddress(secur32, "QueryContextAttributesW"));

    if (!connectTarget || !sendTarget || !wsaSendTarget || !closeSocketTarget || !encryptMessageTarget ||
        !queryContextAttributesATarget || !queryContextAttributesWTarget) {
        HOOK_LOG_ERROR("net diag: required export(s) not found");
        return false;
    }

    HookRegistration connectHook;
    connectHook.Name = "connect";
    connectHook.Target = reinterpret_cast<Address>(connectTarget);
    connectHook.Detour = reinterpret_cast<void*>(&DetourConnect);
    connectHook.Original = reinterpret_cast<void**>(&OriginalConnect);

    HookRegistration sendHook;
    sendHook.Name = "send";
    sendHook.Target = reinterpret_cast<Address>(sendTarget);
    sendHook.Detour = reinterpret_cast<void*>(&DetourSend);
    sendHook.Original = reinterpret_cast<void**>(&OriginalSend);

    HookRegistration wsaSendHook;
    wsaSendHook.Name = "WSASend";
    wsaSendHook.Target = reinterpret_cast<Address>(wsaSendTarget);
    wsaSendHook.Detour = reinterpret_cast<void*>(&DetourWSASend);
    wsaSendHook.Original = reinterpret_cast<void**>(&OriginalWSASend);

    HookRegistration closeSocketHook;
    closeSocketHook.Name = "closesocket";
    closeSocketHook.Target = reinterpret_cast<Address>(closeSocketTarget);
    closeSocketHook.Detour = reinterpret_cast<void*>(&DetourCloseSocket);
    closeSocketHook.Original = reinterpret_cast<void**>(&OriginalCloseSocket);

    HookRegistration encryptMessageHook;
    encryptMessageHook.Name = "EncryptMessage";
    encryptMessageHook.Target = reinterpret_cast<Address>(encryptMessageTarget);
    encryptMessageHook.Detour = reinterpret_cast<void*>(&DetourEncryptMessage);
    encryptMessageHook.Original = reinterpret_cast<void**>(&OriginalEncryptMessage);

    HookRegistration queryContextAttributesAHook;
    queryContextAttributesAHook.Name = "QueryContextAttributesA";
    queryContextAttributesAHook.Target = reinterpret_cast<Address>(queryContextAttributesATarget);
    queryContextAttributesAHook.Detour = reinterpret_cast<void*>(&DetourQueryContextAttributesA);
    queryContextAttributesAHook.Original = reinterpret_cast<void**>(&OriginalQueryContextAttributesA);

    HookRegistration queryContextAttributesWHook;
    queryContextAttributesWHook.Name = "QueryContextAttributesW";
    queryContextAttributesWHook.Target = reinterpret_cast<Address>(queryContextAttributesWTarget);
    queryContextAttributesWHook.Detour = reinterpret_cast<void*>(&DetourQueryContextAttributesW);
    queryContextAttributesWHook.Original = reinterpret_cast<void**>(&OriginalQueryContextAttributesW);

    bool ok = InstallHook(connectHook);
    ok = InstallHook(sendHook) && ok;
    ok = InstallHook(wsaSendHook) && ok;
    ok = InstallHook(closeSocketHook) && ok;
    ok = InstallHook(encryptMessageHook) && ok;
    ok = InstallHook(queryContextAttributesAHook) && ok;
    ok = InstallHook(queryContextAttributesWHook) && ok;
    return ok;
}
