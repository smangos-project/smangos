#include "Sql/SqlStatementSplitter.h"

#include <catch_amalgamated.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace
{
    std::string ReadMigration(char const* name)
    {
        std::ifstream file(std::string(DATABASE_TOOL_TEST_MIGRATIONS_DIR) + "/" + name, std::ios::binary);
        REQUIRE(file.is_open());
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool EndsWith(std::string const& text, std::string const& suffix)
    {
        return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}

TEST_CASE("SplitMysqlStatements splits on the default delimiter, excluding it", "[SqlStatementSplitter]")
{
    auto statements = SplitMysqlStatements("SELECT 1; SELECT 2;");
    REQUIRE(statements.size() == 2);
    CHECK(statements[0].text == "SELECT 1");
    CHECK(statements[1].text == "SELECT 2");
}

TEST_CASE("SplitMysqlStatements ignores the delimiter inside strings, identifiers and comments", "[SqlStatementSplitter]")
{
    auto statements = SplitMysqlStatements("SELECT 'a;b', \"c;d\", `e;f`; -- g;h\nSELECT 1;\n");
    REQUIRE(statements.size() == 2);
    CHECK(statements[0].text == "SELECT 'a;b', \"c;d\", `e;f`");
    CHECK(statements[1].text.find("-- g;h") != std::string::npos);
    CHECK(statements[1].text.find("SELECT 1") != std::string::npos);
}

TEST_CASE("SplitMysqlStatements drops empty and whitespace-only statements", "[SqlStatementSplitter]")
{
    auto statements = SplitMysqlStatements("SELECT 1;;  ;\nSELECT 2;");
    REQUIRE(statements.size() == 2);
    CHECK(statements[0].text == "SELECT 1");
    CHECK(statements[1].text == "SELECT 2");
}

TEST_CASE("SplitMysqlStatements reports each statement's own starting line", "[SqlStatementSplitter]")
{
    // The second statement's real content starts on line 3, one line after
    // the newline that follows the first statement's ";", both of which are
    // scanned as part of the same lexer token (no string/identifier/comment
    // token boundary separates them).
    auto statements = SplitMysqlStatements("SELECT 1;\nSELECT 2;\n");
    REQUIRE(statements.size() == 2);
    CHECK(statements[0].line == 1);
    CHECK(statements[1].line == 2);

    // With a blank line and leading whitespace before the real content too.
    auto spaced = SplitMysqlStatements("SELECT 1;\n\n   SELECT 2;\n");
    REQUIRE(spaced.size() == 2);
    CHECK(spaced[0].line == 1);
    CHECK(spaced[1].line == 3);
}

TEST_CASE("SplitMysqlStatements honours a DELIMITER block", "[SqlStatementSplitter]")
{
    auto statements = SplitMysqlStatements(
        "DROP PROCEDURE IF EXISTS p;\n"
        "delimiter ??\n"
        "CREATE PROCEDURE `p`()\n"
        "BEGIN\n"
        "SET @x = 1; SELECT @x;\n"
        "END??\n"
        "delimiter ;\n"
        "CALL p();\n");

    REQUIRE(statements.size() == 3);
    CHECK(statements[0].text == "DROP PROCEDURE IF EXISTS p");
    CHECK(statements[1].text.find("SET @x = 1; SELECT @x;") != std::string::npos);
    CHECK(EndsWith(statements[1].text, "END"));
    CHECK(statements[2].text == "CALL p()");
}

TEST_CASE("SplitMysqlStatements handles a real world migration (normal, lower-case delimiter)", "[SqlStatementSplitter]")
{
    std::string content = ReadMigration("20210515110157_world.sql");
    auto statements = SplitMysqlStatements(content);

    REQUIRE(statements.size() == 4);
    CHECK(statements[0].text == "DROP PROCEDURE IF EXISTS add_migration");
    CHECK(statements[1].text.find("CREATE PROCEDURE `add_migration`()") == 0);
    CHECK(statements[1].text.find("INSERT INTO `migrations` VALUES ('20210515110157');") != std::string::npos);
    CHECK(EndsWith(statements[1].text, "END"));
    CHECK(statements[2].text == "CALL add_migration()");
    CHECK(statements[3].text == "DROP PROCEDURE IF EXISTS add_migration");
}

TEST_CASE("SplitMysqlStatements handles a world migration with SET NAMES before the envelope", "[SqlStatementSplitter]")
{
    std::string content = ReadMigration("20210628233951_world.sql");
    auto statements = SplitMysqlStatements(content);

    REQUIRE(statements.size() >= 4);
    CHECK(statements[0].text == "SET NAMES utf8");
    CHECK(statements[1].text == "DROP PROCEDURE IF EXISTS add_migration");
    CHECK(statements[2].text.find("CREATE PROCEDURE `add_migration`()") == 0);
}

TEST_CASE("SplitMysqlStatements splits every world migration without throwing", "[SqlStatementSplitter]")
{
    size_t fileCount = 0;
    for (auto const& entry : std::filesystem::directory_iterator(DATABASE_TOOL_TEST_MIGRATIONS_DIR))
    {
        if (entry.path().filename().string().find("_world.sql") == std::string::npos)
            continue;

        ++fileCount;
        std::string content = ReadMigration(entry.path().filename().string().c_str());
        auto statements = SplitMysqlStatements(content);
        CHECK_FALSE(statements.empty());
    }
    CHECK(fileCount > 1000);
}
