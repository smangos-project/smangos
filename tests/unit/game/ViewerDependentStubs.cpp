#include "Objects/Updates/ViewerDependentValues.h"

uint32 GetViewerDynamicFlags(UFObjectData const* data, Object const*, Player const*)
{
    return data->DynamicFlags;
}

uint32 GetViewerNpcFlags(UFUnitData const* data, Unit const*, Player const*)
{
    return data->NpcFlags;
}

ClassicGuid GetViewerTarget(UFUnitData const* data, Unit const*, Player const*)
{
    return data->Target;
}

int32 GetViewerFactionTemplate(UFUnitData const* data, Unit const*, Player const*)
{
    return data->FactionTemplate;
}

uint32 GetViewerUnitFlags(UFUnitData const* data, Unit const*, Player const*)
{
    return data->Flags;
}

int64 GetViewerHealth(UFUnitData const* data, Unit const*, Player const*)
{
    return data->Health;
}

int64 GetViewerMaxHealth(UFUnitData const* data, Unit const*, Player const*)
{
    return data->MaxHealth;
}
