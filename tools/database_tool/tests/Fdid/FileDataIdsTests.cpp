#include "Fdid/FileDataIds.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    struct TempDir
    {
        std::filesystem::path path;

        explicit TempDir(std::string const& name)
            : path(std::filesystem::temp_directory_path() / ("database_tool_test_fdid_" + name))
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempDir() { std::filesystem::remove_all(path); }

        std::filesystem::path Write(std::string const& name, std::string const& content) const
        {
            std::ofstream(path / name, std::ios::binary) << content;
            return path / name;
        }

        std::string Read(std::string const& name) const
        {
            std::ifstream in(path / name, std::ios::binary);
            std::ostringstream buffer;
            buffer << in.rdbuf();
            return buffer.str();
        }
    };

    void CreateTable(DoltLiteDatabase& db)
    {
        db.Execute("CREATE TABLE file_data_id (id INTEGER PRIMARY KEY, path TEXT NOT NULL UNIQUE, "
                   "source TEXT NOT NULL, retired INTEGER NOT NULL DEFAULT 0)");
    }

    int64_t Count(DoltLiteDatabase& db, std::string const& sql)
    {
        DoltLiteStatement statement = db.Prepare(sql);
        REQUIRE(statement.Step());
        return statement.ColumnInt64(0);
    }
}

TEST_CASE("ImportFdidFile adds stock rows and updates them on reimport", "[Fdid]")
{
    TempDir dir("import");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    CreateTable(db);

    FdidImportResult first = ImportFdidFile(db, dir.Write("a.txt", "21;Interface\\Logo.avi\n53;sound/a.mp3\r\n"), "listfile");
    CHECK(first.error.empty());
    CHECK(first.added == 2);

    FdidImportResult second = ImportFdidFile(db, dir.Write("b.txt", "21;interface/logo.avi\n53;sound/b.mp3\n"), "listfile");
    CHECK(second.added == 1);
    CHECK(second.unchanged == 1);
    CHECK(second.conflicts.empty());
    DoltLiteStatement path = db.Prepare("SELECT path FROM file_data_id WHERE id = 53");
    REQUIRE(path.Step());
    CHECK(path.ColumnText(0) == "sound/b.mp3");
}

TEST_CASE("ImportFdidFile reports id and path conflicts without overwriting", "[Fdid]")
{
    TempDir dir("conflicts");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    CreateTable(db);
    ImportFdidFile(db, dir.Write("a.txt", "2000000000;world/a.m2\n"), "model_editor");

    FdidImportResult result = ImportFdidFile(db, dir.Write("b.txt", "2000000000;world/other.m2\n7;world/a.m2\n"), "listfile");
    CHECK(result.added == 0);
    CHECK(result.conflicts.size() == 2);
    CHECK(Count(db, "SELECT COUNT(*) FROM file_data_id") == 1);
}

TEST_CASE("AllocateFdids hands out custom ids, reuses paths and revives retired ones", "[Fdid]")
{
    TempDir dir("allocate");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    CreateTable(db);

    std::vector<int64_t> ids = AllocateFdids(db, { "World\\A.m2", "world/b.m2", "world/a.m2" }, "model_editor");
    REQUIRE(ids.size() == 3);
    CHECK(ids[0] == kFirstCustomId);
    CHECK(ids[1] == kFirstCustomId + 1);
    CHECK(ids[2] == ids[0]);

    CHECK(RetireFdids(db, { "world/b.m2", "world/missing.m2" }) == 1);
    CHECK(AllocateFdids(db, { "world/c.m2" }, "model_editor")[0] == kFirstCustomId + 2);
    CHECK(AllocateFdids(db, { "world/b.m2" }, "model_editor")[0] == kFirstCustomId + 1);
    CHECK(Count(db, "SELECT COUNT(*) FROM file_data_id WHERE retired = 1") == 0);
}

TEST_CASE("ExportFdidIndex writes live rows once and leaves an identical file alone", "[Fdid]")
{
    TempDir dir("export");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    CreateTable(db);
    ImportFdidFile(db, dir.Write("a.txt", "5;b.blp\n3;a.blp\n"), "listfile");
    AllocateFdids(db, { "x.m2", "y.m2" }, "model_editor");
    RetireFdids(db, { "y.m2" });

    std::filesystem::path index = dir.path / "out" / "fdid.index";
    FdidExportResult first = ExportFdidIndex(db, index);
    CHECK(first.error.empty());
    CHECK(first.written);
    CHECK(first.rows == 3);
    CHECK(dir.Read("out/fdid.index") == "3;a.blp\n5;b.blp\n2000000000;x.m2\n");

    CHECK_FALSE(ExportFdidIndex(db, index).written);
}

TEST_CASE("Fdid operations add no dolt commits", "[Fdid]")
{
    TempDir dir("nocommit");
    DoltLiteDatabase db((dir.path / "w.doltlite").string());
    CreateTable(db);
    int64_t const before = Count(db, "SELECT COUNT(*) FROM dolt_log");

    ImportFdidFile(db, dir.Write("a.txt", "1;a\n"), "listfile");
    AllocateFdids(db, { "x" }, "model_editor");
    RetireFdids(db, { "x" });
    ExportFdidIndex(db, dir.path / "fdid.index");

    CHECK(Count(db, "SELECT COUNT(*) FROM dolt_log") == before);
}
