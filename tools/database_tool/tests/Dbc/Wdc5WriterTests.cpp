#include "Dbc/Wdc5Writer.h"

#include "Dbc/Wdc5Reader.h"

#include <catch_amalgamated.hpp>

#include <cstring>
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

    uint32_t U32At(std::vector<uint8_t> const& bytes, size_t pos)
    {
        uint32_t v;
        std::memcpy(&v, bytes.data() + pos, 4);
        return v;
    }

    uint16_t U16At(std::vector<uint8_t> const& bytes, size_t pos)
    {
        uint16_t v;
        std::memcpy(&v, bytes.data() + pos, 2);
        return v;
    }

    // Offsets into a single-section WDC5 file.
    constexpr size_t HeaderFieldCount = 140;
    constexpr size_t HeaderPalletDataSize = 196;
    constexpr size_t HeaderParentLookupCount = 184;
    constexpr size_t SectionParentLookupSize = 232;
    constexpr size_t FieldStructures = 244;

    size_t ColumnMeta(std::vector<uint8_t> const& bytes, uint32_t column)
    {
        return FieldStructures + 4 * U32At(bytes, HeaderFieldCount) + 24 * column;
    }

    uint32_t CompressionType(std::vector<uint8_t> const& bytes, uint32_t column) { return U32At(bytes, ColumnMeta(bytes, column) + 8); }
    uint16_t ColumnBits(std::vector<uint8_t> const& bytes, uint32_t column) { return U16At(bytes, ColumnMeta(bytes, column) + 2); }

    Wdc5FileInfo TestInfo(uint16_t flags = 0)
    {
        Wdc5FileInfo info;
        info.tableHash = 0x1234;
        info.layoutHash = 0xABCD;
        info.locale = 1;
        info.flags = flags;
        return info;
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

TEST_CASE("WriteWdc5File sizes a sparse array field's slot for all its elements", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Values", DbcFieldType::Int, 16, false, 3, false, false, false, nullptr, nullptr },
        { "Tail", DbcFieldType::Int, 32, false, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 3 };

    Wdc5FileInfo info;
    info.tableHash = 0x1234;
    info.layoutHash = 0xABCD;
    info.locale = 1;
    info.flags = 0x1;

    Wdc5Row row(5);
    row[0].intValue = 7;
    row[1].intValue = 1; row[2].intValue = 2; row[3].intValue = 3;
    row[4].intValue = 99;

    Wdc5ReadResult result = RoundTrip(table, info, { row }, "sparse_array");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][3].intValue == 3);
    CHECK(result.rows[0][4].intValue == 99);
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

TEST_CASE("WriteWdc5File keeps uncompressed numeric fields before compressed ones", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "Name", DbcFieldType::String, 0, true, 1, false, false, false, nullptr, nullptr },
        { "Coords", DbcFieldType::Float, 0, true, 2, false, false, false, nullptr, nullptr },
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Weights", DbcFieldType::Int, 32, true, 3, false, false, false, nullptr, nullptr },
        { "Scale", DbcFieldType::Float, 0, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 5 };

    auto makeRow = [](int64_t id, std::vector<int64_t> weights, double scale)
    {
        Wdc5Row row(8);
        row[0].stringValue = "Row" + std::to_string(id);
        row[1].floatValue = 1.5;
        row[2].floatValue = -2.5;
        row[3].intValue = id;
        for (size_t i = 0; i < 3; ++i)
            row[4 + i].intValue = weights[i];
        row[7].floatValue = scale;
        return row;
    };
    std::vector<Wdc5Row> rows = {
        makeRow(1, { 1, 2, 3 }, 0.5),
        makeRow(2, { -4, 5, 6 }, 2.0),
        makeRow(3, { 1, 2, 3 }, 0.5),
    };

    Wdc5WriteResult written = WriteWdc5File(table, TestInfo(), rows);
    REQUIRE(written.error.empty());
    std::vector<uint8_t> const& bytes = written.bytes;

    CHECK(CompressionType(bytes, 0) == 0); // Name: None
    CHECK(CompressionType(bytes, 1) == 0); // Coords: None, nothing compressed before it
    CHECK(CompressionType(bytes, 2) == 1); // ID: Immediate
    CHECK(CompressionType(bytes, 3) == 4); // Weights: PalletArray
    CHECK(CompressionType(bytes, 4) == 1); // Scale: Immediate
    CHECK(U32At(bytes, HeaderPalletDataSize) == 2 * 3 * 4); // identical tuples share one pallet entry

    Wdc5ReadResult result = RoundTrip(table, TestInfo(), rows, "compression_layout");
    INFO(result.error);
    REQUIRE(result.error.empty());
    REQUIRE(result.rows.size() == 3);
    CHECK(result.rows[1][0].stringValue == "Row2");
    CHECK(result.rows[1][1].floatValue == Catch::Approx(1.5));
    CHECK(result.rows[1][2].floatValue == Catch::Approx(-2.5));
    CHECK(result.rows[1][3].intValue == 2);
    CHECK(static_cast<uint32_t>(result.rows[1][4].intValue) == static_cast<uint32_t>(-4));
    CHECK(result.rows[1][6].intValue == 6);
    CHECK(result.rows[1][7].floatValue == Catch::Approx(2.0));
    CHECK(result.rows[2][5].intValue == 2);
    CHECK(result.rows[2][7].floatValue == Catch::Approx(0.5));
}

TEST_CASE("WriteWdc5File writes a parent lookup for an inline relation", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "ParentID", DbcFieldType::Int, 32, false, 1, false, true, false, "Parent", "ID" },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 2 };

    Wdc5Row rowA(2), rowB(2), rowC(2);
    rowA[0].intValue = 10; rowA[1].intValue = 7;
    rowB[0].intValue = 20; rowB[1].intValue = 3;
    rowC[0].intValue = 30; rowC[1].intValue = 7;
    std::vector<Wdc5Row> rows = { rowC, rowA, rowB };

    Wdc5WriteResult written = WriteWdc5File(table, TestInfo(), rows);
    REQUIRE(written.error.empty());
    std::vector<uint8_t> const& bytes = written.bytes;

    CHECK(U32At(bytes, HeaderParentLookupCount) == 1);
    CHECK(U32At(bytes, SectionParentLookupSize) == 12 + 8 * 3);

    // NumEntries, MinId, MaxId, then (parent, record index) in id order.
    size_t lookup = bytes.size() - (12 + 8 * 3);
    CHECK(U32At(bytes, lookup) == 3);
    CHECK(U32At(bytes, lookup + 4) == 3);
    CHECK(U32At(bytes, lookup + 8) == 7);
    CHECK(U32At(bytes, lookup + 12) == 7);
    CHECK(U32At(bytes, lookup + 16) == 0);
    CHECK(U32At(bytes, lookup + 20) == 3);
    CHECK(U32At(bytes, lookup + 24) == 1);
    CHECK(U32At(bytes, lookup + 28) == 7);
    CHECK(U32At(bytes, lookup + 32) == 2);

    Wdc5ReadResult result = RoundTrip(table, TestInfo(), rows, "inline_relation");
    INFO(result.error);
    REQUIRE(result.error.empty());
    REQUIRE(result.rows.size() == 3);
    CHECK(result.rows[0][1].intValue == 7);
    CHECK(result.rows[1][1].intValue == 3);
    CHECK(result.rows[2][1].intValue == 7);
}

TEST_CASE("WriteWdc5File widens an int field only when a value doesn't fit either signedness", "[Wdc5Writer]")
{
    DbcFieldMeta fields[] = {
        { "ID", DbcFieldType::Int, 32, false, 1, true, false, false, nullptr, nullptr },
        { "Small", DbcFieldType::Int, 8, false, 1, false, false, false, nullptr, nullptr },
        { "Overflow", DbcFieldType::Int, 8, false, 1, false, false, false, nullptr, nullptr },
        { "Signed", DbcFieldType::Int, 32, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t layoutHashes[] = { 0xABCDu };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 1, fields, 4 };

    Wdc5Row rowA(4), rowB(4);
    rowA[0].intValue = 1; rowA[1].intValue = 255; rowA[2].intValue = 5;   rowA[3].intValue = 0xFFFFFFFFll;
    rowB[0].intValue = 2; rowB[1].intValue = -1;  rowB[2].intValue = 300; rowB[3].intValue = -2;
    std::vector<Wdc5Row> rows = { rowA, rowB };

    Wdc5WriteResult written = WriteWdc5File(table, TestInfo(), rows);
    REQUIRE(written.error.empty());

    CHECK(ColumnBits(written.bytes, 1) == 8);  // 255 and -1 both fit 8 bits
    CHECK(ColumnBits(written.bytes, 2) == 16); // 300 doesn't
    CHECK(ColumnBits(written.bytes, 3) == 32); // the db's unsigned -1 keeps the declared width

    Wdc5ReadResult result = RoundTrip(table, TestInfo(), rows, "width_rule");
    INFO(result.error);
    REQUIRE(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][1].intValue == 255);
    CHECK(static_cast<uint32_t>(result.rows[0][3].intValue) == 0xFFFFFFFFu);
    CHECK(static_cast<uint32_t>(result.rows[1][3].intValue) == 0xFFFFFFFEu);
}
