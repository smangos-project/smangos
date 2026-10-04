#ifndef DATABASE_TOOL_MYSQLDDLTRANSLATOR_H
#define DATABASE_TOOL_MYSQLDDLTRANSLATOR_H

#include <string>
#include <vector>

// The SQLite statements one MySQL CREATE TABLE statement translates to.
struct TranslatedTable
{
    std::string tableName;
    std::vector<std::string> statements; // [0] is CREATE TABLE, the rest are one CREATE INDEX each
    std::vector<std::string> warnings;    // needs a hand port (see world-sql.md); nothing here is applied automatically
};

// Translates one already-split `CREATE TABLE` statement (see
// SqlStatementSplitter) from MySQL to SQLite dialect: column types, MySQL's
// implicit non-strict defaults, a single-column integer AUTO_INCREMENT
// primary key, secondary keys as separate indexes, and dropped table
// options. Shared by MysqlDumpImport and the port-migration command.
TranslatedTable TranslateMysqlCreateTable(std::string const& statementText);

#endif
