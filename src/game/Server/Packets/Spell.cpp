#include "Spell.h"
#include "Spells/Spell.h"
#include "SpellDefines.h"

void WorldPackets::Spell::OpenItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> bagIndex;
    recv_data >> slot;
}

size_t WorldPackets::Spell::PlaySpellVisual::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           sizeof(spellVisualId);
}

void WorldPackets::Spell::PlaySpellVisual::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    buffer << spellVisualId;
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_8_4
size_t WorldPackets::Spell::PlaySpellImpact::EstimateFinalSize() const
{
    return sizeof(targetGuid) +
           sizeof(spellVisualId);
}

void WorldPackets::Spell::PlaySpellImpact::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << targetGuid;
    buffer << spellVisualId;
}
#endif

size_t WorldPackets::Spell::SetSpellModifier::EstimateFinalSize() const
{
    return sizeof(effectIndex) +
           sizeof(modOp) +
           sizeof(value);
}

void WorldPackets::Spell::SetSpellModifier::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << effectIndex;
    buffer << modOp;
    buffer << value;
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_11_2
size_t WorldPackets::Spell::SpellUpdateChainTargets::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           sizeof(spellId) +
           sizeof(uint32) + /*targets count*/
           targets.size() * sizeof(ObjectGuid);
}

void WorldPackets::Spell::SpellUpdateChainTargets::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    buffer << spellId;
    buffer << uint32(targets.size());
    for (auto const& target : targets)
        buffer << target;
}
#endif

