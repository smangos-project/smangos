#ifndef MANGOS_PACKETS_UPDATE_OBJECT_H
#define MANGOS_PACKETS_UPDATE_OBJECT_H

// SMSG_UPDATE_OBJECT: a map id, the create/values/movement blocks (their shape belongs to
// Objects/Updates, this struct only carries the already-serialized bytes), and the destroyed and
// out-of-range guid lists. There is no separate destroy opcode: a destroyed object rides in this
// packet's destroy list instead.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <vector>

struct UpdateObject
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_UPDATE_OBJECT;

    uint16 MapId = 0;
    uint32 BlockCount = 0;
    std::vector<ClassicGuid> DestroyGUIDs;
    std::vector<ClassicGuid> OutOfRangeGUIDs;
    std::vector<uint8> BlockData;

    void Write(ByteBuffer& data) const
    {
        data << MapId;
        data << BlockCount;
        data.WriteBit(true); // unk, always set
        bool const hasGuidLists = !DestroyGUIDs.empty() || !OutOfRangeGUIDs.empty();
        data.WriteBit(hasGuidLists);
        data.FlushBits();

        if (hasGuidLists)
        {
            data << uint16(DestroyGUIDs.size());
            data << uint32(DestroyGUIDs.size() + OutOfRangeGUIDs.size());
            for (ClassicGuid const& guid : DestroyGUIDs)
                guid.Write(data);
            for (ClassicGuid const& guid : OutOfRangeGUIDs)
                guid.Write(data);
        }

        data << uint32(BlockData.size());
        if (!BlockData.empty())
            data.append(BlockData.data(), BlockData.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint16) + sizeof(uint32))
            return false;
        data >> MapId >> BlockCount;

        if (data.size() - data.rpos() < 1)
            return false;
        data.ReadBit(); // unk
        bool const hasGuidLists = data.ReadBit();
        data.ResetBitPos();

        DestroyGUIDs.clear();
        OutOfRangeGUIDs.clear();
        if (hasGuidLists)
        {
            if (data.size() - data.rpos() < sizeof(uint16) + sizeof(uint32))
                return false;
            uint16 destroyCount;
            uint32 totalCount;
            data >> destroyCount >> totalCount;
            if (totalCount < destroyCount)
                return false;

            for (uint16 i = 0; i < destroyCount; ++i)
            {
                ClassicGuid guid;
                if (!guid.Read(data))
                    return false;
                DestroyGUIDs.push_back(guid);
            }
            for (uint32 i = destroyCount; i < totalCount; ++i)
            {
                ClassicGuid guid;
                if (!guid.Read(data))
                    return false;
                OutOfRangeGUIDs.push_back(guid);
            }
        }

        if (data.size() - data.rpos() < sizeof(uint32))
            return false;
        uint32 blockDataSize;
        data >> blockDataSize;
        if (data.size() - data.rpos() < blockDataSize)
            return false;

        BlockData.resize(blockDataSize);
        if (blockDataSize)
            data.read(BlockData.data(), blockDataSize);
        return true;
    }
};

#endif
