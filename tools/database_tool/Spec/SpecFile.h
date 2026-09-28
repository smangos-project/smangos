#ifndef DATABASE_TOOL_SPEC_SPECFILE_H
#define DATABASE_TOOL_SPEC_SPECFILE_H

#include <filesystem>
#include <string>
#include <vector>

// One row of Spec/columns.tsv: a (source, table, column) inventoried from the reference
// database, plus the hand-filled disposition (phase0-plan.md 3.1). `source` is "srv", "vdbc"
// or "new".
struct SpecColumnRow
{
    std::string source;
    std::string table;
    std::string column;
    std::string disposition;
    std::string target;
    std::string rule;
    std::string reason;
    std::string reviewed;
};

// One row of Spec/dbc-columns.tsv: a column of a `dbc_<table>` target that mangosd keeps
// whole, with the hand-filled reader/value-rule evidence (phase0-plan.md 3.1).
struct SpecDbcColumnRow
{
    std::string table;
    std::string column;
    std::string mangosdReads;
    std::string vanillaValue;
};

// One row of Spec/evidence.tsv: tool-owned, rewritten from scratch by `spec evidence`
// (phase0-plan.md step 4) -- never hand-edited. Columns past `readers` are filled by later
// parts of step 4 (4.2-4.4) and left empty here.
struct SpecEvidenceRow
{
    std::string source;
    std::string table;
    std::string column;
    std::string type;
    std::string readers;
    std::string wireHint;
    std::string counterpart;
    std::string counterpartRule;
    std::string matchRate;
    std::string shape;
};

// Missing file reads as empty (spec init creates it from nothing). A malformed header throws.
std::vector<SpecColumnRow> ReadColumnsTsv(std::filesystem::path const& path);
void WriteColumnsTsv(std::filesystem::path const& path, std::vector<SpecColumnRow> const& rows);

std::vector<SpecDbcColumnRow> ReadDbcColumnsTsv(std::filesystem::path const& path);
void WriteDbcColumnsTsv(std::filesystem::path const& path, std::vector<SpecDbcColumnRow> const& rows);

std::vector<SpecEvidenceRow> ReadEvidenceTsv(std::filesystem::path const& path);
void WriteEvidenceTsv(std::filesystem::path const& path, std::vector<SpecEvidenceRow> const& rows);

#endif
