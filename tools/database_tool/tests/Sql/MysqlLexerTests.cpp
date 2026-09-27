#include "Sql/MysqlLexer.h"

#include <catch_amalgamated.hpp>

TEST_CASE("MysqlLexer scans single-quoted strings with escapes", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql(R"(SELECT 'it''s \'ok\' \\ \n \0';)");
    REQUIRE(tokens.size() == 3);
    CHECK(tokens[0].type == MysqlTokenType::Other);
    CHECK(tokens[1].type == MysqlTokenType::StringLiteral);
    CHECK(tokens[1].text == R"('it''s \'ok\' \\ \n \0')");
    CHECK(tokens[2].type == MysqlTokenType::Other);
}

TEST_CASE("MysqlLexer scans double-quoted strings", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql(R"(SELECT "a ""b"" c";)");
    REQUIRE(tokens.size() == 3);
    CHECK(tokens[1].type == MysqlTokenType::StringLiteral);
    CHECK(tokens[1].text == R"("a ""b"" c")");
}

TEST_CASE("MysqlLexer scans backtick identifiers with doubled backticks", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("SELECT `a``b` FROM `t`;");
    REQUIRE(tokens.size() == 5);
    CHECK(tokens[1].type == MysqlTokenType::QuotedIdentifier);
    CHECK(tokens[1].text == "`a``b`");
    CHECK(tokens[3].type == MysqlTokenType::QuotedIdentifier);
    CHECK(tokens[3].text == "`t`");
}

TEST_CASE("MysqlLexer scans -- and # line comments", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("SELECT 1; -- a comment\nSELECT 2; # another\n");
    std::vector<MysqlToken> comments;
    for (auto const& token : tokens)
        if (token.type == MysqlTokenType::Comment)
            comments.push_back(token);

    REQUIRE(comments.size() == 2);
    CHECK(comments[0].text == "-- a comment");
    CHECK(comments[1].text == "# another");
}

TEST_CASE("MysqlLexer requires a space or newline after --", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("SELECT 1--2;");
    for (auto const& token : tokens)
        CHECK(token.type != MysqlTokenType::Comment);
}

TEST_CASE("MysqlLexer scans block comments and versioned comments separately", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("/* plain */ /*!40101 SET NAMES utf8 */;");
    REQUIRE(tokens.size() >= 2);
    CHECK(tokens[0].type == MysqlTokenType::Comment);
    CHECK(tokens[0].text == "/* plain */");

    bool foundVersioned = false;
    for (auto const& token : tokens)
    {
        if (token.type == MysqlTokenType::VersionedComment)
        {
            foundVersioned = true;
            CHECK(token.text == "/*!40101 SET NAMES utf8 */");
        }
    }
    CHECK(foundVersioned);
}

TEST_CASE("MysqlLexer recognizes DELIMITER directives case-insensitively", "[MysqlLexer]")
{
    for (std::string const& keyword : { std::string("DELIMITER"), std::string("delimiter"), std::string("Delimiter") })
    {
        auto tokens = TokenizeMysql(keyword + " ??\nSELECT 1??\n");
        REQUIRE(tokens.size() >= 1);
        CHECK(tokens[0].type == MysqlTokenType::DelimiterDirective);
        CHECK(tokens[0].text == "??");
    }
}

TEST_CASE("MysqlLexer trims trailing whitespace from a DELIMITER directive", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("delimiter ; \nCALL x();");
    REQUIRE(tokens.size() >= 1);
    CHECK(tokens[0].type == MysqlTokenType::DelimiterDirective);
    CHECK(tokens[0].text == ";");
}

TEST_CASE("MysqlLexer does not treat a mid-line word as a DELIMITER directive", "[MysqlLexer]")
{
    auto tokens = TokenizeMysql("SELECT delimiter FROM t;");
    for (auto const& token : tokens)
        CHECK(token.type != MysqlTokenType::DelimiterDirective);
}

TEST_CASE("DecodeMysqlStringLiteral decodes every escape", "[MysqlLexer]")
{
    CHECK(DecodeMysqlStringLiteral(R"('it''s')") == "it's");
    CHECK(DecodeMysqlStringLiteral(R"('\'ok\'')") == "'ok'");
    CHECK(DecodeMysqlStringLiteral(R"('a\\b')") == "a\\b");
    CHECK(DecodeMysqlStringLiteral(R"('a\nb')") == "a\nb");
    CHECK(DecodeMysqlStringLiteral(R"('a\rb')") == "a\rb");
    CHECK(DecodeMysqlStringLiteral(R"('a\tb')") == "a\tb");
    CHECK(DecodeMysqlStringLiteral(R"('a\0b')") == std::string("a\0b", 3));
    CHECK(DecodeMysqlStringLiteral(R"('a\Zb')") == "a" "\x1a" "b");
    CHECK(DecodeMysqlStringLiteral(R"('a\%b')") == "a\\%b");
    CHECK(DecodeMysqlStringLiteral(R"('a\_b')") == "a\\_b");
    CHECK(DecodeMysqlStringLiteral(R"("a""b")") == "a\"b");
}
