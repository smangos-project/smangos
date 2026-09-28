#ifndef DATABASE_TOOL_SPEC_WIREHINT_H
#define DATABASE_TOOL_SPEC_WIREHINT_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// True for QueryHandler.cpp, GossipDef.cpp, UpdateFieldFill.cpp, any `*Handler.cpp` file, or
// any other file that streams into a WorldPacket (declares the type and uses `<<` on it) --
// phase0-plan.md 4.2's definition of "packet code".
bool IsPacketCodeFile(std::filesystem::path const& file, std::string const& text);

struct WireHintUse
{
    std::filesystem::path file;
    uint32_t line = 0;
};

// Every `.field`/`->field` access of `fieldName` (whole-identifier, name-only like
// SourceScan.h's CountFieldUsesInText) in `text`, with 1-based line numbers, in source order.
std::vector<WireHintUse> FindFieldUseLines(std::filesystem::path const& file, std::string const& text, std::string const& fieldName);

#endif
