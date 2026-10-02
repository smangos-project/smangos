#include "BotWorldView.h"
#include "Packets/Auras.h"
#include "Packets/ObjectUpdate.h"
#include "Packets/MonsterMove.h"
#include "Packets/MovementMessages.h"
#include "ByteBuffer.h"

namespace
{
// Named separately from Objects/UpdateData.h's ObjectUpdateType (visible here too when RealBots is
// compiled into `game`): this reader must not depend on the server's own enum.
constexpr uint8_t BOT_UPDATETYPE_VALUES = 0;
constexpr uint8_t BOT_UPDATETYPE_CREATE_OBJECT = 1;
constexpr uint8_t BOT_UPDATETYPE_CREATE_OBJECT2 = 2;

// Fragment bits of a values block's type mask.
constexpr uint32_t BOT_WIRETYPE_OBJECT = 1u << 0;
constexpr uint32_t BOT_WIRETYPE_UNIT = 1u << 5;

// Steps over the spline section of a unit's create block: the spline id and destination, then,
// if the unit is on a path, its flags, timing, facing and nodes. The optional extras the server
// never sends are not understood.
bool SkipCreateSpline(ByteBuffer& data)
{
    if (data.size() - data.rpos() < 4 * sizeof(uint32_t) + 1)
        return false;
    uint32_t splineId;
    float destination[3];
    data >> splineId >> destination[0] >> destination[1] >> destination[2];

    bool const hasSplineMove = data.ReadBit();
    data.ResetBitPos();
    if (!hasSplineMove)
        return true;

    if (data.size() - data.rpos() < 3 * sizeof(uint32_t) + 2 * sizeof(float) + 3)
        return false;
    uint32_t flags, duration;
    int32_t elapsed;
    float durationModifier, nextDurationModifier;
    data >> flags >> elapsed >> duration >> durationModifier >> nextDurationModifier;

    uint32_t const face = data.ReadBits(2);
    bool const hasFadeObjectTime = data.ReadBit();
    uint32_t const nodeCount = data.ReadBits(16);
    bool const hasFilter = data.ReadBit();
    bool const hasSpellEffect = data.ReadBit();
    bool const hasJump = data.ReadBit();
    bool const hasAnimTier = data.ReadBit();
    bool const hasUnknown = data.ReadBit();
    data.ResetBitPos();
    if (hasFadeObjectTime || hasFilter || hasSpellEffect || hasJump || hasAnimTier || hasUnknown)
        return false;

    switch (face)
    {
        case 1: // facing a spot
        {
            if (data.size() - data.rpos() < 3 * sizeof(float))
                return false;
            float spot[3];
            data >> spot[0] >> spot[1] >> spot[2];
            break;
        }
        case 2: // facing a unit
        {
            ClassicGuid target;
            if (!target.Read(data))
                return false;
            break;
        }
        case 3: // facing an angle
        {
            if (data.size() - data.rpos() < sizeof(float))
                return false;
            float angle;
            data >> angle;
            break;
        }
        default:
            break;
    }

    if (data.size() - data.rpos() < uint64_t(nodeCount) * 3 * sizeof(float))
        return false;
    for (uint32_t i = 0; i < nodeCount; ++i)
    {
        float node[3];
        data >> node[0] >> node[1] >> node[2];
    }
    return true;
}

// Mirrors Objects/Updates/UpdateObjectWriter.cpp's BuildMovementUpdate bit for bit, but only
// understands the shapes it currently produces (a standing unit/player, a stationary object's
// position and a game object's packed rotation, the active player's own action-button array): an
// unexpected true bit means the wire moved and this needs updating too.
bool ParseMovementFrame(ByteBuffer& data, float& x, float& y, float& z, float& o)
{
    data.ReadBit(); // HasPositionFragment
    bool const noBirthAnim = data.ReadBit();
    bool const enablePortals = data.ReadBit();
    data.ReadBit(); // PlayHoverAnim
    bool const movementUpdate = data.ReadBit();
    bool const movementTransport = data.ReadBit();
    bool const stationary = data.ReadBit();
    bool const combatVictim = data.ReadBit();
    bool const serverTime = data.ReadBit();
    bool const vehicle = data.ReadBit();
    bool const animKit = data.ReadBit();
    bool const rotation = data.ReadBit();
    bool const areaTrigger = data.ReadBit();
    bool const gameObject = data.ReadBit();
    bool const smoothPhasing = data.ReadBit();
    data.ReadBit(); // ThisIsYou
    bool const sceneObject = data.ReadBit();
    bool const activePlayer = data.ReadBit();
    bool const conversation = data.ReadBit();
    data.ResetBitPos();

    if (noBirthAnim || enablePortals || movementTransport || combatVictim || serverTime || vehicle ||
        animKit || areaTrigger || gameObject || smoothPhasing || sceneObject || conversation)
        return false;

    if (movementUpdate)
    {
        ClassicGuid mover;
        if (!mover.Read(data))
            return false;

        uint32_t moveFlags, extraFlags, extraFlags2, moveTime;
        data >> moveFlags >> extraFlags >> extraFlags2 >> moveTime;
        data >> x >> y >> z >> o;

        float pitch, stepUpStartElevation;
        data >> pitch >> stepUpStartElevation;

        uint32_t removeForcesCount, moveIndex;
        data >> removeForcesCount >> moveIndex;

        data.ReadBit(); // HasStandingOnGameObjectGUID
        bool const hasTransport = data.ReadBit();
        bool const hasFall = data.ReadBit();
        bool const hasSpline = data.ReadBit();
        data.ReadBit(); // HeightChangeFailed
        data.ReadBit(); // RemoteTimeValid
        bool const hasInertia = data.ReadBit();
        bool const hasAdvFlying = data.ReadBit();
        data.ResetBitPos();

        if (hasTransport || hasFall || hasInertia || hasAdvFlying)
            return false;

        for (int i = 0; i < 9; ++i) // speed floats
        {
            float speed;
            data >> speed;
        }

        uint32_t movementForcesCount;
        float movementForcesMagnitude;
        data >> movementForcesCount >> movementForcesMagnitude;

        for (int i = 0; i < 17; ++i) // advFlying tuning floats
        {
            float tuning;
            data >> tuning;
        }

        bool const trailingHasSpline = data.ReadBit();
        data.ResetBitPos();
        if (trailingHasSpline != hasSpline)
            return false;
        if (trailingHasSpline && !SkipCreateSpline(data))
            return false;
    }

    uint32_t pauseTimesCount;
    data >> pauseTimesCount;
    if (pauseTimesCount != 0)
        return false;

    if (stationary)
        data >> x >> y >> z >> o;

    if (rotation)
    {
        uint64_t packedRotation;
        data >> packedRotation;
    }

    if (activePlayer)
    {
        bool const hasSceneInstanceIDs = data.ReadBit();
        bool const hasRuneState = data.ReadBit();
        bool const hasActionButtons = data.ReadBit();
        data.ResetBitPos();

        if (hasSceneInstanceIDs || hasRuneState)
            return false;

        if (hasActionButtons)
        {
            for (uint32_t i = 0; i < 180; ++i)
            {
                uint32_t button;
                data >> button;
            }
        }
    }

    return true;
}

// Reads the header of a values block and, when present, the ObjectData fields and the unit's
// health. Stops at the first field whose size depends on data this reader doesn't track.
void ParseValuesPayload(ByteBuffer& data, std::size_t end, BotKnownObject& obj)
{
    auto remaining = [&]() { return data.rpos() <= end ? end - data.rpos() : 0; };

    if (remaining() < 3 + sizeof(uint32_t))
        return;
    uint8_t isOwner, idsChanged, contentsChangedMask;
    uint32_t typeMask;
    data >> isOwner >> idsChanged >> contentsChangedMask >> typeMask;
    obj.LastFragmentMask = contentsChangedMask;

    if (typeMask & BOT_WIRETYPE_OBJECT)
    {
        if (remaining() < 1)
            return;
        uint32_t const mask = data.ReadBits(4);
        data.ResetBitPos();
        if (mask & 0x1)
        {
            if (mask & 0x2)
            {
                if (remaining() < sizeof(int32_t))
                    return;
                int32_t entry;
                data >> entry;
            }
            if (mask & 0x4)
            {
                if (remaining() < sizeof(uint32_t))
                    return;
                uint32_t dynamicFlags;
                data >> dynamicFlags;
                obj.DynamicFlags = dynamicFlags;
            }
            if (mask & 0x8)
            {
                if (remaining() < sizeof(float))
                    return;
                float scale;
                data >> scale;
            }
        }
    }

    if (!(typeMask & BOT_WIRETYPE_UNIT))
        return;

    if (remaining() < 1)
        return;
    uint32_t const blocksMask = data.ReadBits(8);
    uint32_t blocks[8] = {};
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (!(blocksMask & (1u << i)))
            continue;
        if (remaining() < sizeof(uint32_t))
            return;
        blocks[i] = data.ReadBits(32);
    }
    data.ResetBitPos();

    // Bits 1-4 are the world-effect and passive-spell lists, which come before health.
    if (!(blocks[0] & 0x1) || (blocks[0] & 0x1E))
        return;
    if (blocks[0] & 0x20)
    {
        if (remaining() < sizeof(int64_t))
            return;
        int64_t health;
        data >> health;
        obj.Health = health;
    }
    if (blocks[0] & 0x40)
    {
        if (remaining() < sizeof(int64_t))
            return;
        int64_t maxHealth;
        data >> maxHealth;
        obj.MaxHealth = maxHealth;
    }
}
}

void BotWorldView::OnUpdateObject(UpdateObject const& packet)
{
    for (ClassicGuid const& gone : packet.DestroyGUIDs)
        Remove(gone);
    for (ClassicGuid const& gone : packet.OutOfRangeGUIDs)
        Remove(gone);

    ByteBuffer data;
    data.append(packet.BlockData);
    data.rpos(0);

    for (uint32_t i = 0; i < packet.BlockCount; ++i)
    {
        if (data.size() - data.rpos() < 1)
            break;

        uint8_t updateType;
        data >> updateType;

        ClassicGuid guid;
        if (!guid.Read(data))
            break;

        if (updateType == BOT_UPDATETYPE_VALUES)
        {
            if (data.size() - data.rpos() < sizeof(uint32_t))
                break;
            uint32_t size;
            data >> size;
            if (data.size() - data.rpos() < size)
                break;
            std::size_t const end = data.rpos() + size;
            for (BotKnownObject& existing : m_objects)
            {
                if (existing.Guid != guid)
                    continue;
                ++existing.ValuesUpdates;
                ParseValuesPayload(data, end, existing);
            }
            data.rpos(end);
            continue;
        }

        if (updateType != BOT_UPDATETYPE_CREATE_OBJECT && updateType != BOT_UPDATETYPE_CREATE_OBJECT2)
            break; // a block shape this reader doesn't parse yet

        if (data.size() - data.rpos() < 1)
            break;
        uint8_t objectType;
        data >> objectType;

        float x = 0.0f, y = 0.0f, z = 0.0f, o = 0.0f;
        if (!ParseMovementFrame(data, x, y, z, o))
            break;

        if (data.size() - data.rpos() < sizeof(uint32_t))
            break;
        uint32_t size;
        data >> size;
        if (data.size() - data.rpos() < size)
            break;
        data.rpos(data.rpos() + size); // ObjectData/UnitData/... payload: not parsed yet

        BotKnownObject known;
        known.Guid = guid;
        known.ObjectType = objectType;
        known.X = x;
        known.Y = y;
        known.Z = z;
        known.O = o;

        bool replaced = false;
        for (BotKnownObject& existing : m_objects)
        {
            if (existing.Guid == guid)
            {
                existing = known;
                replaced = true;
                break;
            }
        }
        if (!replaced)
            m_objects.push_back(known);
    }
}

void BotWorldView::OnMoveUpdate(ClassicMovementInfo const& status)
{
    for (BotKnownObject& obj : m_objects)
    {
        if (obj.Guid != status.Guid)
            continue;
        obj.X = status.X;
        obj.Y = status.Y;
        obj.Z = status.Z;
        obj.O = status.O;
        ++obj.MoveUpdates;
        return;
    }
}

void BotWorldView::OnMonsterMove(MonsterMove const& move)
{
    for (BotKnownObject& obj : m_objects)
    {
        if (obj.Guid != move.Mover)
            continue;
        if (move.Points.empty())
        {
            obj.X = move.Start.X;
            obj.Y = move.Start.Y;
            obj.Z = move.Start.Z;
        }
        else
        {
            obj.X = move.Points.back().X;
            obj.Y = move.Points.back().Y;
            obj.Z = move.Points.back().Z;
        }
        ++obj.MonsterMoves;
        return;
    }
}

void BotWorldView::Remove(ClassicGuid const& guid)
{
    std::erase_if(m_objects, [&guid](BotKnownObject const& obj) { return obj.Guid == guid; });
}

std::optional<BotKnownObject> BotWorldView::Get(ClassicGuid const& guid) const
{
    for (BotKnownObject const& obj : m_objects)
        if (obj.Guid == guid)
            return obj;
    return std::nullopt;
}

void BotWorldView::OnAuraUpdate(AuraUpdate const& update)
{
    for (BotKnownObject& obj : m_objects)
    {
        if (obj.Guid != update.UnitGuid)
            continue;

        if (update.UpdateAll)
            obj.Auras.clear();
        for (AuraInfo const& info : update.Auras)
        {
            std::erase_if(obj.Auras, [&info](BotAura const& aura) { return aura.Slot == info.Slot; });
            if (info.AuraData)
                obj.Auras.push_back({ info.Slot, info.AuraData->SpellId, info.AuraData->Flags, info.AuraData->Applications });
        }
        return;
    }
}
