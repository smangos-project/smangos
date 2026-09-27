#include "SqlStatementSplitter.h"

#include "MysqlLexer.h"

#include <algorithm>

namespace
{
    int CountNewlines(std::string_view text)
    {
        return static_cast<int>(std::count(text.begin(), text.end(), '\n'));
    }
}

std::vector<SqlScriptStatement> SplitMysqlStatements(std::string_view text)
{
    std::vector<MysqlToken> tokens = TokenizeMysql(text);

    std::vector<SqlScriptStatement> statements;
    std::string delimiter = ";";
    std::string current;
    // The line of current[0] (its very first character, even if that's
    // leading whitespace): flush() below trims that whitespace, and any of
    // it may itself contain newlines from a token that started on an
    // earlier line than the statement's real content does.
    int currentStartLine = 1;
    bool haveStartLine = false;

    auto append = [&](std::string_view chunk, int chunkStartLine)
    {
        if (!haveStartLine && !chunk.empty())
        {
            currentStartLine = chunkStartLine;
            haveStartLine = true;
        }
        current.append(chunk);
    };

    auto flush = [&]()
    {
        size_t begin = current.find_first_not_of(" \t\r\n");
        if (begin != std::string::npos)
        {
            size_t last = current.find_last_not_of(" \t\r\n");
            int skippedNewlines = CountNewlines(std::string_view(current).substr(0, begin));
            statements.push_back({ current.substr(begin, last - begin + 1), currentStartLine + skippedNewlines });
        }
        current.clear();
        haveStartLine = false;
    };

    for (MysqlToken const& token : tokens)
    {
        if (token.type == MysqlTokenType::DelimiterDirective)
        {
            flush();
            if (!token.text.empty())
                delimiter = token.text;
            continue;
        }

        if (token.type != MysqlTokenType::Other)
        {
            append(token.text, token.line);
            continue;
        }

        std::string_view remaining = token.text;
        int line = token.line;
        for (;;)
        {
            size_t pos = remaining.find(delimiter);
            std::string_view chunk = pos == std::string_view::npos ? remaining : remaining.substr(0, pos);
            append(chunk, line);
            line += CountNewlines(chunk);
            if (pos == std::string_view::npos)
                break;
            flush();
            remaining.remove_prefix(pos + delimiter.size());
        }
    }
    flush();
    return statements;
}
