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
#include "Unit.h"
#include "Packets/Classic/GuidMapping.h"
#include "Packets/MonsterMove.h"
#include "World.h"

namespace Movement
{
namespace
{
uint32 ToModernSplineFlags(MoveSplineFlag const& flags)
{
    uint32 modern = CLASSIC_SPLINEFLAG_UNCOMPRESSED_PATH;
    if (flags.falling)
        modern |= CLASSIC_SPLINEFLAG_FALLING;
    if (flags.flying)
        modern |= CLASSIC_SPLINEFLAG_FLYING | CLASSIC_SPLINEFLAG_CATMULLROM;
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
    Vector3 const& start = spline.getPoint(spline.first());
    packet.Start = { start.x, start.y, start.z };
    packet.SplineId = move_spline.GetId();

    MoveSplineFlag const flags = move_spline.splineflags;
    packet.Flags = ToModernSplineFlags(flags);
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
            packet.FaceDirection = move_spline.facing.angle;
            break;
        case MoveSplineFlag::Final_Point:
            packet.Face = CLASSIC_MONSTERMOVE_FACING_SPOT;
            packet.FaceSpot = { move_spline.facing.f.x, move_spline.facing.f.y, move_spline.facing.f.z };
            break;
        default:
            break;
    }

    // The first and last control points are padding for the interpolation; a cyclic path also has
    // no separate start.
    uint32 const count = spline.getPointCount() - 3;
    uint32 const firstNode = flags.cyclic ? 1 : 2;
    for (uint32 i = 0; i < count; ++i)
    {
        Vector3 const& node = spline.getPoint(firstNode + i);
        packet.Points.push_back({ node.x, node.y, node.z });
    }
}
}
