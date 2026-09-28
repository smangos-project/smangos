#include "Spec/SourceScan.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <map>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_sourcescan_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir() { std::filesystem::remove_all(path); }
    };
}

TEST_CASE("ParseFmtStrings reads plain declarations", "[SourceScan]")
{
    std::string text =
        "char const AreaTableEntryfmt[]=\"niiiixxxxxissssssssxixxxi\";\n"
        "char const AreaTriggerEntryfmt[]=\"niffffffff\";\n";
    std::map<std::string, std::string> fmts = ParseFmtStrings(text);
    REQUIRE(fmts.count("AreaTableEntryfmt"));
    CHECK(fmts["AreaTableEntryfmt"] == "niiiixxxxxissssssssxixxxi");
    CHECK(fmts["AreaTriggerEntryfmt"] == "niffffffff");
}

TEST_CASE("ParseFmtStrings picks the first '>' branch for this project's fixed vanilla build", "[SourceScan]")
{
    std::string text =
        "#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_10_2\n"
        "char const ItemSetEntryfmt[]=\"short\";\n"
        "#elif SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1\n"
        "char const ItemSetEntryfmt[] = \"medium\";\n"
        "#else\n"
        "char const ItemSetEntryfmt[] = \"long\";\n"
        "#endif\n";
    std::map<std::string, std::string> fmts = ParseFmtStrings(text);
    REQUIRE(fmts.count("ItemSetEntryfmt"));
    CHECK(fmts["ItemSetEntryfmt"] == "short");
}

TEST_CASE("ParseFmtStrings falls back to the last branch when it can't read the condition", "[SourceScan]")
{
    std::string text =
        "#if defined(SOME_UNKNOWN_MACRO)\n"
        "char const Xfmt[] = \"a\";\n"
        "#else\n"
        "char const Xfmt[] = \"b\";\n"
        "#endif\n";
    std::map<std::string, std::string> fmts = ParseFmtStrings(text);
    REQUIRE(fmts.count("Xfmt"));
    CHECK(fmts["Xfmt"] == "b");
}

TEST_CASE("ParseDbcStorageLinks links storage variables to their struct and format", "[SourceScan]")
{
    std::string text =
        "DBCStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);\n"
        "//DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently\n"
        "DBCStorage <WMOAreaTableEntry>  sWMOAreaTableStore(WMOAreaTableEntryfmt);\n";
    std::vector<DbcStorageLink> links = ParseDbcStorageLinks(text);
    REQUIRE(links.size() == 2);
    CHECK(links[0].varName == "sAuctionHouseStore");
    CHECK(links[0].structName == "AuctionHouseEntry");
    CHECK(links[0].fmtName == "AuctionHouseEntryfmt");
    CHECK(links[1].varName == "sWMOAreaTableStore");
}

TEST_CASE("ParseLoadDbcCalls extracts the storage var and dbc file stem", "[SourceScan]")
{
    std::string text =
        "    LoadDBC(availableDbcLocales, bar, badDbcFiles, sAuctionHouseStore,        dbcPath, \"AuctionHouse.dbc\");\n"
        "    //LoadDBC(availableDbcLocales,bar,badDbcFiles,sItemDisplayInfoStore,     dbcPath,\"ItemDisplayInfo.dbc\");\n"
        "    LoadDBC(availableDbcLocales, bar, badDbcFiles, sWMOAreaTableStore,        dbcPath, \"WMOAreaTable.dbc\");\n";
    std::vector<DbcLoadCall> calls = ParseLoadDbcCalls(text);
    REQUIRE(calls.size() == 2);
    CHECK(calls[0].varName == "sAuctionHouseStore");
    CHECK(calls[0].fileStem == "AuctionHouse");
    CHECK(calls[1].varName == "sWMOAreaTableStore");
    CHECK(calls[1].fileStem == "WMOAreaTable");
    CHECK(calls[1].line == 3);
}

TEST_CASE("ParseSqlStorageLinks handles both the single-fmt and src/dst-fmt forms", "[SourceScan]")
{
    std::string text =
        "SQLStorage sPageTextStore(PageTextfmt, \"entry\", \"page_text\");\n"
        "SQLStorage sMapStorage(MapEntrysrcfmt, MapEntrydstfmt, \"entry\", \"map_template\");\n";
    std::vector<SqlStorageLink> links = ParseSqlStorageLinks(text);
    REQUIRE(links.size() == 2);
    CHECK(links[0].varName == "sPageTextStore");
    CHECK(links[0].fmtName == "PageTextfmt");
    CHECK(links[0].key == "entry");
    CHECK(links[0].table == "page_text");
    CHECK(links[1].varName == "sMapStorage");
    CHECK(links[1].fmtName == "MapEntrydstfmt"); // the destination fmt, matching the in-memory struct
    CHECK(links[1].table == "map_template");
}

TEST_CASE("ParseStructFields walks a struct positionally using explicit widths", "[SourceScan]")
{
    // WMOAreaTableEntry (DBCStructure.h), trimmed to its essentials: real fields, a run of
    // commented-out scalar fields, and a commented-out array field with no position comment.
    std::string text =
        "struct WMOAreaTableEntry\n"
        "{\n"
        "    uint32 Id;                                              // 0        m_ID index\n"
        "    int32 rootId;                                           // 1        m_WMOID\n"
        "    int32 adtId;                                            // 2        m_NameSetID\n"
        "    int32 groupId;                                          // 3        m_WMOGroupID\n"
        "    //uint32 field4;                                        // 4        m_SoundProviderPref\n"
        "    //uint32 field5;                                        // 5\n"
        "    //uint32 field6;                                        // 6\n"
        "    //uint32 field7;                                        // 7\n"
        "    //uint32 field8;                                        // 8\n"
        "    uint32 Flags;                                           // 9        m_flags\n"
        "    uint32 areaId;                                          // 10       m_AreaTableID\n"
        "    //char *Name[8];                                        //          m_AreaName_lang\n"
        "    //uint32 nameFlags;\n"
        "};\n";

    std::vector<StructField> fields = ParseStructFields(text, "WMOAreaTableEntry");

    uint32_t total = 0;
    for (StructField const& f : fields)
        total += f.width;
    CHECK(total == 20); // matches WMOAreaTableEntryfmt's length ("niiixxxxxiixxxxxxxxx")

    // areaId is the 11th consumed position (0-indexed 10), matching its own "// 10" comment --
    // used here only as a cross-check on the positional walk, not to drive it.
    size_t position = 0;
    std::string fieldAtTen;
    for (StructField const& f : fields)
    {
        if (position <= 10 && 10 < position + f.width)
            fieldAtTen = f.name;
        position += f.width;
    }
    CHECK(fieldAtTen == "areaId");

    bool foundAreaId = false, foundName = false;
    for (StructField const& f : fields)
    {
        if (f.name == "areaId") { foundAreaId = true; CHECK(f.isReal); CHECK(f.width == 1); }
        if (f.name == "Name") { foundName = true; CHECK_FALSE(f.isReal); CHECK(f.width == 8); }
    }
    CHECK(foundAreaId);
    CHECK(foundName);
}

TEST_CASE("ParseStructFields resolves a #if block inside a struct body", "[SourceScan]")
{
    // ItemSetEntry's build-gated tail fields: SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1 is
    // always true for this project's fixed vanilla build, so both extra fields are present.
    std::string text =
        "struct ItemSetEntry\n"
        "{\n"
        "    uint32    spells[8];\n"
        "#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_6_1\n"
        "    uint32    required_skill_id;\n"
        "    uint32    required_skill_value;\n"
        "#endif\n"
        "};\n";
    std::vector<StructField> fields = ParseStructFields(text, "ItemSetEntry");
    REQUIRE(fields.size() == 3);
    CHECK(fields[0].name == "spells");
    CHECK(fields[0].width == 8);
    CHECK(fields[1].name == "required_skill_id");
    CHECK(fields[2].name == "required_skill_value");
}

TEST_CASE("ParseStructFields skips methods and access specifiers", "[SourceScan]")
{
    std::string text =
        "class ConditionEntry\n"
        "{\n"
        "    public:\n"
        "        ConditionEntry() : m_entry(0) {}\n"
        "        bool Meets(int x) const { return x != 0; }\n"
        "    private:\n"
        "        uint32 m_entry;\n"
        "        int32 m_value1;\n"
        "};\n";
    std::vector<StructField> fields = ParseStructFields(text, "ConditionEntry");
    REQUIRE(fields.size() == 2);
    CHECK(fields[0].name == "m_entry");
    CHECK(fields[1].name == "m_value1");
}

TEST_CASE("ParseStructFields returns nothing for a struct that isn't in the text", "[SourceScan]")
{
    CHECK(ParseStructFields("struct Foo { uint32 x; };", "Bar").empty());
}

TEST_CASE("CountFieldUsesInText counts -> and . accesses, not declarations", "[SourceScan]")
{
    std::string text =
        "uint32 areaId; // declaration, not a use\n"
        "if (entry->areaId == 1) DoSomething();\n"
        "return other.areaId + entry->areaId;\n";
    CHECK(CountFieldUsesInText(text, "areaId") == 3);
    CHECK(CountFieldUsesInText(text, "flags") == 0);
}

TEST_CASE("ScanHandwrittenQueries parses a plain column list and rejects * and JOIN", "[SourceScan]")
{
    TempDir dir("handwritten");

    std::filesystem::path file = dir.path / "Loader.cpp";
    {
        std::ofstream out(file);
        out << "void LoadThings()\n{\n"
               "    result = WorldDatabase.Query(\"SELECT `guid`, `entry`, `map` FROM `creature`\");\n"
               "    Field* fields = result->Fetch();\n"
               "    data.guid = fields[0].GetUInt32();\n"
               "    data.entry = fields[1].GetUInt32();\n"
               "    HandleMap(fields[2].GetUInt32());\n"
               "}\n"
               "void LoadOther()\n{\n"
               "    result = WorldDatabase.Query(\"SELECT * FROM `other_table`\");\n"
               "}\n"
               "void LoadJoined()\n{\n"
               "    result = WorldDatabase.Query(\"SELECT a.x FROM a JOIN b ON a.id = b.id\");\n"
               "}\n";
    }

    std::vector<HandwrittenQuery> queries = ScanHandwrittenQueries(dir.path);
    REQUIRE(queries.size() == 3);

    HandwrittenQuery const* creature = nullptr;
    HandwrittenQuery const* otherTable = nullptr;
    HandwrittenQuery const* joined = nullptr;
    for (HandwrittenQuery const& q : queries)
    {
        if (q.table == "creature") creature = &q;
        if (q.table == "other_table") otherTable = &q;
        if (q.table == "a") joined = &q;
    }
    REQUIRE(creature);
    REQUIRE(otherTable);
    REQUIRE(joined);

    CHECK(creature->parsable);
    REQUIRE(creature->columns.size() == 3);
    CHECK(creature->columns[0] == "guid");
    CHECK(creature->columns[1] == "entry");
    CHECK(creature->columns[2] == "map");

    CHECK_FALSE(otherTable->parsable); // "*"
    CHECK_FALSE(joined->parsable);     // JOIN
}

TEST_CASE("FindFieldsAccessLines finds a fields[N] use for each column, clean or not", "[SourceScan]")
{
    std::string text =
        "void LoadThings()\n{\n"
        "    result = WorldDatabase.Query(\"SELECT `guid`, `entry`, `map` FROM `creature`\");\n"
        "    Field* fields = result->Fetch();\n"
        "    data.guid = fields[0].GetUInt32();\n"
        "    HandleMap(fields[2].GetUInt32());\n"
        "}\n";

    HandwrittenQuery query;
    query.table = "creature";
    query.line = 3;
    query.parsable = true;
    query.columns = { "guid", "entry", "map" };

    std::vector<uint32_t> lines = FindFieldsAccessLines(text, query);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == 5); // clean assignment
    CHECK(lines[1] == 0); // entry (fields[1]) is never read
    CHECK(lines[2] == 6); // used as a call argument, not a plain assignment -- still found
}
