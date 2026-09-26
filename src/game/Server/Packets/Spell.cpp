#include "Spell.h"
#include "Spells/Spell.h"
#include "SpellDefines.h"

void WorldPackets::Spell::UseItem::ReadFromWorldPacket(WorldPacket& recv_data)
{
    recv_data >> bagIndex;
    recv_data >> slot;
    recv_data >> spellSlot;
    recv_data >> targets;
}

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

size_t WorldPackets::Spell::SpellLogMiss::EstimateFinalSize() const
{
    return sizeof(spellId) +
           sizeof(casterGuid) +
           sizeof(uint8) + /*useExtendedInfo*/
           sizeof(uint32) + /*miss entries count*/
           missEntries.size() * (sizeof(SpellLogMissEntry::targetGuid) +
                                 sizeof(uint8) /*miss info*/);
}

void WorldPackets::Spell::SpellLogMiss::AppendBodyTo(ByteBuffer& buffer) const
{
    bool constexpr useExtendedInfo = false; // Seems unused in client

    buffer << spellId;
    buffer << casterGuid;
    buffer << useExtendedInfo;
    buffer << static_cast<uint32>(missEntries.size());
    for (auto const& entry : missEntries)
    {
        buffer << entry.targetGuid;
        buffer << static_cast<uint8>(entry.missInfo);
        if (useExtendedInfo)
        {
            buffer << 0.0f;
            buffer << 0.0f;
        }
    }
}

size_t WorldPackets::Spell::ProcResist::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           sizeof(targetGuid) +
           sizeof(spellId) +
           sizeof(logFormat);
}

void WorldPackets::Spell::ProcResist::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    buffer << targetGuid;
    buffer << spellId;
    buffer << logFormat;
}

size_t WorldPackets::Spell::SpellOrDamageImmune::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           sizeof(targetGuid) +
           sizeof(spellId) +
           sizeof(logFormat);
}

void WorldPackets::Spell::SpellOrDamageImmune::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    buffer << targetGuid;
    buffer << spellId;
    buffer << logFormat;
}

#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_9_4
size_t WorldPackets::Spell::SpellHealLog::EstimateFinalSize() const
{
    return sizeof(uint8) + sizeof(targetGuid) + /*packed*/
           sizeof(uint8) + sizeof(healerGuid) + /*packed*/
           sizeof(spellId) +
           sizeof(healAmount) +
           sizeof(uint8) /*isCritical*/;
}

void WorldPackets::Spell::SpellHealLog::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << targetGuid.WriteAsPacked();
    buffer << healerGuid.WriteAsPacked();
    buffer << spellId;
    buffer << healAmount;
    buffer << isCritical;
}

size_t WorldPackets::Spell::SpellEnergizeLog::EstimateFinalSize() const
{
    return sizeof(uint8) + sizeof(targetGuid) + /*packed*/
           sizeof(uint8) + sizeof(casterGuid) + /*packed*/
           sizeof(spellId) +
           sizeof(powerType) +
           sizeof(amount);
}

void WorldPackets::Spell::SpellEnergizeLog::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << targetGuid.WriteAsPacked();
    buffer << casterGuid.WriteAsPacked();
    buffer << spellId;
    buffer << powerType;
    buffer << amount;
}
#endif

size_t WorldPackets::Spell::SpellNonMeleeDamageLog::EstimateFinalSize() const
{
    size_t size = sizeof(uint8) + sizeof(targetGuid) + /*packed*/
                  sizeof(uint8) + sizeof(attackerGuid) + /*packed*/
                  sizeof(spellId) +
                  sizeof(damage) +
                  sizeof(school) +
                  sizeof(absorbedDamage) +
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_5_1
                  sizeof(resist) +
#endif
                  sizeof(uint8) + /*periodicLog*/
                  sizeof(uint8) + /*unused*/
                  sizeof(blocked) +
                  sizeof(hitTypeFlags) +
                  sizeof(uint8) /*extended data present*/;

    if (extendedData.has_value())
    {
        if (hitTypeFlags & SPELL_HIT_TYPE_CRIT_DEBUG)
            size += sizeof(ExtendedData::critRoll) +
                    sizeof(ExtendedData::critNeeded);
        if (hitTypeFlags & SPELL_HIT_TYPE_HIT_DEBUG)
            size += sizeof(ExtendedData::hitRoll) +
                    sizeof(ExtendedData::hitNeeded);
    }

    return size;
}

void WorldPackets::Spell::SpellNonMeleeDamageLog::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << targetGuid.WriteAsPackedClientBuildAware();
    buffer << attackerGuid.WriteAsPackedClientBuildAware();
    buffer << spellId;
    buffer << damage;
    buffer << school;
    buffer << absorbedDamage;
#if SUPPORTED_CLIENT_BUILD > CLIENT_BUILD_1_5_1
    buffer << resist;
#endif
    buffer << periodicLog;
    buffer << unused;
    buffer << blocked;
    buffer << hitTypeFlags;
    buffer << uint8(extendedData.has_value());

    if (extendedData.has_value())
    {
        if (hitTypeFlags & SPELL_HIT_TYPE_CRIT_DEBUG)
        {
            buffer << extendedData->critRoll;
            buffer << extendedData->critNeeded;
        }
        if (hitTypeFlags & SPELL_HIT_TYPE_HIT_DEBUG)
        {
            buffer << extendedData->hitRoll;
            buffer << extendedData->hitNeeded;
        }
    }
}

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

size_t WorldPackets::Spell::SpellLogExecute::EstimateFinalSize() const
{
    size_t size = sizeof(uint8) + sizeof(casterGuid) + /*packed*/
                  sizeof(uint32) + /*spell id*/
                  sizeof(uint32) /*effect count*/;

    for (auto const& executeLogInfo : executeLogInfos)
    {
        if (executeLogInfo.empty())
            continue;

        // The largest effect block is a target guid followed by three values.
        size += sizeof(uint32) + /*effect*/
                sizeof(uint32) + /*info count*/
                executeLogInfo.size() * (sizeof(ObjectGuid) + sizeof(uint32) * 3);
    }

    return size;
}

void WorldPackets::Spell::SpellLogExecute::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid.WriteAsPackedClientBuildAware();
    buffer << uint32(pSpellEntry->Id);

    uint32 effectCount = 0;

    for (const auto& i : executeLogInfos)
    {
        if (!i.empty())
            effectCount++;
    }

    buffer << uint32(effectCount);

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (executeLogInfos[i].empty())
            continue;

        buffer << uint32(pSpellEntry->Effect[i]);
        buffer << uint32(executeLogInfos[i].size());

        for (uint32 j = 0; j < executeLogInfos[i].size(); ++j)
        {
            auto info = executeLogInfos[i][j];
            switch (pSpellEntry->Effect[i])
            {
                case SPELL_EFFECT_POWER_DRAIN:
                    buffer << info.targetGuid;
                    buffer << info.powerDrain.amount;
                    buffer << info.powerDrain.power;
                    buffer << info.powerDrain.multiplier;
                    break;
                case SPELL_EFFECT_HEAL:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                    buffer << info.targetGuid;
                    buffer << info.heal.amount;
                    buffer << info.heal.critical;
                    break;
                case SPELL_EFFECT_ENERGIZE:
                    buffer << info.targetGuid;
                    buffer << info.energize.amount;
                    buffer << info.energize.powerType;
                    break;
                case SPELL_EFFECT_ADD_EXTRA_ATTACKS:
                    buffer << info.targetGuid;
                    buffer << info.extraAttacks.count;
                    break;
                case SPELL_EFFECT_CREATE_ITEM:
                    buffer << info.createItem.itemEntry;
                    break;
                case SPELL_EFFECT_INTERRUPT_CAST:
                    buffer << info.targetGuid;
                    buffer << info.interruptCast.spellId;
                    break;
                case SPELL_EFFECT_FEED_PET:
                    buffer << info.feedPet.itemEntry;
                    break;
                case SPELL_EFFECT_DURABILITY_DAMAGE:
                    buffer << info.targetGuid;
                    buffer << info.durabilityDamage.itemEntry;
                    buffer << info.durabilityDamage.unk;
                    break;
                case SPELL_EFFECT_INSTAKILL:
                case SPELL_EFFECT_RESURRECT:
                case SPELL_EFFECT_DISPEL:
                case SPELL_EFFECT_THREAT:
                case SPELL_EFFECT_DISTRACT:
                case SPELL_EFFECT_SANCTUARY:
                case SPELL_EFFECT_THREAT_ALL:
                case SPELL_EFFECT_DISPEL_MECHANIC:
                case SPELL_EFFECT_RESURRECT_NEW:
                case SPELL_EFFECT_ATTACK_ME:
                case SPELL_EFFECT_SKIN_PLAYER_CORPSE:
                case SPELL_EFFECT_MODIFY_THREAT_PERCENT:
                case SPELL_EFFECT_126:
                case SPELL_EFFECT_OPEN_LOCK:
                case SPELL_EFFECT_OPEN_LOCK_ITEM:
                case SPELL_EFFECT_DISMISS_PET:
                case SPELL_EFFECT_TRANS_DOOR:
                case SPELL_EFFECT_SUMMON:
                case SPELL_EFFECT_SUMMON_PET:
                case SPELL_EFFECT_SUMMON_WILD:
                case SPELL_EFFECT_SUMMON_GUARDIAN:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:
                case SPELL_EFFECT_SUMMON_POSSESSED:
                case SPELL_EFFECT_SUMMON_TOTEM:
                case SPELL_EFFECT_SUMMON_CRITTER:
                case SPELL_EFFECT_SUMMON_OBJECT_WILD:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:
                case SPELL_EFFECT_SUMMON_DEMON:
                    buffer << info.targetGuid;
                    break;
                // Every effect that can add execute log info is handled above.
                // Bailing out here would send a half written effect block.
                default:
                    break;
            }
        }
    }
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

size_t WorldPackets::Spell::ResurrectRequest::EstimateFinalSize() const
{
    return sizeof(casterGuid) +
           sizeof(uint32) + /*caster name length*/
           casterName.size() + sizeof(char) + /*null terminator*/
           sizeof(uint8) + /*sickness*/
           sizeof(uint8) /*delayed*/;
}

void WorldPackets::Spell::ResurrectRequest::AppendBodyTo(ByteBuffer& buffer) const
{
    buffer << casterGuid;
    buffer << uint32(casterName.length() + 1);
    buffer << casterName;
    buffer << uint8(sickness);
    buffer << uint8(delayed);
}

