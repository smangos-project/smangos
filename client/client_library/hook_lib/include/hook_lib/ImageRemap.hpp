#pragma once

#include <cstddef>
#include <span>

// WowClassic.exe re-maps its own image early in startup: the loader's
// MEM_IMAGE mapping is replaced by MEM_MAPPED views of a section created
// with SEC_NO_CHANGE, so VirtualProtect on anything inside them (.text,
// .rdata, ...) fails with ERROR_INVALID_PARAMETER and nothing can be
// patched or hooked there.
//
// MakeMappedViewPatchable swaps the view containing address for a copy
// backed by an ordinary section, mapped at the same address with the same
// contents and the same protection, after which WriteMemory / InstallHook
// work there as usual. Every other thread is suspended while the view is
// swapped, since for a moment nothing is mapped at that address.
//
// writes are applied inside that same suspended window, before any other
// thread can run code from the new view -- for patches that must be in
// place the moment the swap becomes observable (e.g. disabling the client's
// own checks for exactly this swap). If no swap was needed they are applied
// with WriteMemory instead.
//
// Does nothing else (and returns true) if address isn't in a MEM_MAPPED
// view, or the view already accepts protection changes.

struct PendingWrite
{
    std::byte* Address = nullptr;
    std::span<std::byte const> Bytes;
};

bool MakeMappedViewPatchable(void const* address, std::span<PendingWrite const> writes = {});
