#ifndef DATABASE_TOOL_SPEC_COUNTERPART_H
#define DATABASE_TOOL_SPEC_COUNTERPART_H

#include "Spec/SpecFile.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class DoltLiteDatabase;

// The best cdbc_ counterpart found for one (source, table, column), phase0-plan.md step 4.3.
struct CounterpartMatch
{
    std::string counterpart;     // "cdbc_<table>.<column>"
    std::string counterpartRule; // a Spec/rules.tsv name
    std::string matchRate;       // "matched/joined"
};

struct ComputeCounterpartsResult
{
    // Keyed by "source\ttable\tcolumn", one entry per column with at least one joined row
    // against a type-compatible cdbc column under some rule.
    std::map<std::string, CounterpartMatch> matches;
    uint32_t pairsJoined = 0; // `pairs` rows with joinKind "id" that both tables existed for
    uint32_t pairsSkipped = 0; // "unpivot"/"skip" rows, or "id" rows whose tables/keys didn't exist
};

// For every `pairs` row with `joinKind` "id", joins `source_<table>` to `cdbc_<cdbcTable>` on
// `sourceKey = cdbcKey` and, for every source column, finds the type-compatible cdbc column and
// `rules` entry (kind "value"; identity is tried first, unconditionally) with the highest
// fraction of joined rows equal under that rule -- recorded even below 90%, per phase0-plan.md
// 4.3. A source table with more than one pairs.tsv row (multiple cdbc tables) is searched
// against all of them. Rows with `joinKind` "unpivot" or "skip", or whose tables/key columns
// don't exist in `reference`, are counted in `pairsSkipped` and contribute no matches.
ComputeCounterpartsResult ComputeCounterparts(DoltLiteDatabase& reference, std::vector<SpecPairRow> const& pairs,
    std::vector<SpecRuleRow> const& rules);

#endif
