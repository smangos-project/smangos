#ifndef UPDATES_WOWCS_ENTITY_DEFINITIONS_H
#define UPDATES_WOWCS_ENTITY_DEFINITIONS_H

#include "Platform/Define.h"

// Client-side entity composition tags, written after a create block's values size and terminated
// by End. Computed from the object's actual type at serialize time (UpdateObjectWriter.cpp), not
// held as per-object state.
enum class EntityFragment : uint8
{
    CGObject          = 0,
    Tag_Item          = 1,
    Tag_Container     = 2,
    Tag_Unit          = 5,
    Tag_Player        = 6,
    Tag_GameObject    = 7,
    Tag_DynamicObject = 8,
    Tag_Corpse        = 9,
    End               = 255,
};

constexpr uint8 CGObjectActiveMask  = 0x1;
constexpr uint8 CGObjectChangedMask = 0x2;

#endif
