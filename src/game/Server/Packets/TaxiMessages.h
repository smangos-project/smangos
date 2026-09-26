#ifndef MANGOS_PACKETS_TAXIMESSAGES_H
#define MANGOS_PACKETS_TAXIMESSAGES_H

// The modern taxi wire: flight master interaction and starting a flight. Light like
// Packets/PlayerSetup.h, so RealBots can include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <array>
#include <optional>

enum ClassicTaxiStatus : uint8
{
    CLASSIC_TAXISTATUS_NONE         = 0,
    CLASSIC_TAXISTATUS_LEARNED      = 1,
    CLASSIC_TAXISTATUS_UNLEARNED    = 2,
    CLASSIC_TAXISTATUS_NOT_ELIGIBLE = 3,
};

// The wire's taxi node bitmask: 8 packed uint32s, same layout vmangos already stores per player.
using ClassicTaxiMask = std::array<uint8, 32>;

// Asks whether the player already knows the node nearest a flight master.
struct TaxiNodeStatusQuery
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TAXI_NODE_STATUS_QUERY;

    ClassicGuid UnitGuid;

    void Write(ByteBuffer& data) const { UnitGuid.Write(data); }
    bool Read(ByteBuffer& data) { return UnitGuid.Read(data); }
};

struct TaxiNodeStatus
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_TAXI_NODE_STATUS;

    ClassicGuid UnitGuid;
    uint8 Status = CLASSIC_TAXISTATUS_NONE;

    void Write(ByteBuffer& data) const
    {
        UnitGuid.Write(data);
        data.WriteBits(Status, 2);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (!UnitGuid.Read(data))
            return false;
        if (data.size() - data.rpos() < 1)
            return false;
        Status = uint8(data.ReadBits(2));
        data.ResetBitPos();
        return true;
    }
};

// Opens the flight map: asks for the node list, learning the current node first if new.
struct TaxiQueryAvailableNodes
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_TAXI_QUERY_AVAILABLE_NODES;

    ClassicGuid UnitGuid;

    void Write(ByteBuffer& data) const { UnitGuid.Write(data); }
    bool Read(ByteBuffer& data) { return UnitGuid.Read(data); }
};

struct EnableTaxiNode
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ENABLE_TAXI_NODE;

    ClassicGuid UnitGuid;

    void Write(ByteBuffer& data) const { UnitGuid.Write(data); }
    bool Read(ByteBuffer& data) { return UnitGuid.Read(data); }
};

// The taxi map window: which nodes the player can land at (known) and pick from (usable).
struct ShowTaxiNodes
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_SHOW_TAXI_NODES;

    std::optional<ClassicGuid> UnitGuid;
    std::optional<int32> CurrentNode;
    ClassicTaxiMask CanLandNodes{};
    ClassicTaxiMask CanUseNodes{};

    void Write(ByteBuffer& data) const
    {
        data.WriteBit(UnitGuid.has_value());
        data.FlushBits();

        data << uint32(CanLandNodes.size() / 8);
        data << uint32(CanUseNodes.size() / 8);

        if (UnitGuid)
        {
            UnitGuid->Write(data);
            data << int32(CurrentNode.value_or(0));
        }

        data.append(CanLandNodes.data(), CanLandNodes.size());
        data.append(CanUseNodes.data(), CanUseNodes.size());
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        bool const hasWindow = data.ReadBit();
        data.ResetBitPos();

        if (data.size() - data.rpos() < 2 * sizeof(uint32))
            return false;
        uint32 landBlocks, useBlocks;
        data >> landBlocks >> useBlocks;
        if (landBlocks != CanLandNodes.size() / 8 || useBlocks != CanUseNodes.size() / 8)
            return false;

        UnitGuid.reset();
        CurrentNode.reset();
        if (hasWindow)
        {
            ClassicGuid guid;
            if (!guid.Read(data))
                return false;
            if (data.size() - data.rpos() < sizeof(int32))
                return false;
            int32 node;
            data >> node;
            UnitGuid = guid;
            CurrentNode = node;
        }

        if (data.size() - data.rpos() < CanLandNodes.size() + CanUseNodes.size())
            return false;
        for (uint8& b : CanLandNodes)
            data >> b;
        for (uint8& b : CanUseNodes)
            data >> b;
        return true;
    }
};

// The client's request to start a flight: the destination node only, the source is whichever node
// is nearest the flight master it interacted with.
struct ActivateTaxi
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_CMSG_ACTIVATE_TAXI;

    ClassicGuid Vendor;
    uint32 Node = 0;
    uint32 GroundMountID = 0;
    uint32 FlyingMountID = 0;

    void Write(ByteBuffer& data) const
    {
        Vendor.Write(data);
        data << Node << GroundMountID << FlyingMountID;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Vendor.Read(data))
            return false;
        if (data.size() - data.rpos() < 3 * sizeof(uint32))
            return false;
        data >> Node >> GroundMountID >> FlyingMountID;
        return true;
    }
};

// Sent once for a newly learned node, ahead of the taxi map window or the node status update.
struct NewTaxiPath
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_NEW_TAXI_PATH;

    int32 TaxiNodesId = 0;

    void Write(ByteBuffer& data) const { data << TaxiNodesId; }
    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < sizeof(int32))
            return false;
        data >> TaxiNodesId;
        return true;
    }
};

struct ActivateTaxiReply
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ACTIVATE_TAXI_REPLY;

    uint8 Reply = 0;

    void Write(ByteBuffer& data) const
    {
        data.WriteBits(Reply, 4);
        data.FlushBits();
    }

    bool Read(ByteBuffer& data)
    {
        if (data.size() - data.rpos() < 1)
            return false;
        Reply = uint8(data.ReadBits(4));
        data.ResetBitPos();
        return true;
    }
};

#endif // MANGOS_PACKETS_TAXIMESSAGES_H
