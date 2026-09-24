// The .realbot chat command's handler (Chat.h declares it, Chat.cpp's command table points at
// it, both guarded by #ifdef REALBOTS -- bot-phase-1.md, "Hosts"). Kept out of RealBotCommands.cpp
// itself since that file also compiles into contrib/realbotrunner and tests-realbots, neither of
// which link Chat.h's WorldSession/Player dependencies (bot-plan.md: "Bots touch no server runtime
// state" -- this file is the one place that's allowed to, since it's only ever built into `game`).
#include "Chat.h"
#include "RealBotCommands.h"

bool ChatHandler::HandleRealBotCommand(char* args)
{
    if (!args || !*args)
    {
        SendSysMessage("usage: .realbot <verb> [args] -- try '.realbot help'");
        return true;
    }

    RealBotCommands::Handle(args, [this](std::string const& line) { SendSysMessage(line.c_str()); });
    return true;
}
