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

size_t WorldPackets::Spell::SpellCooldown::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           cooldownEntries.size() * (sizeof(SpellCooldownEntry::spellId) +
                                     sizeof(uint32) /*cooldown*/);
}

void WorldPackets::Spell::SpellCooldown::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    for (auto const& cooldownEntry : cooldownEntries)
    {
        buffer << cooldownEntry.spellId;
        buffer << static_cast<uint32>(cooldownEntry.cooldown.count());
    }
}

size_t WorldPackets::Spell::ClearCooldown::EstimateFinalSize() const
{
    return sizeof(spellId) +
           sizeof(targetGuid);
}

void WorldPackets::Spell::ClearCooldown::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << spellId;
    buffer << targetGuid;
}

size_t WorldPackets::Spell::CooldownCheat::EstimateFinalSize() const
{
    return sizeof(targetGuid);
}

void WorldPackets::Spell::CooldownCheat::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << targetGuid;
}

size_t WorldPackets::Spell::CooldownEvent::EstimateFinalSize() const
{
    return sizeof(spellId) +
           sizeof(casterGuid);
}

void WorldPackets::Spell::CooldownEvent::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << spellId;
    buffer << casterGuid;
}

size_t WorldPackets::Spell::SupercededSpell::EstimateFinalSize() const
{
    return sizeof(uint16) + /*oldSpellId*/
           sizeof(uint16) /*newSpellId*/;
}

void WorldPackets::Spell::SupercededSpell::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << static_cast<uint16>(oldSpellId);
    buffer << static_cast<uint16>(newSpellId);
}

size_t WorldPackets::Spell::LearnedSpell::EstimateFinalSize() const
{
    return sizeof(uint16) + /*spellId*/
           sizeof(int16) /*actionBarSlot*/;
}

void WorldPackets::Spell::LearnedSpell::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << static_cast<uint16>(spellId);
    buffer << static_cast<int16>(actionBarSlot); // not used
}

size_t WorldPackets::Spell::RemovedSpell::EstimateFinalSize() const
{
    return sizeof(uint16) /*spellId*/;
}

void WorldPackets::Spell::RemovedSpell::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << static_cast<uint16>(spellId);
}

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

