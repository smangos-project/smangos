// custom_launcher.exe -- starts WowClassic.exe with custom.dll loaded before
// any of the client's own code runs.
//
// Place it next to WowClassic.exe and custom.dll. Every argument is passed
// through to the client unchanged, except custom.dll's own options (see
// IsDllOption), which the client would reject; those go to custom.dll through
// the environment instead.
//
// How: the client is created suspended and LoadLibraryW(custom.dll) is
// queued as an APC on its main thread. A new thread's first act is to run
// the Windows loader's process initialization, which drains queued APCs as
// soon as it has finished (after the client's TLS callbacks); only after
// that does it jump to the client's entry point. So custom.dll's DllMain
// runs after kernel32 & co. are ready but before the client's CRT startup
// and main init, where it builds its network keys.
//
// kernel32.dll is mapped at the same address in every process of a boot
// session, so this process's LoadLibraryW address is valid in the client.

#include "custom/LaunchProtocol.hpp"

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>


constexpr DWORD ReadyTimeoutMs = 30000;

std::wstring ModuleDirectory()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring directory = path;
    return directory.substr(0, directory.find_last_of(L"\\/"));
}

struct Arguments {
    std::wstring client; // for WowClassic.exe
    std::wstring dll;    // custom.dll's own options, see IsDllOption
};

// The launcher's command line minus the program path, split into what goes to
// the client and what is custom.dll's. Tokens keep their exact text (quotes
// included), so arguments reach the client as they were typed.
Arguments SplitArguments()
{
    std::vector<std::wstring> tokens;
    wchar_t const* p = GetCommandLineW();
    while (*p) {
        while (*p == L' ' || *p == L'\t')
            ++p;
        if (!*p)
            break;
        wchar_t const* start = p;
        bool quoted = false;
        while (*p && (quoted || (*p != L' ' && *p != L'\t'))) {
            if (*p == L'"')
                quoted = !quoted;
            ++p;
        }
        tokens.emplace_back(start, p);
    }

    auto unquoted = [](std::wstring token) {
        token.erase(std::remove(token.begin(), token.end(), L'"'), token.end());
        return token;
    };

    Arguments arguments;
    std::size_t i = 1; // tokens[0] is the launcher itself
    while (i < tokens.size()) {
        bool takesValue = false;
        std::wstring& target = IsDllOption(unquoted(tokens[i]), takesValue) ? arguments.dll : arguments.client;
        auto take = [&] {
            if (!target.empty())
                target += L' ';
            target += tokens[i++];
        };
        take();
        if (takesValue && i < tokens.size())
            take();
    }
    return arguments;
}

bool Fail(wchar_t const* what)
{
    fwprintf(stderr, L"custom_launcher: %ls (error %lu)\n", what, GetLastError());
    return false;
}

bool Launch()
{
    std::wstring const directory = ModuleDirectory();
    std::wstring const clientPath = directory + L"\\WowClassic.exe";
    std::wstring const dllPath = directory + L"\\custom.dll";

    if (GetFileAttributesW(clientPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        return Fail(L"WowClassic.exe not found next to the launcher");
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        return Fail(L"custom.dll not found next to the launcher");

    auto* loadLibrary = reinterpret_cast<PAPCFUNC>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"));
    if (!loadLibrary)
        return Fail(L"LoadLibraryW not found");

    Arguments const arguments = SplitArguments();
    std::wstring commandLine = L"\"" + clientPath + L"\" " + arguments.client;

    // Inherited by the client, which is created with this process's environment.
    SetEnvironmentVariableW(DllArgumentsVariable, arguments.dll.empty() ? nullptr : arguments.dll.c_str());

    STARTUPINFOW startupInfo = { sizeof(startupInfo) };
    PROCESS_INFORMATION process = {};
    if (!CreateProcessW(clientPath.c_str(),
                        commandLine.data(),
                        nullptr,
                        nullptr,
                        FALSE,
                        CREATE_SUSPENDED,
                        nullptr,
                        directory.c_str(),
                        &startupInfo,
                        &process))
        return Fail(L"could not start WowClassic.exe");

    bool ok = false;
    HANDLE ready = nullptr;

    do {
        wchar_t eventName[64];
        FormatReadyEventName(eventName, std::size(eventName), process.dwProcessId);
        ready = CreateEventW(nullptr, TRUE, FALSE, eventName);
        if (!ready) {
            Fail(L"could not create the ready event");
            break;
        }

        SIZE_T const pathBytes = (dllPath.size() + 1) * sizeof(wchar_t);
        void* remotePath = VirtualAllocEx(process.hProcess, nullptr, pathBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!remotePath || !WriteProcessMemory(process.hProcess, remotePath, dllPath.c_str(), pathBytes, nullptr)) {
            Fail(L"could not write the DLL path into the client");
            break;
        }

        if (!QueueUserAPC(loadLibrary, process.hThread, reinterpret_cast<ULONG_PTR>(remotePath))) {
            Fail(L"could not queue the DLL load");
            break;
        }

        ResumeThread(process.hThread);

        HANDLE waits[] = { ready, process.hProcess };
        switch (WaitForMultipleObjects(2, waits, FALSE, ReadyTimeoutMs)) {
        case WAIT_OBJECT_0:
            ok = true;
            break;
        case WAIT_OBJECT_0 + 1:
            fwprintf(stderr, L"custom_launcher: the client exited during startup\n");
            break;
        default:
            fwprintf(stderr,
                     L"custom_launcher: custom.dll did not report ready within %lu s -- see WowClassic.exe.custom.log\n",
                     ReadyTimeoutMs / 1000);
            break;
        }
    } while (false);

    if (!ok)
        TerminateProcess(process.hProcess, 1);

    if (ready)
        CloseHandle(ready);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return ok;
}


int wmain()
{
    if (Launch())
        return 0;

    // Keep the console up long enough to read the error when started by
    // double-click.
    Sleep(5000);
    return 1;
}
