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
    // Conformance-suite knobs (bot-phase-2.md, "Targets"), read only by tests/conformance: how long
    // the server may take to close a connection it has decided to reject (also the bound for a
    // vanished client to be able to log in again), how long an in-world bot idling on default
    // keepalives must stay connected, and the most bots a [load] scenario may use (0: as many
    // accounts as the pool has).
    uint32_t ServerCloseTimeoutMs = 30000;
    uint32_t IdleSessionMs = 120000;
    uint32_t LoadBots = 0;
    // The server's own PlayerLimit, when forced low for the session-queue scenario; 0 skips it.
    uint32_t PlayerLimit = 0;

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
