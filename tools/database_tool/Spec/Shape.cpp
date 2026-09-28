#include "Shape.h"

#include "World/DoltLiteDatabase.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

struct ShapeColumnInfo
{
    std::string name;
    std::string type;
};

static std::vector<std::string> TablesWithPrefix(DoltLiteDatabase& db, std::string const& prefix)
{
    std::vector<std::string> names;
    DoltLiteStatement stmt = db.Prepare(
        "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '" + prefix + "\\_%' ESCAPE '\\' ORDER BY name");
    while (stmt.Step())
        names.push_back(stmt.ColumnText(0));
    return names;
}

static std::vector<ShapeColumnInfo> TableColumns(DoltLiteDatabase& db, std::string const& table)
{
    std::vector<ShapeColumnInfo> columns;
    DoltLiteStatement stmt = db.Prepare("PRAGMA table_info(\"" + table + "\")");
    while (stmt.Step())
        columns.push_back({ stmt.ColumnText(1), stmt.ColumnText(2) });
    return columns;
}

// Matches Counterpart.cpp's TypeCategory: only text needs to be kept apart from everything else.
static std::string TypeCategory(std::string type)
{
    std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    if (type.find("CHAR") != std::string::npos || type.find("TEXT") != std::string::npos || type.find("CLOB") != std::string::npos)
        return "text";
    return "numeric";
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

// One batched query per table: COUNT(*) once, then COUNT(DISTINCT ...)/null-or-empty count/MIN(...)
// per column -- MIN() doubles as "the value if constant" for free when distinct == 1, without a
// second pass over the table.
static std::map<std::string, ColumnStats> ComputeBasicStats(DoltLiteDatabase& db, std::string const& table, std::vector<ShapeColumnInfo> const& columns)
{
    std::map<std::string, ColumnStats> stats;
    if (columns.empty())
        return stats;

    std::string sql = "SELECT COUNT(*)";
    for (ShapeColumnInfo const& column : columns)
    {
        std::string ref = "\"" + column.name + "\"";
        sql += ", COUNT(DISTINCT " + ref + ")";
        sql += ", SUM(CASE WHEN " + ref + " IS NULL OR CAST(" + ref + " AS TEXT) = '' THEN 1 ELSE 0 END)";
        sql += ", MIN(" + ref + ")";
    }
    sql += " FROM \"" + table + "\"";

    DoltLiteStatement stmt = db.Prepare(sql);
    if (!stmt.Step())
        return stats;

    int64_t rows = stmt.ColumnInt64(0);
    int col = 1;
    for (ShapeColumnInfo const& column : columns)
    {
        ColumnStats s;
        s.rows = rows;
        s.distinct = stmt.ColumnIsNull(col) ? 0 : stmt.ColumnInt64(col);
        ++col;
        s.nullOrEmpty = stmt.ColumnIsNull(col) ? 0 : stmt.ColumnInt64(col);
        ++col;
        if (s.distinct == 1 && !stmt.ColumnIsNull(col))
        {
            s.hasConstant = true;
            s.constantValue = stmt.ColumnText(col);
        }
        ++col;
        stats[column.name] = std::move(s);
    }
    return stats;
}

struct FormulaHit
{
    std::string name;
    int64_t matched = 0;
    int64_t total = 0;
};

// spell-plan.md's damage-formula rule: `baseDice = DieSides ? 1 : 0` agrees with DB2's dropped
// EffectBaseDice/EffectDicePerLevel for 26,893 of 27,013 vanilla effects. Applies wherever a
// same-table `effectDieSides<N>` sibling exists, not just spell_template by name.
static bool TryEffectBaseDiceFormula(DoltLiteDatabase& db, std::string const& table, ArraySlot const& slot,
    std::vector<ShapeColumnInfo> const& columns, FormulaHit& hit)
{
    if (slot.stem != "effectBaseDice")
        return false;

    std::string diceSidesColumn = "effectDieSides" + slot.index;
    bool hasDiceSides = std::any_of(columns.begin(), columns.end(), [&](ShapeColumnInfo const& c) { return c.name == diceSidesColumn; });
    if (!hasDiceSides)
        return false;

    std::string diceRef = "\"effectBaseDice" + slot.index + "\"";
    std::string sidesRef = "\"" + diceSidesColumn + "\"";
    DoltLiteStatement stmt = db.Prepare("SELECT COUNT(*), SUM(CASE WHEN " + diceRef + " = (CASE WHEN " + sidesRef +
                                         " != 0 THEN 1 ELSE 0 END) THEN 1 ELSE 0 END) FROM \"" + table + "\"");
    if (!stmt.Step())
        return false;

    int64_t total = stmt.ColumnInt64(0);
    if (total == 0)
        return false;
    hit = { "effect_base_dice_from_die_sides", stmt.ColumnIsNull(1) ? 0 : stmt.ColumnInt64(1), total };
    return true;
}

// The generic candidate from phase0-plan.md 4.4: an array slot equal to another column -- here,
// the sibling slot of the same stem (`effectRadiusIndex2` against `effectRadiusIndex1`,
// `effectRadiusIndex3`, ...) with the highest equality rate, all compatible siblings batched into
// one query like Counterpart.cpp's rule search.
static bool TrySiblingArraySlotFormula(DoltLiteDatabase& db, std::string const& table, ShapeColumnInfo const& column,
    ArraySlot const& slot, std::vector<ShapeColumnInfo> const& columns, FormulaHit& hit)
{
    std::string category = TypeCategory(column.type);
    std::vector<ShapeColumnInfo> siblings;
    for (ShapeColumnInfo const& other : columns)
    {
        if (other.name == column.name || TypeCategory(other.type) != category)
            continue;
        if (SplitArraySlot(other.name).stem == slot.stem)
            siblings.push_back(other);
    }
    if (siblings.empty())
        return false;

    std::string sql = "SELECT COUNT(*)";
    std::string colRef = "\"" + column.name + "\"";
    for (ShapeColumnInfo const& sibling : siblings)
        sql += ", SUM(CASE WHEN " + colRef + " = \"" + sibling.name + "\" THEN 1 ELSE 0 END)";
    sql += " FROM \"" + table + "\"";

    DoltLiteStatement stmt = db.Prepare(sql);
    if (!stmt.Step())
        return false;

    int64_t total = stmt.ColumnInt64(0);
    if (total == 0)
        return false;

    int64_t bestMatched = -1;
    std::string bestName;
    for (size_t i = 0; i < siblings.size(); ++i)
    {
        if (stmt.ColumnIsNull(static_cast<int>(i) + 1))
            continue;
        int64_t matched = stmt.ColumnInt64(static_cast<int>(i) + 1);
        if (matched > bestMatched)
        {
            bestMatched = matched;
            bestName = siblings[i].name;
        }
    }
    if (bestMatched < 0)
        return false;
    hit = { "equals_" + bestName, bestMatched, total };
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
        std::vector<ShapeColumnInfo> columns = TableColumns(reference, qualified);
        std::map<std::string, ColumnStats> stats = ComputeBasicStats(reference, qualified, columns);

        for (ShapeColumnInfo const& column : columns)
        {
            FormulaHit hit;
            bool found = false;
            ArraySlot slot = SplitArraySlot(column.name);
            if (!slot.stem.empty())
            {
                try
                {
                    found = TryEffectBaseDiceFormula(reference, qualified, slot, columns, hit);
                    if (!found)
                        found = TrySiblingArraySlotFormula(reference, qualified, column, slot, columns, hit);
                }
                catch (DoltLiteError const&)
                {
                    // A comparison that doesn't type-check between two columns SQLite affinities
                    // disagree about -- no formula hit, not an error for the whole column.
                    found = false;
                }
            }

            result.shapes[source + "\t" + table + "\t" + column.name] = FormatShape(stats.at(column.name), found ? &hit : nullptr);
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
