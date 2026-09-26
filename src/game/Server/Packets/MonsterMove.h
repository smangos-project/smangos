#ifndef MANGOS_PACKETS_MONSTERMOVE_H
#define MANGOS_PACKETS_MONSTERMOVE_H

// A server-driven unit's movement along a path. Light like Packets/PlayerSetup.h, so RealBots can
// include it.

#include "ByteBuffer.h"
#include "ClassicOpcodes.h"
#include "Packets/Classic/Guid.h"
#include <vector>

// The modern spline flag bits the server sets (the rest of the word is left to the client).
enum ClassicSplineFlag : uint32
{
    CLASSIC_SPLINEFLAG_DONE             = 0x00000020,
    CLASSIC_SPLINEFLAG_FALLING          = 0x00000040,
    CLASSIC_SPLINEFLAG_NO_SPLINE        = 0x00000080,
    CLASSIC_SPLINEFLAG_FLYING           = 0x00000200,
    CLASSIC_SPLINEFLAG_CATMULLROM       = 0x00000800,
    CLASSIC_SPLINEFLAG_CYCLIC           = 0x00001000,
    CLASSIC_SPLINEFLAG_ENTER_CYCLE      = 0x00002000,
    CLASSIC_SPLINEFLAG_FROZEN           = 0x00004000,
    CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH = 0x00400000,
};

enum ClassicMonsterMoveFacing : uint8
{
    CLASSIC_MONSTERMOVE_FACING_NONE     = 0,
    CLASSIC_MONSTERMOVE_FACING_SPOT     = 1,
    CLASSIC_MONSTERMOVE_FACING_TARGET   = 2,
    CLASSIC_MONSTERMOVE_FACING_ANGLE    = 3,
};

struct MonsterMove
{
    static constexpr ClassicOpcodes Opcode = CLASSIC_SMSG_ON_MONSTER_MOVE;

    struct Point
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    ClassicGuid Mover;
    // Where the mover is when the movement starts.
    Point Start;

    uint32 SplineId = 0;
    bool CrzTeleport = false;
    uint8 StopDistanceTolerance = 0;

    uint32 Flags = 0;
    int32 Elapsed = 0;
    uint32 MoveTime = 0;
    uint32 FadeObjectTime = 0;
    uint8 Mode = 0;
    ClassicGuid Transport;
    int8 VehicleSeat = -1;
    uint8 Face = CLASSIC_MONSTERMOVE_FACING_NONE;
    bool VehicleExitVoluntary = false;
    bool Interpolate = false;

    Point FaceSpot;
    float FaceDirection = 0.0f;
    ClassicGuid FaceTarget;

    // The path after Start. With CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH every point is here;
    // otherwise only the last is, and PackedDeltas holds the rest, one word per point.
    std::vector<Point> Points;
    std::vector<uint32> PackedDeltas;

    void Write(ByteBuffer& data) const
    {
        Mover.Write(data);
        data << Start.X << Start.Y << Start.Z;

        data << SplineId;
        data.WriteBit(CrzTeleport);
        data.WriteBits(StopDistanceTolerance, 3);

        data << Flags << Elapsed << MoveTime << FadeObjectTime << Mode;
        Transport.Write(data);
        data << VehicleSeat;
        data.WriteBits(Face, 2);
        data.WriteBits(uint32(Points.size()), 16);
        data.WriteBit(VehicleExitVoluntary);
        data.WriteBit(Interpolate);
        data.WriteBits(uint32(PackedDeltas.size()), 16);
        data.WriteBit(false); // spline filter
        data.WriteBit(false); // spell effect extra data
        data.WriteBit(false); // jump extra data
        data.WriteBit(false); // animation tier transition
        data.FlushBits();

        switch (Face)
        {
            case CLASSIC_MONSTERMOVE_FACING_SPOT:
                data << FaceSpot.X << FaceSpot.Y << FaceSpot.Z;
                break;
            case CLASSIC_MONSTERMOVE_FACING_TARGET:
                data << FaceDirection;
                FaceTarget.Write(data);
                break;
            case CLASSIC_MONSTERMOVE_FACING_ANGLE:
                data << FaceDirection;
                break;
            default:
                break;
        }

        for (Point const& point : Points)
            data << point.X << point.Y << point.Z;
        for (uint32 packed : PackedDeltas)
            data << packed;
    }

    bool Read(ByteBuffer& data)
    {
        if (!Mover.Read(data))
            return false;
        if (data.size() - data.rpos() < 4 * sizeof(float) + 1)
            return false;
        data >> Start.X >> Start.Y >> Start.Z;

        data >> SplineId;
        CrzTeleport = data.ReadBit();
        StopDistanceTolerance = uint8(data.ReadBits(3));
        data.ResetBitPos();

        if (data.size() - data.rpos() < 4 * sizeof(uint32) + sizeof(uint8))
            return false;
        data >> Flags >> Elapsed >> MoveTime >> FadeObjectTime >> Mode;
        if (!Transport.Read(data))
            return false;
        if (data.size() - data.rpos() < sizeof(int8) + 5)
            return false;
        data >> VehicleSeat;
        Face = uint8(data.ReadBits(2));
        uint32 const pointCount = data.ReadBits(16);
        VehicleExitVoluntary = data.ReadBit();
        Interpolate = data.ReadBit();
        uint32 const packedCount = data.ReadBits(16);
        bool const hasFilter = data.ReadBit();
        bool const hasSpellEffect = data.ReadBit();
        bool const hasJump = data.ReadBit();
        bool const hasAnimTier = data.ReadBit();
        data.ResetBitPos();
        if (hasFilter || hasSpellEffect || hasJump || hasAnimTier)
            return false;

        FaceTarget = ClassicGuid();
        switch (Face)
        {
            case CLASSIC_MONSTERMOVE_FACING_SPOT:
                if (data.size() - data.rpos() < 3 * sizeof(float))
                    return false;
                data >> FaceSpot.X >> FaceSpot.Y >> FaceSpot.Z;
                break;
            case CLASSIC_MONSTERMOVE_FACING_TARGET:
                if (data.size() - data.rpos() < sizeof(float))
                    return false;
                data >> FaceDirection;
                if (!FaceTarget.Read(data))
                    return false;
                break;
            case CLASSIC_MONSTERMOVE_FACING_ANGLE:
                if (data.size() - data.rpos() < sizeof(float))
                    return false;
                data >> FaceDirection;
                break;
            default:
                break;
        }

        if (data.size() - data.rpos() < uint64(pointCount) * 3 * sizeof(float) + uint64(packedCount) * sizeof(uint32))
            return false;
        Points.clear();
        for (uint32 i = 0; i < pointCount; ++i)
        {
            Point point;
            data >> point.X >> point.Y >> point.Z;
            Points.push_back(point);
        }
        PackedDeltas.clear();
        for (uint32 i = 0; i < packedCount; ++i)
        {
            uint32 packed;
            data >> packed;
            PackedDeltas.push_back(packed);
        }
        return true;
    }
};

#endif // MANGOS_PACKETS_MONSTERMOVE_H
