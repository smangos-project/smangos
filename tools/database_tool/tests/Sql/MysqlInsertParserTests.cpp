#include "Sql/MysqlInsertParser.h"

#include <catch_amalgamated.hpp>

TEST_CASE("ParseMysqlInsert parses typed values from a multi-row INSERT", "[MysqlInsertParser]")
{
    ParsedInsert result = ParseMysqlInsert(
        "INSERT INTO `t` (`id`,`name`,`score`,`note`) VALUES "
        "(1,'hello',3.5,NULL),"
        "(-2,'it''s a test',-1.25e2,'line\\nbreak')");

    INFO(result.error);
    REQUIRE(result.error.empty());
    CHECK(result.tableName == "t");
    CHECK(result.columns == std::vector<std::string>{"id", "name", "score", "note"});
    REQUIRE(result.rows.size() == 2);

    auto const& row0 = result.rows[0];
    CHECK(row0[0].kind == MysqlValue::Kind::Integer);
    CHECK(row0[0].integerValue == 1);
    CHECK(row0[1].kind == MysqlValue::Kind::Text);
    CHECK(row0[1].textValue == "hello");
    CHECK(row0[2].kind == MysqlValue::Kind::Real);
    CHECK(row0[2].realValue == Catch::Approx(3.5));
    CHECK(row0[3].kind == MysqlValue::Kind::Null);

    auto const& row1 = result.rows[1];
    CHECK(row1[0].integerValue == -2);
    CHECK(row1[1].textValue == "it's a test");
    CHECK(row1[2].realValue == Catch::Approx(-125.0));
    CHECK(row1[3].textValue == "line\nbreak");
}

TEST_CASE("ParseMysqlInsert reports a row/column count mismatch", "[MysqlInsertParser]")
{
    ParsedInsert result = ParseMysqlInsert("INSERT INTO `t` (`a`,`b`) VALUES (1)");
    CHECK_FALSE(result.error.empty());
}

TEST_CASE("ParseMysqlInsert reports an unrecognized value", "[MysqlInsertParser]")
{
    ParsedInsert result = ParseMysqlInsert("INSERT INTO `t` (`a`) VALUES (TRUE)");
    CHECK_FALSE(result.error.empty());
}

TEST_CASE("ParseMysqlInsert requires an explicit column list", "[MysqlInsertParser]")
{
    ParsedInsert result = ParseMysqlInsert("INSERT INTO `t` VALUES (1,2)");
    CHECK_FALSE(result.error.empty());
}
