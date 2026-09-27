#include "ToolConfig.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>

TEST_CASE("LoadToolConfig reads the three MySQL info strings", "[ToolConfig]")
{
    auto path = std::filesystem::temp_directory_path() / "database_tool_test_tool_config.conf";
    {
        std::ofstream file(path);
        file << "LoginDatabase.Info = \"127.0.0.1;3306;mangos;mangos;realmd\"\n";
        file << "WorldDatabase.Info = \"127.0.0.1;3306;mangos;mangos;mangos\"\n";
        file << "CharacterDatabase.Info = \"127.0.0.1;3306;mangos;mangos;characters\"\n";
        file << "LogsDatabase.Info = \"127.0.0.1;3306;mangos;mangos;logs\"\n";
    }

    ToolConfig config;
    REQUIRE(LoadToolConfig(path.string(), config));
    CHECK(config.loginDatabaseInfo == "127.0.0.1;3306;mangos;mangos;realmd");
    CHECK(config.characterDatabaseInfo == "127.0.0.1;3306;mangos;mangos;characters");
    CHECK(config.logsDatabaseInfo == "127.0.0.1;3306;mangos;mangos;logs");

    std::filesystem::remove(path);
}

TEST_CASE("LoadToolConfig returns false for a missing file", "[ToolConfig]")
{
    ToolConfig config;
    CHECK_FALSE(LoadToolConfig("no-such-file.conf", config));
}
