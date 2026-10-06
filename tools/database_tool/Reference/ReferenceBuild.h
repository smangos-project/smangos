#ifndef DATABASE_TOOL_REFERENCE_REFERENCEBUILD_H
#define DATABASE_TOOL_REFERENCE_REFERENCEBUILD_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

class DoltLiteDatabase;

// One populated reference table.
struct ReferenceTableInfo
{
    std::string table;  // e.g. "srv_spell_template", "vdbc_areatable", "cdbc_areatable"
    std::string source; // "srv", "vdbc" or "cdbc"
    uint64_t rowCount = 0;
};

struct ReferenceBuildResult
{
    std::vector<ReferenceTableInfo> tables;
    std::vector<std::string> tableErrors; // "<name>: <message>", one per table that failed
    std::string error;                    // a fatal, non-per-table failure
    bool alreadyExisted = false;
};

// Reads every vanilla .dbc in `vanillaDbcDir` into "vdbc_*" tables of `db`, appending to `result`.
void LoadVanillaDbcTables(DoltLiteDatabase& db, std::filesystem::path const& vanillaDbcDir, ReferenceBuildResult& result);

// Builds `referencePath` (typically .world/reference.doltlite) from three pinned inputs
// (phase0-plan.md step 2.2): `worldPath`, an existing .world/world.doltlite, becomes every
// non-dbc_/bookkeeping table renamed "srv_<table>", collapsed to the vanilla build/patch;
// `vanillaDbcDir` (.client/vanilla/dbc) becomes the "vdbc_*" tables via ReadWdbcFile; and
// `clientDbfilesclientDir` (.client/Content/dbfilesclient) becomes the "cdbc_*" tables via
// ReadWdc5File, untouched. Refuses if `referencePath` already exists, like `create`.
ReferenceBuildResult BuildReferenceDatabase(std::filesystem::path const& referencePath,
                                             std::filesystem::path const& worldPath,
                                             std::filesystem::path const& vanillaDbcDir,
                                             std::filesystem::path const& clientDbfilesclientDir);

#endif
