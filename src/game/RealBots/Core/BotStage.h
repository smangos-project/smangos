#ifndef REALBOTS_BOTSTAGE_H
#define REALBOTS_BOTSTAGE_H

// A bot's position in the login/world-entry sequence (bot-phase-1.md, "Login").
enum class BotStage
{
    Disconnected,
    LoginPortal,
    LoginRpc,
    RealmSelect,
    WorldConnecting,
    WorldHandshake,
    CharSelect,
    EnteringWorld,
    InWorld,
    LoggingOut,
    Failed,
};

// The outcome of a BotTask::Update or a finished BotRoutine.
enum class BotTaskResult
{
    Running,
    Done,
    Failed,
};

// For logs, "info"/"list"/"status" command output -- never parsed back.
char const* BotStageName(BotStage stage);

#endif
