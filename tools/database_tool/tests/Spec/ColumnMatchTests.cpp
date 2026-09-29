#include "Spec/ColumnMatch.h"

#include "Spec/SpecFile.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

struct ColumnMatchTempDb
{
    std::filesystem::path path;

    explicit ColumnMatchTempDb(std::string const& name)
        : path(std::filesystem::temp_directory_path() / ("database_tool_test_columnmatch_" + name + ".doltlite"))
    {
        std::filesystem::remove(path);
    }

    ~ColumnMatchTempDb() { std::filesystem::remove(path); }
};

// Integers vs reals, text, NULLs, an unmatched key, a duplicated target key and text stored in a
// numeric column.
static void MakeColumnMatchFixture(DoltLiteDatabase& db)
{
    db.Execute("CREATE TABLE src (id INTEGER, a INTEGER, r REAL, t TEXT)");
    db.Execute("INSERT INTO src VALUES (1, 5, 1.5, 'x'), (2, 7, 2.0, 'y'), (3, NULL, NULL, NULL), (4, 9, 0.5, '9'), "
               "(5, 3, 3.0, 'z'), (6, 'junk', 1.0, 'w')");
    db.Execute("CREATE TABLE tgt (ID INTEGER, x INTEGER, y REAL, z TEXT, w INTEGER)");
    db.Execute("INSERT INTO tgt VALUES (1, 5, 1.5, 'x', 6), (2, 7, 2, 'Y', 8), (2, 7, 3.0, 'y', 7), (3, NULL, NULL, NULL, 1), "
               "(4, 10, 0.5, '9', 9), (6, 'junk', 1, 'w', 0), (99, 1, 1, 'q', 1)");
}

static std::vector<SpecRuleRow> ColumnMatchRules()
{
    return {
        { "identity", "value", "{v}", "{v}", "" },
        { "plus_one", "value", "({v} + 1)", "", "" },
        { "as_text", "value", "CAST({v} AS TEXT)", "", "" },
        { "same_as_identity", "value", "CASE WHEN {v} = 12345 THEN 0 ELSE {v} END", "", "" },
    };
}

TEST_CASE("ScoreJoin counts what SQLite's (rule) = column counts over the join", "[ColumnMatch]")
{
    ColumnMatchTempDb file("counts");
    DoltLiteDatabase db(file.path.string());
    MakeColumnMatchFixture(db);

    ValueEncoder encoder;
    LoadedTable source = LoadTable(db, "src", encoder);
    LoadedTable target = LoadTable(db, "tgt", encoder);
    PreparedJoin join = PrepareJoin(source, source.ColumnIndex("id"), "tgt", target, target.ColumnIndex("ID"));
    CHECK(join.sourceRows.size() == 6);

    for (std::string const& sourceColumn : { "a", "r", "t" })
    {
        for (SpecRuleRow const& rule : ColumnMatchRules())
        {
            std::vector<SpecRuleRow> single = { rule };
            std::vector<RuleVariant> variants = ApplyRules(db, "src", source, source.ColumnIndex(sourceColumn), single, encoder);
            REQUIRE(variants.size() == 1);
            std::string category = TypeCategory(source.columns[source.ColumnIndex(sourceColumn)].type);

            for (MatchColumn const& targetColumn : target.columns)
            {
                if (TypeCategory(targetColumn.type) != category)
                    continue;

                PreparedJoin oneColumn = join;
                int index = target.ColumnIndex(targetColumn.name);
                oneColumn.targetColumns = { join.targetColumns[index] };
                oneColumn.targetValues = { join.targetValues[index] };
                BestColumnMatch best;
                ScoreJoin(oneColumn, variants, category, best);

                std::string expr = rule.expr;
                for (size_t at = expr.find("{v}"); at != std::string::npos; at = expr.find("{v}"))
                    expr.replace(at, 3, "s.\"" + std::string(sourceColumn) + "\"");
                DoltLiteStatement stmt = db.Prepare("SELECT SUM(CASE WHEN (" + expr + ") = c.\"" + targetColumn.name +
                    "\" THEN 1 ELSE 0 END) FROM src s JOIN tgt c ON s.id = c.ID");
                REQUIRE(stmt.Step());
                INFO(sourceColumn << " " << rule.name << " " << targetColumn.name);
                CHECK(best.matched == stmt.ColumnInt64(0));
                CHECK(best.joined == 6);
            }
        }
    }
}

TEST_CASE("ApplyRules drops a rule identical to an earlier one and fails as a whole", "[ColumnMatch]")
{
    ColumnMatchTempDb file("rules");
    DoltLiteDatabase db(file.path.string());
    MakeColumnMatchFixture(db);

    ValueEncoder encoder;
    LoadedTable source = LoadTable(db, "src", encoder);
    std::vector<SpecRuleRow> rules = ColumnMatchRules();

    std::vector<RuleVariant> variants = ApplyRules(db, "src", source, source.ColumnIndex("a"), rules, encoder);
    // as_text compares like identity against a numeric column: affinity turns '5' back into 5.
    REQUIRE(variants.size() == 2);
    CHECK(variants[0].rule->name == "identity");
    CHECK(variants[1].rule->name == "plus_one");

    rules.push_back({ "broken", "value", "no_such_function({v})", "", "" });
    CHECK(ApplyRules(db, "src", source, source.ColumnIndex("a"), rules, encoder).empty());
}

TEST_CASE("ScoreJoin keeps the first of equal rates", "[ColumnMatch]")
{
    ColumnMatchTempDb file("ties");
    DoltLiteDatabase db(file.path.string());
    MakeColumnMatchFixture(db);

    ValueEncoder encoder;
    LoadedTable source = LoadTable(db, "src", encoder);
    LoadedTable target = LoadTable(db, "tgt", encoder);
    PreparedJoin join = PrepareJoin(source, source.ColumnIndex("id"), "tgt", target, target.ColumnIndex("ID"));
    std::vector<SpecRuleRow> const rules = ColumnMatchRules();
    std::vector<RuleVariant> variants = ApplyRules(db, "src", source, source.ColumnIndex("a"), rules, encoder);

    BestColumnMatch best;
    ScoreJoin(join, variants, "numeric", best);
    CHECK(best.targetColumn == "x");
    CHECK(best.ruleName == "identity");
    CHECK(best.matched == 4);
}

TEST_CASE("ParallelFor runs every index once and rethrows a failure", "[ColumnMatch]")
{
    std::vector<int> seen(1000, 0);
    ParallelFor(seen.size(), [&](size_t i) { ++seen[i]; });
    CHECK(std::all_of(seen.begin(), seen.end(), [](int count) { return count == 1; }));

    CHECK_THROWS(ParallelFor(10, [](size_t i) { if (i == 7) throw std::runtime_error("seven"); }));
}
