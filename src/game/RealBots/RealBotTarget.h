#ifndef REALBOTS_REALBOTTARGET_H
#define REALBOTS_REALBOTTARGET_H

#include "Common.h"
#include <string>
#include <vector>

// One existing Battle.net login a target lists under RealBots.Target.<name>.Accounts, leased one
// per bot by BotAccountLease.
struct RealBotAccountCredential
{
    std::string Name;
    std::string Password;
};

// A named server profile a bot can log into (bot-phase-1.md, "Targets"): addresses, realm, client
// build, tags and a pool of existing accounts. Adding a target needs no code, only a
// RealBots.Target.<name>.* config block plus its name added to RealBots.Targets.
struct RealBotTarget
{
    std::string Name;
    std::string LoginAddress;
    uint16_t RestPort = 8081;
    uint16_t RpcPort = 1119;
    std::string RealmName;    // wins over RealmId when set
    uint32_t RealmId = 1;
    int32_t ClientBuild = 0;
    std::string ClientVersion;
    std::vector<std::string> Tags;
    std::vector<RealBotAccountCredential> Accounts;
    bool VerifyServerSignature = true;
    uint32_t ConnectTimeoutMs = 10000;
    uint32_t StageTimeoutMs = 30000;

    bool HasTag(std::string const& tag) const;

    // Reads RealBots.Target.<name>.* from sConfig. False (with *error* set) if LoginAddress, the
    // one required key, is missing.
    static bool Load(std::string const& name, RealBotTarget& out, std::string& error);
};

// Every target named by RealBots.Targets, loaded once (RealBotMgr::Start).
class RealBotTargetList
{
    public:
        void Load();

        RealBotTarget const* Find(std::string const& name) const;
        std::vector<RealBotTarget> const& All() const { return m_targets; }

    private:
        std::vector<RealBotTarget> m_targets;
};

#endif
