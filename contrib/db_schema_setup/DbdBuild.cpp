#include "DbdBuild.h"

#include <array>
#include <charconv>
#include <stdexcept>

namespace
{
    bool ParseComponent(std::string_view text, int32_t& out)
    {
        int value = 0;
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc() || result.ptr != text.data() + text.size())
            return false;
        out = value;
        return true;
    }

    bool ParseComponent(std::string_view text, uint32_t& out)
    {
        unsigned value = 0;
        auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc() || result.ptr != text.data() + text.size())
            return false;
        out = value;
        return true;
    }
}

std::string DbdBuild::ToString() const
{
    return std::to_string(expansion) + "." + std::to_string(major) + "." +
           std::to_string(minor) + "." + std::to_string(build);
}

bool DbdBuild::TryParse(std::string_view text, DbdBuild& out)
{
    std::array<std::string_view, 4> parts;
    size_t partCount = 0;
    size_t start = 0;

    for (size_t i = 0; i <= text.size(); ++i)
    {
        if (i == text.size() || text[i] == '.')
        {
            if (partCount >= parts.size())
                return false;
            parts[partCount++] = text.substr(start, i - start);
            start = i + 1;
        }
    }

    if (partCount != 4)
        return false;

    DbdBuild parsed;
    if (!ParseComponent(parts[0], parsed.expansion)) return false;
    if (!ParseComponent(parts[1], parsed.major)) return false;
    if (!ParseComponent(parts[2], parsed.minor)) return false;
    if (!ParseComponent(parts[3], parsed.build)) return false;

    out = parsed;
    return true;
}

DbdBuild DbdBuild::Parse(std::string_view text)
{
    DbdBuild build;
    if (!TryParse(text, build))
        throw std::invalid_argument("Invalid build string: " + std::string(text));
    return build;
}

bool DbdBuildRange::Contains(DbdBuild const& build) const
{
    return build.expansion >= min.expansion && build.expansion <= max.expansion &&
           build.major >= min.major && build.major <= max.major &&
           build.build >= min.build && build.build <= max.build;
}

bool DbdBuildRange::TryParse(std::string_view text, DbdBuildRange& out)
{
    auto dash = text.find('-');
    if (dash == std::string_view::npos)
        return false;

    DbdBuildRange range;
    if (!DbdBuild::TryParse(text.substr(0, dash), range.min))
        return false;
    if (!DbdBuild::TryParse(text.substr(dash + 1), range.max))
        return false;

    out = range;
    return true;
}

DbdBuildRange DbdBuildRange::Parse(std::string_view text)
{
    DbdBuildRange range;
    if (!TryParse(text, range))
        throw std::invalid_argument("Invalid build range string: " + std::string(text));
    return range;
}
