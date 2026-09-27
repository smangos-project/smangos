#include "Query.h"

void WorldPackets::Query::ItemNameQuery::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> itemId;
    recv_data.read_skip<uint64>(); // guid, not used
}
