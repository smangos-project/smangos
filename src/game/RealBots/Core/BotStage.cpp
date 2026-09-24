#include "BotStage.h"

char const* BotStageName(BotStage stage)
{
    switch (stage)
    {
        case BotStage::Disconnected: return "Disconnected";
        case BotStage::LoginPortal: return "LoginPortal";
        case BotStage::LoginRpc: return "LoginRpc";
        case BotStage::RealmSelect: return "RealmSelect";
        case BotStage::WorldConnecting: return "WorldConnecting";
        case BotStage::WorldHandshake: return "WorldHandshake";
        case BotStage::CharSelect: return "CharSelect";
        case BotStage::EnteringWorld: return "EnteringWorld";
        case BotStage::InWorld: return "InWorld";
        case BotStage::LoggingOut: return "LoggingOut";
        case BotStage::Failed: return "Failed";
        default: return "Unknown";
    }
}
