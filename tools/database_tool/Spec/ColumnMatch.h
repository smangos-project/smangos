#ifndef DATABASE_TOOL_SPEC_COLUMNMATCH_H
#define DATABASE_TOOL_SPEC_COLUMNMATCH_H

#include "Spec/SpecFile.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class DoltLiteDatabase;
class DoltLiteStatement;

// In-memory column matching for the counterpart searches (spec evidence, spec gaps): tables are
// loaded once, rules are applied once per distinct value, and match counts are plain comparisons.
// Results equal SQLite's `(rule) = c.column` over the joined rows.

// A cell reduced to one comparable word: equal SQL values (5 and 5.0 included) get equal
// encodings, NULL never equals anything.
struct EncodedValue
{
    enum Kind : uint8_t { Null, Integer, Real, Text };

    int64_t value = 0;
    Kind kind = Null;
};

inline bool SqlEqual(EncodedValue a, EncodedValue b)
{
    return a.kind == b.kind && a.kind != EncodedValue::Null && a.value == b.value;
}

// Interns text so every encoded value is a fixed-size word. Not thread-safe.
class ValueEncoder
{
public:
    EncodedValue Read(DoltLiteStatement const& stmt, int index);
    EncodedValue Text(std::string const& text);
    EncodedValue Number(double number);
    void Bind(DoltLiteStatement& stmt, int index, EncodedValue value) const;

private:
    std::unordered_map<std::string, int64_t> m_textIds;
    std::vector<std::string> m_texts;
};

struct MatchColumn
{
    std::string name;
    std::string type;
};

struct LoadedTable
{
    std::vector<MatchColumn> columns;
    std::vector<std::vector<EncodedValue>> values; // per column, per row
    size_t rowCount = 0;

    int ColumnIndex(std::string const& name) const;
};

// "text" for character types, "numeric" for everything else: SQLite already compares INTEGER and
// REAL values with each other.
std::string TypeCategory(std::string type);

LoadedTable LoadTable(DoltLiteDatabase& db, std::string const& table, ValueEncoder& encoder);

// The rows of `source JOIN target ON source.sourceKey = target.targetKey`, with the target's
// columns gathered per joined row.
struct PreparedJoin
{
    std::string targetTable;
    std::vector<uint32_t> sourceRows;
    std::vector<MatchColumn> targetColumns;
    std::vector<std::vector<EncodedValue>> targetValues; // per target column, per joined row
};

PreparedJoin PrepareJoin(LoadedTable const& source, int sourceKey, std::string const& targetTable, LoadedTable const& target,
    int targetKey);

// A source column's value under one rule, per source row.
struct RuleVariant
{
    SpecRuleRow const* rule = nullptr; // points into the `rules` passed to ApplyRules
    std::vector<EncodedValue> values;
};

// Applies every rule in `rules` to `sourceTable`.`column` (one query over its distinct values).
// A rule whose result equals an earlier rule's on every value is left out: it can never score
// higher. Empty when any rule expression fails.
std::vector<RuleVariant> ApplyRules(DoltLiteDatabase& db, std::string const& sourceTable, LoadedTable const& source, int column,
    std::vector<SpecRuleRow> const& rules, ValueEncoder& encoder);

// ApplyRules for several columns of one table, in one query per declared column type (falls back
// to ApplyRules column by column if that query fails). Indexed like `columns`.
std::vector<std::vector<RuleVariant>> ApplyRulesToTable(DoltLiteDatabase& db, std::string const& sourceTable, LoadedTable const& source,
    std::vector<int> const& columns, std::vector<SpecRuleRow> const& rules, ValueEncoder& encoder);

struct BestColumnMatch
{
    std::string targetTable;
    std::string targetColumn;
    std::string ruleName;
    int64_t matched = 0;
    int64_t joined = 0;

    bool BeatsRate(int64_t candidateMatched, int64_t candidateJoined) const
    {
        return joined == 0 || candidateMatched * joined > matched * candidateJoined;
    }
};

// Scores every (rule variant, target column of `sourceCategory`) over `join`, in that order, and
// keeps a strictly better rate in `best`.
void ScoreJoin(PreparedJoin const& join, std::vector<RuleVariant> const& variants, std::string const& sourceCategory,
    BestColumnMatch& best);

// Runs fn(0..count-1) on the hardware's threads.
void ParallelFor(size_t count, std::function<void(size_t)> const& fn);

#endif
