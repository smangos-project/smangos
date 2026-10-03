#include "ViewerDependentValues.h"
#include "Objects/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectDefines.h"
#include "Objects/Player.h"
#include "Objects/Unit.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "ObjectMgr.h"
#include "World.h"
#include <mutex>

namespace
{
ClassicGuid ToClassic(ObjectGuid const& guid)
{
    return ClassicGuidMapping::ToClassic(guid, realmID);
}

uint32 ViewerGameObjectDynamicFlags(GameObject const& gameObject, uint32 stored, Player const& receiver)
{
    if (gameObject.IsTransport())
        return stored;

    bool const activate = gameObject.ActivateToQuest(&receiver) || receiver.IsGameMaster();
    {
        std::unique_lock<std::mutex> lock(receiver.m_visibleGobjsQuestAct_lock);
        const_cast<Player&>(receiver).m_visibleGobjQuestActivated[gameObject.GetObjectGuid()] = activate;
    }

    uint32 flags = GO_DYNFLAG_LO_STATE_TRANSITION_ANIM_DONE;
    if (!activate)
        return flags;

    switch (gameObject.GetGoType())
    {
        case GAMEOBJECT_TYPE_QUESTGIVER:
        case GAMEOBJECT_TYPE_GOOBER:
            flags |= GO_DYNFLAG_LO_ACTIVATE;
            break;
        case GAMEOBJECT_TYPE_CHEST:
            flags |= GO_DYNFLAG_LO_ACTIVATE | GO_DYNFLAG_LO_SPARKLE;
            break;
        case GAMEOBJECT_TYPE_GENERIC:
        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            flags |= GO_DYNFLAG_LO_SPARKLE;
            break;
        default:
            break;
    }
    return flags;
}

// Modern UnitDynFlags bits (TrinityCore SharedDefines.h); the 1.12 values array keeps the old layout.
constexpr uint32 MODERN_UNIT_DYNFLAG_LOOTABLE    = 0x0004;
constexpr uint32 MODERN_UNIT_DYNFLAG_TRACK_UNIT  = 0x0008;
constexpr uint32 MODERN_UNIT_DYNFLAG_TAPPED      = 0x0010;
constexpr uint32 MODERN_UNIT_DYNFLAG_SPECIALINFO = 0x0020;

uint32 ViewerUnitDynamicFlags(Unit const& unit, uint32 stored, Player const& receiver)
{
    uint32 flags = 0;

    if ((stored & UNIT_DYNFLAG_TRACK_UNIT) && unit.HasAuraTypeByCaster(SPELL_AURA_MOD_STALKED, receiver.GetObjectGuid()))
        flags |= MODERN_UNIT_DYNFLAG_TRACK_UNIT;

    if (Creature const* creature = unit.ToCreature())
    {
        // TAPPED means tapped by someone other than the receiver.
        if (creature->HasLootRecipient() && !creature->IsTappedBy(&receiver))
            flags |= MODERN_UNIT_DYNFLAG_TAPPED;

        if ((stored & UNIT_DYNFLAG_LOOTABLE) && const_cast<Player&>(receiver).IsAllowedToLoot(creature))
            flags |= MODERN_UNIT_DYNFLAG_LOOTABLE;
    }

    if ((stored & UNIT_DYNFLAG_SPECIALINFO) || receiver.HasCheatOption(PLAYER_CHEAT_DEBUG_TARGET_INFO))
        flags |= MODERN_UNIT_DYNFLAG_SPECIALINFO;

    return flags;
}
}

uint32 GetViewerDynamicFlags(UFObjectData const* data, Object const* owner, Player const* receiver)
{
    uint32 const stored = data->DynamicFlags;
    if (!owner || !receiver)
        return stored;

    if (Unit const* unit = owner->ToUnit())
        return ViewerUnitDynamicFlags(*unit, stored, *receiver);
    if (GameObject const* gameObject = owner->ToGameObject())
        return ViewerGameObjectDynamicFlags(*gameObject, stored, *receiver);
    return stored;
}

namespace
{
uint32 ToClientNpcFlags(uint32 flags)
{
    static constexpr std::pair<uint32, uint32> map[] =
    {
        { UNIT_NPC_FLAG_GOSSIP,         0x00000001 },
        { UNIT_NPC_FLAG_QUESTGIVER,     0x00000002 },
        { UNIT_NPC_FLAG_VENDOR,         0x00000080 },
        { UNIT_NPC_FLAG_FLIGHTMASTER,   0x00002000 },
        { UNIT_NPC_FLAG_TRAINER,        0x00000010 },
        { UNIT_NPC_FLAG_SPIRITHEALER,   0x00004000 },
        { UNIT_NPC_FLAG_SPIRITGUIDE,    0x00008000 },
        { UNIT_NPC_FLAG_INNKEEPER,      0x00010000 },
        { UNIT_NPC_FLAG_BANKER,         0x00020000 },
        { UNIT_NPC_FLAG_PETITIONER,     0x00040000 },
        { UNIT_NPC_FLAG_TABARDDESIGNER, 0x00080000 },
        { UNIT_NPC_FLAG_BATTLEMASTER,   0x00100000 },
        { UNIT_NPC_FLAG_AUCTIONEER,     0x00200000 },
        { UNIT_NPC_FLAG_STABLEMASTER,   0x00400000 },
        { UNIT_NPC_FLAG_REPAIR,         0x00001000 },
    };
    uint32 out = 0;
    for (auto const& [vanilla, client] : map)
        if (flags & vanilla)
            out |= client;
    return out;
}
}

uint32 GetViewerNpcFlags(UFUnitData const* data, Unit const* owner, Player const* receiver)
{
    uint32 flags = data->NpcFlags;
    Creature const* creature = owner ? owner->ToCreature() : nullptr;
    if (!creature || !receiver)
        return ToClientNpcFlags(flags);

    if ((flags & UNIT_NPC_FLAG_TRAINER) && !creature->IsTrainerOf(const_cast<Player*>(receiver), false))
        flags &= ~UNIT_NPC_FLAG_TRAINER;

    if ((flags & UNIT_NPC_FLAG_STABLEMASTER) && receiver->GetClass() != CLASS_HUNTER)
        flags &= ~UNIT_NPC_FLAG_STABLEMASTER;

    if (flags & UNIT_NPC_FLAG_FLIGHTMASTER)
    {
        QuestRelationsMapBounds bounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(creature->GetEntry());
        for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            if (receiver->CanSeeStartQuest(sObjectMgr.GetQuestTemplate(itr->second)))
            {
                flags &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                break;
            }
        }

        if (flags & UNIT_NPC_FLAG_FLIGHTMASTER)
        {
            bounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(creature->GetEntry());
            for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
            {
                if (receiver->CanRewardQuest(sObjectMgr.GetQuestTemplate(itr->second), false))
                {
                    flags &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                    break;
                }
            }
        }
    }

    return ToClientNpcFlags(flags);
}

ClassicGuid GetViewerTarget(UFUnitData const* data, Unit const* owner, Player const* /*receiver*/)
{
    if (Creature const* creature = owner ? owner->ToCreature() : nullptr)
        if (creature->m_castingTargetGuid)
            return ToClassic(ObjectGuid(creature->m_castingTargetGuid));
    return data->Target;
}

int32 GetViewerFactionTemplate(UFUnitData const* data, Unit const* owner, Player const* receiver)
{
    int32 faction = data->FactionTemplate;
    if (!owner || !receiver)
        return faction;

    Unit const* controller = owner->GetCharmerOrOwner();
    if (!controller)
        controller = owner->ToPlayer();
    if (!controller || !controller->IsPlayer())
        return faction;

    FactionTemplateEntry const* ownerFaction = controller->GetFactionTemplateEntry();
    FactionTemplateEntry const* receiverFaction = receiver->GetFactionTemplateEntry();
    if (ownerFaction && receiverFaction && !ownerFaction->IsFriendlyTo(*receiverFaction))
    {
        Player const* ownerPlayer = static_cast<Player const*>(controller);
        if (ownerPlayer->IsInSameRaidWith(receiver) && ownerPlayer->IsInInterFactionMode() && receiver->IsInInterFactionMode())
            return int32(receiver->GetFactionTemplateId());
    }
    return faction;
}

uint32 GetViewerUnitFlags(UFUnitData const* data, Unit const* /*owner*/, Player const* receiver)
{
    if (receiver && receiver->IsGameMaster())
        return (data->Flags | UNIT_FLAG_AURAS_VISIBLE) & ~UNIT_FLAG_NOT_SELECTABLE;
    return data->Flags;
}

int64 GetViewerHealth(UFUnitData const* data, Unit const* owner, Player const* receiver)
{
    if (!owner || !receiver || sWorld.getConfig(CONFIG_BOOL_OBJECT_HEALTH_VALUE_SHOW) || receiver->CanSeeHealthOf(owner))
        return data->Health;

    if (data->Health <= 0)
        return 0;
    int64 pct = (data->Health * 100) / std::max<int64>(data->MaxHealth, 1);
    return std::clamp<int64>(pct, 1, 100);
}

int64 GetViewerMaxHealth(UFUnitData const* data, Unit const* owner, Player const* receiver)
{
    if (!owner || !receiver || sWorld.getConfig(CONFIG_BOOL_OBJECT_HEALTH_VALUE_SHOW) || receiver->CanSeeHealthOf(owner))
        return data->MaxHealth;
    return 100;
}
