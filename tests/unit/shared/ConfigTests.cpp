#include "Config/Config.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>

static std::string WriteConfig(char const* name, std::string const& content)
{
    std::filesystem::path path = std::filesystem::temp_directory_path() / ("config-tests-" + std::string(name));
    std::ofstream(path) << content;
    return path.string();
}

TEST_CASE("Config layers merge into one map", "[config]")
{
    std::string const a = WriteConfig("merge-a.conf", "First = 1\nShared = \"x\"\n");
    std::string const b = WriteConfig("merge-b.conf", "Second = 2\n");

    std::string error;
    REQUIRE(sConfig.LoadLayers({ a, b }, error));
    CHECK(sConfig.GetIntDefault("First", 0) == 1);
    CHECK(sConfig.GetIntDefault("Second", 0) == 2);
    CHECK(sConfig.GetStringDefault("Shared", "") == "x");
    CHECK(sConfig.GetFilename() == a + ", " + b);
}

TEST_CASE("Config rejects a key set in two layers and keeps the old values", "[config]")
{
    std::string const good = WriteConfig("dup-good.conf", "Kept = 7\n");
    std::string const a = WriteConfig("dup-a.conf", "Same = 1\n");
    std::string const b = WriteConfig("dup-b.conf", "Same = 2\n");

    std::string error;
    REQUIRE(sConfig.LoadLayers({ good }, error));
    CHECK_FALSE(sConfig.LoadLayers({ a, b }, error));
    CHECK(error.find("'Same'") != std::string::npos);
    CHECK(error.find(a) != std::string::npos);
    CHECK(error.find(b) != std::string::npos);
    CHECK(sConfig.GetIntDefault("Kept", 0) == 7);
    CHECK_FALSE(sConfig.IsSet("Same"));
}

TEST_CASE("Config fails on a missing file", "[config]")
{
    std::string const a = WriteConfig("missing-a.conf", "Key = 1\n");
    std::string const missing = (std::filesystem::temp_directory_path() / "config-tests-does-not-exist.conf").string();

    std::string error;
    CHECK_FALSE(sConfig.LoadLayers({ a, missing }, error));
    CHECK(error.find(missing) != std::string::npos);
}

TEST_CASE("Config reload re-reads every layer", "[config]")
{
    std::string const a = WriteConfig("reload-a.conf", "A = 1\n");
    std::string const b = WriteConfig("reload-b.conf", "B = 1\n");

    std::string error;
    REQUIRE(sConfig.LoadLayers({ a, b }, error));
    WriteConfig("reload-b.conf", "B = 5\nC = 3\n");

    REQUIRE(sConfig.Reload());
    CHECK(sConfig.GetIntDefault("A", 0) == 1);
    CHECK(sConfig.GetIntDefault("B", 0) == 5);
    CHECK(sConfig.GetIntDefault("C", 0) == 3);
}

TEST_CASE("Config single file keeps the first of a repeated key", "[config]")
{
    std::string const a = WriteConfig("single.conf", "Twice = 1\nTwice = 2\n");

    REQUIRE(sConfig.LoadFromFile(a));
    CHECK(sConfig.GetIntDefault("Twice", 0) == 1);
}
