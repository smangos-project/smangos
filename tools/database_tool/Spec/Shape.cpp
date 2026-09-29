#include "Shape.h"

#include "Spec/ColumnMatch.h"
#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>
#include <vector>

static std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& db, std::string const& prefix)
{
    std::vector<std::string> names;
    DoltLiteStatement stmt = db.Prepare(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
    while (stmt.Step())
        names.push_back(stmt.ColumnText(0));
    return names;
}

struct ArraySlot
{
    std::string stem;  // "" when `name` doesn't end in digits (not an array slot)
    std::string index; // the trailing digit run, e.g. "2"
};

static ArraySlot SplitArraySlot(std::string const& name)
{
    size_t end = name.size();
    size_t start = end;
    while (start > 0 && std::isdigit(static_cast<unsigned char>(name[start - 1])))
        --start;
    if (start == end || start == 0)
        return {};
    return { name.substr(0, start), name.substr(start) };
}

struct ColumnStats
{
    int64_t rows = 0;
    int64_t distinct = 0;
    int64_t nullOrEmpty = 0;
    bool hasConstant = false;
    std::string constantValue;
};

struct EncodedValueKeyHash
{
    size_t operator()(EncodedValue v) const { return std::hash<int64_t>()(v.value) * 31 + v.kind; }
};

struct EncodedValueKeySame
{
    bool operator()(EncodedValue a, EncodedValue b) const { return a.kind == b.kind && a.value == b.value; }
};

// COUNT(*), COUNT(DISTINCT), NULL-or-'' count and the constant value (as SQLite prints it) per column.
static std::vector<ColumnStats> ComputeBasicStats(DoltLiteDatabase& db, std::string const& table, LoadedTable const& data,
    ValueEncoder& encoder)
{
    EncodedValue const empty = encoder.Text("");
    std::vector<ColumnStats> stats(data.columns.size());
    std::string constantsSql;
    std::vector<size_t> constantColumns;
    for (size_t i = 0; i < data.columns.size(); ++i)
    {
        std::unordered_set<EncodedValue, EncodedValueKeyHash, EncodedValueKeySame> distinct;
        ColumnStats& s = stats[i];
        s.rows = static_cast<int64_t>(data.rowCount);
        for (EncodedValue value : data.values[i])
        {
            if (value.kind == EncodedValue::Null)
            {
                ++s.nullOrEmpty;
                continue;
            }
            if (value.kind == empty.kind && value.value == empty.value)
                ++s.nullOrEmpty;
            distinct.insert(value);
        }
        s.distinct = static_cast<int64_t>(distinct.size());
        if (s.distinct == 1)
        {
            constantsSql += (constantsSql.empty() ? "SELECT " : ", ") + std::string("MIN(\"") + data.columns[i].name + "\")";
            constantColumns.push_back(i);
        }
    }

    if (!constantColumns.empty())
    {
        DoltLiteStatement stmt = db.Prepare(constantsSql + " FROM \"" + table + "\"");
        if (stmt.Step())
        {
            for (size_t k = 0; k < constantColumns.size(); ++k)
            {
                stats[constantColumns[k]].hasConstant = true;
                stats[constantColumns[k]].constantValue = stmt.ColumnText(static_cast<int>(k));
            }
        }
    }
    return stats;
}

struct FormulaHit
{
    std::string name;
    int64_t matched = 0;
    int64_t total = 0;
};

// effectBaseDiceN = (effectDieSidesN != 0 ? 1 : 0).
static bool TryEffectBaseDiceFormula(LoadedTable const& data, size_t column, ArraySlot const& slot, FormulaHit& hit)
{
    if (slot.stem != "effectBaseDice" || data.rowCount == 0)
        return false;
    int sides = data.ColumnIndex("effectDieSides" + slot.index);
    if (sides < 0)
        return false;

    int64_t matched = 0;
    for (size_t row = 0; row < data.rowCount; ++row)
    {
        EncodedValue side = data.values[sides][row];
        bool nonZero = side.kind != EncodedValue::Null && !(side.kind == EncodedValue::Integer && side.value == 0);
        matched += SqlEqual(data.values[column][row], { nonZero ? 1 : 0, EncodedValue::Integer });
    }
    hit = { "effect_base_dice_from_die_sides", matched, static_cast<int64_t>(data.rowCount) };
    return true;
}

// The same-stem, same-category column this one equals most often.
static bool TrySiblingArraySlotFormula(LoadedTable const& data, size_t column, ArraySlot const& slot, FormulaHit& hit)
{
    if (data.rowCount == 0)
        return false;
    std::string const category = TypeCategory(data.columns[column].type);

    int64_t bestMatched = -1;
    std::string bestName;
    for (size_t other = 0; other < data.columns.size(); ++other)
    {
        if (other == column || TypeCategory(data.columns[other].type) != category ||
            SplitArraySlot(data.columns[other].name).stem != slot.stem)
            continue;
        int64_t matched = 0;
        for (size_t row = 0; row < data.rowCount; ++row)
            matched += SqlEqual(data.values[column][row], data.values[other][row]);
        if (matched > bestMatched)
        {
            bestMatched = matched;
            bestName = data.columns[other].name;
        }
    }
    if (bestMatched < 0)
        return false;
    hit = { "equals_" + bestName, bestMatched, static_cast<int64_t>(data.rowCount) };
    return true;
}

static std::string FormatShape(ColumnStats const& stats, FormulaHit const* hit)
{
    std::ostringstream out;
    out << "rows=" << stats.rows << " distinct=" << stats.distinct << " null=" << stats.nullOrEmpty;
    if (stats.hasConstant)
        out << " const=" << stats.constantValue;
    if (hit)
        out << " formula=" << hit->name << ":" << hit->matched << "/" << hit->total;
    return out.str();
}

static void ComputeShapeForPrefix(DoltLiteDatabase& reference, std::string const& source, std::string const& prefix,
    size_t tableNameOffset, ComputeShapeResult& result)
{
    for (std::string const& qualified : TablesWithPrefix(reference, prefix))
    {
        std::string table = qualified.substr(tableNameOffset);
        ValueEncoder encoder;
        LoadedTable data = LoadTable(reference, qualified, encoder);
        std::vector<ColumnStats> stats = ComputeBasicStats(reference, qualified, data, encoder);

        for (size_t column = 0; column < data.columns.size(); ++column)
        {
            FormulaHit hit;
            bool found = false;
            ArraySlot slot = SplitArraySlot(data.columns[column].name);
            if (!slot.stem.empty())
            {
                found = TryEffectBaseDiceFormula(data, column, slot, hit);
                if (!found)
                    found = TrySiblingArraySlotFormula(data, column, slot, hit);
            }

            result.shapes[source + "	" + table + "	" + data.columns[column].name] =
                FormatShape(stats[column], found ? &hit : nullptr);
        }
    }
}

ComputeShapeResult ComputeShape(DoltLiteDatabase& reference)
{
    ComputeShapeResult result;
    ComputeShapeForPrefix(reference, "vdbc", "vdbc", 5, result);
    ComputeShapeForPrefix(reference, "srv", "srv", 4, result);
    return result;
}
