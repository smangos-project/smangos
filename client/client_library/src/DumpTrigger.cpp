#include "DumpTrigger.hpp"

#include "hook_lib/Log.hpp"

#include <windows.h>
#include <dbghelp.h>

#include <string>
#include <thread>


using MiniDumpWriteDumpFn = BOOL(WINAPI*)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION,
                                          PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

std::wstring ExeDirectory()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring directory = path;
    return directory.substr(0, directory.find_last_of(L"\\/"));
}

void WriteDump(std::wstring const& dumpPath)
{
    HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll");
    auto writeDump = dbghelp ? reinterpret_cast<MiniDumpWriteDumpFn>(GetProcAddress(dbghelp, "MiniDumpWriteDump"))
                             : nullptr;
    if (!writeDump) {
        HOOK_LOG_ERROR("dump trigger: MiniDumpWriteDump unavailable");
        return;
    }

    HANDLE file = CreateFileW(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        HOOK_LOG_ERROR("dump trigger: could not create the dump file (%lu)", GetLastError());
        return;
    }

    // Stacks and thread state are what a hang needs; module data sections
    // add the globals the stacks point into without dumping the whole heap.
    auto type = static_cast<MINIDUMP_TYPE>(MiniDumpWithThreadInfo | MiniDumpWithDataSegs |
                                           MiniDumpWithUnloadedModules | MiniDumpWithIndirectlyReferencedMemory);
    BOOL ok = writeDump(GetCurrentProcess(), GetCurrentProcessId(), file, type, nullptr, nullptr, nullptr);
    DWORD error = ok ? 0 : GetLastError();
    CloseHandle(file);

    if (ok)
        HOOK_LOG_INFO("dump trigger: wrote WowClassic.custom.dmp");
    else
        HOOK_LOG_ERROR("dump trigger: MiniDumpWriteDump failed (%lu)", error);
    FlushLog();
}


void StartDumpTrigger()
{
    std::wstring const directory = ExeDirectory();
    std::thread([directory] {
        std::wstring const trigger = directory + L"\\custom.dumpnow";
        std::wstring const dump = directory + L"\\WowClassic.custom.dmp";
        for (;;) {
            Sleep(1000);
            if (GetFileAttributesW(trigger.c_str()) == INVALID_FILE_ATTRIBUTES)
                continue;
            DeleteFileW(trigger.c_str());
            HOOK_LOG_INFO("dump trigger: custom.dumpnow found, writing a minidump");
            WriteDump(dump);
        }
    }).detach();
}
