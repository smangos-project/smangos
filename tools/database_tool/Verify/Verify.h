#ifndef DATABASE_TOOL_VERIFY_VERIFY_H
#define DATABASE_TOOL_VERIFY_VERIFY_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// One mapping file (Verify/srv/<table>.sql, Verify/vdbc/<Name>.sql): a single SELECT over the
// world database that rebuilds one original table, and the columns rows are matched by.
struct VerifyMapping
{
    std::vector<std::string> key;
    std::string select;
    std::string originalWhere; // limits the original's rows, for rows the server never loaded
};

// A "-- key: a, b" line names the key (empty or absent: every column), a "-- where: <condition>" line
// limits the original's rows; other "--" lines are
// comments; the rest is the SELECT. Throws when the file can't be read.
VerifyMapping ReadVerifyMapping(std::filesystem::path const& path);
void WriteVerifyMapping(std::filesystem::path const& path, std::string const& header, VerifyMapping const& mapping);

struct VerifyProblem
{
    std::string check; // "V0".."V5"
    std::string table;
    std::string message;
};

struct VerifyOptions
{
    bool final = false;
    bool update = false; // rewrite the measured lists (client-delta.tsv, references.tsv) instead of checking them
    std::vector<std::string> checks; // empty: every check
    std::vector<std::string> tables; // empty: every table
};

struct VerifyResult
{
    std::vector<VerifyProblem> problems;
    std::vector<std::string> notes; // progress that isn't a failure yet (not gated without --final)
    uint32_t tablesCompared = 0;
    std::string error;
};

// Runs the checks of plan.md 6.2 against `worldPath` with `referencePath` attached as `ref`.
// Dropped columns come from `specDir`'s columns.tsv.
VerifyResult RunVerify(std::filesystem::path const& worldPath, std::filesystem::path const& referencePath,
                       std::filesystem::path const& specDir, std::filesystem::path const& verifyDir,
                       VerifyOptions const& options);

struct VerifyInitResult
{
    uint32_t written = 0;
    std::vector<std::string> skipped; // "<Name>: why columns.tsv implies no vdbc mapping"
    std::string error;
};

// Writes an identity mapping (the world table collapsed to the vanilla version, as the reference
// database was built) for every srv_ table that has no mapping file yet, and for the vanilla
// tables named in `vanillaTables` ("all": every one) the mapping columns.tsv implies: each kept
// column read back from its dbc_ target through the rule's inverse.
VerifyInitResult InitVerifyMappings(std::filesystem::path const& referencePath, std::filesystem::path const& specDir,
                                    std::filesystem::path const& verifyDir,
                                    std::vector<std::string> const& vanillaTables = {});

#endif
