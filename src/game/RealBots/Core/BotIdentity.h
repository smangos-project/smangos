#ifndef REALBOTS_BOTIDENTITY_H
#define REALBOTS_BOTIDENTITY_H

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
    uint64 CharacterGuid = 0;
};

#endif
