#include "UpdateObjectWriter.h"
#include "UpdateFieldFill.h"
#include "UpdateFieldStructs.h"
#include "WowCSEntityDefinitions.h"
#include "Objects/Object.h"
#include "Objects/Unit.h"
#include "Objects/UnitDefines.h"
#include "Objects/Player.h"
#include "Objects/Item.h"
#include "Objects/Bag.h"
#include "Objects/GameObject.h"
#include "Objects/Corpse.h"
#include "Objects/DynamicObject.h"
#include "Objects/UpdateData.h"
#include "Objects/MovementInfo.h"
#include "Chat/MasterPlayer.h"
#include "Server/WorldSession.h"
#include "Server/Packets/Classic/GuidMapping.h"
#include "Server/Packets/PlayerSetup.h"
#include "World.h"

namespace
{
ClassicGuid ToClassic(ObjectGuid const& guid)
{
    return ClassicGuidMapping::ToClassic(guid, realmID);
}

// The header bits every create block starts with (after the lone "HasPositionFragment" bit): which
// optional sections the movement frame carries. Most stay false until a later slice gives them a
// real payload writer (game objects, transports, vehicles, ...); setting one true without also
// extending BuildMovementUpdate below would desync the wire.
struct CreateObjectBits
{
    bool NoBirthAnim = false;
    bool EnablePortals = false;
    bool PlayHoverAnim = false;
    bool MovementUpdate = false;
    bool MovementTransport = false;
    bool Stationary = false;
    bool CombatVictim = false;
    bool ServerTime = false;
    bool Vehicle = false;
    bool AnimKit = false;
    bool Rotation = false;
    bool AreaTrigger = false;
    bool GameObject = false;
    bool SmoothPhasing = false;
    bool ThisIsYou = false;
    bool SceneObject = false;
    bool ActivePlayer = false;
    bool Conversation = false;
};

CreateObjectBits ComputeCreateObjectBits(Object const& object, Player const* target)
{
    CreateObjectBits flags;

    if (Unit const* unit = object.ToUnit())
    {
        flags.MovementUpdate = true;
        if (unit->IsHovering())
            flags.PlayHoverAnim = true;
    }
    else if (object.ToWorldObject())
    {
        // Items carry no position (not a WorldObject); everything else that isn't a unit is a
        // fixed-position object whose transform rides the movement frame's stationary section.
        flags.Stationary = true;
        if (object.GetTypeId() == TYPEID_GAMEOBJECT)
            flags.Rotation = true;
    }

    if (target && static_cast<Object const*>(target) == &object)
    {
        flags.ThisIsYou = true;
        flags.ActivePlayer = true;
    }

    return flags;
}

// Only the flags a unit can carry while standing still are translated; the transient ones
// (forward/strafe/turn/jump/fall) only matter once movement packets exist to set them.
uint32 ToModernMovementFlags(MovementInfo const& info)
{
    uint32 flags = 0;
    if (info.HasMovementFlag(MOVEFLAG_ROOT))
        flags |= 0x00000400; // MOVEMENTFLAG_ROOT
    if (info.HasMovementFlag(MOVEFLAG_SWIMMING))
        flags |= 0x00100000; // MOVEMENTFLAG_SWIMMING
    if (info.HasMovementFlag(MOVEFLAG_WATERWALKING))
        flags |= 0x04000000; // MOVEMENTFLAG_WATERWALKING
    if (info.HasMovementFlag(MOVEFLAG_SAFE_FALL))
        flags |= 0x08000000; // MOVEMENTFLAG_FALLING_SLOW
    if (info.HasMovementFlag(MOVEFLAG_HOVER))
        flags |= 0x10000000; // MOVEMENTFLAG_HOVER
    return flags;
}

void WriteEntityFragments(ByteBuffer& data, Object const& object)
{
    data << uint8(EntityFragment::CGObject);

    switch (object.GetTypeId())
    {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            data << uint8(EntityFragment::Tag_Item);
            if (object.GetTypeId() == TYPEID_CONTAINER)
                data << uint8(EntityFragment::Tag_Container);
            break;
        case TYPEID_UNIT:
            data << uint8(EntityFragment::Tag_Unit);
            break;
        case TYPEID_PLAYER:
            data << uint8(EntityFragment::Tag_Unit);
            data << uint8(EntityFragment::Tag_Player);
            break;
        case TYPEID_GAMEOBJECT:
            data << uint8(EntityFragment::Tag_GameObject);
            break;
        case TYPEID_DYNAMICOBJECT:
            data << uint8(EntityFragment::Tag_DynamicObject);
            break;
        case TYPEID_CORPSE:
            data << uint8(EntityFragment::Tag_Corpse);
            break;
        default:
            break;
    }

    data << uint8(EntityFragment::End);
}

EnumFlag<UpdateFieldFlag> GetUpdateFieldFlagsFor(Object const& object, Player const* target)
{
    UpdateFieldFlag flags = UpdateFieldFlag::None;
    if (!target)
        return flags;

    if (&object == static_cast<Object const*>(target))
        flags |= UpdateFieldFlag::Owner;
    else if (Item const* item = object.ToItem())
    {
        if (item->GetOwnerGuid() == target->GetObjectGuid())
            flags |= UpdateFieldFlag::Owner;
    }

    // Party/raid visibility and the empathy/special-info flag still aren't filled (bot-phase-4.md
    // slice C6); nothing reads them until then.
    return flags;
}

void BuildMovementUpdate(Object const& object, ByteBuffer& data, CreateObjectBits const& flags)
{
    data.WriteBit(object.ToWorldObject() != nullptr); // HasPositionFragment
    data.WriteBit(flags.NoBirthAnim);
    data.WriteBit(flags.EnablePortals);
    data.WriteBit(flags.PlayHoverAnim);
    data.WriteBit(flags.MovementUpdate);
    data.WriteBit(flags.MovementTransport);
    data.WriteBit(flags.Stationary);
    data.WriteBit(flags.CombatVictim);
    data.WriteBit(flags.ServerTime);
    data.WriteBit(flags.Vehicle);
    data.WriteBit(flags.AnimKit);
    data.WriteBit(flags.Rotation);
    data.WriteBit(flags.AreaTrigger);
    data.WriteBit(flags.GameObject);
    data.WriteBit(flags.SmoothPhasing);
    data.WriteBit(flags.ThisIsYou);
    data.WriteBit(flags.SceneObject);
    data.WriteBit(flags.ActivePlayer);
    data.WriteBit(flags.Conversation);
    data.FlushBits();

    if (flags.MovementUpdate)
    {
        Unit const& unit = static_cast<Unit const&>(object);
        MovementInfo const& mi = unit.m_movementInfo;

        data << ToClassic(object.GetObjectGuid());              // MoverGUID

        data << uint32(ToModernMovementFlags(mi));
        data << uint32(0);                                       // extra movement flags: not tracked yet
        data << uint32(0);                                       // extra movement flags 2: not tracked yet

        data << uint32(mi.stime);                                // MoveTime
        data << float(unit.GetPositionX());
        data << float(unit.GetPositionY());
        data << float(unit.GetPositionZ());
        data << float(unit.GetOrientation());

        data << float(mi.s_pitch);
        data << float(0.0f);                                     // StepUpStartElevation: no 1.12 source

        data << uint32(0);                                       // RemoveForcesIDs.size()
        data << uint32(0);                                       // MoveIndex

        data.WriteBit(false);                                    // HasStandingOnGameObjectGUID
        data.WriteBit(false);                                    // HasTransport: not wired up yet
        data.WriteBit(false);                                    // HasFall
        data.WriteBit(false);                                    // HasSpline
        data.WriteBit(false);                                    // HeightChangeFailed
        data.WriteBit(false);                                    // RemoteTimeValid
        data.WriteBit(false);                                    // HasInertia
        data.WriteBit(false);                                    // HasAdvFlying
        data.FlushBits();

        data << float(unit.GetSpeed(MOVE_WALK));
        data << float(unit.GetSpeed(MOVE_RUN));
        data << float(unit.GetSpeed(MOVE_RUN_BACK));
        data << float(unit.GetSpeed(MOVE_SWIM));
        data << float(unit.GetSpeed(MOVE_SWIM_BACK));
        data << float(7.0f);                                     // MOVE_FLIGHT: no 1.12 concept, retail default
        data << float(4.5f);                                     // MOVE_FLIGHT_BACK: retail default
        data << float(unit.GetSpeed(MOVE_TURN_RATE));
        data << float(unit.GetSpeed(MOVE_TURN_RATE));            // MOVE_PITCH_RATE: same default as turn rate

        data << uint32(0);                                       // MovementForces.size()
        data << float(1.0f);                                     // MovementForcesModMagnitude

        data << float(2.0f);                                     // advFlyingAirFriction
        data << float(65.0f);                                    // advFlyingMaxVel
        data << float(1.0f);                                     // advFlyingLiftCoefficient
        data << float(3.0f);                                     // advFlyingDoubleJumpVelMod
        data << float(10.0f);                                    // advFlyingGlideStartMinHeight
        data << float(100.0f);                                   // advFlyingAddImpulseMaxSpeed
        data << float(90.0f);                                    // advFlyingMinBankingRate
        data << float(140.0f);                                   // advFlyingMaxBankingRate
        data << float(180.0f);                                   // advFlyingMinPitchingRateDown
        data << float(360.0f);                                   // advFlyingMaxPitchingRateDown
        data << float(90.0f);                                    // advFlyingMinPitchingRateUp
        data << float(270.0f);                                   // advFlyingMaxPitchingRateUp
        data << float(30.0f);                                    // advFlyingMinTurnVelocityThreshold
        data << float(80.0f);                                    // advFlyingMaxTurnVelocityThreshold
        data << float(2.75f);                                    // advFlyingSurfaceFriction
        data << float(7.0f);                                     // advFlyingOverMaxDeceleration
        data << float(0.4f);                                     // advFlyingLaunchSpeedCoefficient

        data.WriteBit(false);                                    // HasSpline
        data.FlushBits();
    }

    data << uint32(0);                                           // PauseTimes.size(): no transport support yet

    if (flags.Stationary)
    {
        if (GameObject const* gameObject = object.ToGameObject())
        {
            data << float(gameObject->GetStationaryX());
            data << float(gameObject->GetStationaryY());
            data << float(gameObject->GetStationaryZ());
            data << float(gameObject->GetStationaryO());
        }
        else
        {
            WorldObject const& worldObject = static_cast<WorldObject const&>(object);
            data << float(worldObject.GetPositionX());
            data << float(worldObject.GetPositionY());
            data << float(worldObject.GetPositionZ());
            data << float(worldObject.GetOrientation());
        }
    }

    if (flags.Rotation)
        data << uint64(static_cast<GameObject const&>(object).GetRotation());

    if (flags.ActivePlayer)
    {
        Player const& player = static_cast<Player const&>(object);

        data.WriteBit(false);                                    // HasSceneInstanceIDs: no scene manager
        data.WriteBit(false);                                    // HasRuneState: no runes in classic
        data.WriteBit(true);                                     // HasActionButtons
        data.FlushBits();

        MasterPlayer* masterPlayer = player.GetSession() ? player.GetSession()->GetMasterPlayer() : nullptr;
        for (uint32 i = 0; i < CLASSIC_ACTION_BUTTON_COUNT; ++i)
        {
            uint32 packedData = 0;
            if (masterPlayer)
            {
                ActionButtonList& buttons = masterPlayer->GetActionButtons();
                ActionButtonList::const_iterator itr = buttons.find(uint8(i));
                if (itr != buttons.end() && itr->second.uState != ACTIONBUTTON_DELETED)
                    packedData = itr->second.packedData;
            }
            data << packedData;
        }
    }
}

void BuildValuesCreate(Object const& object, ByteBuffer& data, EnumFlag<UpdateFieldFlag> flags, Player const* target)
{
    bool const isSelf = (target && &object == static_cast<Object const*>(target));

    object.m_objectData->WriteCreate(data, flags, &object, target);

    if (Unit const* unit = object.ToUnit())
    {
        unit->m_unitData->WriteCreate(data, flags, unit, target);

        if (Player const* player = object.ToPlayer())
        {
            player->m_playerData->WriteCreate(data, flags, player, target);

            if (isSelf)
                player->m_activePlayerData->WriteCreate(data, flags, player, target);
        }
    }
    else if (Item const* item = object.ToItem())
    {
        item->m_itemData->WriteCreate(data, flags, item, target);
        if (Bag const* bag = item->ToBag())
            bag->m_containerData->WriteCreate(data, flags, bag, target);
    }
    else if (GameObject const* gameObject = object.ToGameObject())
        gameObject->m_gameObjectData->WriteCreate(data, flags, gameObject, target);
    else if (Corpse const* corpse = object.ToCorpse())
        corpse->m_corpseData->WriteCreate(data, flags, corpse, target);
    else if (DynamicObject const* dynamicObject = object.ToDynamicObject())
        dynamicObject->m_dynamicObjectData->WriteCreate(data, flags, dynamicObject, target);
}
}

void BuildCreateUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target)
{
    if (!target)
        return;

    // The typed structs are a derived cache of the flat values array (decision 5); refreshing it
    // here, rather than trusting the per-tick values pump to have already run, also covers items,
    // which never enter that pump (Item::BuildUpdateData only rebuilds values blocks for players
    // who already see the item).
    FillUpdateFieldData(const_cast<Object&>(object));

    uint8 updateType = object.IsNewObject() ? UPDATETYPE_CREATE_OBJECT2 : UPDATETYPE_CREATE_OBJECT;
    uint8 objectType = object.GetTypeId();

    CreateObjectBits flags = ComputeCreateObjectBits(object, target);
    if (static_cast<Object const*>(target) == &object)
        objectType = TYPEID_ACTIVE_PLAYER;

    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();

    buf << uint8(updateType);
    buf << ToClassic(object.GetObjectGuid());
    buf << uint8(objectType);

    BuildMovementUpdate(object, buf, flags);

    EnumFlag<UpdateFieldFlag> fieldFlags = GetUpdateFieldFlagsFor(object, target);
    std::size_t sizePos = buf.wpos();
    buf << uint32(0);
    buf << uint8(UpdateFieldFlag(fieldFlags));
    WriteEntityFragments(buf, object);
    buf << uint8(1);                                             // IndirectFragmentActive: CGObject
    BuildValuesCreate(object, buf, fieldFlags, target);
    buf.put<uint32>(sizePos, uint32(buf.wpos() - sizePos - 4));
}
