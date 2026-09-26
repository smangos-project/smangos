#ifndef MANGOS_PACKETS_WORLDTRANSFER_H
#define MANGOS_PACKETS_WORLDTRANSFER_H

// The far-teleport (map change) wire: transfer packets and the client's world-port response. Light
// like Packets/PlayerSetup.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include <optional>

// Shown as a load screen while the destination map is prepared.
struct TransferPending
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRANSFER_PENDING;

    int32 MapId = -1;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    std::optional<uint32> ShipEntry;
    std::optional<int32> ShipOriginMapId;
    std::optional<int32> TransferSpellId;

    void Write(ByteBuffer& data) const
    {
        data << MapId << X << Y << Z;
        data.WriteBit(ShipEntry.has_value());
        data.WriteBit(TransferSpellId.has_value());
        data.FlushBits();
        if (ShipEntry)
            data << *ShipEntry << int32(ShipOriginMapId.value_or(-1));
        if (TransferSpellId)
            data << *TransferSpellId;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 4 * sizeof(int32) + 1)
            return false;
        data >> MapId >> X >> Y >> Z;
        bool const hasShip = data.ReadBit();
        bool const hasSpell = data.ReadBit();
        data.ResetBitPos();

        ShipEntry.reset();
        ShipOriginMapId.reset();
        if (hasShip)
        {
            if (data.size() - data.rpos() < 2 * sizeof(int32))
                return false;
            uint32 entry;
            int32 originMapId;
            data >> entry >> originMapId;
            ShipEntry = entry;
            ShipOriginMapId = originMapId;
        }

        TransferSpellId.reset();
        if (hasSpell)
        {
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            int32 spellId;
            data >> spellId;
            TransferSpellId = spellId;
        }
        return true;
    }
};

// Rejects a teleport already accepted client-side (SMSG_TRANSFER_PENDING already shown), or one the
// server never allowed to begin with.
struct TransferAborted
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TRANSFER_ABORTED;

    uint32 MapId = 0;
    uint8 Arg = 0;
    int32 MapDifficultyXConditionId = 0;
    uint32 Reason = 0;

    void Write(ByteBuffer& data) const
    {
        data << MapId << Arg << MapDifficultyXConditionId;
        data.WriteBits(Reason, 6);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(uint32) + sizeof(uint8) + sizeof(int32) + 1)
            return false;
        data >> MapId >> Arg >> MapDifficultyXConditionId;
        Reason = data.ReadBits(6);
        data.ResetBitPos();
        return true;
    }
};

// Tells the client its transfer landed and it can finish loading into the destination map.
struct NewWorld
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_NEW_WORLD;

    int32 MapId = 0;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    float O = 0.0f;
    uint32 Reason = 0;
    float OffsetX = 0.0f;
    float OffsetY = 0.0f;
    float OffsetZ = 0.0f;
    int32 Counter = 0;

    void Write(ByteBuffer& data) const
    {
        data << MapId;
        data << X << Y << Z << O;
        data << int32(-1) << int32(-1); // unused
        data << Reason;
        data << OffsetX << OffsetY << OffsetZ;
        data << Counter;
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 12 * sizeof(int32))
            return false;
        int32 unused1, unused2;
        data >> MapId >> X >> Y >> Z >> O >> unused1 >> unused2 >> Reason >> OffsetX >> OffsetY >> OffsetZ >> Counter;
        return true;
    }
};

// The client's only reply to a far teleport: an empty ack that the load screen finished.
struct WorldPortResponse
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_WORLD_PORT_RESPONSE;

    void Write(ByteBuffer&) const {}
    bool Read(ByteBuffer&) { return true; }
};

#endif // MANGOS_PACKETS_WORLDTRANSFER_H
