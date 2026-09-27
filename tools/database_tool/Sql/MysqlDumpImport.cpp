#include "MysqlDumpImport.h"

#include "MysqlDdlTranslator.h"
#include "MysqlInsertParser.h"
#include "SqlStatementSplitter.h"

#include "World/DoltLiteDatabase.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace
{
    std::string ReadFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool StartsWithKeyword(std::string_view text, std::string const& keywordUpper)
    {
        size_t i = 0;
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i])))
            ++i;
        if (text.size() - i < keywordUpper.size())
            return false;
        for (size_t k = 0; k < keywordUpper.size(); ++k)
            if (std::toupper(static_cast<unsigned char>(text[i + k])) != keywordUpper[k])
                return false;
        return true;
    }

    // Every real statement in the dump is preceded by a `-- Dumping ...`
    // comment (HeidiSQL's own), and comments have no delimiter of their own
    // to end a statement early, so SqlStatementSplitter glues each one onto
    // the front of whatever follows. Classifying a statement by keyword
    // needs to see past that (execution doesn't: MysqlDdlTranslator/
    // ParseMysqlInsert already skip comment tokens, and SQLite itself
    // accepts a leading `--`/`/* */` comment in a statement it's given).
    std::string_view SkipLeadingNoise(std::string_view text)
    {
        for (;;)
        {
            while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
                text.remove_prefix(1);

            if (text.size() >= 2 && text[0] == '-' && text[1] == '-')
            {
                size_t newline = text.find('\n');
                text = newline == std::string_view::npos ? std::string_view() : text.substr(newline + 1);
                continue;
            }
            if (!text.empty() && text.front() == '#')
            {
                size_t newline = text.find('\n');
                text = newline == std::string_view::npos ? std::string_view() : text.substr(newline + 1);
                continue;
            }
            // "/*! ... */" is a versioned comment: real, executable content
            // (world-sql.md), not noise to skip past.
            if (text.size() >= 3 && text[0] == '/' && text[1] == '*' && text[2] != '!')
            {
                size_t end = text.find("*/");
                text = end == std::string_view::npos ? std::string_view() : text.substr(end + 2);
                continue;
            }
            break;
        }
        return text;
    }

    // A standalone `/*!40101 SET ... */` line and a plain `SET ...` both mean
    // "connection/session setup this tool doesn't need" (world-sql.md).
    bool IsSkippableStatement(std::string_view text)
    {
        return text.rfind("/*!", 0) == 0 || StartsWithKeyword(text, "SET");
    }

    struct PreparedInsert
    {
        std::vector<std::string> columns;
        DoltLiteStatement statement;
    };

    std::string BuildInsertSql(ParsedInsert const& parsed)
    {
        std::string sql = "INSERT INTO `" + parsed.tableName + "` (";
        for (size_t i = 0; i < parsed.columns.size(); ++i)
            sql += (i ? ", `" : "`") + parsed.columns[i] + "`";
        sql += ") VALUES (";
        for (size_t i = 0; i < parsed.columns.size(); ++i)
            sql += (i ? ", ?" : "?");
        sql += ")";
        return sql;
    }

    void BindRow(DoltLiteStatement& statement, std::vector<MysqlValue> const& row)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            int index = static_cast<int>(i) + 1;
            MysqlValue const& value = row[i];
            switch (value.kind)
            {
                case MysqlValue::Kind::Null:    statement.BindNull(index); break;
                case MysqlValue::Kind::Integer: statement.BindInt64(index, value.integerValue); break;
                case MysqlValue::Kind::Real:    statement.BindDouble(index, value.realValue); break;
                case MysqlValue::Kind::Text:    statement.BindText(index, value.textValue); break;
            }
        }
        statement.Step();
        statement.Reset();
    }

    std::string WithLine(int line, std::string const& message)
    {
        return "line " + std::to_string(line) + ": " + message;
    }
}

DumpImportResult ImportMysqlDump(DoltLiteDatabase& db, std::filesystem::path const& dumpPath, std::string const& commitMessage)
{
    DumpImportResult result;

    std::string content = ReadFile(dumpPath);
    if (content.empty())
    {
        result.error = "cannot read " + dumpPath.string();
        return result;
    }

    std::vector<SqlScriptStatement> statements = SplitMysqlStatements(content);

    std::unordered_map<std::string, PreparedInsert> preparedInserts;
    std::unordered_map<std::string, uint64_t> expectedRowCounts;
    std::vector<std::string> deferredIndexStatements;

    int currentLine = 0;
    try
    {
        db.Begin();

        for (SqlScriptStatement const& statement : statements)
        {
            currentLine = statement.line;
            std::string const& text = statement.text;
            std::string_view kind = SkipLeadingNoise(text);

            if (StartsWithKeyword(kind, "DROP TABLE"))
            {
                db.Execute(text);
                continue;
            }

            if (StartsWithKeyword(kind, "CREATE TABLE"))
            {
                TranslatedTable translated = TranslateMysqlCreateTable(text);
                if (!translated.warnings.empty())
                    throw std::runtime_error(translated.warnings.front());

                db.Execute(translated.statements.front());
                deferredIndexStatements.insert(deferredIndexStatements.end(),
                    translated.statements.begin() + 1, translated.statements.end());
                ++result.tablesCreated;
                continue;
            }

            if (StartsWithKeyword(kind, "INSERT INTO") || StartsWithKeyword(kind, "INSERT IGNORE INTO"))
            {
                ParsedInsert parsed = ParseMysqlInsert(text);
                if (!parsed.error.empty())
                    throw std::runtime_error(parsed.error);

                auto it = preparedInserts.find(parsed.tableName);
                if (it == preparedInserts.end() || it->second.columns != parsed.columns)
                {
                    it = preparedInserts.insert_or_assign(parsed.tableName,
                        PreparedInsert{ parsed.columns, db.Prepare(BuildInsertSql(parsed)) }).first;
                }

                for (std::vector<MysqlValue> const& row : parsed.rows)
                    BindRow(it->second.statement, row);

                expectedRowCounts[parsed.tableName] += parsed.rows.size();
                result.rowsInserted += parsed.rows.size();
                continue;
            }

            if (IsSkippableStatement(kind))
                continue;

            throw std::runtime_error("unrecognized statement");
        }

        preparedInserts.clear(); // release before the deferred index DDL

        for (std::string const& indexStatement : deferredIndexStatements)
            db.Execute(indexStatement);

        for (auto const& [tableName, expectedCount] : expectedRowCounts)
        {
            DoltLiteStatement count = db.Prepare("SELECT COUNT(*) FROM `" + tableName + "`");
            if (!count.Step() || static_cast<uint64_t>(count.ColumnInt64(0)) != expectedCount)
                throw std::runtime_error("row count mismatch for `" + tableName + "`");
        }

        db.Commit();
    }
    catch (std::exception const& e)
    {
        result.error = currentLine ? WithLine(currentLine, e.what()) : e.what();
        try { db.Rollback(); } catch (std::exception const&) {}
        return result;
    }

    DoltLiteStatement integrity = db.Prepare("PRAGMA integrity_check");
    if (!integrity.Step() || integrity.ColumnText(0) != "ok")
    {
        result.error = "PRAGMA integrity_check failed";
        return result;
    }

    db.DoltCommit(commitMessage);

    result.ok = true;
    return result;
}
