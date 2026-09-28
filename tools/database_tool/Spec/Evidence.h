#ifndef DATABASE_TOOL_SPEC_EVIDENCE_H
#define DATABASE_TOOL_SPEC_EVIDENCE_H

#include <cstdint>
#include <filesystem>
#include <string>

struct SpecEvidenceResult
{
    uint32_t rowsWritten = 0;
    std::string error;
};

// database_tool spec evidence (phase0-plan.md step 4.1): rewrites `specDir`/evidence.tsv from
// scratch, one row per srv_*/vdbc_* column of `referencePath` (the same universe as
// columns.tsv, sorted the same way). Fills `type` from the reference database and `readers`
// from a source scan of `repoRoot`/src/game (DBCfmt.h, DBCStores.cpp, DBCStructure.h,
// SQLStorages.cpp and hand-written `SELECT ... FROM` loaders) and `repoRoot`/src/scripts.
// wire_hint/counterpart/counterpart_rule/match_rate/shape are left empty for later parts of
// step 4.
SpecEvidenceResult BuildEvidence(std::filesystem::path const& referencePath, std::filesystem::path const& repoRoot,
    std::filesystem::path const& specDir);

#endif
