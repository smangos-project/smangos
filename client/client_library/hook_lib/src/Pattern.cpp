#include "hook_lib/Pattern.hpp"

#include "hook_lib/Memory.hpp"

#include <windows.h>

#include <cstring>

BytePattern ParsePattern(std::string_view text)
{
    BytePattern pattern;

    std::size_t i = 0;
    while (i < text.size()) {
        if (text[i] == ' ') {
            ++i;
            continue;
        }

        std::size_t end = text.find(' ', i);
        if (end == std::string_view::npos)
            end = text.size();

        std::string_view token = text.substr(i, end - i);
        if (token[0] == '?')
            pattern.emplace_back(std::nullopt);
        else
            pattern.emplace_back(static_cast<std::byte>(std::strtoul(std::string(token).c_str(), nullptr, 16)));

        i = end;
    }

    return pattern;
}

BytePattern LiteralPattern(std::string_view text)
{
    BytePattern pattern;
    pattern.reserve(text.size());
    for (char c : text)
        pattern.emplace_back(static_cast<std::byte>(c));
    return pattern;
}

std::span<std::byte> FindSection(ProcessImage const& image, char const* name)
{
    if (!image.BaseAddress)
        return {};

    auto const* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER const*>(image.BaseAddress);
    auto const* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS64 const*>(image.BaseAddress + dosHeader->e_lfanew);

    IMAGE_SECTION_HEADER const* section = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section) {
        char sectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = {};
        std::memcpy(sectionName, section->Name, IMAGE_SIZEOF_SHORT_NAME);
        if (std::strcmp(sectionName, name) == 0)
            return { image.BaseAddress + section->VirtualAddress, section->Misc.VirtualSize };
    }

    return {};
}

std::byte* FindPattern(std::span<std::byte> range, BytePattern const& pattern)
{
    if (pattern.empty() || range.size() < pattern.size())
        return nullptr;

    // Anchor the scan on the first concrete byte so memchr does the heavy
    // lifting; patterns always start with at least one in practice.
    std::size_t anchor = 0;
    while (anchor < pattern.size() && !pattern[anchor])
        ++anchor;
    if (anchor == pattern.size())
        return range.data();

    auto const anchorByte = static_cast<int>(*pattern[anchor]);
    std::byte* const last = range.data() + range.size() - pattern.size();

    std::byte* cursor = range.data();
    while (cursor <= last) {
        auto* hit = static_cast<std::byte*>(
            std::memchr(cursor + anchor, anchorByte, static_cast<std::size_t>(last - cursor) + 1));
        if (!hit)
            return nullptr;

        std::byte* candidate = hit - anchor;
        bool matches = true;
        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (pattern[i] && candidate[i] != *pattern[i]) {
                matches = false;
                break;
            }
        }

        if (matches)
            return candidate;

        cursor = candidate + 1;
    }

    return nullptr;
}

std::byte* VerifyPatternAt(ProcessImage const& image, std::size_t rva, BytePattern const& pattern)
{
    std::byte* target = ResolveRva(image, rva);
    if (!target || !IsInsideImage(image, target, pattern.size()))
        return nullptr;
    return FindPattern({ target, pattern.size() }, pattern);
}
