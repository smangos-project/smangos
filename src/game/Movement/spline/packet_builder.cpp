/*
 * Copyright (C) 2005-2013 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "packet_builder.h"
#include "MoveSpline.h"
#include "MonsterMovePacking.h"
#include "Unit.h"
#include "Packets/Classic/GuidMapping.h"
#include "Packets/MonsterMove.h"
#include "World.h"

namespace Movement
{
namespace
{
uint32 ToModernSplineFlags(MoveSplineFlag const& flags, bool allowUncompressedPath)
{
    uint32 modern = 0;
    if (flags.falling)
        modern |= CLASSIC_SPLINEFLAG_FALLING;
    if (flags.flying)
    {
        modern |= CLASSIC_SPLINEFLAG_FLYING | CLASSIC_SPLINEFLAG_CATMULLROM;
        if (allowUncompressedPath)
            modern |= CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH;
    }
    if (flags.no_spline)
        modern |= CLASSIC_SPLINEFLAG_NO_SPLINE;
    if (flags.cyclic)
        modern |= CLASSIC_SPLINEFLAG_CYCLIC | CLASSIC_SPLINEFLAG_ENTER_CYCLE;
    if (flags.frozen)
        modern |= CLASSIC_SPLINEFLAG_FROZEN;
    return modern;
}
}

void PacketBuilder::BuildMonsterMove(Unit const& unit, MoveSpline const& move_spline, MonsterMove& packet)
{
    packet.Mover = ClassicGuidMapping::ToClassic(unit.GetObjectGuid(), realmID);

    Spline<int32> const& spline = move_spline.spline;
    int32 const first = int32(spline.first());
    int32 const last = int32(spline.last());
    int32 const span = last - first;

    Vector3 const& start = spline.getPoint(first);
    packet.Start = { start.x, start.y, start.z };
    packet.SplineId = move_spline.GetId();

    MoveSplineFlag const flags = move_spline.splineflags;
    packet.Flags = ToModernSplineFlags(flags, true);
    packet.Elapsed = move_spline.timePassed();
    packet.MoveTime = move_spline.Duration();

    switch (flags & MoveSplineFlag::Mask_Final_Facing)
    {
        case MoveSplineFlag::Final_Target:
            packet.Face = CLASSIC_MONSTERMOVE_FACING_TARGET;
            packet.FaceTarget = ClassicGuidMapping::ToClassic(ObjectGuid(move_spline.facing.target), realmID);
            break;
        case MoveSplineFlag::Final_Angle:
            packet.Face = CLASSIC_MONSTERMOVE_FACING_ANGLE;
            packet.FaceDirection = G3D::wrap(move_spline.facing.angle, 0.0f, float(G3D::twoPi()));
            break;
        case MoveSplineFlag::Final_Point:
            packet.Face = CLASSIC_MONSTERMOVE_FACING_SPOT;
            packet.FaceSpot = { move_spline.facing.f.x, move_spline.facing.f.y, move_spline.facing.f.z };
            break;
        default:
            break;
    }

    if (span <= 0)
        return;

    std::vector<Vector3> pathAfterStart;
    pathAfterStart.reserve(span);
    for (int32 i = first + 1; i <= last; ++i)
        pathAfterStart.push_back(spline.getPoint(i));

    PackMonsterMovePath(start, pathAfterStart, (packet.Flags & CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH) != 0, packet);
}

void PacketBuilder::WriteCreateSpline(MoveSpline const& move_spline, ByteBuffer& data)
{
    data << uint32(move_spline.GetId());

    if (move_spline.isCyclic())
        data << float(0.0f) << float(0.0f) << float(0.0f);
    else
    {
        Vector3 const destination = move_spline.FinalDestination();
        data << destination.x << destination.y << destination.z;
    }

    // The path's nodes, padding included, are what the client interpolates over here; the
    // compressed form monster moves use is not accepted in a create.
    auto const& path = move_spline.getPath();
    bool const hasSplineMove = !move_spline.Finalized() && !path.empty();
    data.WriteBit(hasSplineMove);
    data.FlushBits();
    if (!hasSplineMove)
        return;

    MoveSplineFlag const flags = move_spline.splineflags;
    data << uint32(ToModernSplineFlags(flags, false));
    data << int32(move_spline.timePassed());
    data << uint32(move_spline.Duration());
    data << float(1.0f);                                        // duration modifier
    data << float(1.0f);                                        // next duration modifier

    uint8 face = CLASSIC_MONSTERMOVE_FACING_NONE;
    switch (flags & MoveSplineFlag::Mask_Final_Facing)
    {
        case MoveSplineFlag::Final_Point:   face = CLASSIC_MONSTERMOVE_FACING_SPOT; break;
        case MoveSplineFlag::Final_Target:  face = CLASSIC_MONSTERMOVE_FACING_TARGET; break;
        case MoveSplineFlag::Final_Angle:   face = CLASSIC_MONSTERMOVE_FACING_ANGLE; break;
        default:                            break;
    }

    data.WriteBits(face, 2);
    data.WriteBit(false);                                       // fade object time
    data.WriteBits(uint32(path.size()), 16);
    data.WriteBit(false);                                       // spline filter
    data.WriteBit(false);                                       // spell effect extra data
    data.WriteBit(false);                                       // jump extra data
    data.WriteBit(false);                                       // animation tier transition
    data.WriteBit(false);                                       // unknown
    data.FlushBits();

    switch (face)
    {
        case CLASSIC_MONSTERMOVE_FACING_SPOT:
            data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
            break;
        case CLASSIC_MONSTERMOVE_FACING_TARGET:
            ClassicGuidMapping::ToClassic(ObjectGuid(move_spline.facing.target), realmID).Write(data);
            break;
        case CLASSIC_MONSTERMOVE_FACING_ANGLE:
            data << G3D::wrap(move_spline.facing.angle, 0.0f, float(G3D::twoPi()));
            break;
        default:
            break;
    }

    for (Vector3 const& node : path)
        data << node.x << node.y << node.z;
}
}
