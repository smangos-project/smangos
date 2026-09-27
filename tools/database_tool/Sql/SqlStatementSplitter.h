#ifndef DATABASE_TOOL_SQLSTATEMENTSPLITTER_H
#define DATABASE_TOOL_SQLSTATEMENTSPLITTER_H

#include <string>
#include <string_view>
#include <vector>

struct SqlStatement
{
    std::string text; // trimmed, without the trailing delimiter
    int line;          // 1-based line the statement starts on
};

// Splits a MySQL script into statements, honouring DELIMITER lines the way
// the mysql command-line client does (needed for the `DELIMITER ??` stored
// procedure envelope every world migration is wrapped in). Delimiters found
// inside string/identifier/comment text, via MysqlLexer, never split a
// statement early.
std::vector<SqlStatement> SplitMysqlStatements(std::string_view text);

#endif
