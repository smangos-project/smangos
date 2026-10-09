#ifndef MANGOS_PERSISTENCE_CHARTABLES_H
#define MANGOS_PERSISTENCE_CHARTABLES_H

#include "Persistence/TableRegistry.h"

namespace Persistence
{
    // Character store tables the epoch capture writes, ids index CharTableRegistry()
    enum CharTable : uint16
    {
        Characters, Customizations, BattlegroundData, Inventory, ItemInstance, QuestStatus, Spell, SpellCooldown, Aura,
        Skills, Reputation, HonorCp, Action, Tutorial, Social, Mail, MailItems, Pet, PetSpell, PetAura, PetSpellCooldown,
        Homebind, Instance,
        CharTableCount
    };

    TableRegistry const& CharTableRegistry();
}

#endif
