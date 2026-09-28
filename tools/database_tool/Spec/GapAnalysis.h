#ifndef DATABASE_TOOL_SPEC_GAPANALYSIS_H
#define DATABASE_TOOL_SPEC_GAPANALYSIS_H

#include <cstdint>
#include <filesystem>
#include <string>

struct SpecGapResult
{
    uint32_t rowsWritten = 0;
    std::string error;
};

// database_tool spec gaps (phase0-plan.md step 4.5): rewrites `specDir`/gaps.tsv from scratch --
// what vanilla has that the 1.15 client files don't cover, without relying on pairs.tsv already
// joining it. Five independent passes, each its own "kind" of row (Spec/SpecFile.h):
//
//  1. Schema diff: every WoWDBDefs table resolved at 1.12.1.5875 with no matching, narrower or
//     retyped resolution at 1.15.6.58797/58725, restricted to tables actually imported into
//     `referencePath`'s vdbc_* tables.
//  2. Row coverage: for every "id" pairs.tsv join, vanilla ids absent from the joined cdbc_
//     table -- the rows phase 4 has to insert.
//  3. Counterpart discovery outside the pairs: for a source table that already has a trusted
//     "id" pair, other non-empty cdbc_ tables sharing that pair's key column name are searched
//     the same way Counterpart.cpp searches a declared pair, for any evidence.tsv column with a
//     reader and no 90%-or-better counterpart.
//  4. Empty client tables: every cdbc_ table with zero rows, with its vdbc_/srv_ twin if one is
//     known.
//  5. Client-only fields: the fixed list of 1.15-only tables/columns `data-plan.md` section 4
//     already names as ones mangosd will need, so dbc-columns.tsv's review can mark them
//     `mangosd_reads` up front.
//
// Every value is a hint for the reviewer, not a decision, like the rest of `spec evidence`.
SpecGapResult BuildGaps(std::filesystem::path const& referencePath, std::filesystem::path const& definitionsDir,
    std::filesystem::path const& specDir);

#endif
