#ifndef REALBOTS_BUILTINBOTTASKS_H
#define REALBOTS_BUILTINBOTTASKS_H

// Registers every built-in BotRoutine task (bot-phase-1.md, "Tasks/BuiltinBotTasks.h/.cpp"):
// "login" (a new bot's default bottom task -- drives BotLogin::Login through to InWorld, then
// holds), "idle" (holds and does nothing else), "disconnect_after" (delays, then disconnects).
// Call once, before any bot is added (RealBotMgr::Start).
void RegisterBuiltinBotTasks();

#endif
