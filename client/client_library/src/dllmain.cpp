// custom.dll -- loaded into WowClassic.exe by custom_launcher.exe, which
// starts the client suspended and queues LoadLibraryW on its main thread.
// That makes DllMain run as soon as the Windows loader has finished its own
// setup (including the client's TLS callbacks), before the client's CRT
// startup and main init: nothing in the client has read its keys or
// connected anywhere yet, so every patch and hook installed here is in
// place before its first use.
//
// Add hooks with InstallHook() from hook_lib/HookManager.hpp; they're all
// enabled together by EnableAllHooks() below.

#include "CascBypass.hpp"
#include "CertBypass.hpp"
#include "ClientPatches.hpp"
#include "ControlServer.hpp"
#include "DumpTrigger.hpp"
#include "NetDiagnostics.hpp"

#include "custom/LaunchProtocol.hpp"

#include "hook_lib/HookManager.hpp"
#include "hook_lib/Log.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        // Only custom_launcher.exe creates this event, so its presence also
        // tells us we were loaded early enough.
        wchar_t eventName[64];
        FormatReadyEventName(eventName, std::size(eventName), GetCurrentProcessId());
        HANDLE ready = OpenEventW(EVENT_MODIFY_STATE, FALSE, eventName);

        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        char logPath[MAX_PATH + 32];
        snprintf(logPath, sizeof(logPath), "%s.custom.log", path);
        OpenLog(logPath);

        HOOK_LOG_INFO("custom.dll loaded");

        ProcessImage image = GetProcessImage();
        HOOK_LOG_INFO("process image: base=%p size=%zu path=%s",
                      static_cast<void*>(image.BaseAddress),
                      image.ImageSize,
                      image.ExecutablePath.c_str());

        InitializeHooks();
        bool ok = ApplyClientPatches(ready != nullptr);
        ok = InstallCertBypassHook() && ok;
        ok = InstallCascBypassHook() && ok;
        InstallNetDiagnosticsHooks();
        ok = EnableAllHooks() && ok;
        StartDumpTrigger();
        StartControlServerIfRequested();

        FlushLog();

        // A client that is only partly patched can't connect anyway, so let
        // the launcher report the failure instead of starting it.
        if (ready) {
            if (ok)
                SetEvent(ready);
            CloseHandle(ready);
        }
    } else if (reason == DLL_PROCESS_DETACH) {
        DisableAllHooks();
        CloseLog();
    }
    return TRUE;
}
