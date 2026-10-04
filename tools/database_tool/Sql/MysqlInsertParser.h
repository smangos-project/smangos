#ifndef DATABASE_TOOL_MYSQLINSERTPARSER_H
#define DATABASE_TOOL_MYSQLINSERTPARSER_H

#include <cstdint>
#include <string>
#include <vector>

// One VALUES(...) tuple's cell, already decoded from MySQL's own escaping
// (see DecodeMysqlStringLiteral) rather than re-emitted as SQLite literal
// text.
struct MysqlValue
{
    enum class Kind { Null, Integer, Real, Text };
    Kind kind = Kind::Null;
    int64_t integerValue = 0;
    double realValue = 0;
    std::string textValue;
};

struct ParsedInsert
{
    std::string tableName;                          // without backticks
    std::vector<std::string> columns;                // without backticks, in the statement's own order; empty for a positional insert
    std::vector<std::vector<MysqlValue>> rows;
    std::string error;                               // empty on success
};

// Parses one already-split `INSERT INTO `t` (`c1`,`c2`,...) VALUES (...),(...)`
// statement (see SqlStatementSplitter; the trailing `;` is expected to
// already be gone) into typed values. A positional INSERT with no column
// list yields empty `columns`; the caller supplies the table's own order.
ParsedInsert ParseMysqlInsert(std::string const& statementText);

#endif
