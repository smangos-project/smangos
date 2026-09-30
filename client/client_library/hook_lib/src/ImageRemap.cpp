#include "hook_lib/ImageRemap.hpp"

#include "hook_lib/Log.hpp"
#include "hook_lib/Memory.hpp"

#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>

#include <cstring>
#include <vector>


using NtCreateSectionFn = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, PVOID, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
using NtMapViewOfSectionFn = NTSTATUS(NTAPI*)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, DWORD, ULONG, ULONG);
using NtUnmapViewOfSectionFn = NTSTATUS(NTAPI*)(HANDLE, PVOID);

constexpr DWORD ViewUnmap = 2;

// Whether the protection of the page at address can be changed at all. A
// SEC_NO_CHANGE view refuses any VirtualProtect, so flip the page to a
// writable variant of its current protection and straight back.
bool AcceptsProtectionChange(void* address, DWORD protect)
{
    constexpr DWORD ExecuteMask = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    DWORD const probe = (protect & ExecuteMask) ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

    DWORD oldProtect = 0;
    if (!VirtualProtect(address, 1, probe, &oldProtect))
        return false;

    VirtualProtect(address, 1, oldProtect, &oldProtect);
    return true;
}

// Handles of every other thread in the process, suspended. Collected before
// anything is suspended so no heap allocation happens while another thread
// might be frozen holding the heap lock.
std::vector<HANDLE> SuspendOtherThreads()
{
    std::vector<DWORD> threadIds;
    DWORD const processId = GetCurrentProcessId();
    DWORD const currentThreadId = GetCurrentThreadId();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        THREADENTRY32 entry = { sizeof(entry) };
        for (BOOL more = Thread32First(snapshot, &entry); more; more = Thread32Next(snapshot, &entry)) {
            if (entry.th32OwnerProcessID == processId && entry.th32ThreadID != currentThreadId)
                threadIds.push_back(entry.th32ThreadID);
        }
        CloseHandle(snapshot);
    }

    std::vector<HANDLE> threads;
    threads.reserve(threadIds.size());
    for (DWORD threadId : threadIds) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadId);
        if (!thread)
            continue;

        if (SuspendThread(thread) == static_cast<DWORD>(-1)) {
            CloseHandle(thread);
            continue;
        }

        threads.push_back(thread);
    }

    return threads;
}

void ResumeThreads(std::vector<HANDLE> const& threads)
{
    for (HANDLE thread : threads) {
        ResumeThread(thread);
        CloseHandle(thread);
    }
}


bool MakeMappedViewPatchable(void const* address, std::span<PendingWrite const> writes)
{
    MEMORY_BASIC_INFORMATION info = {};
    if (!VirtualQuery(address, &info, sizeof(info))) {
        HOOK_LOG_ERROR("image remap: VirtualQuery failed at %p: %lu", address, GetLastError());
        return false;
    }

    if (info.Type != MEM_MAPPED || AcceptsProtectionChange(info.BaseAddress, info.Protect)) {
        bool ok = true;
        for (PendingWrite const& write : writes)
            ok = WriteBytes(write.Address, write.Bytes) && ok;
        return ok;
    }

    // The whole view, which may span several regions of differing protection.
    auto* const viewBase = static_cast<std::byte*>(info.AllocationBase);
    std::size_t viewSize = 0;
    std::vector<std::pair<std::size_t, MEMORY_BASIC_INFORMATION>> regions;
    for (MEMORY_BASIC_INFORMATION region = {};
         VirtualQuery(viewBase + viewSize, &region, sizeof(region)) && region.AllocationBase == viewBase;
         viewSize += region.RegionSize) {
        regions.emplace_back(viewSize, region);
    }

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    auto ntCreateSection = reinterpret_cast<NtCreateSectionFn>(GetProcAddress(ntdll, "NtCreateSection"));
    auto ntMapViewOfSection = reinterpret_cast<NtMapViewOfSectionFn>(GetProcAddress(ntdll, "NtMapViewOfSection"));
    auto ntUnmapViewOfSection = reinterpret_cast<NtUnmapViewOfSectionFn>(GetProcAddress(ntdll, "NtUnmapViewOfSection"));
    if (!ntCreateSection || !ntMapViewOfSection || !ntUnmapViewOfSection) {
        HOOK_LOG_ERROR("image remap: section APIs not found in ntdll");
        return false;
    }

    HANDLE section = nullptr;
    LARGE_INTEGER maximumSize = {};
    maximumSize.QuadPart = static_cast<LONGLONG>(viewSize);
    NTSTATUS status = ntCreateSection(
        &section, SECTION_ALL_ACCESS, nullptr, &maximumSize, PAGE_EXECUTE_READWRITE, SEC_COMMIT, nullptr);
    if (status < 0) {
        HOOK_LOG_ERROR("image remap: NtCreateSection failed: 0x%08lx", static_cast<unsigned long>(status));
        return false;
    }

    // Fill the new section through a scratch view first, so the window in
    // which nothing is mapped at viewBase is just an unmap + map.
    void* scratch = nullptr;
    SIZE_T scratchSize = 0;
    LARGE_INTEGER offset = {};
    status = ntMapViewOfSection(
        section, GetCurrentProcess(), &scratch, 0, 0, &offset, &scratchSize, ViewUnmap, 0, PAGE_READWRITE);
    if (status < 0) {
        HOOK_LOG_ERROR("image remap: mapping scratch view failed: 0x%08lx", static_cast<unsigned long>(status));
        CloseHandle(section);
        return false;
    }

    // Only committed regions can be read; anything else stays zero.
    for (auto const& [regionOffset, region] : regions) {
        if (region.State == MEM_COMMIT && !(region.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            std::memcpy(static_cast<std::byte*>(scratch) + regionOffset, viewBase + regionOffset, region.RegionSize);
    }
    ntUnmapViewOfSection(GetCurrentProcess(), scratch);

    // From here until the threads are resumed: no logging, no heap, and
    // nothing that could call into the (briefly unmapped) image.
    std::vector<HANDLE> threads = SuspendOtherThreads();

    NTSTATUS unmapStatus = ntUnmapViewOfSection(GetCurrentProcess(), viewBase);
    NTSTATUS mapStatus = -1;
    if (unmapStatus >= 0) {
        void* mappedBase = viewBase;
        SIZE_T mappedSize = 0;
        LARGE_INTEGER mappedOffset = {};
        mapStatus = ntMapViewOfSection(section,
                                       GetCurrentProcess(),
                                       &mappedBase,
                                       0,
                                       0,
                                       &mappedOffset,
                                       &mappedSize,
                                       ViewUnmap,
                                       0,
                                       PAGE_EXECUTE_READWRITE);

        // The view is still read-write-execute here, so the pending writes
        // are plain copies; then put each region's original protection back.
        if (mapStatus >= 0) {
            for (PendingWrite const& write : writes) {
                std::memcpy(write.Address, write.Bytes.data(), write.Bytes.size());
                FlushInstructionCache(GetCurrentProcess(), write.Address, write.Bytes.size());
            }

            for (auto const& [regionOffset, region] : regions) {
                DWORD unused = 0;
                if (region.State == MEM_COMMIT)
                    VirtualProtect(viewBase + regionOffset, region.RegionSize, region.Protect, &unused);
            }
        }
    }

    ResumeThreads(threads);
    CloseHandle(section);

    if (unmapStatus < 0) {
        HOOK_LOG_ERROR("image remap: unmapping view at %p failed: 0x%08lx", viewBase, static_cast<unsigned long>(unmapStatus));
        return false;
    }

    if (mapStatus < 0) {
        // Nothing is mapped at viewBase any more; the process is not going
        // to survive this, so at least say why.
        HOOK_LOG_ERROR("image remap: remapping view at %p failed: 0x%08lx", viewBase, static_cast<unsigned long>(mapStatus));
        FlushLog();
        return false;
    }

    HOOK_LOG_INFO("image remap: view at %p (%zu byte(s), %zu region(s)) is now patchable "
                  "(suspended %zu thread(s), %zu write(s) applied in between)",
                  viewBase,
                  viewSize,
                  regions.size(),
                  threads.size(),
                  writes.size());
    return true;
}
