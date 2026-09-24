#ifndef REALBOTS_BOTACCOUNTLEASE_H
#define REALBOTS_BOTACCOUNTLEASE_H

#include "RealBotTarget.h"
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// Leases one of a target's configured accounts to a bot at a time, so two bots never race to log
// into the same account (bot-phase-1.md, "Accounts are existing Battle.net logins, leased one per
// bot"). Thread-safe: RealBotMgr::Add/Remove may run from the console/chat thread while the
// worker thread ticks bots concurrently.
class BotAccountLease
{
    public:
        explicit BotAccountLease(std::vector<RealBotAccountCredential> accounts = {});

        // The first free account, or nullopt if every configured account is already leased.
        std::optional<RealBotAccountCredential> Lease();
        void Release(std::string const& accountName);

        size_t Free() const;
        size_t Total() const;

    private:
        mutable std::mutex m_lock;
        std::vector<RealBotAccountCredential> m_accounts;
        std::vector<bool> m_leased;
};

#endif
