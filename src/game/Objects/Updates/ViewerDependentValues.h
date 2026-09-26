#ifndef VIEWERDEPENDENTVALUES_H
#define VIEWERDEPENDENTVALUES_H

#include "UpdateFieldStructs.h"

class Object;
class Unit;
class GameObject;
class Player;

// Fields whose value depends on who's looking are read through this hook at serialize time.
template<typename Tag>
class ViewerDependentValue
{
};

// Defined in ViewerDependentValues.cpp.
uint32 GetViewerDynamicFlags(UFObjectData const* data, Object const* owner, Player const* receiver);
uint32 GetViewerNpcFlags(UFUnitData const* data, Unit const* owner, Player const* receiver);
ClassicGuid GetViewerTarget(UFUnitData const* data, Unit const* owner, Player const* receiver);
int32 GetViewerFactionTemplate(UFUnitData const* data, Unit const* owner, Player const* receiver);
uint32 GetViewerUnitFlags(UFUnitData const* data, Unit const* owner, Player const* receiver);
int64 GetViewerHealth(UFUnitData const* data, Unit const* owner, Player const* receiver);
int64 GetViewerMaxHealth(UFUnitData const* data, Unit const* owner, Player const* receiver);

template<>
class ViewerDependentValue<UFObjectData::EntryIDTag>
{
public:
    using value_type = UFObjectData::EntryIDTag::value_type;

    static value_type GetValue(UFObjectData const* data, Object const* /*owner*/, Player const* /*receiver*/)
    {
        return data->EntryID;
    }
};

template<>
class ViewerDependentValue<UFObjectData::DynamicFlagsTag>
{
public:
    using value_type = UFObjectData::DynamicFlagsTag::value_type;

    static value_type GetValue(UFObjectData const* data, Object const* owner, Player const* receiver)
    {
        return GetViewerDynamicFlags(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::StateWorldEffectIDsTag>
{
public:
    using value_type = UFUnitData::StateWorldEffectIDsTag::value_type const*;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return &*data->StateWorldEffectIDs;
    }
};

template<>
class ViewerDependentValue<UFUnitData::HealthTag>
{
public:
    using value_type = UFUnitData::HealthTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerHealth(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::MaxHealthTag>
{
public:
    using value_type = UFUnitData::MaxHealthTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerMaxHealth(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::DisplayIDTag>
{
public:
    using value_type = UFUnitData::DisplayIDTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->DisplayID;
    }
};

template<>
class ViewerDependentValue<UFUnitData::NpcFlagsTag>
{
public:
    using value_type = UFUnitData::NpcFlagsTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerNpcFlags(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::NpcFlags2Tag>
{
public:
    using value_type = UFUnitData::NpcFlags2Tag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->NpcFlags2;
    }
};

template<>
class ViewerDependentValue<UFUnitData::StateSpellVisualIDTag>
{
public:
    using value_type = UFUnitData::StateSpellVisualIDTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->StateSpellVisualID;
    }
};

template<>
class ViewerDependentValue<UFUnitData::StateAnimIDTag>
{
public:
    using value_type = UFUnitData::StateAnimIDTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->StateAnimID;
    }
};

template<>
class ViewerDependentValue<UFUnitData::StateAnimKitIDTag>
{
public:
    using value_type = UFUnitData::StateAnimKitIDTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->StateAnimKitID;
    }
};

template<>
class ViewerDependentValue<UFUnitData::TargetTag>
{
public:
    using value_type = UFUnitData::TargetTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerTarget(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::FactionTemplateTag>
{
public:
    using value_type = UFUnitData::FactionTemplateTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerFactionTemplate(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::FlagsTag>
{
public:
    using value_type = UFUnitData::FlagsTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* owner, Player const* receiver)
    {
        return GetViewerUnitFlags(data, owner, receiver);
    }
};

template<>
class ViewerDependentValue<UFUnitData::Flags2Tag>
{
public:
    using value_type = UFUnitData::Flags2Tag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->Flags2;
    }
};

template<>
class ViewerDependentValue<UFUnitData::Flags3Tag>
{
public:
    using value_type = UFUnitData::Flags3Tag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->Flags3;
    }
};

template<>
class ViewerDependentValue<UFUnitData::Flags4Tag>
{
public:
    using value_type = UFUnitData::Flags4Tag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->Flags4;
    }
};

template<>
class ViewerDependentValue<UFUnitData::AuraStateTag>
{
public:
    using value_type = UFUnitData::AuraStateTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->AuraState;
    }
};

template<>
class ViewerDependentValue<UFUnitData::PvpFlagsTag>
{
public:
    using value_type = UFUnitData::PvpFlagsTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->PvpFlags;
    }
};

template<>
class ViewerDependentValue<UFUnitData::InteractSpellIDTag>
{
public:
    using value_type = UFUnitData::InteractSpellIDTag::value_type;

    static value_type GetValue(UFUnitData const* data, Unit const* /*owner*/, Player const* /*receiver*/)
    {
        return data->InteractSpellID;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::StateWorldEffectIDsTag>
{
public:
    using value_type = UFGameObjectData::StateWorldEffectIDsTag::value_type const*;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return &*data->StateWorldEffectIDs;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::StateSpellVisualIDTag>
{
public:
    using value_type = UFGameObjectData::StateSpellVisualIDTag::value_type;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return data->StateSpellVisualID;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::SpawnTrackingStateAnimIDTag>
{
public:
    using value_type = UFGameObjectData::SpawnTrackingStateAnimIDTag::value_type;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return data->SpawnTrackingStateAnimID;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::SpawnTrackingStateAnimKitIDTag>
{
public:
    using value_type = UFGameObjectData::SpawnTrackingStateAnimKitIDTag::value_type;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return data->SpawnTrackingStateAnimKitID;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::FlagsTag>
{
public:
    using value_type = UFGameObjectData::FlagsTag::value_type;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return data->Flags;
    }
};

template<>
class ViewerDependentValue<UFGameObjectData::StateTag>
{
public:
    using value_type = UFGameObjectData::StateTag::value_type;

    static value_type GetValue(UFGameObjectData const* data, GameObject const* /*owner*/, Player const* /*receiver*/)
    {
        return data->State;
    }
};

#endif // VIEWERDEPENDENTVALUES_H
