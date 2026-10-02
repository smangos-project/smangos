#pragma once

#include "hook_lib/ProcessImage.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>

using Address = std::uintptr_t;

std::byte* ResolveRva(ProcessImage const& image, Address rva);
bool IsInsideImage(ProcessImage const& image, void const* address, std::size_t size = 1);
bool WriteMemory(void* destination, void const* source, std::size_t size);
bool WriteBytes(void* destination, std::span<std::byte const> bytes);

template <typename T>
bool WriteValue(void* destination, T const& value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    return WriteMemory(destination, &value, sizeof(T));
}

template <typename T>
T ReadValue(void const* source)
{
    static_assert(std::is_trivially_copyable_v<T>);

    T value = {};
    std::memcpy(&value, source, sizeof(T));
    return value;
}
