#ifndef MANGOS_SUMMON_KIND_H
#define MANGOS_SUMMON_KIND_H

#include "Common.h"

struct SummonPropertiesEntry
{
    uint32 ID;
    uint32 Control;
    uint32 Faction;
    uint32 Title;
    uint32 Slot;
    uint32 Flags;
};

enum SummonKind : uint8
{
    SUMMON_KIND_NONE = 0,
    SUMMON_KIND_PET,
    SUMMON_KIND_WILD,
    SUMMON_KIND_GUARDIAN,
    SUMMON_KIND_POSSESSED,
    SUMMON_KIND_TOTEM,
    SUMMON_KIND_TOTEM_SLOT1,
    SUMMON_KIND_TOTEM_SLOT2,
    SUMMON_KIND_TOTEM_SLOT3,
    SUMMON_KIND_TOTEM_SLOT4,
    SUMMON_KIND_CRITTER,
    SUMMON_KIND_DEMON,
    SUMMON_KIND_TRANS_DOOR
};

// Classifies a SummonProperties row by id, falling back on the row's fields for ids not listed.
SummonKind GetSummonKind(uint32 propertiesId, SummonPropertiesEntry const* entry);

#endif
