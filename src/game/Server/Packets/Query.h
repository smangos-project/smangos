#ifndef MANGOS_PACKETS_QUERY_H
#define MANGOS_PACKETS_QUERY_H

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets { namespace Query
{
    class ItemNameQuery final : public ClientPacket
    {
    public:
        uint32 itemId = 0;

        explicit ItemNameQuery() : ClientPacket(CMSG_ITEM_NAME_QUERY) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

}} // namespace WorldPackets::Query

#endif // MANGOS_PACKETS_QUERY_H
