#ifndef REALBOTS_BOTTASKARGS_H
#define REALBOTS_BOTTASKARGS_H

#include "Common.h"
#include <string>
#include <unordered_map>

// A task's arguments, e.g. "interval=1000,count=5" from a .realbot/runner command
// or a target-config task list.
class BotTaskArgs
{
    public:
        BotTaskArgs() = default;

        static BotTaskArgs Parse(std::string const& text);

        void Set(std::string key, std::string value);

        std::string GetString(std::string const& key, std::string const& def = "") const;
        uint32 GetUInt32(std::string const& key, uint32 def = 0) const;
        float GetFloat(std::string const& key, float def = 0.0f) const;
        bool GetBool(std::string const& key, bool def = false) const;

    private:
        std::unordered_map<std::string, std::string> m_values;
};

#endif
