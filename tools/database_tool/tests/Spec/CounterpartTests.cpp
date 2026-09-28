#include "Spec/Counterpart.h"

#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <filesystem>

namespace
{
    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_counterpart_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };

    std::vector<SpecRuleRow> FixtureRules()
    {
        return {
            { "identity", "value", "{v}", "{v}", "default" },
            { "added_bit", "value", "({v} | 4)", "({v} & ~4)", "flags gain a bit" },
        };
    }
}

TEST_CASE("ComputeCounterparts finds an identity match over an id join", "[Counterpart]")
{
    TempDoltLiteFile reference("identity");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_areatable\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (1, 5), (2, 6)");
    db.Execute("INSERT INTO \"cdbc_areatable\" VALUES (1, 5), (2, 6)");

    std::vector<SpecPairRow> pairs = { { "srv", "area_template", "areatable", "id", "entry", "ID", "" } };
    ComputeCounterpartsResult result = ComputeCounterparts(db, pairs, FixtureRules());

    CHECK(result.pairsJoined == 1);
    CHECK(result.pairsSkipped == 0);

    auto it = result.matches.find("srv\tarea_template\tflags");
    REQUIRE(it != result.matches.end());
    CHECK(it->second.counterpart == "cdbc_areatable.Flags");
    CHECK(it->second.counterpartRule == "identity");
    CHECK(it->second.matchRate == "2/2");
}

TEST_CASE("ComputeCounterparts prefers the rule with the higher match rate", "[Counterpart]")
{
    TempDoltLiteFile reference("bestrule");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_areatable\" (\"ID\" INTEGER, \"Flags\" INTEGER)");
    // Every row needs the +4 bit to match: identity matches 0/3, added_bit matches 3/3. The flag
    // values (1, 2, 3) deliberately avoid the row ids (101, 102, 103), so identity can't win by
    // coincidentally matching the id column instead of Flags.
    db.Execute("INSERT INTO \"srv_area_template\" VALUES (101, 1), (102, 2), (103, 3)");
    db.Execute("INSERT INTO \"cdbc_areatable\" VALUES (101, 5), (102, 6), (103, 7)");

    std::vector<SpecPairRow> pairs = { { "srv", "area_template", "areatable", "id", "entry", "ID", "" } };
    ComputeCounterpartsResult result = ComputeCounterparts(db, pairs, FixtureRules());

    auto it = result.matches.find("srv\tarea_template\tflags");
    REQUIRE(it != result.matches.end());
    CHECK(it->second.counterpart == "cdbc_areatable.Flags");
    CHECK(it->second.counterpartRule == "added_bit");
    CHECK(it->second.matchRate == "3/3");
}

TEST_CASE("ComputeCounterparts searches every cdbc table paired with the same source table", "[Counterpart]")
{
    TempDoltLiteFile reference("multitable");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_item_template\" (\"entry\" INTEGER, \"quality\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_item\" (\"ID\" INTEGER, \"ClassID\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_itemsparse\" (\"ID\" INTEGER, \"Quality\" INTEGER)");
    db.Execute("INSERT INTO \"srv_item_template\" VALUES (1, 3)");
    db.Execute("INSERT INTO \"cdbc_item\" VALUES (1, 9)");
    db.Execute("INSERT INTO \"cdbc_itemsparse\" VALUES (1, 3)");

    std::vector<SpecPairRow> pairs = {
        { "srv", "item_template", "item", "id", "entry", "ID", "" },
        { "srv", "item_template", "itemsparse", "id", "entry", "ID", "" },
    };
    ComputeCounterpartsResult result = ComputeCounterparts(db, pairs, FixtureRules());

    CHECK(result.pairsJoined == 2);
    auto it = result.matches.find("srv\titem_template\tquality");
    REQUIRE(it != result.matches.end());
    CHECK(it->second.counterpart == "cdbc_itemsparse.Quality");
}

TEST_CASE("ComputeCounterparts skips unpivot/skip pairs and pairs with missing tables or keys", "[Counterpart]")
{
    TempDoltLiteFile reference("skips");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_area_template\" (\"entry\" INTEGER, \"flags\" INTEGER)");
    db.Execute("CREATE TABLE \"cdbc_areatable\" (\"ID\" INTEGER, \"Flags\" INTEGER)");

    std::vector<SpecPairRow> pairs = {
        { "srv", "spell_template", "spelleffect", "unpivot", "entry", "SpellID", "" },
        { "srv", "playercreateinfo_item", "charstartoutfit", "skip", "", "", "" },
        { "srv", "no_such_table", "areatable", "id", "id", "ID", "" },
        { "srv", "area_template", "no_such_cdbc_table", "id", "entry", "ID", "" },
        { "srv", "area_template", "areatable", "id", "no_such_column", "ID", "" },
    };
    ComputeCounterpartsResult result = ComputeCounterparts(db, pairs, FixtureRules());

    CHECK(result.pairsJoined == 0);
    CHECK(result.pairsSkipped == 5);
    CHECK(result.matches.empty());
}

TEST_CASE("ComputeCounterparts leaves a column unmatched when nothing joins", "[Counterpart]")
{
    TempDoltLiteFile reference("empty");
    DoltLiteDatabase db(reference.path.string());
    db.Execute("CREATE TABLE \"srv_broadcast_text\" (\"entry\" INTEGER, \"male_text\" TEXT)");
    db.Execute("CREATE TABLE \"cdbc_broadcasttext\" (\"ID\" INTEGER, \"Text\" TEXT)");

    std::vector<SpecPairRow> pairs = { { "srv", "broadcast_text", "broadcasttext", "id", "entry", "ID", "" } };
    ComputeCounterpartsResult result = ComputeCounterparts(db, pairs, FixtureRules());

    CHECK(result.pairsJoined == 1);
    CHECK(result.matches.empty());
}
