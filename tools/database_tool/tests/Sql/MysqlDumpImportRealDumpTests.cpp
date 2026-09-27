#include "Sql/MysqlDumpImport.h"

#include "World/DoltLiteDatabase.h"
#include "World/WorldArchive.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>

TEST_CASE("ImportMysqlDump imports the real pinned world base dump", "[MysqlDumpImport][network]")
{
    // Opt-in: downloads the real 16MB archive and imports its 912,894 rows.
    if (!std::getenv("DATABASE_TOOL_WORLD_DUMP"))
    {
        WARN("DATABASE_TOOL_WORLD_DUMP not set, skipping (downloads and imports the real world dump)");
        return;
    }

    std::filesystem::path cacheDir = std::filesystem::temp_directory_path() / "database_tool_test_world_dump_cache";
    WorldArchiveResult archive = FetchAndUnpackWorldArchive(WorldBaseDumpArchiveSpec(), cacheDir);
    INFO(archive.error);
    REQUIRE(archive.error.empty());

    std::filesystem::path worldPath = std::filesystem::temp_directory_path() / "database_tool_test_world_dump.doltlite";
    std::filesystem::remove(worldPath);

    DumpImportResult result;
    {
        DoltLiteDatabase db(worldPath.string());
        result = ImportMysqlDump(db, archive.sqlPath, "world: import world_full_14_june_2021");
    } // closed before removing the file below

    INFO(result.error);
    REQUIRE(result.ok);
    CHECK(result.tablesCreated == 169);
    CHECK(result.rowsInserted == 912894);

    std::filesystem::remove(worldPath);
}
