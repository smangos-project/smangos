#ifndef MANGOS_PACKETS_MOUNTSPECIALANIM_H
#define MANGOS_PACKETS_MOUNTSPECIALANIM_H

#include "ByteBuffer.h"
#include "Classic/Guid.h"
#include "ClassicOpcodes.h"
#include <vector>

// Client request to play a mount's special animation (the /mountspecial console command).
struct MountSpecial
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_MOUNT_SPECIAL_ANIM;

    std::vector<int32> SpellVisualKitIDs;
    int32 SequenceVariation = 0;

    void Write(ByteBuffer& data) const
    {
        data << uint32(SpellVisualKitIDs.size()) << SequenceVariation;
        for (int32 id : SpellVisualKitIDs)
            data << id;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        uint32 count;
        data >> count >> SequenceVariation;
        if (data.size() - data.rpos() < size_t(count) * sizeof(int32))
            return false;
        SpellVisualKitIDs.resize(count);
        for (int32& id : SpellVisualKitIDs)
            data >> id;
        return true;
    }
};

// Broadcast of the special animation to nearby players.
struct SpecialMountAnim
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SPECIAL_MOUNT_ANIM;

    ClassicGuid UnitGuid;
    std::vector<int32> SpellVisualKitIDs;
    int32 SequenceVariation = 0;

    void Write(ByteBuffer& data) const
    {
        UnitGuid.Write(data);
        data << uint32(SpellVisualKitIDs.size()) << SequenceVariation;
        for (int32 id : SpellVisualKitIDs)
            data << id;
    }

    bool Read(ByteBuffer& data)
    {
        if (!UnitGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 2 * sizeof(int32))
            return false;
        uint32 count;
        data >> count >> SequenceVariation;
        if (data.size() - data.rpos() < size_t(count) * sizeof(int32))
            return false;
        SpellVisualKitIDs.resize(count);
        for (int32& id : SpellVisualKitIDs)
            data >> id;
        return true;
    }
};

#endif // MANGOS_PACKETS_MOUNTSPECIALANIM_H
