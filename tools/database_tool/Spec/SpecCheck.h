#ifndef DATABASE_TOOL_SPEC_SPECCHECK_H
#define DATABASE_TOOL_SPEC_SPECCHECK_H

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct SpecCheckProblem
{
    uint32_t check = 0; // 1-9, the numbered rule of phase0-plan.md 7.1
    std::string message;
};

struct SpecCheckResult
{
    std::vector<SpecCheckProblem> problems;
    uint32_t columnsChecked = 0;
    std::string error;
};

// database_tool spec check (phase0-plan.md step 7.1). Reads `referencePath` and the files in
// `specDir` and reports every violation of the numbered rules:
//
//  1. every srv_/vdbc_ column has exactly one columns.tsv row with a valid disposition, and every
//     non-`new` row names a real column;
//  2. `dbc` rows name an existing cdbc_ column, a rule in rules.tsv and a recorded match rate;
//  3. `server` rows have a reader in evidence.tsv or a reason;
//  4. `derived` rows have a formula (rule), `constant` a value (target), `drop` a reason;
//  5. no two rows fill the same target column, except a srv and a vdbc row sharing one rule;
//  7. every dbc_ table a `dbc` row writes into has a vanilla_value on each of its columns that
//     no `dbc` row fills (the 1.15-only ones);
//  8. every removed_table/removed_column entry of gaps.tsv has a row (any disposition) in that
//     vdbc_ table;
//  9. with `final`: every row is reviewed=yes.
//
// Rule 6 (tables.tsv) is not checked: that file doesn't exist yet.
SpecCheckResult CheckSpec(std::filesystem::path const& referencePath, std::filesystem::path const& specDir, bool final);

#endif
