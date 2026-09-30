#include "hook_lib/Memory.hpp"

#include "hook_lib/Log.hpp"

#include <windows.h>

std::byte* ResolveRva(ProcessImage const& image, Address rva)
{
    if (!image.BaseAddress || rva >= image.ImageSize)
        return nullptr;

    return image.BaseAddress + rva;
}

bool IsInsideImage(ProcessImage const& image, void const* address, std::size_t size)
{
    if (!image.BaseAddress || !address)
        return false;

    auto const* cursor = static_cast<std::byte const*>(address);
    auto const* begin = image.BaseAddress;
    auto const* end = image.BaseAddress + image.ImageSize;
    return cursor >= begin && cursor <= end && size <= static_cast<std::size_t>(end - cursor);
}

bool WriteMemory(void* destination, void const* source, std::size_t size)
{
    if (!destination || !source || !size)
        return false;

    // Only ask for execute access when the target already has it: patching
    // plain data (e.g. .rdata) as RWX would needlessly trip a dynamic-code
    // policy such as ACG if the process ever enables one.
    MEMORY_BASIC_INFORMATION info = {};
    if (!VirtualQuery(destination, &info, sizeof(info))) {
        HOOK_LOG_ERROR("VirtualQuery failed at %p: %lu", destination, GetLastError());
        return false;
    }

    constexpr DWORD ExecuteMask = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    DWORD const writableProtect = (info.Protect & ExecuteMask) ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

    DWORD oldProtect = 0;
    if (!VirtualProtect(destination, size, writableProtect, &oldProtect)) {
        HOOK_LOG_ERROR("VirtualProtect failed at %p (%zu byte(s)): %lu", destination, size, GetLastError());
        return false;
    }

    std::memcpy(destination, source, size);
    FlushInstructionCache(GetCurrentProcess(), destination, size);

    DWORD unusedProtect = 0;
    if (!VirtualProtect(destination, size, oldProtect, &unusedProtect))
        HOOK_LOG_WARN("VirtualProtect restore failed at %p (%zu byte(s)): %lu", destination, size, GetLastError());

    return true;
}

bool WriteBytes(void* destination, std::span<std::byte const> bytes)
{
    return WriteMemory(destination, bytes.data(), bytes.size());
}
