#include "MysqlDdlTranslator.h"

#include "MysqlLexer.h"

#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace
{
    std::string Lower(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    // Column definitions and table options are split into words: each
    // string/backtick-identifier/comment token from MysqlLexer stays atomic
    // (so a default value's punctuation is never split), and everything
    // else is split on whitespace with "(", ")" and "," as their own words.
    void SplitOtherIntoWords(std::string const& text, std::vector<std::string>& words)
    {
        size_t i = 0;
        while (i < text.size())
        {
            char c = text[i];
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                ++i;
                continue;
            }
            if (c == '(' || c == ')' || c == ',')
            {
                words.emplace_back(1, c);
                ++i;
                continue;
            }
            size_t start = i;
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) &&
                   text[i] != '(' && text[i] != ')' && text[i] != ',')
                ++i;
            words.push_back(text.substr(start, i - start));
        }
    }

    std::vector<std::string> Words(std::string const& statementText)
    {
        std::vector<std::string> words;
        for (MysqlToken const& token : TokenizeMysql(statementText))
        {
            if (token.type == MysqlTokenType::Comment || token.type == MysqlTokenType::VersionedComment)
                continue;
            if (token.type == MysqlTokenType::Other)
                SplitOtherIntoWords(token.text, words);
            else
                words.push_back(token.text);
        }
        return words;
    }

    std::string StripIdentifierQuotes(std::string const& word)
    {
        if (word.size() >= 2 && word.front() == '`' && word.back() == '`')
        {
            std::string inner = word.substr(1, word.size() - 2);
            std::string result;
            for (size_t i = 0; i < inner.size(); ++i)
            {
                if (inner[i] == '`' && i + 1 < inner.size() && inner[i + 1] == '`')
                    ++i;
                result += inner[i];
            }
            return result;
        }
        return word;
    }

    // Skips a balanced "(" ... ")" group starting at words[i] == "(", leaving
    // i just past the matching ")". No-op if words[i] isn't "(".
    void SkipParenGroup(std::vector<std::string> const& words, size_t& i)
    {
        if (i >= words.size() || words[i] != "(")
            return;
        int depth = 1;
        ++i;
        while (i < words.size() && depth > 0)
        {
            if (words[i] == "(")
                ++depth;
            else if (words[i] == ")")
                --depth;
            ++i;
        }
    }

    enum class TypeCategory
    {
        Numeric,
        DateTime,
        Other
    };

    TypeCategory CategorizeType(std::string const& mysqlTypeLower)
    {
        static std::unordered_set<std::string> const numeric = {
            "tinyint", "smallint", "mediumint", "int", "integer", "bigint",
            "float", "double", "real", "decimal", "numeric"
        };
        static std::unordered_set<std::string> const dateTime = { "date", "datetime", "timestamp" };

        if (numeric.count(mysqlTypeLower))
            return TypeCategory::Numeric;
        if (dateTime.count(mysqlTypeLower))
            return TypeCategory::DateTime;
        return TypeCategory::Other;
    }

    std::string MapType(std::string const& mysqlTypeLower)
    {
        switch (CategorizeType(mysqlTypeLower))
        {
            case TypeCategory::Numeric:
                return mysqlTypeLower == "float" || mysqlTypeLower == "double" || mysqlTypeLower == "real"
                    ? "REAL"
                    : mysqlTypeLower == "decimal" || mysqlTypeLower == "numeric" ? "NUMERIC" : "INTEGER";
            case TypeCategory::DateTime:
                return "TEXT";
            default:
                break;
        }

        static std::unordered_set<std::string> const text = {
            "char", "varchar", "tinytext", "text", "mediumtext", "longtext", "enum", "set", "time", "year"
        };
        static std::unordered_set<std::string> const blob = {
            "tinyblob", "blob", "mediumblob", "longblob", "binary", "varbinary"
        };

        if (text.count(mysqlTypeLower))
            return "TEXT";
        if (blob.count(mysqlTypeLower))
            return "BLOB";

        throw std::runtime_error("MysqlDdlTranslator: unrecognized column type '" + mysqlTypeLower + "', needs a hand port");
    }

    std::string ImplicitDefaultLiteral(TypeCategory category)
    {
        switch (category)
        {
            case TypeCategory::Numeric: return "0";
            case TypeCategory::DateTime: return "'0000-00-00 00:00:00'";
            default: return "''";
        }
    }

    // MySQL dumps quote every DEFAULT literal, including numeric ones
    // (DEFAULT '0'); SQLite wants those unquoted.
    std::string NormalizeDefaultLiteral(std::string const& rawWord, TypeCategory category)
    {
        std::string lower = Lower(rawWord);
        if (lower == "null")
            return "NULL";
        if (lower == "current_timestamp")
            return "CURRENT_TIMESTAMP";
        if (category == TypeCategory::Numeric && rawWord.size() >= 2 &&
            (rawWord.front() == '\'' || rawWord.front() == '"'))
        {
            return rawWord.substr(1, rawWord.size() - 2);
        }
        return rawWord;
    }

    struct ColumnDef
    {
        std::string name;
        std::string sqliteType;
        bool notNull = false;
        bool hasDefault = false;
        std::string defaultLiteral;
        bool isTextType = false;
        bool autoIncrement = false;
        bool primaryKeyInline = false;
    };

    struct KeyDef
    {
        std::string name; // empty for PRIMARY KEY, or an unnamed secondary key
        bool isPrimary = false;
        bool unique = false;
        std::vector<std::string> columns;
    };

    // words[i] must be "(" (the key's column list); advances i past the ")".
    void ParseKeyColumns(std::vector<std::string> const& words, size_t& i, std::vector<std::string>& columns)
    {
        if (i >= words.size() || words[i] != "(")
            throw std::runtime_error("MysqlDdlTranslator: expected '(' in a key definition");
        ++i;
        while (i < words.size() && words[i] != ")")
        {
            columns.push_back(StripIdentifierQuotes(words[i++]));

            // A prefix-length index, e.g. `name`(20): SQLite has no such thing.
            SkipParenGroup(words, i);

            if (i < words.size() && (Lower(words[i]) == "asc" || Lower(words[i]) == "desc"))
                ++i;
            if (i < words.size() && words[i] == ",")
                ++i;
        }
        if (i < words.size())
            ++i; // ")"
    }

    ColumnDef ParseColumn(std::vector<std::string> const& words, std::vector<std::string>& warnings)
    {
        if (words.size() < 2)
            throw std::runtime_error("MysqlDdlTranslator: malformed column definition");

        size_t i = 0;
        ColumnDef col;
        col.name = StripIdentifierQuotes(words[i++]);

        std::string mysqlType = Lower(words[i++]);
        SkipParenGroup(words, i); // (length), (precision, scale), (enum values...)

        TypeCategory category = CategorizeType(mysqlType);
        col.sqliteType = MapType(mysqlType);
        col.isTextType = col.sqliteType == "TEXT";

        while (i < words.size())
        {
            std::string w = Lower(words[i]);
            if (w == "unsigned" || w == "zerofill")
            {
                ++i;
            }
            else if (w == "character")
            {
                ++i;
                if (i < words.size() && Lower(words[i]) == "set")
                    ++i;
                if (i < words.size())
                    ++i; // the charset name
            }
            else if (w == "collate")
            {
                ++i;
                if (i < words.size())
                    ++i; // we force COLLATE NOCASE ourselves below
            }
            else if (w == "not")
            {
                ++i;
                if (i < words.size() && Lower(words[i]) == "null")
                    ++i;
                col.notNull = true;
            }
            else if (w == "null")
            {
                ++i;
                col.notNull = false;
            }
            else if (w == "default")
            {
                ++i;
                if (i >= words.size())
                    throw std::runtime_error("MysqlDdlTranslator: DEFAULT with no value for column " + col.name);
                col.hasDefault = true;
                col.defaultLiteral = NormalizeDefaultLiteral(words[i++], category);
            }
            else if (w == "auto_increment")
            {
                col.autoIncrement = true;
                ++i;
            }
            else if (w == "on")
            {
                ++i;
                if (i < words.size() && Lower(words[i]) == "update")
                {
                    ++i;
                    if (i < words.size())
                        ++i; // e.g. CURRENT_TIMESTAMP
                }
            }
            else if (w == "comment")
            {
                ++i;
                if (i < words.size())
                    ++i;
            }
            else if (w == "primary")
            {
                ++i;
                if (i < words.size() && Lower(words[i]) == "key")
                    ++i;
                col.primaryKeyInline = true;
            }
            else if (w == "unique")
            {
                ++i;
                if (i < words.size() && Lower(words[i]) == "key")
                    ++i;
            }
            else
            {
                warnings.push_back("MysqlDdlTranslator: unrecognized modifier '" + words[i] + "' on column " + col.name);
                ++i;
            }
        }

        if (!col.hasDefault && col.notNull)
        {
            col.hasDefault = true;
            col.defaultLiteral = ImplicitDefaultLiteral(category);
        }
        return col;
    }

    std::string Join(std::vector<std::string> const& words)
    {
        std::string text;
        for (size_t i = 0; i < words.size(); ++i)
        {
            if (i)
                text += ' ';
            text += words[i];
        }
        return text;
    }
}

TranslatedTable TranslateMysqlCreateTable(std::string const& statementText)
{
    std::vector<std::string> words = Words(statementText);
    size_t i = 0;

    auto expect = [&](char const* keyword)
    {
        if (i >= words.size() || Lower(words[i]) != keyword)
            throw std::runtime_error(std::string("MysqlDdlTranslator: expected '") + keyword + "'");
        ++i;
    };

    expect("create");
    expect("table");
    if (i < words.size() && Lower(words[i]) == "if")
    {
        expect("if");
        expect("not");
        expect("exists");
    }
    if (i >= words.size())
        throw std::runtime_error("MysqlDdlTranslator: missing table name");

    TranslatedTable result;
    result.tableName = StripIdentifierQuotes(words[i++]);

    if (i >= words.size() || words[i] != "(")
        throw std::runtime_error("MysqlDdlTranslator: expected '(' after the table name");
    ++i;

    std::vector<std::vector<std::string>> entries;
    std::vector<std::string> current;
    int depth = 1;
    for (; i < words.size() && depth > 0; ++i)
    {
        std::string const& w = words[i];
        if (w == "(")
        {
            ++depth;
            current.push_back(w);
        }
        else if (w == ")")
        {
            --depth;
            if (depth == 0)
                break;
            current.push_back(w);
        }
        else if (w == "," && depth == 1)
        {
            entries.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(w);
        }
    }
    if (!current.empty())
        entries.push_back(current);
    // Anything after the closing ')' is a table option (ENGINE=, CHARSET=, COMMENT=, ...); all dropped.

    std::vector<ColumnDef> columns;
    std::vector<KeyDef> keys;

    for (std::vector<std::string> const& entry : entries)
    {
        if (entry.empty())
            continue;

        std::string first = Lower(entry[0]);
        size_t j = 0;

        if (first == "primary")
        {
            ++j;
            if (j < entry.size() && Lower(entry[j]) == "key")
                ++j;
            KeyDef key;
            key.isPrimary = true;
            key.unique = true;
            ParseKeyColumns(entry, j, key.columns);
            keys.push_back(std::move(key));
        }
        else if (first == "unique" || first == "key" || first == "index")
        {
            bool unique = first == "unique";
            ++j;
            if (unique && j < entry.size() && (Lower(entry[j]) == "key" || Lower(entry[j]) == "index"))
                ++j;
            KeyDef key;
            key.unique = unique;
            if (j < entry.size() && entry[j] != "(")
                key.name = StripIdentifierQuotes(entry[j++]);
            ParseKeyColumns(entry, j, key.columns);
            keys.push_back(std::move(key));
        }
        else if (first == "constraint" || first == "foreign")
        {
            result.warnings.push_back("MysqlDdlTranslator: not translated, needs a hand port: " + Join(entry));
        }
        else
        {
            columns.push_back(ParseColumn(entry, result.warnings));
        }
    }

    KeyDef* primaryKey = nullptr;
    for (KeyDef& key : keys)
        if (key.isPrimary)
            primaryKey = &key;

    // A column marked PRIMARY KEY inline (rather than a separate `PRIMARY
    // KEY (...)` table constraint, which is how mysqldump/HeidiSQL normally
    // emit it) still needs a KeyDef, unless AUTO_INCREMENT already accounts
    // for it below.
    if (!primaryKey)
    {
        for (ColumnDef& col : columns)
        {
            if (!col.primaryKeyInline)
                continue;
            KeyDef key;
            key.isPrimary = true;
            key.unique = true;
            key.columns.push_back(col.name);
            keys.push_back(std::move(key));
            primaryKey = &keys.back();
            break;
        }
    }

    // A single-column INTEGER AUTO_INCREMENT primary key becomes SQLite's
    // rowid alias (INTEGER PRIMARY KEY); any other AUTO_INCREMENT can't be
    // expressed the same way and needs a hand port.

    ColumnDef* autoIncrementColumn = nullptr;
    for (ColumnDef& col : columns)
        if (col.autoIncrement)
            autoIncrementColumn = &col;

    // A table with no primary key whose AUTO_INCREMENT column has a sole unique key (`UNIQUE KEY
    // id (id)`) is effectively keyed by that column.
    if (autoIncrementColumn && !primaryKey && !autoIncrementColumn->primaryKeyInline)
    {
        for (KeyDef& key : keys)
            if (key.unique && key.columns.size() == 1 && key.columns[0] == autoIncrementColumn->name)
            {
                key.isPrimary = true;
                primaryKey = &key;
                break;
            }
    }

    bool inlineIntegerPrimaryKey = false;
    if (autoIncrementColumn)
    {
        bool singleColumnPrimaryKey =
            (primaryKey && primaryKey->columns.size() == 1 && primaryKey->columns[0] == autoIncrementColumn->name) ||
            (!primaryKey && autoIncrementColumn->primaryKeyInline);

        if (singleColumnPrimaryKey && autoIncrementColumn->sqliteType == "INTEGER")
        {
            inlineIntegerPrimaryKey = true; // suppresses the table-level PRIMARY KEY(...) clause below; it stays a primary key either way, so it's still excluded from the CREATE INDEX loop
        }
        else
        {
            result.warnings.push_back(
                "MysqlDdlTranslator: AUTO_INCREMENT on '" + autoIncrementColumn->name +
                "' isn't a single-column INTEGER primary key, needs a hand port");
        }
    }

    std::string createTable = "CREATE TABLE `" + result.tableName + "` (\n";
    for (size_t ci = 0; ci < columns.size(); ++ci)
    {
        ColumnDef const& col = columns[ci];
        createTable += "  `" + col.name + "` " + col.sqliteType;
        if (&col == autoIncrementColumn && inlineIntegerPrimaryKey)
            createTable += " PRIMARY KEY";
        if (col.notNull)
            createTable += " NOT NULL";
        if (col.hasDefault)
            createTable += " DEFAULT " + col.defaultLiteral;
        if (col.isTextType)
            createTable += " COLLATE NOCASE";

        bool more = ci + 1 < columns.size();
        if (!inlineIntegerPrimaryKey)
            for (KeyDef const& key : keys)
                if (key.isPrimary)
                    more = true; // a table-level PRIMARY KEY clause still follows
        createTable += more ? ",\n" : "\n";
    }
    for (KeyDef const& key : keys)
    {
        if (!key.isPrimary || inlineIntegerPrimaryKey)
            continue;
        createTable += "  PRIMARY KEY (";
        for (size_t k = 0; k < key.columns.size(); ++k)
            createTable += (k ? ", `" : "`") + key.columns[k] + "`";
        createTable += ")\n";
    }
    createTable += ")";
    result.statements.push_back(createTable);

    for (KeyDef const& key : keys)
    {
        if (key.isPrimary)
            continue;

        std::string indexName = result.tableName + "__" + (key.name.empty() ? key.columns.front() : key.name);
        std::string statement = std::string("CREATE ") + (key.unique ? "UNIQUE " : "") + "INDEX \"" + indexName +
            "\" ON `" + result.tableName + "` (";
        for (size_t k = 0; k < key.columns.size(); ++k)
            statement += (k ? ", `" : "`") + key.columns[k] + "`";
        statement += ")";
        result.statements.push_back(statement);
    }

    return result;
}
