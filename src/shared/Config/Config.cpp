/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Config.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_2(Config, Config::Lock);
INSTANTIATE_CLASS_MUTEX(Config, std::shared_timed_mutex);

static bool IsLineEndChar(char chr)
{
    switch (chr)
    {
        case '\0':
        case '\n':
        case '\r':
            return true;
    }

    return false;
}

static std::string JoinFiles(std::vector<std::string> const& files)
{
    std::string joined;
    for (std::string const& file : files)
    {
        if (!joined.empty())
            joined += ", ";
        joined += file;
    }
    return joined;
}

bool Config::LoadFromFile(std::string const& filename)
{
    std::string error;
    if (LoadLayers({ filename }, error))
        return true;

    printf("%s\n", error.c_str());
    return false;
}

bool Config::LoadLayers(std::vector<std::string> const& files, std::string& error)
{
    ConfigMap merged;
    std::unordered_map<std::string, size_t> origin;

    for (size_t i = 0; i < files.size(); ++i)
    {
        FILE* pFile = fopen(files[i].c_str(), "r");
        if (!pFile)
        {
            error = "cannot open " + files[i];
            return false;
        }

        ConfigMap layer;
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pFile))
            ProcessLine(buffer, layer);
        fclose(pFile);

        for (auto& [name, value] : layer)
        {
            auto const previous = origin.find(name);
            if (previous != origin.end())
            {
                error = "'" + name + "' is set in both " + files[previous->second] + " and " + files[i];
                return false;
            }

            origin.emplace(name, i);
            merged.emplace(name, std::move(value));
        }
    }

    if (merged.empty())
    {
        error = "no settings found in " + JoinFiles(files);
        return false;
    }

    std::lock_guard<std::shared_timed_mutex> guard(m_configLock);
    m_configMap = std::move(merged);
    m_files = files;
    return true;
}

bool Config::Reload()
{
    std::vector<std::string> files;
    {
        std::shared_lock<std::shared_timed_mutex> guard(m_configLock);
        files = m_files;
    }

    std::string error;
    if (LoadLayers(files, error))
        return true;

    printf("%s\n", error.c_str());
    return false;
}

enum LineReadStage
{
    STAGE_FIND_NAME,
    STAGE_READ_NAME,
    STAGE_FIND_VALUE,
    STAGE_READ_VALUE
};

bool Config::ProcessLine(char const* line, ConfigMap& map)
{
    LineReadStage stage = STAGE_FIND_NAME;
    std::string name;
    std::string value;

    int i = 0;
    while (!IsLineEndChar(line[i]))
    {
        bool stop = false;
        bool quoteFound = false;

        switch (stage)
        {
            case STAGE_FIND_NAME:
            case STAGE_READ_NAME:
            {
                switch (line[i])
                {
                    case '#': // comment at unexpected place
                    case '[': // section
                        return false;
                    case ' ': // skip white space
                    case '\t':
                        break;
                    case '=': // name has been read
                        if (stage == STAGE_FIND_NAME)
                            return false;
                        if (name.empty())
                            return false;
                        stage = STAGE_FIND_VALUE;
                        break;
                    default:
                        name += line[i];
                        stage = STAGE_READ_NAME;
                        break;
                }
                break;
            }
            case STAGE_FIND_VALUE:
            case STAGE_READ_VALUE:
            {
                switch (line[i])
                {
                    case '#': // comment can only be at end of line, stop reading
                        if (!quoteFound)
                            stop = true;
                        break;
                    case '"': // handle quoted text
                        if (quoteFound)
                            stop = true;
                        else
                        {
                            quoteFound = true;
                            stage = STAGE_READ_VALUE;
                        }
                        break;
                    case ' ': // ignore white space until text found
                    case '\t':
                        if (stage == STAGE_FIND_VALUE)
                            break;
                    default:
                        value += line[i];
                        stage = STAGE_READ_VALUE;
                        break;
                }
                break;
            }
        }

        if (stop)
            break;

        ++i;
    }

    if (name.empty() || value.empty())
        return false;

    if (!map.insert({ name, value }).second)
    {
        printf("Config setting '%s' appear twice in config! Ignoring second occurrence.\n", name.c_str());
        return false;
    }

    return true;
}

std::string Config::GetFilename() const
{
    std::shared_lock<std::shared_timed_mutex> guard(m_configLock);
    return JoinFiles(m_files);
}

bool Config::GetValueHelper(char const* name, std::string &result) const
{
    std::shared_lock<std::shared_timed_mutex> guard(m_configLock);

    auto itr = m_configMap.find(name);
    if (itr == m_configMap.end())
        return false;

    result = itr->second;
    return true;
}

bool Config::IsSet(char const* name) const
{
    std::string val; // we are not interested in the value
    return GetValueHelper(name, val);
}

std::string Config::GetStringDefault(char const* name, char const* def) const
{
    std::string val;
    return GetValueHelper(name, val) ? val.c_str() : def;
}

bool Config::GetBoolDefault(char const* name, bool def) const
{
    std::string val;
    if (!GetValueHelper(name, val))
        return def;

    return val == "true"
        || val == "TRUE"
        || val == "yes"
        || val == "YES"
        || val == "1";
}

int32 Config::GetIntDefault(char const* name, int32 def) const
{
    std::string val;
    return GetValueHelper(name, val) ? std::stoi(val) : def;
}

float Config::GetFloatDefault(char const* name, float def) const
{
    std::string val;
    return GetValueHelper(name, val) ? std::stof(val) : def;
}
