#ifndef REALBOTS_BOTWORLDVIEW_H
#define REALBOTS_BOTWORLDVIEW_H

#include "Packets/Classic/Guid.h"
#include <cstdint>
#include <optional>
#include <vector>

struct UpdateObject;
struct ClassicMovementInfo;

// The client's object type ids, as a create block's objectType byte carries them. The receiving
// player's own object is BOT_TYPEID_ACTIVE_PLAYER, everyone else's is BOT_TYPEID_PLAYER.
constexpr uint8_t BOT_TYPEID_ITEM = 1;
constexpr uint8_t BOT_TYPEID_CONTAINER = 2;
constexpr uint8_t BOT_TYPEID_UNIT = 5;
constexpr uint8_t BOT_TYPEID_PLAYER = 6;
constexpr uint8_t BOT_TYPEID_ACTIVE_PLAYER = 7;
constexpr uint8_t BOT_TYPEID_GAMEOBJECT = 8;
constexpr uint8_t BOT_TYPEID_DYNAMICOBJECT = 9;
constexpr uint8_t BOT_TYPEID_CORPSE = 10;

// One object as this bot currently knows it, from parsing SMSG_UPDATE_OBJECT itself -- written
// from the wire layout, not by sharing the server's serializer (bot-phase-4.md's decision 5 risk
// note). A create block gives guid, type and position; values blocks are only counted.
struct BotKnownObject
{
    ClassicGuid Guid;
    uint8_t ObjectType = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;
    uint32_t ValuesUpdates = 0;
    uint32_t MoveUpdates = 0;
};

// A per-bot registry of objects seen through SMSG_UPDATE_OBJECT. A block this reader doesn't
// recognize is not parsed further; the objects already known stay as they were.
class BotWorldView
{
public:
    void OnUpdateObject(UpdateObject const& packet);
    // Another unit's movement: moves the object to where the block says it now is.
    void OnMoveUpdate(ClassicMovementInfo const& status);

    std::optional<BotKnownObject> Get(ClassicGuid const& guid) const;
    std::vector<BotKnownObject> GetAll() const { return m_objects; }

private:
    void Remove(ClassicGuid const& guid);

    std::vector<BotKnownObject> m_objects;
};

#endif
