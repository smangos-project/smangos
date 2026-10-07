#include "Dbc/DbcExport.h"

#include "Dbc/DbcImport.h"
#include "Dbc/DbcSchema.h"
#include "Dbc/Wdc5Reader.h"
#include "World/DoltLiteDatabase.h"

#include <catch_amalgamated.hpp>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dbcexport_" + name))
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
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_dbcexport_" + name + ".doltlite"))
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

    // Same minimal WDC5 shape as DbcImportTests.cpp's BuildSimpleDb2: two int32 fields (ID,
    // Value), None compression, one section, inline id.
    std::vector<uint8_t> BuildSimpleDb2(uint32_t layoutHash, std::vector<std::pair<int32_t, int32_t>> const& rows)
    {
        std::vector<uint8_t> file;
        PutU32(file, 0x35434457u);
        PutU32(file, 5);
        std::string schema = "Test";
        file.insert(file.end(), schema.begin(), schema.end());
        file.insert(file.end(), 128 - schema.size(), 0);
        PutU32(file, static_cast<uint32_t>(rows.size()));
        PutU32(file, 2);
        PutU32(file, 8);
        PutU32(file, 0);
        PutU32(file, 0x1234);
        PutU32(file, layoutHash);
        PutU32(file, 0);
        PutU32(file, 1000);
        PutU32(file, 1);
        PutU16(file, 0);
        PutI16(file, 0);
        PutU32(file, 2);
        PutU32(file, 8);
        PutU32(file, 0);
        PutU32(file, 2 * 24);
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 1);

        size_t sectionPos = file.size();
        PutU64(file, 0);
        PutU32(file, 0);
        PutU32(file, static_cast<uint32_t>(rows.size()));
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 0);
        PutU32(file, 0);

        file.insert(file.end(), 2 * 4, 0);

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

    std::vector<uint8_t> ReadFile(std::filesystem::path const& path)
    {
        std::ifstream in(path, std::ios::binary);
        return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    DbcFieldMeta TestFields[] = {
        { "ID", DbcFieldType::Int, 32, true, 1, true, false, false, nullptr, nullptr },
        { "Value", DbcFieldType::Int, 32, true, 1, false, false, false, nullptr, nullptr },
    };
    uint32_t TestLayoutHashes[] = { 0xABCDu };
    DbcTableMeta TestTable{ "Test", "test", "dbc_test", TestLayoutHashes, 1, TestFields, 2 };
}

TEST_CASE("ExportDb2Files writes a file that reads back the same rows", "[DbcExport]")
{
    TempDir dir("basic");
    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 2, 200 }, { 1, 100 } }));

    TempDoltLiteFile file("basic");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());
    REQUIRE(ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).error.empty());

    DbcExportResult result = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1));

    REQUIRE(result.tables.size() == 1);
    CHECK(result.tables[0].hasFilesRow);
    CHECK(result.tables[0].error.empty());
    CHECK(result.tables[0].rowsExported == 2);

    Wdc5ReadResult reread = ReadWdc5File(dir.path / "test.db2", TestTable);
    INFO(reread.error);
    CHECK(reread.error.empty());
    REQUIRE(reread.rows.size() == 2);
    CHECK(reread.info.tableHash == 0x1234);
    CHECK(reread.info.layoutHash == 0xABCD);
    CHECK(reread.info.schema == "Test");

    std::sort(reread.rows.begin(), reread.rows.end(),
              [](Wdc5Row const& a, Wdc5Row const& b) { return a[0].intValue < b[0].intValue; });
    CHECK(reread.rows[0][0].intValue == 1);
    CHECK(reread.rows[0][1].intValue == 100);
    CHECK(reread.rows[1][0].intValue == 2);
    CHECK(reread.rows[1][1].intValue == 200);
}

TEST_CASE("ExportDb2Files reports a table with no dbc_files row without error", "[DbcExport]")
{
    TempDir dir("no_files_row");
    TempDoltLiteFile file("no_files_row");
    DoltLiteDatabase db(file.path.string());
    db.Execute("CREATE TABLE \"dbc_files\" (\"table_name\" TEXT PRIMARY KEY, \"format\" TEXT, "
               "\"layout_hash\" INTEGER, \"table_hash\" INTEGER, \"locale\" INTEGER, \"flags\" INTEGER, "
               "\"schema\" TEXT, \"row_count\" INTEGER)");

    DbcExportResult result = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1));

    REQUIRE(result.tables.size() == 1);
    CHECK_FALSE(result.tables[0].hasFilesRow);
    CHECK(result.tables[0].rowsExported == 0);
    CHECK_FALSE(std::filesystem::exists(dir.path / "test.db2"));
}

TEST_CASE("ExportDb2Files filters by table name", "[DbcExport]")
{
    TempDir dir("filter");
    TempDoltLiteFile file("filter");
    DoltLiteDatabase db(file.path.string());

    DbcExportResult result = ExportDb2Files(db, dir.path, { "somethingelse" }, std::span(&TestTable, 1));

    CHECK(result.tables.empty());
}

TEST_CASE("ExportDb2Files with onlyChanged leaves a matching file untouched", "[DbcExport]")
{
    TempDir dir("only_changed_same");
    std::vector<uint8_t> const stock = BuildSimpleDb2(0xABCD, { { 1, 100 }, { 2, 200 } });
    WriteFile(dir.path / "test.db2", stock);

    TempDoltLiteFile file("only_changed_same");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());
    REQUIRE(ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).error.empty());

    ExportDb2Options options;
    options.onlyChanged = true;
    options.backupDir = dir.path / "backup";
    DbcExportResult result = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1), options);

    REQUIRE(result.tables.size() == 1);
    CHECK(result.tables[0].unchanged);
    CHECK(result.tables[0].error.empty());
    CHECK(ReadFile(dir.path / "test.db2") == stock);
    CHECK_FALSE(std::filesystem::exists(options.backupDir / "test.db2"));
}

TEST_CASE("ExportDb2Files with onlyChanged treats a negative int32 as unchanged", "[DbcExport]")
{
    TempDir dir("only_changed_negative");
    WriteFile(dir.path / "test.db2", BuildSimpleDb2(0xABCD, { { 1, 100 }, { 2, 200 } }));

    TempDoltLiteFile file("only_changed_negative");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());
    REQUIRE(ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).error.empty());
    db.Execute("UPDATE \"dbc_test\" SET \"Value\" = -1 WHERE \"ID\" = 2");

    REQUIRE(ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).tables[0].error.empty());

    ExportDb2Options options;
    options.onlyChanged = true;
    DbcExportResult result = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1), options);

    REQUIRE(result.tables.size() == 1);
    CHECK(result.tables[0].unchanged);
}

TEST_CASE("ExportDb2Files with onlyChanged rewrites a changed file and keeps the first backup", "[DbcExport]")
{
    TempDir dir("only_changed_diff");
    std::vector<uint8_t> const stock = BuildSimpleDb2(0xABCD, { { 1, 100 }, { 2, 200 } });
    WriteFile(dir.path / "test.db2", stock);

    TempDoltLiteFile file("only_changed_diff");
    DoltLiteDatabase db(file.path.string());
    db.Execute(BuildDbcFilesTableDdl());
    REQUIRE(ImportDb2Files(db, dir.path, {}, std::span(&TestTable, 1)).error.empty());

    ExportDb2Options options;
    options.onlyChanged = true;
    options.backupDir = dir.path / "backup";

    db.Execute("UPDATE \"dbc_test\" SET \"Value\" = 250 WHERE \"ID\" = 2");
    DbcExportResult first = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1), options);
    REQUIRE(first.tables.size() == 1);
    CHECK_FALSE(first.tables[0].unchanged);
    CHECK(first.tables[0].error.empty());
    CHECK(ReadFile(options.backupDir / "test.db2") == stock);

    db.Execute("UPDATE \"dbc_test\" SET \"Value\" = 300 WHERE \"ID\" = 2");
    DbcExportResult second = ExportDb2Files(db, dir.path, {}, std::span(&TestTable, 1), options);
    REQUIRE(second.tables.size() == 1);
    CHECK_FALSE(second.tables[0].unchanged);
    CHECK(ReadFile(options.backupDir / "test.db2") == stock); // an existing backup is never replaced

    Wdc5ReadResult reread = ReadWdc5File(dir.path / "test.db2", TestTable);
    INFO(reread.error);
    REQUIRE(reread.error.empty());
    REQUIRE(reread.rows.size() == 2);
    CHECK(reread.rows[1][1].intValue == 300);
}
