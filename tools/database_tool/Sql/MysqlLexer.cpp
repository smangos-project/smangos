#include "MysqlLexer.h"

#include <cctype>

namespace
{
    bool IsHorizontalSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\r';
    }

    // Case-insensitive match for "delimiter" as a whole word starting at pos.
    bool MatchesDelimiterKeyword(std::string_view text, size_t pos)
    {
        static const std::string_view keyword = "delimiter";
        if (pos + keyword.size() > text.size())
            return false;

        for (size_t i = 0; i < keyword.size(); ++i)
        {
            if (std::tolower(static_cast<unsigned char>(text[pos + i])) != keyword[i])
                return false;
        }

        // Must be followed by whitespace: "delimiter" alone or "delimiterx" isn't the directive.
        return pos + keyword.size() < text.size() && IsHorizontalSpace(text[pos + keyword.size()]);
    }

    class Scanner
    {
    public:
        explicit Scanner(std::string_view text)
            : m_text(text)
        {
        }

        std::vector<MysqlToken> Run()
        {
            std::vector<MysqlToken> tokens;
            while (m_pos < m_text.size())
            {
                if (m_atLineStart && TryScanDelimiterDirective(tokens))
                    continue;

                char c = m_text[m_pos];
                if (c == '\'' || c == '"')
                {
                    tokens.push_back(ScanStringLiteral());
                }
                else if (c == '`')
                {
                    tokens.push_back(ScanQuotedIdentifier());
                }
                else if (c == '-' && Peek(1) == '-' && (Peek(2) == '\0' || IsHorizontalSpace(Peek(2)) || Peek(2) == '\n'))
                {
                    tokens.push_back(ScanLineComment());
                }
                else if (c == '#')
                {
                    tokens.push_back(ScanLineComment());
                }
                else if (c == '/' && Peek(1) == '*')
                {
                    tokens.push_back(ScanBlockComment());
                }
                else
                {
                    tokens.push_back(ScanOther());
                }
            }
            return tokens;
        }

    private:
        std::string_view m_text;
        size_t m_pos = 0;
        int m_line = 1;
        bool m_atLineStart = true;

        char Peek(size_t ahead) const
        {
            size_t pos = m_pos + ahead;
            return pos < m_text.size() ? m_text[pos] : '\0';
        }

        void Advance()
        {
            char c = m_text[m_pos];
            ++m_pos;
            if (c == '\n')
            {
                ++m_line;
                m_atLineStart = true;
            }
            else if (!IsHorizontalSpace(c))
            {
                m_atLineStart = false;
            }
        }

        bool TryScanDelimiterDirective(std::vector<MysqlToken>& tokens)
        {
            size_t wordStart = m_pos;
            while (wordStart < m_text.size() && IsHorizontalSpace(m_text[wordStart]))
                ++wordStart;

            if (!MatchesDelimiterKeyword(m_text, wordStart))
                return false;

            int line = m_line;
            while (m_pos < wordStart + 9) // consume up to and including "delimiter"
                Advance();
            while (m_pos < m_text.size() && IsHorizontalSpace(m_text[m_pos]))
                Advance();

            std::string newDelimiter;
            while (m_pos < m_text.size() && m_text[m_pos] != '\n')
            {
                newDelimiter += m_text[m_pos];
                Advance();
            }
            if (m_pos < m_text.size())
                Advance(); // consume the newline itself

            while (!newDelimiter.empty() && IsHorizontalSpace(newDelimiter.back()))
                newDelimiter.pop_back();

            tokens.push_back({ MysqlTokenType::DelimiterDirective, newDelimiter, line });
            return true;
        }

        MysqlToken ScanStringLiteral()
        {
            int line = m_line;
            char quote = m_text[m_pos];
            std::string raw(1, quote);
            Advance();

            while (m_pos < m_text.size())
            {
                char c = m_text[m_pos];
                if (c == '\\' && m_pos + 1 < m_text.size())
                {
                    raw += c;
                    Advance();
                    raw += m_text[m_pos];
                    Advance();
                    continue;
                }
                if (c == quote)
                {
                    if (Peek(1) == quote)
                    {
                        raw += quote;
                        Advance();
                        raw += quote;
                        Advance();
                        continue;
                    }
                    raw += quote;
                    Advance();
                    break;
                }
                raw += c;
                Advance();
            }
            return { MysqlTokenType::StringLiteral, raw, line };
        }

        MysqlToken ScanQuotedIdentifier()
        {
            int line = m_line;
            std::string raw = "`";
            Advance();

            while (m_pos < m_text.size())
            {
                char c = m_text[m_pos];
                if (c == '`')
                {
                    if (Peek(1) == '`')
                    {
                        raw += "``";
                        Advance();
                        Advance();
                        continue;
                    }
                    raw += '`';
                    Advance();
                    break;
                }
                raw += c;
                Advance();
            }
            return { MysqlTokenType::QuotedIdentifier, raw, line };
        }

        MysqlToken ScanLineComment()
        {
            int line = m_line;
            std::string raw;
            while (m_pos < m_text.size() && m_text[m_pos] != '\n')
            {
                raw += m_text[m_pos];
                Advance();
            }
            return { MysqlTokenType::Comment, raw, line };
        }

        MysqlToken ScanBlockComment()
        {
            int line = m_line;
            bool versioned = Peek(2) == '!';
            std::string raw = "/*";
            Advance();
            Advance();

            while (m_pos < m_text.size())
            {
                if (m_text[m_pos] == '*' && Peek(1) == '/')
                {
                    raw += "*/";
                    Advance();
                    Advance();
                    break;
                }
                raw += m_text[m_pos];
                Advance();
            }
            return { versioned ? MysqlTokenType::VersionedComment : MysqlTokenType::Comment, raw, line };
        }

        MysqlToken ScanOther()
        {
            int line = m_line;
            std::string raw;
            while (m_pos < m_text.size())
            {
                if (m_atLineStart)
                {
                    size_t wordStart = m_pos;
                    while (wordStart < m_text.size() && IsHorizontalSpace(m_text[wordStart]))
                        ++wordStart;
                    if (MatchesDelimiterKeyword(m_text, wordStart))
                        break;
                }

                char c = m_text[m_pos];
                if (c == '\'' || c == '"' || c == '`' || c == '#')
                    break;
                if (c == '-' && Peek(1) == '-' && (Peek(2) == '\0' || IsHorizontalSpace(Peek(2)) || Peek(2) == '\n'))
                    break;
                if (c == '/' && Peek(1) == '*')
                    break;

                raw += c;
                Advance();
            }
            return { MysqlTokenType::Other, raw, line };
        }
    };
}

std::vector<MysqlToken> TokenizeMysql(std::string_view text)
{
    return Scanner(text).Run();
}

bool StartsWithMysqlKeyword(std::string_view text, std::string const& keywordUpper)
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

std::string_view SkipLeadingMysqlNoise(std::string_view text)
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

std::string DecodeMysqlStringLiteral(std::string const& tokenText)
{
    if (tokenText.size() < 2)
        return std::string();

    char quote = tokenText.front();
    size_t end = tokenText.size() - 1; // index of the closing quote
    std::string result;

    size_t i = 1;
    while (i < end)
    {
        char c = tokenText[i];
        if (c == '\\' && i + 1 < end)
        {
            char next = tokenText[i + 1];
            switch (next)
            {
                case '0': result += '\0'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'Z': result += '\x1a'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                case '"': result += '"'; break;
                case '%': result += "\\%"; break;
                case '_': result += "\\_"; break;
                default: result += next; break;
            }
            i += 2;
            continue;
        }
        if (c == quote && i + 1 < end && tokenText[i + 1] == quote)
        {
            result += quote;
            i += 2;
            continue;
        }
        result += c;
        ++i;
    }
    return result;
}
