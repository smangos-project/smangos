#ifndef MANGOS_PACKETS_QUERY_H
#define MANGOS_PACKETS_QUERY_H

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets { namespace Query
{
    class Whois final : public ClientPacket
    {
    public:
        std::string charName;

        explicit Whois() : ClientPacket(CMSG_WHOIS) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    class ItemNameQuery final : public ClientPacket
    {
    public:
        uint32 itemId = 0;

        explicit ItemNameQuery() : ClientPacket(CMSG_ITEM_NAME_QUERY) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

}} // namespace WorldPackets::Query

#endif // MANGOS_PACKETS_QUERY_H
