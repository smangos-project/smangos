#pragma once

#include <cstddef>
#include <string>

// Base address, size, and path of the current process's main module.
// Hook targets are usually expressed as an RVA into this image (so they
// keep working across ASLR-randomized load addresses).
struct ProcessImage
{
    std::byte* BaseAddress = nullptr;
    std::size_t ImageSize = 0;
    std::string ExecutablePath;
};

ProcessImage GetProcessImage();
