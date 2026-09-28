#ifndef DATABASE_TOOL_SPEC_SPECRENDER_H
#define DATABASE_TOOL_SPEC_SPECRENDER_H

#include <cstdint>
#include <filesystem>
#include <string>

struct SpecRenderResult
{
    uint32_t pagesWritten = 0;
    std::string error;
};

// database_tool spec render (phase0-plan.md step 7.2): writes the review document under `outDir`
// from the files in `specDir`; needs no reference database. Rewrites index.md and the dbc/,
// server/ and gone/ folders from scratch:
//
//  - dbc/<table>.md: one page per `dbc_` target table -- every column a `dbc` row fills (source,
//     rule, match rate) and the 1.15-only columns with their vanilla_value;
//  - server/<source>_<table>.md: one page per source table that keeps a server, derived or
//     constant column, with every column's disposition;
//  - gone/<source>_<table>.md: one page per source table whose columns all go to a `dbc_`
//     table or are dropped -- each column's destination or reason;
//  - index.md: coverage counts per source and disposition, and the page lists.
SpecRenderResult RenderSpec(std::filesystem::path const& specDir, std::filesystem::path const& outDir);

#endif
