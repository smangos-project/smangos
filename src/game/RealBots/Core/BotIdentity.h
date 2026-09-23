#ifndef REALBOTS_BOTIDENTITY_H
#define REALBOTS_BOTIDENTITY_H

#include "ClassicPackets.h"
#include "Common.h"
#include <string>

// A bot's login credentials and, once known, its chosen character. Filled in by the
// account lease (port-plan.md, "Accounts") and by CharSelect; empty fields simply
// aren't known yet.
struct BotIdentity
{
    std::string AccountName;
    std::string Password;
    std::string CharacterName;
    ClassicGuid CharacterGuid;
};

#endif
