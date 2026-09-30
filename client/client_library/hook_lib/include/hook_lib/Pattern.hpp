#pragma once

#include "hook_lib/ProcessImage.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

// A byte pattern; nullopt entries are wildcards.
using BytePattern = std::vector<std::optional<std::byte>>;

// Parses IDA-style text ("48 8D 0D ? ? ? ?"); "?" or "??" is a wildcard.
BytePattern ParsePattern(std::string_view text);

// The exact bytes of text (no terminator is added -- append '\0' to the
// string_view yourself if the match should include one).
BytePattern LiteralPattern(std::string_view text);

// The mapped range of a PE section of the image, or an empty span if the
// section doesn't exist.
std::span<std::byte> FindSection(ProcessImage const& image, char const* name);

// First match of pattern inside range, or nullptr.
std::byte* FindPattern(std::span<std::byte> range, BytePattern const& pattern);

// Checks the expected bytes sit at a known RVA into the image, instead of
// scanning a whole section for them: cheaper (no scan) and unambiguous (no
// risk of matching the same instruction shape elsewhere), at the cost of only
// being correct for the exact build the rva was recorded against. Use this
// instead of FindPattern when hooking one pinned client build rather than
// guarding against rebuilds. Same fail-loud contract: nullptr means "bytes
// don't match, don't hook this".
std::byte* VerifyPatternAt(ProcessImage const& image, std::size_t rva, BytePattern const& pattern);
