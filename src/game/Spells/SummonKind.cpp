#include "SummonKind.h"

SummonKind GetSummonKind(uint32 propertiesId, SummonPropertiesEntry const* entry)
{
    switch (propertiesId)
    {
        case 67:   return SUMMON_KIND_PET;
        case 64:   return SUMMON_KIND_WILD;
        case 61:
        case 5121: return SUMMON_KIND_GUARDIAN;
        case 65:   return SUMMON_KIND_POSSESSED;
        case 121:
        case 5100: return SUMMON_KIND_TOTEM;
        case 63:   return SUMMON_KIND_TOTEM_SLOT1;
        case 81:   return SUMMON_KIND_TOTEM_SLOT2;
        case 82:   return SUMMON_KIND_TOTEM_SLOT3;
        case 83:   return SUMMON_KIND_TOTEM_SLOT4;
        case 41:   return SUMMON_KIND_CRITTER;
        case 66:   return SUMMON_KIND_DEMON;
        case 5878: return SUMMON_KIND_TRANS_DOOR;
    }

    if (!entry)
        return SUMMON_KIND_WILD;

    if (entry->Slot >= 1 && entry->Slot <= 4)
        return SummonKind(SUMMON_KIND_TOTEM_SLOT1 + entry->Slot - 1);
    if (entry->Control == 3)
        return SUMMON_KIND_POSSESSED;
    if (entry->Title == 5)
        return SUMMON_KIND_CRITTER;
    if (entry->Control == 2)
        return SUMMON_KIND_PET;
    if (entry->Control == 1)
        return SUMMON_KIND_GUARDIAN;
    return SUMMON_KIND_WILD;
}
