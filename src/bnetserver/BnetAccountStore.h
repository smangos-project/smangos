#ifndef BNETSERVER_BNETACCOUNTSTORE_H
#define BNETSERVER_BNETACCOUNTSTORE_H

#include "Common.h"
#include "nonstd/optional.hpp"

#include <map>
#include <string>
#include <vector>

// What a Battle.net login resolves an account to: its SRP material, before the password is proven.
struct BnetSrpCredentials
{
    uint32 bnetAccountId = 0;
    uint8 srpVersion = 1;
    std::vector<uint8> salt;
    std::vector<uint8> verifier;
};

// The one game account behind a Battle.net account (`account` row) and what the realm join needs
// to know about it.
struct BnetGameAccount
{
    uint32 id = 0;
    std::string name;
    uint8 expansion = 0;
    bool banned = false;
    bool permanentlyBanned = false;
    uint32 unbanDate = 0;
    std::string banReason;
    AccountTypes defaultSecurity = SEC_PLAYER;
    std::map<uint32, AccountTypes> securityByRealm;

    AccountTypes GetSecurityOn(uint32 realmId) const;
};

struct BnetAccount
{
    uint32 id = 0;
    std::string login;
    bool banned = false;
    bool permanentlyBanned = false;
    uint32 unbanDate = 0;
    std::vector<BnetGameAccount> gameAccounts;

    BnetGameAccount const* FindGameAccount(uint32 gameAccountId) const;
};

// Database access for the login server. Everything here is synchronous and runs on the IO thread,
// like the rest of the login path's queries.
class BnetAccountStore
{
    public:
        // Upper-cases (Latin letters only) `login` the way the game account name is stored. False if
        // it is not valid UTF-8.
        static bool NormalizeLogin(std::string& login);

        static nonstd::optional<BnetSrpCredentials> LoadSrpCredentials(std::string const& normalizedLogin);
        static nonstd::optional<BnetAccount> LoadAccount(uint32 bnetAccountId);

        static bool IsIpBanned(std::string const& ip);

        // Character counts per realm id for one game account.
        static std::map<uint32, uint32> LoadCharacterCounts(uint32 gameAccountId);

        struct LastPlayedCharacter
        {
            uint32 realmId = 0;
            std::string name;
            uint64 guid = 0;
            uint32 lastPlayedTime = 0;
        };
        static nonstd::optional<LastPlayedCharacter> LoadLastPlayedCharacter(uint32 gameAccountId, uint8 region, uint8 battlegroup);

        // Stores the world session key (hex) and the client details the world socket checks against.
        static bool StoreJoinInfo(uint32 gameAccountId, std::string const& sessionKeyHex, std::string const& ip,
            uint8 locale, std::string const& os);
        static void StoreLogin(uint32 bnetAccountId, std::string const& ip);
};

#endif
