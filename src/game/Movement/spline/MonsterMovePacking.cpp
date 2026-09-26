#include "MonsterMovePacking.h"

namespace Movement
{
uint32 PackXYZ(Vector3 const& point)
{
    uint32 packed = uint32(int32(point.x * 4.0f)) & 0x7FF;
    packed |= (uint32(int32(point.y * 4.0f)) & 0x7FF) << 11;
    packed |= (uint32(int32(point.z * 4.0f)) & 0x3FF) << 22;
    return packed;
}

void PackMonsterMovePath(Vector3 const& start, std::vector<Vector3> const& pathAfterStart, bool uncompressed, MonsterMove& packet)
{
    if (pathAfterStart.empty())
        return;

    if (uncompressed)
    {
        for (Vector3 const& node : pathAfterStart)
            packet.Points.push_back({ node.x, node.y, node.z });
        return;
    }

    Vector3 const& destination = pathAfterStart.back();
    packet.Points.push_back({ destination.x, destination.y, destination.z });

    Vector3 const midpoint = (start + destination) * 0.5f;
    for (size_t i = 0; i + 1 < pathAfterStart.size(); ++i)
        packet.PackedDeltas.push_back(PackXYZ(midpoint - pathAfterStart[i]));
}
}
