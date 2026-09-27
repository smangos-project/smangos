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

    class PlaySpellVisual final : public ServerPacket
    {
    public:
        ObjectGuid casterGuid;
        uint32 spellVisualId = 0; // SpellVisualKit.dbc index

        explicit PlaySpellVisual() : ServerPacket(SMSG_PLAY_SPELL_VISUAL) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

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


    // SMSG_SET_FLAT_SPELL_MODIFIER / SMSG_SET_PCT_SPELL_MODIFIER
    class SetSpellModifier final : public ServerPacket
    {
    public:
        uint8 effectIndex = 0; // which spell effect index (0-63)
        uint8 modOp = 0;       // SpellModOp enum value
        int32 value = 0;       // flat or percent modifier value

        explicit SetSpellModifier(uint32 opcode) : ServerPacket(opcode) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_11_2
    class SpellUpdateChainTargets final : public ServerPacket
    {
    public:
        ObjectGuid casterGuid;
        uint32 spellId = 0;
        std::vector<ObjectGuid> targets;

        explicit SpellUpdateChainTargets() : ServerPacket(SMSG_SPELL_UPDATE_CHAIN_TARGETS) {}
        size_t EstimateFinalSize() const override;
        void AppendBodyTo(ByteBuffer& buffer) const override;
    };
#endif

}} // namespace WorldPackets::Spell

#endif // MANGOS_PACKETS_SPELL_H
