#ifndef DATABASE_TOOL_MYSQLLEXER_H
#define DATABASE_TOOL_MYSQLLEXER_H

#include <string>
#include <string_view>
#include <vector>

// MySQL scripts (the world dump and the 1140 world migrations) mix real SQL
// with mysql-client-only syntax: quoted strings and identifiers with their
// own escaping rules, comments, and the DELIMITER directive that lets a
// migration's CREATE PROCEDURE body contain semicolons. Every rewrite in
// MigrationPorter and every statement split in SqlStatementSplitter needs to
// tell those apart from plain SQL text without re-implementing this each time.
enum class MysqlTokenType
{
    Other,             // SQL text outside strings/identifiers/comments; may contain the active delimiter
    StringLiteral,     // '...' or "...", escapes left encoded (see DecodeMysqlStringLiteral)
    QuotedIdentifier,  // `...`
    Comment,           // -- ..., # ..., or /* ... */
    VersionedComment,  // /*! ... */
    DelimiterDirective // a `DELIMITER <text>` line; text is the new delimiter, already trimmed
};

struct MysqlToken
{
    MysqlTokenType type;
    std::string text; // exactly as written, including quotes/comment markers (not for DelimiterDirective)
    int line;          // 1-based line the token starts on
};

// Splits MySQL script text into tokens without parsing SQL grammar: just
// enough to tell string/identifier/comment contents apart from everything
// else, and to recognize the DELIMITER directive.
std::vector<MysqlToken> TokenizeMysql(std::string_view text);

// Decodes a StringLiteral token's quotes and escapes (\', \", \\, \n, \r, \t,
// \0, \Z, \%, \_ and doubled quotes) into the string MySQL would store.
std::string DecodeMysqlStringLiteral(std::string const& tokenText);

#endif
