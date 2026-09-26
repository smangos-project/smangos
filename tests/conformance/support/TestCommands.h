#ifndef REALBOTS_TESTCOMMANDS_H
#define REALBOTS_TESTCOMMANDS_H

#include "Packets/ChatMessages.h"
#include "TestBot.h"
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

// Types a chat command (".learn 6673") as the bot's character.
inline void SayCommand(TestBot const& bot, std::string const& text)
{
    ChatMessageSay say;
    say.Language = 7; // LANG_COMMON
    say.Message = text;
    bot.Send(say);
}

// The guids of every unit the bot knows about.
inline std::vector<ClassicGuid> KnownUnits(TestBot const& bot)
{
    std::vector<ClassicGuid> units;
    for (BotKnownObject const& object : bot.GetKnownObjects())
        if (object.ObjectType == BOT_TYPEID_UNIT)
            units.push_back(object.Guid);
    return units;
}

// A unit the bot knows now that it didn't in `before`. Spawning beside a bot doesn't make the new
// creature the only unit around (Northshire Abbey has its own wildlife), so it is found by diffing.
inline std::optional<ClassicGuid> FindNewUnit(TestBot const& bot, std::vector<ClassicGuid> const& before)
{
    for (ClassicGuid const& guid : KnownUnits(bot))
        if (std::find(before.begin(), before.end(), guid) == before.end())
            return guid;
    return std::nullopt;
}

#endif
