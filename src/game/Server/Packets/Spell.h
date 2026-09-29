#ifndef MANGOS_PACKETS_SPELL_H
#define MANGOS_PACKETS_SPELL_H

#include "Duration.h"
#include "Packet.h"
#include "SpellCastTargetsInfo.h"
#include "ObjectGuid.h"
#include "SpellDefines.h"
#include "nonstd/optional.hpp"
#include <vector>

class SpellEntry;

namespace WorldPackets { namespace Spell
{
    class OpenItem final : public ClientPacket
    {
    public:
        uint8 bagIndex = 0;
        uint8 slot = 0;

        explicit OpenItem() : ClientPacket(CMSG_OPEN_ITEM) {}
        void ReadFromWorldPacket(WorldPacket& recv_data) override;
    };

    // --- Server Packets ---

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_8_4
    class PlaySpellImpact final : public ServerPacket
    {
    public:
        ObjectGuid targetGuid;
        uint32 spellVisualId = 0; // spell visual id

        explicit PlaySpellImpact() : ServerPacket(SMSG_PLAY_SPELL_IMPACT) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };
#endif


}} // namespace WorldPackets::Spell

#endif // MANGOS_PACKETS_SPELL_H
