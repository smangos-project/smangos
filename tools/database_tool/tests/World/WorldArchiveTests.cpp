#include "World/WorldArchive.h"

#include <catch_amalgamated.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    // A small real .7z archive (World/fixtures/fixture.7z) so the
    // cached/verify/unpack path can be tested with no network: one file,
    // fixture.txt, containing "hello from a 7z fixture\n".
    WorldArchiveSpec FixtureSpec()
    {
        WorldArchiveSpec spec;
        spec.url = "unused://fixture-is-pre-cached";
        spec.archiveFileName = "fixture.7z";
        spec.unpackedFileName = "fixture.txt";
        spec.expectedSize = 158;
        spec.expectedSha256Hex = "dc23b0e5674d7adb40087f29406de506a402081d1e1f8cb85dd5b6e7771f5c52";
        return spec;
    }

    struct TempCacheDir
    {
        std::filesystem::path path;

        TempCacheDir()
            : path(std::filesystem::temp_directory_path() / "database_tool_test_world_archive")
        {
            std::filesystem::remove_all(path);
            std::filesystem::create_directories(path);
        }

        ~TempCacheDir()
        {
            std::filesystem::remove_all(path);
        }
    };

    void PlaceFixtureArchive(std::filesystem::path const& cacheDirectory, std::string const& archiveFileName)
    {
        std::filesystem::path source = std::filesystem::path(DATABASE_TOOL_TEST_WORLD_FIXTURES_DIR) / "fixture.7z";
        std::filesystem::copy_file(source, cacheDirectory / archiveFileName);
    }

    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
}

TEST_CASE("FetchAndUnpackWorldArchive unpacks an already-cached archive with no download", "[WorldArchive]")
{
    TempCacheDir cache;
    WorldArchiveSpec spec = FixtureSpec();
    PlaceFixtureArchive(cache.path, spec.archiveFileName);

    WorldArchiveResult result = FetchAndUnpackWorldArchive(spec, cache.path);

    INFO(result.error);
    REQUIRE(result.error.empty());
    CHECK_FALSE(result.downloaded);
    CHECK(result.sqlPath == cache.path / spec.unpackedFileName);
    CHECK(ReadFile(result.sqlPath) == "hello from a 7z fixture\n");
}

TEST_CASE("FetchAndUnpackWorldArchive reports a SHA256 mismatch without unpacking", "[WorldArchive]")
{
    TempCacheDir cache;
    WorldArchiveSpec spec = FixtureSpec();
    spec.expectedSha256Hex = std::string(64, '0');
    PlaceFixtureArchive(cache.path, spec.archiveFileName);

    WorldArchiveResult result = FetchAndUnpackWorldArchive(spec, cache.path);

    CHECK_FALSE(result.error.empty());
    CHECK(result.error.find("SHA256") != std::string::npos);
    CHECK_FALSE(std::filesystem::exists(cache.path / spec.unpackedFileName));
}

TEST_CASE("FetchAndUnpackWorldArchive reports a size mismatch without unpacking", "[WorldArchive]")
{
    TempCacheDir cache;
    WorldArchiveSpec spec = FixtureSpec();
    spec.expectedSize = spec.expectedSize + 1;
    PlaceFixtureArchive(cache.path, spec.archiveFileName);

    WorldArchiveResult result = FetchAndUnpackWorldArchive(spec, cache.path);

    CHECK_FALSE(result.error.empty());
    CHECK(result.error.find("size mismatch") != std::string::npos);
    CHECK_FALSE(std::filesystem::exists(cache.path / spec.unpackedFileName));
}

TEST_CASE("FetchAndUnpackWorldArchive downloads and unpacks the real pinned world dump", "[WorldArchive][network]")
{
    // Opt-in: downloads the real 16MB archive from the network.
    if (!std::getenv("DATABASE_TOOL_TEST_WORLD_ARCHIVE_DOWNLOAD"))
    {
        WARN("DATABASE_TOOL_TEST_WORLD_ARCHIVE_DOWNLOAD not set, skipping (downloads a real 16MB file)");
        return;
    }

    TempCacheDir cache;
    WorldArchiveResult result = FetchAndUnpackWorldArchive(WorldBaseDumpArchiveSpec(), cache.path);

    INFO(result.error);
    REQUIRE(result.error.empty());
    CHECK(result.downloaded);
    CHECK(std::filesystem::file_size(result.sqlPath) == 142940922);

    // A second call must find the archive already cached and skip downloading.
    WorldArchiveResult second = FetchAndUnpackWorldArchive(WorldBaseDumpArchiveSpec(), cache.path);
    INFO(second.error);
    REQUIRE(second.error.empty());
    CHECK_FALSE(second.downloaded);
}
