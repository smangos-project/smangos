#include "Dbc/Wdbc.h"

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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_wdbc_" + name + ".dbc"))
        {
        }

        ~TempFile() { std::filesystem::remove(path); }
    };

    void PutU32(std::vector<uint8_t>& buf, uint32_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PatchU32(std::vector<uint8_t>& buf, size_t pos, uint32_t v) { std::memcpy(buf.data() + pos, &v, 4); }

    DbcFieldMeta MakeField(char const* name, DbcFieldType type, uint8_t bits, bool isSigned, uint16_t arrayLength,
                            bool isId = false)
    {
        return { name, type, bits, isSigned, arrayLength, isId, false, false, nullptr, nullptr };
    }

    WdbcReadResult RoundTrip(DbcTableMeta const& table, std::vector<Wdc5Row> const& rows, std::string const& name,
                             WdbcLayout const& layout = {})
    {
        WdbcWriteResult written = WriteWdbcFile(table, rows, layout);
        REQUIRE(written.error.empty());

        TempFile file(name);
        std::ofstream out(file.path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<char const*>(written.bytes.data()), static_cast<std::streamsize>(written.bytes.size()));
        out.close();

        return ReadWdbcFile(file.path, table, layout);
    }
}

TEST_CASE("WriteWdbcFile round-trips int, float, string and array fields", "[Wdbc]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Weight", DbcFieldType::Float, 0, true, 1),
        MakeField("Name", DbcFieldType::String, 0, true, 1),
        MakeField("Flags", DbcFieldType::Int, 16, false, 2),
    };
    uint32_t layoutHashes[] = { 0 }; // unused by WDBC
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 0, fields, 4 };

    Wdc5Row rowA(5), rowB(5);
    rowA[0].intValue = 1; rowA[1].floatValue = 3.5; rowA[2].stringValue = "Alpha"; rowA[3].intValue = 10; rowA[4].intValue = 20;
    rowB[0].intValue = 2; rowB[1].floatValue = -1.25; rowB[2].stringValue = ""; rowB[3].intValue = 30; rowB[4].intValue = 40;

    WdbcReadResult result = RoundTrip(table, { rowA, rowB }, "basic");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 2);
    CHECK(result.rows[0][0].intValue == 1);
    CHECK(result.rows[0][1].floatValue == Catch::Approx(3.5));
    CHECK(result.rows[0][2].stringValue == "Alpha");
    CHECK(result.rows[0][3].intValue == 10);
    CHECK(result.rows[0][4].intValue == 20);
    CHECK(result.rows[1][0].intValue == 2);
    CHECK(result.rows[1][1].floatValue == Catch::Approx(-1.25));
    CHECK(result.rows[1][2].stringValue.empty());
}

TEST_CASE("WriteWdbcFile round-trips a signed field", "[Wdbc]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, true),
        MakeField("Delta", DbcFieldType::Int, 16, true, 1),
    };
    uint32_t layoutHashes[] = { 0 };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 0, fields, 2 };

    Wdc5Row row(2);
    row[0].intValue = 1;
    row[1].intValue = -5;

    WdbcReadResult result = RoundTrip(table, { row }, "signed");

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    CHECK(result.rows[0][1].intValue == -5);
}

TEST_CASE("WriteWdbcFile round-trips an empty table", "[Wdbc]")
{
    DbcFieldMeta fields[] = { MakeField("ID", DbcFieldType::Int, 32, false, 1, true) };
    uint32_t layoutHashes[] = { 0 };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 0, fields, 1 };

    WdbcReadResult result = RoundTrip(table, {}, "empty");

    INFO(result.error);
    CHECK(result.error.empty());
    CHECK(result.rows.empty());
}

TEST_CASE("WriteWdbcFile round-trips a 9-slot locstring", "[Wdbc]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, /*isId*/ true),
        MakeField("Name_lang", DbcFieldType::LocString, 0, true, 1),
    };
    uint32_t layoutHashes[] = { 0 };
    DbcTableMeta table{ "Test", "test", "vdbc_test", layoutHashes, 0, fields, 2 };
    WdbcLayout layout{ /*locStringSlots*/ 9 };

    // 9 slots for the locstring: enUS, 7 more locales, then the flags mask.
    Wdc5Row row(10);
    row[0].intValue = 1;
    row[1].stringValue = "enUS";
    row[2].stringValue = "koKR";
    row[3].stringValue = "";
    row[4].stringValue = "frFR";
    row[5].stringValue = "";
    row[6].stringValue = "";
    row[7].stringValue = "";
    row[8].stringValue = "";
    row[9].intValue = 0x3; // flags mask: enUS | koKR present

    WdbcReadResult result = RoundTrip(table, { row }, "locstring9", layout);

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.rows.size() == 1);
    Wdc5Row const& read = result.rows[0];
    REQUIRE(read.size() == 10);
    CHECK(read[0].intValue == 1);
    CHECK(read[1].stringValue == "enUS");
    CHECK(read[2].stringValue == "koKR");
    CHECK(read[3].stringValue.empty());
    CHECK(read[4].stringValue == "frFR");
    CHECK(read[9].intValue == 0x3);
}

TEST_CASE("ReadWdbcFile rejects a bad signature", "[Wdbc]")
{
    DbcFieldMeta fields[] = { MakeField("ID", DbcFieldType::Int, 32, false, 1, true) };
    uint32_t layoutHashes[] = { 0 };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 0, fields, 1 };

    std::vector<uint8_t> file;
    PutU32(file, 0x11111111);
    PutU32(file, 0); PutU32(file, 1); PutU32(file, 4); PutU32(file, 0);

    TempFile temp("bad_signature");
    std::ofstream out(temp.path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<char const*>(file.data()), static_cast<std::streamsize>(file.size()));
    out.close();

    WdbcReadResult result = ReadWdbcFile(temp.path, table);
    CHECK(result.error.find("WDBC") != std::string::npos);
}

TEST_CASE("ReadWdbcFile rejects a field count mismatch", "[Wdbc]")
{
    DbcFieldMeta fields[] = {
        MakeField("ID", DbcFieldType::Int, 32, false, 1, true),
        MakeField("Value", DbcFieldType::Int, 32, false, 1),
    };
    uint32_t layoutHashes[] = { 0 };
    DbcTableMeta table{ "Test", "test", "dbc_test", layoutHashes, 0, fields, 2 };

    std::vector<uint8_t> file;
    PutU32(file, 0x43424457u);
    PutU32(file, 0); // recordCount
    PutU32(file, 1); // fieldCount: metadata expects 2
    PutU32(file, 4); // recordSize
    PutU32(file, 0); // stringBlockSize

    TempFile temp("bad_fieldcount");
    std::ofstream out(temp.path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<char const*>(file.data()), static_cast<std::streamsize>(file.size()));
    out.close();

    WdbcReadResult result = ReadWdbcFile(temp.path, table);
    CHECK(result.error.find("field count") != std::string::npos);
}
