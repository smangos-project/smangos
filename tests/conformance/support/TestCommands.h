#ifndef REALBOTS_TESTCOMMANDS_H
#define REALBOTS_TESTCOMMANDS_H

#include "Packets/ChatMessages.h"
#include "TestBot.h"
#include <string>

// Types a chat command (".learn 6673") as the bot's character.
inline void SayCommand(TestBot const& bot, std::string const& text)
{
    ChatMessageSay say;
    say.Language = 7; // LANG_COMMON
    say.Message = text;
    bot.Send(say);
}

#endif
