#include "SqlStatementSplitter.h"

#include "MysqlLexer.h"

std::vector<SqlScriptStatement> SplitMysqlStatements(std::string_view text)
{
    std::vector<MysqlToken> tokens = TokenizeMysql(text);

    std::vector<SqlScriptStatement> statements;
    std::string delimiter = ";";
    std::string current;
    int currentLine = 1;
    bool needLine = true;

    auto flush = [&]()
    {
        size_t begin = current.find_first_not_of(" \t\r\n");
        if (begin != std::string::npos)
        {
            size_t last = current.find_last_not_of(" \t\r\n");
            statements.push_back({ current.substr(begin, last - begin + 1), currentLine });
        }
        current.clear();
        needLine = true;
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
            if (needLine)
            {
                currentLine = token.line;
                needLine = false;
            }
            current += token.text;
            continue;
        }

        std::string_view remaining = token.text;
        for (;;)
        {
            size_t pos = remaining.find(delimiter);
            std::string_view chunk = pos == std::string_view::npos ? remaining : remaining.substr(0, pos);
            if (!chunk.empty() && needLine)
            {
                currentLine = token.line;
                needLine = false;
            }
            current.append(chunk);
            if (pos == std::string_view::npos)
                break;
            flush();
            remaining.remove_prefix(pos + delimiter.size());
        }
    }
    flush();
    return statements;
}
