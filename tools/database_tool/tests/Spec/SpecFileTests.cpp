#include "Spec/SpecFile.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_specfile_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir() { std::filesystem::remove_all(path); }
    };
}

TEST_CASE("ReadColumnsTsv returns nothing for a missing file", "[SpecFile]")
{
    TempDir dir("missing");
    CHECK(ReadColumnsTsv(dir.path / "columns.tsv").empty());
    CHECK(ReadDbcColumnsTsv(dir.path / "dbc-columns.tsv").empty());
}

TEST_CASE("columns.tsv round-trips every field", "[SpecFile]")
{
    TempDir dir("roundtrip");
    std::filesystem::path path = dir.path / "columns.tsv";

    std::vector<SpecColumnRow> rows = {
        { "srv", "spell_template", "id", "server", "", "", "primary key", "yes" },
        { "vdbc", "areatable", "Flags", "dbc", "dbc_areatable.Flags", "identity", "", "" },
    };
    WriteColumnsTsv(path, rows);

    std::vector<SpecColumnRow> read = ReadColumnsTsv(path);
    REQUIRE(read.size() == 2);
    CHECK(read[0].source == "srv");
    CHECK(read[0].table == "spell_template");
    CHECK(read[0].column == "id");
    CHECK(read[0].disposition == "server");
    CHECK(read[0].reason == "primary key");
    CHECK(read[0].reviewed == "yes");
    CHECK(read[1].target == "dbc_areatable.Flags");
    CHECK(read[1].rule == "identity");
}

TEST_CASE("WriteColumnsTsv sanitizes embedded tabs and newlines in hand-written fields", "[SpecFile]")
{
    TempDir dir("sanitize");
    std::filesystem::path path = dir.path / "columns.tsv";

    WriteColumnsTsv(path, { { "srv", "t", "c", "drop", "", "", "line one\nline two\twith tab", "" } });

    std::vector<SpecColumnRow> read = ReadColumnsTsv(path);
    REQUIRE(read.size() == 1);
    CHECK(read[0].reason == "line one line two with tab");
}

TEST_CASE("ReadColumnsTsv rejects a file with the wrong header", "[SpecFile]")
{
    TempDir dir("badheader");
    std::filesystem::path path = dir.path / "columns.tsv";

    std::ofstream(path) << "not\tthe\tright\theader\n";
    CHECK_THROWS(ReadColumnsTsv(path));
}

TEST_CASE("dbc-columns.tsv round-trips every field", "[SpecFile]")
{
    TempDir dir("dbcroundtrip");
    std::filesystem::path path = dir.path / "dbc-columns.tsv";

    WriteDbcColumnsTsv(path, { { "dbc_areatable", "ContentTuningID", "AreaTableEntry::flags", "client" } });

    std::vector<SpecDbcColumnRow> read = ReadDbcColumnsTsv(path);
    REQUIRE(read.size() == 1);
    CHECK(read[0].table == "dbc_areatable");
    CHECK(read[0].column == "ContentTuningID");
    CHECK(read[0].mangosdReads == "AreaTableEntry::flags");
    CHECK(read[0].vanillaValue == "client");
}
