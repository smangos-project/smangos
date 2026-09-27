#include "DbdFile.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <sstream>

namespace
{
    std::string Trim(std::string_view text)
    {
        size_t begin = 0;
        while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin])))
            ++begin;

        size_t end = text.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
            --end;

        return std::string(text.substr(begin, end - begin));
    }

    bool IsBlank(std::string const& line)
    {
        return std::all_of(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); });
    }

    std::vector<std::string> SplitLines(std::string text)
    {
        if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
            static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF)
            text.erase(0, 3);

        std::vector<std::string> lines;
        std::string current;
        for (size_t i = 0; i < text.size(); ++i)
        {
            char c = text[i];
            if (c == '\r')
            {
                if (i + 1 < text.size() && text[i + 1] == '\n')
                    ++i;
                lines.push_back(current);
                current.clear();
            }
            else if (c == '\n')
            {
                lines.push_back(current);
                current.clear();
            }
            else
            {
                current += c;
            }
        }
        lines.push_back(current);
        return lines;
    }

    std::vector<std::string> SplitOnCommaSpace(std::string_view text)
    {
        std::vector<std::string> parts;
        size_t start = 0;
        while (true)
        {
            size_t sep = text.find(", ", start);
            if (sep == std::string_view::npos)
            {
                parts.emplace_back(text.substr(start));
                break;
            }
            parts.emplace_back(text.substr(start, sep - start));
            start = sep + 2;
        }
        return parts;
    }

    bool ParseInt(std::string_view text, int& out)
    {
        int value = 0;
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc() || result.ptr != text.data() + text.size())
            return false;
        out = value;
        return true;
    }

    // Parses one COLUMNS block line into `columns`. Throws DbdParseError on malformed input;
    // a duplicate column name is only ever emitted by malformed files upstream tolerates, so it
    // is kept (first definition wins), matching WoWDBDefs' own reader.
    void ParseColumnLine(std::string const& fileName, int lineNumber, std::string const& line,
                          std::unordered_map<std::string, DbdColumn>& columns)
    {
        size_t typeEnd = line.find_first_of(" <");
        if (typeEnd == std::string::npos)
            throw DbdParseError(fileName, lineNumber, "column line has no space between type and name: " + line);

        std::string type = line.substr(0, typeEnd);
        static std::string const validTypes[] = { "int", "float", "string", "locstring" };
        if (std::find(std::begin(validTypes), std::end(validTypes), type) == std::end(validTypes))
            throw DbdParseError(fileName, lineNumber, "invalid column type: " + type);

        DbdColumn column;
        column.type = type;

        if (line.compare(0, type.size() + 1, type + "<") == 0)
        {
            size_t open = line.find('<');
            size_t close = line.find('>', open);
            if (close == std::string::npos)
                throw DbdParseError(fileName, lineNumber, "unterminated foreign key: " + line);

            std::string foreignKey = line.substr(open + 1, close - open - 1);
            size_t sep = foreignKey.find("::");
            if (sep == std::string::npos)
                throw DbdParseError(fileName, lineNumber, "invalid foreign key: " + foreignKey);

            column.foreignTable = foreignKey.substr(0, sep);
            column.foreignColumn = foreignKey.substr(sep + 2);
        }

        size_t firstSpace = line.find(' ');
        size_t lastSpace = line.rfind(' ');
        std::string name;
        if (firstSpace == lastSpace)
        {
            name = line.substr(firstSpace + 1);
        }
        else
        {
            size_t nextSpace = line.find(' ', firstSpace + 1);
            name = line.substr(firstSpace + 1, nextSpace - firstSpace - 1);
        }

        if (!name.empty() && name.back() == '?')
        {
            column.verified = false;
            name.pop_back();
        }

        size_t commentAt = line.find("//");
        if (commentAt != std::string::npos)
            column.comment = Trim(line.substr(commentAt + 2));

        columns.try_emplace(name, std::move(column));
    }

    // Parses one field line of a version block. Throws DbdParseError on malformed input or an
    // unknown column name.
    DbdFieldDefinition ParseFieldLine(std::string const& fileName, int lineNumber, std::string line,
                                       std::unordered_map<std::string, DbdColumn> const& columns)
    {
        DbdFieldDefinition field;

        if (!line.empty() && line.front() == '$')
        {
            size_t close = line.find('$', 1);
            if (close == std::string::npos)
                throw DbdParseError(fileName, lineNumber, "unterminated annotation: " + line);

            std::string annotationText = line.substr(1, close - 1);
            size_t start = 0;
            while (start <= annotationText.size())
            {
                size_t comma = annotationText.find(',', start);
                std::string annotation = annotationText.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
                if (annotation == "id") field.isId = true;
                else if (annotation == "relation") field.isRelation = true;
                else if (annotation == "noninline") field.isNonInline = true;

                if (comma == std::string::npos)
                    break;
                start = comma + 1;
            }

            line.erase(0, close + 1);
        }

        size_t open = line.find('<');
        if (open != std::string::npos)
        {
            size_t close = line.find('>', open);
            if (close == std::string::npos)
                throw DbdParseError(fileName, lineNumber, "unterminated size: " + line);

            std::string sizeText = line.substr(open + 1, close - open - 1);
            if (!sizeText.empty() && sizeText.front() == 'u')
            {
                field.isUnsigned = true;
                sizeText.erase(0, 1);
            }

            if (!ParseInt(sizeText, field.bits))
                throw DbdParseError(fileName, lineNumber, "invalid field size: " + line);

            line.erase(open, close - open + 1);
        }

        size_t bracketOpen = line.find('[');
        if (bracketOpen != std::string::npos)
        {
            size_t bracketClose = line.find(']', bracketOpen);
            if (bracketClose == std::string::npos)
                throw DbdParseError(fileName, lineNumber, "unterminated array length: " + line);

            std::string lengthText = line.substr(bracketOpen + 1, bracketClose - bracketOpen - 1);
            if (!ParseInt(lengthText, field.arrayLength))
                throw DbdParseError(fileName, lineNumber, "invalid array length: " + line);

            line.erase(bracketOpen, bracketClose - bracketOpen + 1);
        }

        size_t commentAt = line.find("//");
        if (commentAt != std::string::npos)
        {
            field.comment = Trim(line.substr(commentAt + 2));
            line = line.substr(0, commentAt);
        }

        field.name = Trim(line);

        if (!columns.contains(field.name))
            throw DbdParseError(fileName, lineNumber, "unknown column in version definition: " + field.name);

        return field;
    }
}

DbdParseError::DbdParseError(std::string file_, int line_, std::string const& message)
    : std::runtime_error(file_ + ":" + std::to_string(line_) + ": " + message)
    , file(std::move(file_))
    , line(line_)
{
}

DbdFile ParseDbdText(std::string const& text, std::string const& fileName)
{
    std::vector<std::string> lines = SplitLines(text);

    DbdFile file;
    size_t lineIndex = 0;

    if (lines.empty() || lines[0].compare(0, 7, "COLUMNS") != 0)
        throw DbdParseError(fileName, 1, "file does not start with column definitions");

    lineIndex = 1;
    while (lineIndex < lines.size())
    {
        std::string const& line = lines[lineIndex++];
        if (IsBlank(line))
            break;

        ParseColumnLine(fileName, static_cast<int>(lineIndex), line, file.columns);
    }

    DbdVersionBlock current;
    bool hasPendingContent = false;

    auto flushIfPending = [&](int lineNumberForError)
    {
        if (!current.builds.empty() || !current.buildRanges.empty() || !current.layoutHashes.empty())
        {
            file.versions.push_back(std::move(current));
        }
        else if (hasPendingContent)
        {
            throw DbdParseError(fileName, lineNumberForError, "no BUILD or LAYOUT, but non-empty definitions");
        }

        current = DbdVersionBlock();
        hasPendingContent = false;
    };

    for (size_t i = lineIndex; i < lines.size(); ++i)
    {
        std::string const& line = lines[i];
        int lineNumber = static_cast<int>(i + 1);
        bool isLast = (i + 1 == lines.size());

        if (IsBlank(line))
        {
            flushIfPending(lineNumber);
        }
        else if (line.compare(0, 7, "LAYOUT ") == 0)
        {
            for (auto& hash : SplitOnCommaSpace(line.substr(7)))
                current.layoutHashes.push_back(hash);
            hasPendingContent = true;
        }
        else if (line.compare(0, 6, "BUILD ") == 0)
        {
            for (auto const& part : SplitOnCommaSpace(line.substr(6)))
            {
                if (part.find('-') != std::string::npos)
                {
                    DbdBuildRange range;
                    if (!DbdBuildRange::TryParse(part, range))
                        throw DbdParseError(fileName, lineNumber, "invalid build range: " + part);
                    current.buildRanges.push_back(range);
                }
                else
                {
                    DbdBuild build;
                    if (!DbdBuild::TryParse(part, build))
                        throw DbdParseError(fileName, lineNumber, "invalid build: " + part);
                    current.builds.push_back(build);
                }
            }
            hasPendingContent = true;
        }
        else if (line.compare(0, 7, "COMMENT") == 0)
        {
            current.comment = Trim(line.substr(7));
            hasPendingContent = true;
        }
        else
        {
            current.fields.push_back(ParseFieldLine(fileName, lineNumber, line, file.columns));
            hasPendingContent = true;
        }

        if (isLast)
            flushIfPending(lineNumber);
    }

    return file;
}

DbdFile ParseDbdFile(std::string const& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
        throw DbdParseError(path, 0, "unable to open file");

    std::ostringstream buffer;
    buffer << stream.rdbuf();

    return ParseDbdText(buffer.str(), path);
}
