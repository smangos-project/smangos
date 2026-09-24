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

// Why a bot last reached BotStage::Disconnected (bot-examples.md #7's
// `bot.GetDisconnectReason() == BotDisconnectReason::Requested`). Set once, when SetStage
// actually transitions to Disconnected; overwritten on every later disconnect.
enum class BotDisconnectReason
{
    None,          // never disconnected (still connecting, connected, or brand new)
    Requested,     // RealBot::Disconnect() was called first, clean or not
    ConnectionLost, // the connection's own onDisconnect fired without Disconnect() being called
};

// For logs, "info"/"list"/"status" command output -- never parsed back.
char const* BotStageName(BotStage stage);
char const* BotDisconnectReasonName(BotDisconnectReason reason);

#endif
