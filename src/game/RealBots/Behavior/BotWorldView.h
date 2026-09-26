#ifndef REALBOTS_BOTWORLDVIEW_H
#define REALBOTS_BOTWORLDVIEW_H

#include "Packets/Classic/Guid.h"
#include <cstdint>
#include <optional>
#include <vector>

struct UpdateObject;

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
// note). A create block gives guid, type and position; nothing else is read yet.
struct BotKnownObject
{
    ClassicGuid Guid;
    uint8_t ObjectType = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;
};

// A per-bot registry of objects seen through SMSG_UPDATE_OBJECT create blocks. Values, destroy and
// out-of-range handling are later slices' job (bot-phase-4.md slice C6); a block this reader
// doesn't recognize yet is simply not parsed further (the objects already known stay as they were).
class BotWorldView
{
public:
    void OnUpdateObject(UpdateObject const& packet);

    std::optional<BotKnownObject> Get(ClassicGuid const& guid) const;
    std::vector<BotKnownObject> GetAll() const { return m_objects; }

private:
    std::vector<BotKnownObject> m_objects;
};

#endif
