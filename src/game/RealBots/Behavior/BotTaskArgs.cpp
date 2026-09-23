#include "BotTaskArgs.h"
#include <cstdlib>
#include <sstream>

BotTaskArgs BotTaskArgs::Parse(std::string const& text)
{
    BotTaskArgs args;

    std::stringstream ss(text);
    std::string pair;
    while (std::getline(ss, pair, ','))
    {
        size_t eq = pair.find('=');
        if (eq == std::string::npos)
            continue;

        args.Set(pair.substr(0, eq), pair.substr(eq + 1));
    }

    return args;
}

void BotTaskArgs::Set(std::string key, std::string value)
{
    m_values[std::move(key)] = std::move(value);
}

std::string BotTaskArgs::GetString(std::string const& key, std::string const& def) const
{
    auto it = m_values.find(key);
    return it != m_values.end() ? it->second : def;
}

uint32 BotTaskArgs::GetUInt32(std::string const& key, uint32 def) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
        return def;

    return static_cast<uint32>(std::strtoul(it->second.c_str(), nullptr, 10));
}

bool BotTaskArgs::GetBool(std::string const& key, bool def) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
        return def;

    return it->second == "1" || it->second == "true";
}
