#include "MysqlInsertParser.h"

#include "MysqlLexer.h"

#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace
{
    enum class LexemeKind { LParen, RParen, Comma, Word, Str, Ident };

    struct Lexeme
    {
        LexemeKind kind;
        std::string text;
    };

    std::string Lower(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    // Splits an "Other" token's raw text on '(', ')', ',' and whitespace, the
    // way MysqlDdlTranslator's own SplitOtherIntoWords does: everything that
    // isn't one of those punctuation marks is a keyword, NULL, or a numeric
    // literal, and none of those ever contain whitespace or punctuation.
    void SplitOtherIntoLexemes(std::string const& text, std::vector<Lexeme>& lexemes)
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
                lexemes.push_back({ c == '(' ? LexemeKind::LParen : c == ')' ? LexemeKind::RParen : LexemeKind::Comma,
                    std::string(1, c) });
                ++i;
                continue;
            }
            size_t start = i;
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) &&
                   text[i] != '(' && text[i] != ')' && text[i] != ',')
                ++i;
            lexemes.push_back({ LexemeKind::Word, text.substr(start, i - start) });
        }
    }

    std::vector<Lexeme> Lex(std::string const& statementText)
    {
        std::vector<Lexeme> lexemes;
        for (MysqlToken const& token : TokenizeMysql(statementText))
        {
            switch (token.type)
            {
                case MysqlTokenType::Other:
                    SplitOtherIntoLexemes(token.text, lexemes);
                    break;
                case MysqlTokenType::StringLiteral:
                    lexemes.push_back({ LexemeKind::Str, token.text });
                    break;
                case MysqlTokenType::QuotedIdentifier:
                    lexemes.push_back({ LexemeKind::Ident, token.text });
                    break;
                default:
                    break; // comments/versioned comments/DELIMITER don't belong inside an INSERT
            }
        }
        return lexemes;
    }

    std::string StripBackticks(std::string const& raw)
    {
        std::string inner = raw.substr(1, raw.size() - 2);
        std::string result;
        for (size_t i = 0; i < inner.size(); ++i)
        {
            if (inner[i] == '`' && i + 1 < inner.size() && inner[i + 1] == '`')
                ++i;
            result += inner[i];
        }
        return result;
    }

    bool ParseNumber(std::string const& word, MysqlValue& value)
    {
        if (word.empty())
            return false;

        bool isReal = word.find('.') != std::string::npos || word.find('e') != std::string::npos ||
            word.find('E') != std::string::npos;

        char const* begin = word.c_str();
        char* end = nullptr;
        if (isReal)
        {
            value.realValue = std::strtod(begin, &end);
            value.kind = MysqlValue::Kind::Real;
        }
        else
        {
            value.integerValue = std::strtoll(begin, &end, 10);
            value.kind = MysqlValue::Kind::Integer;
        }
        return end == begin + word.size();
    }

    class Parser
    {
    public:
        explicit Parser(std::vector<Lexeme> lexemes)
            : m_lexemes(std::move(lexemes))
        {
        }

        ParsedInsert Run()
        {
            ParsedInsert result;
            try
            {
                ExpectWord("insert");
                if (PeekIsWord("ignore"))
                    Advance();
                ExpectWord("into");
                result.tableName = StripBackticks(ExpectIdent());

                if (PeekIsKind(LexemeKind::LParen))
                {
                    Advance();
                    while (!PeekIsKind(LexemeKind::RParen))
                    {
                        result.columns.push_back(StripBackticks(ExpectIdent()));
                        if (PeekIsKind(LexemeKind::Comma))
                            Advance();
                    }
                    Advance(); // ")"
                }

                ExpectWord("values");

                for (;;)
                {
                    ExpectKind(LexemeKind::LParen);
                    std::vector<MysqlValue> row;
                    while (!PeekIsKind(LexemeKind::RParen))
                    {
                        row.push_back(ParseValue());
                        if (PeekIsKind(LexemeKind::Comma))
                            Advance();
                    }
                    Advance(); // ")"

                    if (!result.columns.empty() && row.size() != result.columns.size())
                        throw std::runtime_error("row has " + std::to_string(row.size()) +
                            " values but the column list has " + std::to_string(result.columns.size()));
                    result.rows.push_back(std::move(row));

                    if (PeekIsKind(LexemeKind::Comma))
                        Advance();
                    else
                        break;
                }

                if (m_pos != m_lexemes.size())
                    throw std::runtime_error("unexpected trailing content after the last row");
            }
            catch (std::exception const& e)
            {
                result.error = e.what();
            }
            return result;
        }

    private:
        std::vector<Lexeme> m_lexemes;
        size_t m_pos = 0;

        Lexeme const& Peek() const
        {
            if (m_pos >= m_lexemes.size())
                throw std::runtime_error("unexpected end of the INSERT statement");
            return m_lexemes[m_pos];
        }

        bool PeekIsKind(LexemeKind kind) const
        {
            return m_pos < m_lexemes.size() && m_lexemes[m_pos].kind == kind;
        }

        bool PeekIsWord(std::string const& lowerWord) const
        {
            return m_pos < m_lexemes.size() && m_lexemes[m_pos].kind == LexemeKind::Word &&
                Lower(m_lexemes[m_pos].text) == lowerWord;
        }

        void Advance() { ++m_pos; }

        void ExpectKind(LexemeKind kind)
        {
            if (!PeekIsKind(kind))
                throw std::runtime_error("unexpected token in the INSERT statement");
            Advance();
        }

        void ExpectWord(std::string const& lowerWord)
        {
            if (!PeekIsWord(lowerWord))
                throw std::runtime_error("expected '" + lowerWord + "'");
            Advance();
        }

        std::string ExpectIdent()
        {
            Lexeme const& lexeme = Peek();
            if (lexeme.kind != LexemeKind::Ident)
                throw std::runtime_error("expected a `quoted` identifier");
            std::string text = lexeme.text;
            Advance();
            return text;
        }

        MysqlValue ParseValue()
        {
            Lexeme const& lexeme = Peek();
            MysqlValue value;
            if (lexeme.kind == LexemeKind::Str)
            {
                value.kind = MysqlValue::Kind::Text;
                value.textValue = DecodeMysqlStringLiteral(lexeme.text);
                Advance();
                return value;
            }
            if (lexeme.kind == LexemeKind::Word)
            {
                if (Lower(lexeme.text) == "null")
                {
                    Advance();
                    return value; // Kind::Null is the default
                }
                if (ParseNumber(lexeme.text, value))
                {
                    Advance();
                    return value;
                }
            }
            throw std::runtime_error("unrecognized value '" + lexeme.text + "'");
        }
    };
}

ParsedInsert ParseMysqlInsert(std::string const& statementText)
{
    return Parser(Lex(statementText)).Run();
}
