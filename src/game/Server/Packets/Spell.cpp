#include "Spell.h"
#include "Spells/Spell.h"
#include "SpellDefines.h"

void WorldPackets::Spell::OpenItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> bagIndex;
    recv_data >> slot;
}

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

