#include "hook_lib/ProcessImage.hpp"

#include "hook_lib/Log.hpp"

#include <array>
#include <windows.h>

ProcessImage GetProcessImage()
{
    HMODULE executable = GetModuleHandleW(nullptr);

    auto* baseAddress = reinterpret_cast<std::byte*>(executable);
    std::size_t imageSize = 0;

    auto const* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER const*>(baseAddress);
    if (dosHeader && dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
        auto const* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS64 const*>(baseAddress + dosHeader->e_lfanew);
        if (ntHeaders->Signature == IMAGE_NT_SIGNATURE)
            imageSize = ntHeaders->OptionalHeader.SizeOfImage;
    }

    if (!imageSize)
        HOOK_LOG_WARN("could not read process PE image size");

    std::array<char, MAX_PATH> path = {};
    GetModuleFileNameA(executable, path.data(), static_cast<DWORD>(path.size()));

    ProcessImage info;
    info.BaseAddress = baseAddress;
    info.ImageSize = imageSize;
    info.ExecutablePath = path.data();
    return info;
}
