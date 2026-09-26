#ifndef MANGOS_MONSTERMOVEPACKING_H
#define MANGOS_MONSTERMOVEPACKING_H

#include "typedefs.h"
#include "Packets/MonsterMove.h"
#include <G3D/Vector3.h>
#include <vector>

namespace Movement
{
// A point packed into a word: x and y in 11 bits, z in 10, all at quarter-yard precision (the
// wire format SMSG_ON_MONSTER_MOVE's compressed path uses).
uint32 PackXYZ(Vector3 const& point);

// Fills a path's Points/PackedDeltas, given every node from just after the spline's start through
// its destination, in order. A ground path (uncompressed = false) puts the destination in Points
// and the interior nodes in PackedDeltas, as PackXYZ deltas from the start/destination midpoint. A
// flying path (uncompressed = true) copies every node into Points verbatim instead.
void PackMonsterMovePath(Vector3 const& start, std::vector<Vector3> const& pathAfterStart, bool uncompressed, MonsterMove& packet);
}

#endif
