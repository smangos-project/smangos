#ifndef DATABASE_TOOL_SPEC_EVIDENCE_H
#define DATABASE_TOOL_SPEC_EVIDENCE_H

#include <cstdint>
#include <filesystem>
#include <string>

struct SpecEvidenceResult
{
    uint32_t rowsWritten = 0;
    uint32_t pairsJoined = 0;
    uint32_t pairsSkipped = 0;
    std::string error;
};

// database_tool spec evidence (phase0-plan.md step 4.1-4.2): rewrites `specDir`/evidence.tsv
// from scratch, one row per srv_*/vdbc_* column of `referencePath` (the same universe as
// columns.tsv, sorted the same way). Fills `type` from the reference database and `readers`
// from a source scan of `repoRoot`/src/game (DBCfmt.h, DBCStores.cpp, DBCStructure.h,
// SQLStorages.cpp and hand-written `SELECT ... FROM` loaders) and `repoRoot`/src/scripts. For
// every row whose `readers` names a resolved struct field, `wire_hint` records whether that
// field is also accessed in packet code (`wire=yes <file>:<line>, ...` or `wire=no`); `none`
// and `manual:` readers leave it empty, since there is no field name to grep for. For every
// column named by an "id"-kind row of `specDir`/pairs.tsv (step 4.3), `counterpart`,
// `counterpart_rule` and `match_rate` record the best-matching cdbc_ column found by
// Counterpart.cpp; `pairsJoined`/`pairsSkipped` count how many pairs.tsv rows that covered.
// `shape` is left empty for step 4.4.
SpecEvidenceResult BuildEvidence(std::filesystem::path const& referencePath, std::filesystem::path const& repoRoot,
    std::filesystem::path const& specDir);

#endif
