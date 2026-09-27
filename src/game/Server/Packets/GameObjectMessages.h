#ifndef MANGOS_PACKETS_GAMEOBJECTMESSAGES_H
#define MANGOS_PACKETS_GAMEOBJECTMESSAGES_H

// Game object animation cues and the fishing-bobber miss signals. Light like
// Packets/CombatMessages.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"

// A game object's custom animation (traps, the eternal flame, a fishing bobber's bite splash).
// Field order per TrinityCore's GameObjectCustomAnim::Write() (GameObjectPackets.cpp). Also stands
// in for vanilla's separate SMSG_GAMEOBJECT_SPAWN_ANIM/DESPAWN_ANIM, which have no modern opcode at
// all: TrinityCore's client animates a game object's appearance purely from its UPDATE_OBJECT
// create block, so PlayAsDespawn=true is the closest replacement for the old despawn-anim send.
struct GameObjectCustomAnim
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_GAME_OBJECT_CUSTOM_ANIM;

    ClassicGuid ObjectGuid;
    uint32 CustomAnim = 0;
    bool PlayAsDespawn = false;

    void Write(ByteBuffer& data) const
    {
        ObjectGuid.Write(data);
        data << CustomAnim;
        data.WriteBit(PlayAsDespawn);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!ObjectGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(uint32) + 1)
            return false;
        data >> CustomAnim;
        PlayAsDespawn = data.ReadBit();
        data.ResetBitPos();
        return true;
    }
};

// Empty bodies; sent from a fishing bobber's GAMEOBJECT_TYPE_FISHINGNODE update loop.
struct FishNotHooked
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_FISH_NOT_HOOKED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

struct FishEscaped
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_FISH_ESCAPED;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

#endif // MANGOS_PACKETS_GAMEOBJECTMESSAGES_H
