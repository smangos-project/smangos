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

// One row of Spec/pairs.tsv: a srv/vdbc table and the cdbc table it lines up against, with the
// join step 4.3 needs to compare them (phase0-plan.md 3.1). `joinKind` is "id" (an equi-join on
// one column each side, what Counterpart.cpp computes), "unpivot" (a DB2 table with several rows
// per source row, e.g. SpellEffect by EffectIndex) or "skip" (a shape 4.3 doesn't join yet, e.g.
// a composite key); both of the latter are recorded for completeness but produce no match rate
// (a known gap, see phase0-plan.md's Progress notes). Hand-written, seeded from `data-plan.md`
// and `spell-plan.md`.
struct SpecPairRow
{
    std::string source;
    std::string table;
    std::string cdbcTable;
    std::string joinKind;
    std::string sourceKey;
    std::string cdbcKey;
    std::string notes;
};

// One row of Spec/gaps.tsv: tool-owned, rewritten from scratch by `spec gaps` (phase0-plan.md
// step 4.5) -- what vanilla has and the 1.15 client files don't cover, outside anything
// pairs.tsv already joins. `kind` is one of "removed_table"/"removed_column"/"array_length"/
// "type_change" (4.5.1, a WoWDBDefs schema diff), "row_coverage" (4.5.2, vanilla-only ids of a
// joined pair), "counterpart_candidate" (4.5.3, a cdbc_ column found outside pairs.tsv),
// "empty_client_table" (4.5.4) or "client_only_field" (4.5.5). `column` is empty for a
// table-level row.
struct SpecGapRow
{
    std::string kind;
    std::string table;
    std::string column;
    std::string detail;
};

// One row of Spec/rules.tsv: a named conversion rule (phase0-plan.md 3.1), seeded from
// data-plan.md section 5 and spell-plan.md section 4.4. `kind` "value" rules have `expr`, a SQL
// scalar expression over the source column with `{v}` standing in for it, evaluated directly by
// the reference database (Counterpart.cpp); "identity" is `{v}` itself. `kind` "inverse" rules
// need a second lookup against the counterpart table rather than a plain value transform (e.g.
// `superseded_by_spell`'s next/previous-rank rule) and are recorded but not yet evaluated by
// `spec evidence` -- another known gap.
struct SpecRuleRow
{
    std::string name;
    std::string kind;
    std::string expr;
    std::string inverse;
    std::string appliesTo;
};

// One row of Spec/tables.tsv: a table that exists after the port, or an original table that
// disappears. `kind` is "dbc", "server" or "gone". `sources` are the original tables feeding a
// target table, `into` the tables a gone table's data went to. `overrides` names a table whose
// values this table's rows replace when the world database is created (spell_mod over
// spell_template): both may fill the same target column.
struct SpecTableRow
{
    std::string table;
    std::string kind;
    std::string key;
    std::string sources;
    std::string into;
    std::string overrides;
    std::string reason;
};

// Missing file reads as empty (spec init creates it from nothing). A malformed header throws.
std::vector<SpecColumnRow> ReadColumnsTsv(std::filesystem::path const& path);
void WriteColumnsTsv(std::filesystem::path const& path, std::vector<SpecColumnRow> const& rows);

std::vector<SpecDbcColumnRow> ReadDbcColumnsTsv(std::filesystem::path const& path);
void WriteDbcColumnsTsv(std::filesystem::path const& path, std::vector<SpecDbcColumnRow> const& rows);

std::vector<SpecEvidenceRow> ReadEvidenceTsv(std::filesystem::path const& path);
void WriteEvidenceTsv(std::filesystem::path const& path, std::vector<SpecEvidenceRow> const& rows);

std::vector<SpecPairRow> ReadPairsTsv(std::filesystem::path const& path);
void WritePairsTsv(std::filesystem::path const& path, std::vector<SpecPairRow> const& rows);

std::vector<SpecRuleRow> ReadRulesTsv(std::filesystem::path const& path);
void WriteRulesTsv(std::filesystem::path const& path, std::vector<SpecRuleRow> const& rows);

std::vector<SpecGapRow> ReadGapsTsv(std::filesystem::path const& path);
void WriteGapsTsv(std::filesystem::path const& path, std::vector<SpecGapRow> const& rows);

std::vector<SpecTableRow> ReadTablesTsv(std::filesystem::path const& path);
void WriteTablesTsv(std::filesystem::path const& path, std::vector<SpecTableRow> const& rows);

#endif
