#include "Dbc/Wdc5Writer.h"

#include "Dbc/Wdc5Reader.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    struct TempFile
    {
        std::filesystem::path path;

        explicit TempFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_wdc5writer_" + name + ".db2"))
        {
        }

        ~TempFile() { std::filesystem::remove(path); }
    };

    Wdc5ReadResult RoundTrip(DbcTableMeta const& table, Wdc5FileInfo const& info, std::vector<Wdc5Row> const& rows,
                             std::string const& name)
    {
        Wdc5WriteResult written = WriteWdc5File(table, info, rows);
        INFO(written.error);
        REQUIRE(written.error.empty());

        TempFile file(name);
        std::ofstream out(file.path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<char const*>(written.bytes.data()), static_cast<std::streamsize>(written.bytes.size()));
        out.close();

        return ReadWdc5File(file.path, table);
    }

    Wdc5Row MakeRow(int64_t id, double weight, std::string name)
    {
        Wdc5Row row(3);
        row[0].intValue = id;
        row[1].floatValue = weight;
        row[2].stringValue = std::move(name);
        return row;
    }
}

TEST_CASE("WriteWdc5File round-trips a regular table with an inline id", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Weight", DbcFieldType::Float, 0, true, 1, false, false, false, nullptr, nullptr },
        { "Name", DbcFieldType::String, 0, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.locale = 1;
    info.flags = 0;
    info.schema = "TestSchema";

    // Deliberately out of id order, to check the writer sorts by id.
    std::vector<Wdc5Row> rows = {
        MakeRow(20, -1.25, "Bravo"),
        MakeRow(10, 3.5, "Alpha"),
        MakeRow(30, 0.0, ""),
    };

    Wdc5ReadResult result = RoundTrip(table, info, rows, "regular_inline_id");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 3);
    CHECK(result.rows[0][0].intValue == 10);
    CHECK(result.rows[0][1].floatValue == Catch::Approx(3.5));
    CHECK(result.rows[0][2].stringValue == "Alpha");
    CHECK(result.rows[1][0].intValue == 20);
    CHECK(result.rows[1][2].stringValue == "Bravo");
    CHECK(result.rows[2][0].intValue == 30);
    CHECK(result.rows[2][2].stringValue.empty());

    CHECK(result.info.tableHash == 0x1234);
    CHECK(result.info.layoutHash == 0xABCD);
    CHECK(result.info.schema == "TestSchema");
}

TEST_CASE("WriteWdc5File round-trips a non-inline id and a non-inline relation", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, true, nullptr, nullptr },
        { "Value", DbcFieldType::Int, 32, true, 1, false, false, false, nullptr, nullptr },
        { "ParentID", DbcFieldType::Int, 32, false, 1, false, true, true, "Parent", "ID" },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.locale = 1;
    info.flags = 0x4; // has an id list

    Wdc5Row rowA(3), rowB(3);
    rowA[0].intValue = 1; rowA[1].intValue = 111; rowA[2].intValue = 999;
    rowB[0].intValue = 2; rowB[1].intValue = 222; rowB[2].intValue = 0;
    std::vector<Wdc5Row> rows = { rowA, rowB };

    Wdc5ReadResult result = RoundTrip(table, info, rows, "noninline_id_relation");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 1);
    CHECK(result.rows[0][1].intValue == 111);
    CHECK(result.rows[0][2].intValue == 999);
    CHECK(result.rows[1][0].intValue == 2);
    CHECK(result.rows[1][1].intValue == 222);
    CHECK(result.rows[1][2].intValue == 0);
}

TEST_CASE("WriteWdc5File round-trips an array field", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Flags", DbcFieldType::Int, 16, false, 3, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.locale = 1;

    Wdc5Row row(4);
    row[0].intValue = 5;
    row[1].intValue = 10;
    row[2].intValue = 20;
    row[3].intValue = 30;

    Wdc5ReadResult result = RoundTrip(table, info, { row }, "array_field");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][1].intValue == 10);
    CHECK(result.rows[0][2].intValue == 20);
    CHECK(result.rows[0][3].intValue == 30);
}

TEST_CASE("WriteWdc5File round-trips an empty table", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = { { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr } };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 1 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;

    Wdc5ReadResult result = RoundTrip(table, info, {}, "empty");

    INFO(result.error);
    CHECK(result.error.empty());
    CHECK(result.rows.empty());
}

TEST_CASE("WriteWdc5File round-trips a sparse table", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Value", DbcFieldType::Int, 16, false, 1, false, false, false, nullptr, nullptr },
        { "Name", DbcFieldType::String, 0, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.locale = 1;
    info.flags = 0x1; // sparse

    Wdc5Row rowA(3), rowB(3);
    rowA[0].intValue = 7; rowA[1].intValue = 11; rowA[2].stringValue = "Ax";
    rowB[0].intValue = 8; rowB[1].intValue = 22; rowB[2].stringValue = "Bravo";

    Wdc5ReadResult result = RoundTrip(table, info, { rowB, rowA }, "sparse");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 7);
    CHECK(result.rows[0][1].intValue == 11);
    CHECK(result.rows[0][2].stringValue == "Ax");
    CHECK(result.rows[1][0].intValue == 8);
    CHECK(result.rows[1][1].intValue == 22);
    CHECK(result.rows[1][2].stringValue == "Bravo");
    CHECK((result.info.flags & 0x1) != 0);
}

TEST_CASE("WriteWdc5File round-trips a non-inline id on a sparse table", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, true, nullptr, nullptr },
        { "Value", DbcFieldType::Int, 16, false, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.flags = 0x5; // sparse plus id list

    Wdc5Row rowA(2), rowB(2);
    rowA[0].intValue = 42; rowA[1].intValue = 1;
    rowB[0].intValue = 43; rowB[1].intValue = 2;

    Wdc5ReadResult result = RoundTrip(table, info, { rowA, rowB }, "sparse_noninline_id");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 42);
    CHECK(result.rows[0][1].intValue == 1);
    CHECK(result.rows[1][0].intValue == 43);
    CHECK(result.rows[1][1].intValue == 2);
}
