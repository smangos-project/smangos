#include "BotWorldView.h"
#include "Packets/ObjectUpdate.h"
#include "ByteBuffer.h"

namespace
{
// Named separately from Objects/UpdateData.h's ObjectUpdateType (visible here too when RealBots is
// compiled into `game`): this reader must not depend on the server's own enum.
constexpr uint8_t BOT_UPDATETYPE_VALUES = 0;
constexpr uint8_t BOT_UPDATETYPE_CREATE_OBJECT = 2;
constexpr uint8_t BOT_UPDATETYPE_CREATE_OBJECT2 = 3;

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

        if (hasTransport || hasFall || hasSpline || hasInertia || hasAdvFlying)
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
        if (trailingHasSpline)
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
            data.rpos(data.rpos() + size);
            for (BotKnownObject& existing : m_objects)
                if (existing.Guid == guid)
                    ++existing.ValuesUpdates;
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
