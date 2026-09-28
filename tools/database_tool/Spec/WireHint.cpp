#include "WireHint.h"

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

bool IsPacketCodeFile(std::filesystem::path const& file, std::string const& text)
{
    std::string const name = file.filename().string();
    if (name == "QueryHandler.cpp" || name == "GossipDef.cpp" || name == "UpdateFieldFill.cpp")
        return true;

    static std::string const handlerSuffix = "Handler.cpp";
    if (name.size() >= handlerSuffix.size() && name.compare(name.size() - handlerSuffix.size(), handlerSuffix.size(), handlerSuffix) == 0)
        return true;

    return text.find("WorldPacket") != std::string::npos && text.find("<<") != std::string::npos;
}

std::vector<WireHintUse> FindFieldUseLines(std::filesystem::path const& file, std::string const& text, std::string const& fieldName)
{
    std::vector<WireHintUse> uses;
    std::regex const pattern(R"([.>]\s*)" + fieldName + R"(\b)");
    std::vector<std::string> lines = SplitLines(text);
    for (size_t i = 0; i < lines.size(); ++i)
        if (std::regex_search(lines[i], pattern))
            uses.push_back({ file, static_cast<uint32_t>(i + 1) });
    return uses;
}
