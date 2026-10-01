#include "UpdateObjectWriter.h"
#include "UpdateFieldFill.h"
#include "UpdateFieldStructs.h"
#include "WowCSEntityDefinitions.h"
#include "Objects/Object.h"
#include "Objects/Unit.h"
#include "Spells/SpellAuras.h"
#include "Objects/UnitDefines.h"
#include "Objects/Player.h"
#include "Objects/Item.h"
#include "Objects/Bag.h"
#include "Objects/GameObject.h"
#include "Objects/Corpse.h"
#include "Objects/DynamicObject.h"
#include "Objects/UpdateData.h"
#include "Objects/MovementInfoWire.h"
#include "Movement/spline/MoveSpline.h"
#include "Movement/spline/packet_builder.h"
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

    if (Item const* item = object.ToItem())
    {
        if (item->GetOwnerGuid() == target->GetObjectGuid())
            flags |= UpdateFieldFlag::Owner;
    }
    else if (Unit const* unit = object.ToUnit())
    {
        if (target->HasCheatOption(PLAYER_CHEAT_DEBUG_TARGET_INFO))
        {
            flags |= UpdateFieldFlag::Owner;
            flags |= UpdateFieldFlag::PartyMember;
            flags |= UpdateFieldFlag::Empath;
        }
        else
        {
            if (unit->GetOwnerGuid() == target->GetObjectGuid() || unit->GetCharmerGuid() == target->GetObjectGuid())
                flags |= UpdateFieldFlag::Owner;

            if ((unit->GetUInt32Value(UNIT_DYNAMIC_FLAGS) & UNIT_DYNFLAG_SPECIALINFO) && target->CanSeeSpecialInfoOf(unit))
                flags |= UpdateFieldFlag::Empath;

            if (Player const* controller = unit->GetCharmerOrOwnerPlayerOrPlayerItself())
                if (controller->IsInSameRaidWith(target))
                    flags |= UpdateFieldFlag::PartyMember;
        }
    }

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

        // A unit mid-path travels along its spline, and the client only accepts a create that
        // says so consistently: forward movement without walking, and the spline in both places
        // that announce it. A creature not on a path is presented standing.
        bool const hasSpline = unit.movespline && unit.movespline->Initialized() && !unit.movespline->Finalized();
        uint32 moveFlags = mi.moveFlags;
        if (hasSpline)
            moveFlags = (moveFlags & ~MOVEFLAG_WALK_MODE) | MOVEFLAG_FORWARD;
        else if (unit.IsCreature())
            moveFlags &= ~(MOVEFLAG_WALK_MODE | MOVEFLAG_MASK_MOVING_OR_TURN | MOVEFLAG_ROOT);

        data << ToClassic(object.GetObjectGuid());              // MoverGUID

        data << uint32(ToModernMovementFlags(moveFlags));
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

        bool const hasTransport = mi.HasMovementFlag(MOVEFLAG_ONTRANSPORT) && !mi.GetTransportGuid().IsEmpty();

        data.WriteBit(false);                                    // HasStandingOnGameObjectGUID
        data.WriteBit(hasTransport);                             // HasTransport
        data.WriteBit(false);                                    // HasFall
        data.WriteBit(hasSpline);                                // HasSpline
        data.WriteBit(false);                                    // HeightChangeFailed
        data.WriteBit(false);                                    // RemoteTimeValid
        data.WriteBit(false);                                    // HasInertia
        data.WriteBit(false);                                    // HasAdvFlying
        data.FlushBits();

        if (hasTransport)
        {
            ToClassic(mi.GetTransportGuid()).Write(data);
            data << mi.GetTransportPos().x << mi.GetTransportPos().y << mi.GetTransportPos().z << mi.GetTransportPos().o;
            data << int8(-1) << uint32(0);                       // Seat, Time: no 1.12 source
            data.WriteBit(false);                                // HasPrevTime
            data.WriteBit(false);                                // HasVehicleId
            data.FlushBits();
        }

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

        data.WriteBit(hasSpline);                                // HasSpline
        data.FlushBits();

        if (hasSpline)
            Movement::PacketBuilder::WriteCreateSpline(*unit.movespline, data);
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

uint8 ToWireTypeId(uint8 typeId, bool isSelf)
{
    switch (typeId)
    {
        case TYPEID_ITEM:          return 1;
        case TYPEID_CONTAINER:     return 2;
        case TYPEID_UNIT:          return 5;
        case TYPEID_PLAYER:        return isSelf ? 7 : 6;
        case TYPEID_GAMEOBJECT:    return 8;
        case TYPEID_DYNAMICOBJECT: return 9;
        case TYPEID_CORPSE:        return 10;
        default:                   return 0;
    }
}

// Translates the holder's TypeID bits to the client's numbering.
uint32 ToWireTypeMask(uint32 holderMask)
{
    struct Entry
    {
        uint32 holderBit;
        uint32 wireBit;
    };
    static constexpr Entry entries[] =
    {
        { TYPEID_OBJECT, 0 }, { TYPEID_ITEM, 1 }, { TYPEID_CONTAINER, 2 }, { TYPEID_UNIT, 5 },
        { TYPEID_PLAYER, 6 }, { TYPEID_ACTIVE_PLAYER, 7 }, { TYPEID_GAMEOBJECT, 8 },
        { TYPEID_DYNAMICOBJECT, 9 }, { TYPEID_CORPSE, 10 },
    };

    uint32 wireMask = 0;
    for (Entry const& entry : entries)
        if (holderMask & (1u << entry.holderBit))
            wireMask |= 1u << entry.wireBit;
    return wireMask;
}

template<typename StructT, typename FieldT, int32 BlockBit, uint32 Bit>
void MarkFieldInMask(typename StructT::Mask& mask, UpdateField<FieldT, BlockBit, Bit> StructT::* /*field*/)
{
    if constexpr (BlockBit >= 0)
        mask.Set(BlockBit);
    mask.Set(Bit);
}

uint32 GetChangedTypeMask(Object const& object, bool isSelf)
{
    uint32 mask = object.m_values.GetChangedObjectTypeMask();
    if (!isSelf)
        mask &= ~(1u << TYPEID_ACTIVE_PLAYER);
    return mask;
}

void BuildValuesUpdate(Object const& object, ByteBuffer& data, EnumFlag<UpdateFieldFlag> flags, Player const* target, uint32 changedMask, bool isSelf)
{
    data << uint32(ToWireTypeMask(changedMask));

    auto changed = [changedMask](uint32 typeId) { return (changedMask & (1u << typeId)) != 0; };

    if (changed(TYPEID_OBJECT))
        object.m_objectData->WriteUpdate(data, flags, &object, target);

    if (Unit const* unit = object.ToUnit())
    {
        if (changed(TYPEID_UNIT))
            unit->m_unitData->WriteUpdate(data, flags, unit, target);

        if (Player const* player = unit->ToPlayer())
        {
            if (changed(TYPEID_PLAYER))
                player->m_playerData->WriteUpdate(data, flags, player, target);
            if (isSelf && changed(TYPEID_ACTIVE_PLAYER))
                player->m_activePlayerData->WriteUpdate(data, flags, player, target);
        }
    }
    else if (Item const* item = object.ToItem())
    {
        if (changed(TYPEID_ITEM))
            item->m_itemData->WriteUpdate(data, flags, item, target);
        if (Bag const* bag = item->ToBag())
            if (changed(TYPEID_CONTAINER))
                bag->m_containerData->WriteUpdate(data, flags, bag, target);
    }
    else if (GameObject const* gameObject = object.ToGameObject())
    {
        if (changed(TYPEID_GAMEOBJECT))
            gameObject->m_gameObjectData->WriteUpdate(data, flags, gameObject, target);
    }
    else if (Corpse const* corpse = object.ToCorpse())
    {
        if (changed(TYPEID_CORPSE))
            corpse->m_corpseData->WriteUpdate(data, flags, corpse, target);
    }
    else if (DynamicObject const* dynamicObject = object.ToDynamicObject())
    {
        if (changed(TYPEID_DYNAMICOBJECT))
            dynamicObject->m_dynamicObjectData->WriteUpdate(data, flags, dynamicObject, target);
    }
}

std::size_t OpenValuesBlock(ByteBuffer& buf, Object const& object, EnumFlag<UpdateFieldFlag> flags, uint8 contentsChangedMask)
{
    buf << uint8(UPDATETYPE_VALUES);
    buf << ToClassic(object.GetObjectGuid());

    std::size_t sizePos = buf.wpos();
    buf << uint32(0);
    buf << uint8(flags.HasFlag(UpdateFieldFlag::Owner));
    buf << uint8(0);                                             // IdsChanged
    buf << contentsChangedMask;
    return sizePos;
}

// Ships and zeppelins display a WMO (GameObjectDisplayInfo.FileDataID) and need the transport
// section of the create block, which the movement frame does not write yet ("no transport
// support yet"). Without it the client loads the WMO as an M2 and crashes on entering the world,
// so they stay unsent, and so does every update about them, until that section exists.
bool IsUnsentTransport(Object const& object)
{
    GameObject const* gameObject = object.ToGameObject();
    GameObjectInfo const* info = gameObject ? gameObject->GetGOInfo() : nullptr;
    return info && info->type == GAMEOBJECT_TYPE_MO_TRANSPORT;
}

// Under LogNetworkPackets, one Network.log line per object block written into an update, so a
// client that rejects the update (CMSG_LOG_DISCONNECT) can be matched to the object that did it.
void LogUpdateBlock(char const* kind, Object const& object)
{
    if (!sWorld.getConfig(CONFIG_BOOL_LOG_NETWORK_PACKETS))
        return;
    GameObject const* gameObject = object.ToGameObject();
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "UpdateBlock: %s %s entry %u goType %d", kind, object.GetObjectGuid().GetString().c_str(), object.GetEntry(), gameObject && gameObject->GetGOInfo() ? int(gameObject->GetGOInfo()->type) : -1);
}

void CloseValuesBlock(ByteBuffer& buf, std::size_t sizePos)
{
    buf.put<uint32>(sizePos, uint32(buf.wpos() - sizePos - 4));
}
}

void BuildCreateUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target)
{
    if (!target)
        return;

    if (IsUnsentTransport(object))
        return;

    // The typed structs are a derived cache of the flat values array (decision 5); refreshing it
    // here, rather than trusting the per-tick values pump to have already run, also covers items,
    // which never enter that pump (Item::BuildUpdateData only rebuilds values blocks for players
    // who already see the item).
    FillUpdateFieldData(const_cast<Object&>(object));

    uint8 updateType = object.IsNewObject() ? UPDATETYPE_CREATE_OBJECT2 : UPDATETYPE_CREATE_OBJECT;
    CreateObjectBits flags = ComputeCreateObjectBits(object, target);
    uint8 const objectType = ToWireTypeId(object.GetTypeId(), static_cast<Object const*>(target) == &object);

    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();
    LogUpdateBlock("create", object);

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

    if (Unit const* unit = object.ToUnit())
    {
        AuraUpdate auras;
        auras.UpdateAll = true;
        auras.UnitGuid = ToClassic(unit->GetObjectGuid());
        for (auto const& entry : unit->GetSpellAuraHolderMap())
        {
            SpellAuraHolder const* holder = entry.second;
            if (holder->GetAuraSlot() < MAX_AURAS)
                holder->BuildAuraInfo(auras.Auras.emplace_back());
        }
        if (!auras.Auras.empty())
            data.AddAuraUpdate(std::move(auras));
    }
}

void BuildValuesUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target)
{
    if (!target || IsUnsentTransport(object))
        return;

    bool const isSelf = static_cast<Object const*>(target) == &object;
    uint32 const changedMask = GetChangedTypeMask(object, isSelf);
    if (!changedMask)
        return;

    EnumFlag<UpdateFieldFlag> fieldFlags = GetUpdateFieldFlagsFor(object, target);
    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();
    LogUpdateBlock("values", object);
    std::size_t sizePos = OpenValuesBlock(buf, object, fieldFlags, CGObjectChangedMask);
    BuildValuesUpdate(object, buf, fieldFlags, target, changedMask, isSelf);
    CloseValuesBlock(buf, sizePos);
}

void BuildValuesUpdateBlockForPlayerWithFlag(Object const& object, UpdateData& data, EnumFlag<UpdateFieldFlag> flags, Player* target)
{
    if (!target || IsUnsentTransport(object))
        return;

    FillUpdateFieldData(const_cast<Object&>(object));

    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();
    LogUpdateBlock("values", object);
    std::size_t sizePos = OpenValuesBlock(buf, object, flags, CGObjectActiveMask | CGObjectChangedMask);

    Unit const* unit = object.ToUnit();
    Player const* player = object.ToPlayer();
    Item const* item = object.ToItem();

    uint32 holderMask = 0;
    if (unit)
        holderMask |= 1u << TYPEID_UNIT;
    if (player)
        holderMask |= 1u << TYPEID_PLAYER;
    if (item)
        holderMask |= 1u << TYPEID_ITEM;
    buf << uint32(ToWireTypeMask(holderMask));

    if (unit)
    {
        UFUnitData::Mask mask;
        UFUnitData::AppendAllowedFieldsMaskForFlag(mask, flags);
        unit->m_unitData->WriteUpdate(buf, mask, true, unit, target);
    }
    if (player)
    {
        UFPlayerData::Mask mask;
        UFPlayerData::AppendAllowedFieldsMaskForFlag(mask, flags);
        player->m_playerData->WriteUpdate(buf, mask, true, player, target);
    }
    if (item)
    {
        UFItemData::Mask mask;
        UFItemData::AppendAllowedFieldsMaskForFlag(mask, flags);
        item->m_itemData->WriteUpdate(buf, mask, true, item, target);
    }

    CloseValuesBlock(buf, sizePos);
}

void BuildDynamicFlagsUpdateBlockForPlayer(Object const& object, UpdateData& data, Player* target)
{
    if (!target || IsUnsentTransport(object))
        return;

    FillUpdateFieldData(const_cast<Object&>(object));

    EnumFlag<UpdateFieldFlag> fieldFlags = GetUpdateFieldFlagsFor(object, target);
    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();
    LogUpdateBlock("dynamic flags", object);
    std::size_t sizePos = OpenValuesBlock(buf, object, fieldFlags, CGObjectActiveMask | CGObjectChangedMask);

    UFObjectData::Mask mask;
    MarkFieldInMask(mask, &UFObjectData::DynamicFlags);

    buf << uint32(ToWireTypeMask(1u << TYPEID_OBJECT));
    object.m_objectData->WriteUpdate(buf, mask, true, &object, target);
    CloseValuesBlock(buf, sizePos);
}

void BuildUnitFlagsUpdateBlockForPlayer(Unit const& unit, UpdateData& data, Player* target)
{
    if (!target)
        return;

    FillUpdateFieldData(const_cast<Unit&>(unit));

    EnumFlag<UpdateFieldFlag> fieldFlags = GetUpdateFieldFlagsFor(unit, target);
    ByteBuffer& buf = data.AddUpdateBlockAndGetBuffer();
    LogUpdateBlock("unit flags", unit);
    std::size_t sizePos = OpenValuesBlock(buf, unit, fieldFlags, CGObjectActiveMask | CGObjectChangedMask);

    UFUnitData::Mask mask;
    MarkFieldInMask(mask, &UFUnitData::Flags);

    buf << uint32(ToWireTypeMask(1u << TYPEID_UNIT));
    unit.m_unitData->WriteUpdate(buf, mask, true, &unit, target);
    CloseValuesBlock(buf, sizePos);
}
