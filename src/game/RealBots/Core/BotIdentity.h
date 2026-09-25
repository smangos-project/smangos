#ifndef REALBOTS_BOTIDENTITY_H
#define REALBOTS_BOTIDENTITY_H

#include "Packets/Classic/Guid.h"
#include "Common.h"
#include <cstdint>
#include <string>
#include <vector>

// What the Battle.net half of the login handed back, kept for whoever wants to see it: a conformance
// scenario driving the world handshake by hand needs the join key and ticket, and one checking the
// realm join needs to know it returned something real.
struct BotSessionInfo
{
    std::string LoginTicket;          // the portal's login ticket (before any override the RPC step was given)
    std::string WorldAddress;
    uint16_t WorldPort = 0;
    std::vector<uint8_t> JoinSessionKey;
    std::string RealmJoinTicket;
    uint32_t RealmId = 0;             // the realm id the bot was configured to join (0 = by name)
};

// A bot's login credentials and, once known, its chosen character. Filled in by the
// account lease (port-plan.md, "Accounts") and by CharSelect; empty fields simply
// aren't known yet.
struct BotIdentity
{
    std::string AccountName;
    std::string Password;
    std::string CharacterName;
    ClassicGuid CharacterGuid;
    BotSessionInfo Session;
};

#endif
