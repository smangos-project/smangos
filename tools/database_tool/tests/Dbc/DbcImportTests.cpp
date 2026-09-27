#include "Dbc/DbcImport.h"

#include "Dbc/DbcSchema.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <cstring>
#include <filesystem>
#include <fstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dbcimport_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir() { std::filesystem::remove_all(path); }
    };

    struct TempDoltLiteFile
    {
        std::filesystem::path path;

        explicit TempDoltLiteFile(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dbcimport_" + name + ".doltlite"))
        {
            std::filesystem::remove(path);
        }

        ~TempDoltLiteFile() { std::filesystem::remove(path); }
    };

    void PutU16(std::vector<uint8_t>& buf, uint16_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 2); }
    void PutI16(std::vector<uint8_t>& buf, int16_t v) { PutU16(buf, static_cast<uint16_t>(v)); }
    void PutU32(std::vector<uint8_t>& buf, uint32_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 4); }
    void PutU64(std::vector<uint8_t>& buf, uint64_t v) { auto p = reinterpret_cast<uint8_t const*>(&v); buf.insert(buf.end(), p, p + 8); }
    void PatchU32(std::vector<uint8_t>& buf, size_t pos, uint32_t v) { std::memcpy(buf.data() + pos, &v, 4); }

    // A minimal WDC5 file: two int32 fields (ID, Value), None compression, no arrays, one
    // section, inline id -- matching the two-field synthetic table the tests below build.
    std::vector<uint8_t> BuildSimpleDb2(uint32_t layoutHash, std::vector<std::pair<int32_t, int32_t>> const& rows)
    {
        std::vector<uint8_t> file;
        PutU32(file, 0x35434457u);
        PutU32(file, 5);
        std::string schema = "Test";
        file.insert(file.end(), schema.begin(), schema.end());
        file.insert(file.end(), 128 - schema.size(), 0);
        PutU32(file, static_cast<uint32_t>(rows.size())); // recordCount
        PutU32(file, 2);                                  // fieldCount
        PutU32(file, 8);                                  // recordSize
        PutU32(file, 0);                                  // stringTableSize
        PutU32(file, 0x1234);                              // tableHash
        PutU32(file, layoutHash);
        PutU32(file, 0);    // minId
        PutU32(file, 1000); // maxId
        PutU32(file, 1);    // locale
        PutU16(file, 0);    // flags
        PutI16(file, 0);    // indexField
        PutU32(file, 2);    // totalFieldCount
        PutU32(file, 8);    // packedDataOffset
        PutU32(file, 0);    // parentLookupCount
        PutU32(file, 2 * 24); // columnMetaSize
        PutU32(file, 0);    // commonDataSize
        PutU32(file, 0);    // palletDataSize
        PutU32(file, 1);    // sectionCount

        size_t sectionPos = file.size();
        PutU64(file, 0); // tactId
        PutU32(file, 0); // fileOffset (patched)
        PutU32(file, static_cast<uint32_t>(rows.size())); // recordCount
        PutU32(file, 0); // stringTableSize
        PutU32(file, 0); // catalogDataOffset
        PutU32(file, 0); // idTableSize
        PutU32(file, 0); // parentLookupDataSize
        PutU32(file, 0); // catalogDataCount
        PutU32(file, 0); // copyTableCount

        file.insert(file.end(), 2 * 4, 0); // field entries, unused

        // columnMeta: ID then Value, both None-compressed at bitOffset 0/32.
        PutU16(file, 0); PutU16(file, 32); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0);
        PutU16(file, 32); PutU16(file, 32); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0); PutU32(file, 0);

        uint32_t fileOffset = static_cast<uint32_t>(file.size());
        PatchU32(file, sectionPos + 8, fileOffset);
        for (auto const& [id, value] : rows)
        {
            PutU32(file, static_cast<uint32_t>(id));
            PutU32(file, static_cast<uint32_t>(value));
        }
        return file;
    }

    void WriteFile(std::filesystem::path const& path, std::vector<uint8_t> const& bytes)
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<char const*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    DbcFieldMeta TestFields[] = {
        { "ID", DbcFieldType::Int, 32, true, 1, true, false, false, nullptr, nullptr },
        { "Value", DbcFieldType::Int, 32, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t TestLayoutHashes[] = { 0xABCDu };
    DbcTableMeta TestTable{ "Test", "test", "dbc_test", TestLayoutHashes, 1, TestFields, 2 };
}

TEST_CASE("ImportDb2Files imports rows and upserts dbc_files", "[DbcImport]")
{
    TempDir dir("basic");
    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 1, 100 }, { 2, 200 } }));

    TempDoltLiteFile file("basic");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());

    DbcImportResult result = ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1));

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.tables.size() == 1);
    CHECK(result.tables[0].fileFound);
    CHECK(result.tables[0].error.empty());
    CHECK(result.tables[0].rowsImported == 2);

    DoltLiteStatement rows = db.Prepare("SELECT \"ID\", \"Value\" FROM \"dbc_test\" ORDER BY \"ID\"");
    REQUIRE(rows.Step());
    CHECK(rows.ColumnInt64(0) == 1);
    CHECK(rows.ColumnInt64(1) == 100);
    REQUIRE(rows.Step());
    CHECK(rows.ColumnInt64(0) == 2);
    CHECK(rows.ColumnInt64(1) == 200);
    CHECK_FALSE(rows.Step());

    DoltLiteStatement filesRow = db.Prepare(
        "SELECT \"format\", \"layout_hash\", \"table_hash\", \"locale\", \"row_count\" FROM \"dbc_files\" WHERE \"table_name\" = 'Test'");
    REQUIRE(filesRow.Step());
    CHECK(filesRow.ColumnText(0) == "db2");
    CHECK(filesRow.ColumnInt64(1) == 0xABCD);
    CHECK(filesRow.ColumnInt64(2) == 0x1234);
    CHECK(filesRow.ColumnInt64(3) == 1);
    CHECK(filesRow.ColumnInt64(4) == 2);
}

TEST_CASE("ImportDb2Files reports a missing file without error", "[DbcImport]")
{
    TempDir dir("missing");
    TempDoltLiteFile file("missing");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());

    DbcImportResult result = ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1));

    INFO(result.error);
    CHECK(result.error.empty());
    REQUIRE(result.tables.size() == 1);
    CHECK_FALSE(result.tables[0].fileFound);
    CHECK(result.tables[0].rowsImported == 0);
}

TEST_CASE("ImportDb2Files re-imports cleanly, replacing the previous rows", "[DbcImport]")
{
    TempDir dir("reimport");
    TempDoltLiteFile file("reimport");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());

    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 1, 100 }, { 2, 200 }, { 3, 300 } }));
    REQUIRE(ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).error.empty());

    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 9, 900 } }));
    DbcImportResult second = ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1));

    INFO(second.error);
    CHECK(second.error.empty());
    CHECK(second.tables[0].rowsImported == 1);

    DoltLiteStatement count = db.Prepare("SELECT COUNT(*) FROM \"dbc_test\"");
    REQUIRE(count.Step());
    CHECK(count.ColumnInt64(0) == 1);
}

TEST_CASE("ImportDb2Files filters by table name", "[DbcImport]")
{
    TempDir dir("filter");
    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 1, 100 } }));
    TempDoltLiteFile file("filter");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());

    DbcImportResult result = ImportDb2Files(db, dir.path, { "somethingelse" }, std::span(&TestTable, 1));

    CHECK(result.tables.empty());
}
