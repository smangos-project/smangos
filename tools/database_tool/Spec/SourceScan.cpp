#include "SourceScan.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>

static std::vector<std::string> SplitLines(std::string const& text)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

static std::string Trim(std::string const& s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

// This project's fixed vanilla target: cmake/options.cmake defaults SUPPORTED_CLIENT_BUILD to
// CLIENT_BUILD_1_12_1, and cmake/generators/Progression.h.in defines that as 5875, the highest
// build this codebase names. So any `SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_x_y` comparison
// against an earlier build is always true, and `<=`/`==` can be read the same way, without
// needing the actual CMake configuration.
static std::optional<bool> BranchConditionActive(std::string const& condition)
{
    if (condition.find("SUPPORTED_CLIENT_BUILD") == std::string::npos)
        return std::nullopt;
    if (condition.find("&&") != std::string::npos || condition.find("||") != std::string::npos)
        return std::nullopt;

    bool isSelf = condition.find("CLIENT_BUILD_1_12_1") != std::string::npos;
    if (condition.find(">=") != std::string::npos)
        return true; // 5875 >= anything this codebase names
    if (condition.find("<=") != std::string::npos)
        return isSelf; // 5875 <= X only when X is 5875 itself
    if (condition.find("==") != std::string::npos)
        return isSelf;
    if (condition.find('>') != std::string::npos)
        return true; // 5875 > any earlier named build
    if (condition.find('<') != std::string::npos)
        return false;
    return std::nullopt;
}

// Flattens `#if`/`#elif`/`#else`/`#endif` blocks to the one branch active for this project's
// vanilla build (see BranchConditionActive). A block whose branches can't be read that way
// falls back to the last textual branch (usually `#else`) -- a hint for a human reviewer, not
// a claim of correctness.
static std::vector<std::string> ResolveActiveLines(std::vector<std::string> const& lines)
{
    std::vector<std::string> result;
    size_t i = 0;
    while (i < lines.size())
    {
        std::string trimmed = Trim(lines[i]);
        if (trimmed.rfind("#if", 0) == 0 && trimmed.rfind("#ifdef", 0) != 0 && trimmed.rfind("#ifndef", 0) != 0)
        {
            struct Branch
            {
                std::string condition;
                std::vector<std::string> lines;
            };
            std::vector<Branch> branches;
            std::string condition = Trim(trimmed.substr(3));
            std::vector<std::string> current;
            int depth = 0;
            ++i;
            for (; i < lines.size(); ++i)
            {
                std::string t = Trim(lines[i]);
                if (depth > 0)
                {
                    if (t.rfind("#if", 0) == 0)
                        ++depth;
                    else if (t.rfind("#endif", 0) == 0)
                        --depth;
                    current.push_back(lines[i]);
                    continue;
                }
                if (t.rfind("#if", 0) == 0)
                {
                    ++depth;
                    current.push_back(lines[i]);
                    continue;
                }
                if (t.rfind("#elif", 0) == 0)
                {
                    branches.push_back({ condition, current });
                    condition = Trim(t.substr(5));
                    current.clear();
                    continue;
                }
                if (t.rfind("#else", 0) == 0)
                {
                    branches.push_back({ condition, current });
                    condition.clear();
                    current.clear();
                    continue;
                }
                if (t.rfind("#endif", 0) == 0)
                {
                    branches.push_back({ condition, current });
                    ++i;
                    break;
                }
                current.push_back(lines[i]);
            }

            size_t chosen = branches.size();
            for (size_t b = 0; b < branches.size(); ++b)
            {
                std::optional<bool> active = BranchConditionActive(branches[b].condition);
                if (active && *active)
                {
                    chosen = b;
                    break;
                }
            }
            if (chosen == branches.size())
                chosen = branches.empty() ? 0 : branches.size() - 1;

            if (!branches.empty())
            {
                std::vector<std::string> nested = ResolveActiveLines(branches[chosen].lines);
                result.insert(result.end(), nested.begin(), nested.end());
            }
            continue;
        }

        result.push_back(lines[i]);
        ++i;
    }
    return result;
}

std::map<std::string, std::string> ParseFmtStrings(std::string const& text)
{
    static std::regex const declRegex(R"(^\s*char\s+const\s+(\w+)\s*\[\s*\]\s*=\s*\"([^\"]*)\"\s*;)");

    std::map<std::string, std::string> result;
    for (std::string const& line : ResolveActiveLines(SplitLines(text)))
    {
        std::smatch m;
        if (std::regex_search(line, m, declRegex))
            result[m[1].str()] = m[2].str();
    }
    return result;
}

static std::map<std::string, uint32_t> ParseDefines(std::string const& text)
{
    static std::regex const defineRegex(R"(^\s*#define\s+(\w+)\s+(\d+)\b)");
    std::map<std::string, uint32_t> defines;
    for (std::string const& line : SplitLines(text))
    {
        std::smatch m;
        if (std::regex_search(line, m, defineRegex))
            defines[m[1].str()] = static_cast<uint32_t>(std::stoul(m[2].str()));
    }
    return defines;
}

static uint32_t ResolveArrayWidth(std::string const& bracket, std::map<std::string, uint32_t> const& defines)
{
    std::string inner = Trim(bracket.substr(1, bracket.size() >= 2 ? bracket.size() - 2 : 0));
    if (inner.empty())
        return 1;
    if (inner.find_first_not_of("0123456789") == std::string::npos)
        return static_cast<uint32_t>(std::stoul(inner));
    auto it = defines.find(inner);
    return it != defines.end() ? it->second : 1;
}

// `declText` ends with ';', has no '(' and no comment. Pointer/reference markers can sit next
// to the type or the name (`char* name`, `char *Name`, `char const* Name`), so instead of
// anchoring a single regex on that, this strips them and takes the last remaining token as the
// field name -- the type itself isn't needed for the positional walk.
static bool ParseMemberDecl(std::string declText, std::string& name, std::string& arrayPart)
{
    declText.pop_back(); // trailing ';'
    declText = Trim(declText);

    size_t brace = declText.find('{');
    if (brace != std::string::npos)
        declText = Trim(declText.substr(0, brace));
    size_t eq = declText.find('=');
    if (eq != std::string::npos)
        declText = Trim(declText.substr(0, eq));

    arrayPart.clear();
    size_t bracket = declText.find('[');
    if (bracket != std::string::npos)
    {
        size_t close = declText.find(']', bracket);
        if (close == std::string::npos)
            return false;
        arrayPart = declText.substr(bracket, close - bracket + 1);
        declText = Trim(declText.substr(0, bracket));
    }

    for (char& c : declText)
        if (c == '*' || c == '&')
            c = ' ';

    std::istringstream stream(declText);
    std::vector<std::string> tokens;
    std::string tok;
    while (stream >> tok)
        tokens.push_back(tok);
    if (tokens.size() < 2)
        return false;

    name = tokens.back();
    for (char c : name)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
            return false;
    return true;
}

std::vector<StructField> ParseStructFields(std::string const& text, std::string const& structName)
{
    std::vector<std::string> lines = SplitLines(text);
    std::regex const structRegex(R"(^\s*(?:struct|class)\s+)" + structName + R"(\b)");

    size_t declLine = SIZE_MAX;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (std::regex_search(lines[i], structRegex))
        {
            declLine = i;
            break;
        }
    }
    if (declLine == SIZE_MAX)
        return {};

    size_t openLine = declLine;
    while (openLine < lines.size() && lines[openLine].find('{') == std::string::npos)
        ++openLine;
    if (openLine >= lines.size())
        return {};

    int depth = 0;
    for (char c : lines[openLine])
    {
        if (c == '{') ++depth;
        else if (c == '}') --depth;
    }

    std::vector<std::string> rawBody;
    for (size_t i = openLine + 1; i < lines.size() && depth > 0; ++i)
    {
        int delta = 0;
        for (char c : lines[i])
        {
            if (c == '{') ++delta;
            else if (c == '}') --delta;
        }
        if (depth + delta <= 0)
            break;
        depth += delta;
        rawBody.push_back(lines[i]);
    }

    std::map<std::string, uint32_t> defines = ParseDefines(text);
    static std::regex const positionMarker(R"(//\s*(\d+)\s*(?:-\s*(\d+))?\b)");

    std::vector<StructField> fields;
    for (std::string const& raw : ResolveActiveLines(rawBody))
    {
        std::string line = Trim(raw);
        if (line.empty())
            continue;

        bool isCommentLine = line.rfind("//", 0) == 0;
        std::string candidate = isCommentLine ? Trim(line.substr(2)) : line;
        size_t trailingComment = candidate.find("//");
        std::string declText = trailingComment == std::string::npos ? candidate : Trim(candidate.substr(0, trailingComment));

        if (!declText.empty() && declText.back() == ';' && declText.find('(') == std::string::npos)
        {
            std::string name, arrayPart;
            if (ParseMemberDecl(declText, name, arrayPart))
            {
                uint32_t width = arrayPart.empty() ? 1 : ResolveArrayWidth(arrayPart, defines);
                fields.push_back({ name, width, !isCommentLine });
                continue;
            }
        }

        if (isCommentLine)
        {
            std::smatch pm;
            if (std::regex_search(line, pm, positionMarker))
            {
                uint32_t lo = static_cast<uint32_t>(std::stoul(pm[1].str()));
                uint32_t hi = pm[2].matched ? static_cast<uint32_t>(std::stoul(pm[2].str())) : lo;
                if (hi >= lo)
                    fields.push_back({ "", hi - lo + 1, false });
            }
        }
    }
    return fields;
}

// Splits a call's raw argument text on top-level commas (not nested inside `()`/`<>` or
// string literals); good enough for the plain calls this file parses.
static std::vector<std::string> SplitTopLevelArgs(std::string const& args)
{
    std::vector<std::string> result;
    std::string current;
    int depth = 0;
    bool inString = false;
    for (size_t i = 0; i < args.size(); ++i)
    {
        char c = args[i];
        if (inString)
        {
            current += c;
            if (c == '\\' && i + 1 < args.size())
            {
                current += args[++i];
                continue;
            }
            if (c == '"')
                inString = false;
            continue;
        }
        if (c == '"')
        {
            inString = true;
            current += c;
        }
        else if (c == '(' || c == '<')
        {
            ++depth;
            current += c;
        }
        else if (c == ')' || c == '>')
        {
            --depth;
            current += c;
        }
        else if (c == ',' && depth == 0)
        {
            result.push_back(Trim(current));
            current.clear();
        }
        else
        {
            current += c;
        }
    }
    if (!Trim(current).empty())
        result.push_back(Trim(current));
    return result;
}

static std::string StripQuotes(std::string s)
{
    s = Trim(s);
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

// Unlike ParseFmtStrings/ParseStructFields, this doesn't resolve #if branches -- DBCStores.cpp
// has exactly one (around sItemBagFamilyStore's LoadDBC call, `> CLIENT_BUILD_1_8_4`), which is
// true for every build this project defines, so a plain line scan already gives the right
// answer; branch resolution would also cost the accurate `i + 1` line numbers callers rely on.
std::vector<DbcStorageLink> ParseDbcStorageLinks(std::string const& text)
{
    static std::regex const declRegex(R"(^\s*DBCStorage\s*<\s*(\w+)\s*>\s*(\w+)\s*\(([^)]*)\)\s*;)");

    std::vector<DbcStorageLink> links;
    std::vector<std::string> lines = SplitLines(text);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (Trim(lines[i]).rfind("//", 0) == 0)
            continue;
        std::smatch m;
        if (!std::regex_search(lines[i], m, declRegex))
            continue;
        std::vector<std::string> args = SplitTopLevelArgs(m[3].str());
        if (args.size() != 1)
            continue;
        links.push_back({ m[2].str(), m[1].str(), args[0], static_cast<uint32_t>(i + 1) });
    }
    return links;
}

std::vector<DbcLoadCall> ParseLoadDbcCalls(std::string const& text)
{
    static std::regex const callRegex(R"(\bLoadDBC\s*\(([^)]*)\)\s*;)");

    std::vector<DbcLoadCall> calls;
    std::vector<std::string> lines = SplitLines(text);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (Trim(lines[i]).rfind("//", 0) == 0)
            continue;
        std::smatch m;
        if (!std::regex_search(lines[i], m, callRegex))
            continue;
        std::vector<std::string> args = SplitTopLevelArgs(m[1].str());
        if (args.size() != 6)
            continue;
        std::string fileName = StripQuotes(args[5]);
        size_t dot = fileName.rfind(".dbc");
        std::string stem = dot == std::string::npos ? fileName : fileName.substr(0, dot);
        calls.push_back({ args[3], stem, static_cast<uint32_t>(i + 1) });
    }
    return calls;
}

std::vector<SqlStorageLink> ParseSqlStorageLinks(std::string const& text)
{
    static std::regex const declRegex(R"(^\s*SQLStorage\s+(\w+)\s*\(([^)]*)\)\s*;)");

    std::vector<SqlStorageLink> links;
    std::vector<std::string> lines = SplitLines(text);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::smatch m;
        if (!std::regex_search(lines[i], m, declRegex))
            continue;
        std::vector<std::string> args = SplitTopLevelArgs(m[2].str());
        if (args.size() == 3)
            links.push_back({ m[1].str(), args[0], StripQuotes(args[1]), StripQuotes(args[2]), static_cast<uint32_t>(i + 1) });
        else if (args.size() == 4)
            links.push_back({ m[1].str(), args[1], StripQuotes(args[2]), StripQuotes(args[3]), static_cast<uint32_t>(i + 1) });
    }
    return links;
}

uint32_t CountFieldUsesInText(std::string const& text, std::string const& fieldName)
{
    std::regex const usageRegex(R"([.>]\s*)" + fieldName + R"(\b)");
    uint32_t uses = 0;
    for (auto it = std::sregex_iterator(text.begin(), text.end(), usageRegex); it != std::sregex_iterator(); ++it)
        ++uses;
    return uses;
}

// A plain `col1, col2, ... FROM table` list: every item is a bare identifier, no `*`,
// function calls, subqueries or table-qualified names.
static bool ParseColumnList(std::string const& columnList, std::vector<std::string>& columns)
{
    static std::regex const identifier(R"(^[A-Za-z_][A-Za-z0-9_]*$)");
    std::vector<std::string> items = SplitTopLevelArgs(columnList);
    if (items.empty())
        return false;
    for (std::string item : items)
    {
        if (!item.empty() && item.front() == '`' && item.back() == '`')
            item = item.substr(1, item.size() - 2);
        if (!std::regex_match(item, identifier))
            return false;
        columns.push_back(item);
    }
    return true;
}

// Concatenates `"..."` literal `start` with any immediately following (whitespace/newline
// separated) string literals, C++ adjacent-string-literal style, decoding `\"`/`\\`.
static std::string ConcatenateAdjacentLiterals(std::string const& content, size_t start, size_t& endOut)
{
    std::string result;
    size_t pos = start;
    while (pos < content.size())
    {
        while (pos < content.size() && (std::isspace(static_cast<unsigned char>(content[pos]))))
            ++pos;
        if (pos >= content.size() || content[pos] != '"')
            break;
        ++pos;
        while (pos < content.size() && content[pos] != '"')
        {
            if (content[pos] == '\\' && pos + 1 < content.size())
            {
                char next = content[pos + 1];
                result += (next == 'n') ? ' ' : next;
                pos += 2;
                continue;
            }
            result += content[pos];
            ++pos;
        }
        ++pos; // closing quote
    }
    endOut = pos;
    return result;
}

std::vector<HandwrittenQuery> ScanHandwrittenQueries(std::filesystem::path const& root)
{
    std::vector<HandwrittenQuery> queries;
    if (!std::filesystem::exists(root))
        return queries;

    static std::regex const selectFrom(R"(SELECT\s+(.*?)\s+FROM\s+`?([A-Za-z_][A-Za-z0-9_]*)`?)");

    for (std::filesystem::directory_entry const& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".cpp")
            continue;

        std::ifstream in(entry.path());
        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string content = buffer.str();

        size_t searchFrom = 0;
        while (true)
        {
            size_t literalStart = content.find("\"SELECT ", searchFrom);
            if (literalStart == std::string::npos)
                break;

            size_t literalEnd = 0;
            std::string statement = ConcatenateAdjacentLiterals(content, literalStart, literalEnd);
            searchFrom = literalEnd;

            std::smatch m;
            if (!std::regex_search(statement, m, selectFrom))
                continue;

            HandwrittenQuery query;
            query.table = m[2].str();
            query.file = entry.path();
            query.line = static_cast<uint32_t>(std::count(content.begin(), content.begin() + static_cast<long>(literalStart), '\n') + 1);

            std::string columnList = m[1].str();
            bool hasJoin = statement.find("JOIN") != std::string::npos;
            bool hasStar = columnList.find('*') != std::string::npos;

            std::vector<std::string> columns;
            if (!hasJoin && !hasStar && ParseColumnList(columnList, columns))
            {
                query.parsable = true;
                query.columns = columns;
            }

            queries.push_back(query);
        }
    }
    return queries;
}

std::vector<uint32_t> FindFieldsAccessLines(std::string const& text, HandwrittenQuery const& query, uint32_t searchWindowLines)
{
    std::vector<uint32_t> lines(query.columns.size(), 0);
    if (!query.parsable)
        return lines;

    std::vector<std::string> allLines = SplitLines(text);
    if (query.line == 0 || query.line > allLines.size())
        return lines;

    uint32_t windowEnd = static_cast<uint32_t>(std::min<size_t>(allLines.size(), query.line + searchWindowLines));
    for (size_t n = 0; n < query.columns.size(); ++n)
    {
        std::regex const fieldsAccess(R"(\bfields\s*\[\s*)" + std::to_string(n) + R"(\s*\])");
        for (uint32_t lineNo = query.line; lineNo <= windowEnd && lineNo <= allLines.size(); ++lineNo)
        {
            if (std::regex_search(allLines[lineNo - 1], fieldsAccess))
            {
                lines[n] = lineNo;
                break;
            }
        }
    }
    return lines;
}
