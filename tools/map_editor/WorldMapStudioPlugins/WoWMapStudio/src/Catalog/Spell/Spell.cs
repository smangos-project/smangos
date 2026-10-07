using System;
using System.Collections.Generic;
using System.Linq;

namespace WorldMapStudio.WoW;

/// <summary>One `dbc_spelleffect` row of a spell, addressed by its <see cref="EffectIndex"/> rather than list position.</summary>
public readonly record struct SpellEffectEntry
{
    public int EffectIndex { get; init; }
    public int Effect { get; init; }
    public int EffectAura { get; init; }
    public int EffectBasePoints { get; init; }
    public int EffectDieSides { get; init; }
    public float EffectRealPointsPerLevel { get; init; }
    public float EffectBonusCoefficient { get; init; }
    public float EffectAmplitude { get; init; }
    public int EffectAuraPeriod { get; init; }
    public int ImplicitTarget1 { get; init; }
    public int ImplicitTarget2 { get; init; }
    public int EffectMechanic { get; init; }
    public int EffectRadiusIndex1 { get; init; }
    public int EffectRadiusIndex2 { get; init; }
    public long EffectMiscValue1 { get; init; }
    public long EffectMiscValue2 { get; init; }
    public int EffectItemType { get; init; }
    public int EffectTriggerSpell { get; init; }
    public int EffectChainTargets { get; init; }
    public float EffectChainAmplitude { get; init; }
}

/// <summary>
/// A spell as the 1.15 client splits it: `dbc_spell` (texts), `dbc_spellname`, and difficulty-0 rows of
/// `dbc_spellmisc`, `dbc_spellcooldowns`, `dbc_spellcategories`, `dbc_spelllevels`, the first
/// `dbc_spellpower` row and the owned `dbc_spelleffect` list.
/// </summary>
public sealed class Spell : CatalogEntity, IDbcCatalogEntity
{
    public const int AttributeCount = 16;
    public const int ReagentCount = 8;

    private readonly List<SpellEffectEntry> _effects = [];

    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Name { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string NameSubtext { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string Description { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public string AuraDescription { get; set; } = string.Empty;

    /// <summary>`dbc_spellmisc.Attributes1..16`.</summary>
    public int[] Attributes { get; } = new int[AttributeCount];

    [ScriptProperty(Mutable = true)]
    public int SchoolMask { get; set; }

    [ScriptProperty(Mutable = true)]
    public int CastingTimeIndex { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DurationIndex { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RangeIndex { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Speed { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellIconFileDataID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ActiveIconFileDataID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int RecoveryTime { get; set; }

    [ScriptProperty(Mutable = true)]
    public int CategoryRecoveryTime { get; set; }

    [ScriptProperty(Mutable = true)]
    public int StartRecoveryTime { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Category { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DispelType { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Mechanic { get; set; }

    [ScriptProperty(Mutable = true)]
    public int StartRecoveryCategory { get; set; }

    [ScriptProperty(Mutable = true)]
    public int BaseLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MaxLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int PowerType { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ManaCost { get; set; }

    [ScriptProperty(Mutable = true)]
    public float PowerCostPct { get; set; }

    [ScriptProperty(Mutable = true)]
    public int CumulativeAura { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ProcCategoryRecovery { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ProcChance { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ProcCharges { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellProcsPerMinuteID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ProcTypeMask1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ProcTypeMask2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int InterruptFlags { get; set; }

    [ScriptProperty(Mutable = true)]
    public int AuraInterruptFlags1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int AuraInterruptFlags2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ChannelInterruptFlags1 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ChannelInterruptFlags2 { get; set; }

    [ScriptProperty(Mutable = true)]
    public int ModalNextSpell { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SpellClassSet { get; set; }

    [ScriptProperty(Mutable = true)]
    public float ConeDegrees { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MaxTargets { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MaxTargetLevel { get; set; }

    [ScriptProperty(Mutable = true)]
    public int TargetCreatureType { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Targets { get; set; }

    [ScriptProperty(Mutable = true)]
    public float Width { get; set; }

    /// <summary>`dbc_spellclassoptions.SpellClassMask1..4`.</summary>
    public int[] SpellClassMask { get; } = new int[4];

    /// <summary>`dbc_spellreagents.Reagent1..8`.</summary>
    public int[] Reagents { get; } = new int[ReagentCount];

    /// <summary>`dbc_spellreagents.ReagentCount1..8`.</summary>
    public int[] ReagentCounts { get; } = new int[ReagentCount];

    public bool IsSaved { get; set; }

    /// <summary>Ordered by <see cref="SpellEffectEntry.EffectIndex"/>. Change through the effect methods, which return the undoable command.</summary>
    public IReadOnlyList<SpellEffectEntry> Effects => _effects;

    public override string DisplayName => Name.Length > 0 ? $"Spell {ID}: {Name}" : $"Spell {ID}";

    public void ReplaceEffects(IEnumerable<SpellEffectEntry> effects)
    {
        _effects.Clear();
        _effects.AddRange(effects.OrderBy(e => e.EffectIndex));
    }

    [ScriptFunction]
    public IEditCommand AddEffect(int effectIndex)
    {
        if (_effects.Any(e => e.EffectIndex == effectIndex))
        {
            throw new InvalidOperationException($"Effect index {effectIndex} already exists on spell {ID}.");
        }

        List<SpellEffectEntry> after = _effects.ToList();
        after.Add(new SpellEffectEntry { EffectIndex = effectIndex });
        return ReplaceEffectsCommand(after);
    }

    [ScriptFunction]
    public IEditCommand RemoveEffect(int effectIndex) =>
        ReplaceEffectsCommand(_effects.Where(e => e.EffectIndex != effectIndex).ToList());

    /// <summary>Sets one <see cref="SpellEffectEntry"/> property by name; integer fields take the truncated value.</summary>
    [ScriptFunction]
    public IEditCommand SetEffectField(int effectIndex, string field, double value)
    {
        List<SpellEffectEntry> after = _effects.ToList();
        int i = after.FindIndex(e => e.EffectIndex == effectIndex);
        if (i < 0)
        {
            throw new InvalidOperationException($"Effect index {effectIndex} does not exist on spell {ID}.");
        }

        after[i] = ApplyEffectField(after[i], field, value);
        return ReplaceEffectsCommand(after);
    }

    private static SpellEffectEntry ApplyEffectField(SpellEffectEntry e, string field, double value)
    {
        int i = (int)value;
        float f = (float)value;
        return field switch
        {
            nameof(SpellEffectEntry.Effect) => e with { Effect = i },
            nameof(SpellEffectEntry.EffectAura) => e with { EffectAura = i },
            nameof(SpellEffectEntry.EffectBasePoints) => e with { EffectBasePoints = i },
            nameof(SpellEffectEntry.EffectDieSides) => e with { EffectDieSides = i },
            nameof(SpellEffectEntry.EffectRealPointsPerLevel) => e with { EffectRealPointsPerLevel = f },
            nameof(SpellEffectEntry.EffectBonusCoefficient) => e with { EffectBonusCoefficient = f },
            nameof(SpellEffectEntry.EffectAmplitude) => e with { EffectAmplitude = f },
            nameof(SpellEffectEntry.EffectAuraPeriod) => e with { EffectAuraPeriod = i },
            nameof(SpellEffectEntry.ImplicitTarget1) => e with { ImplicitTarget1 = i },
            nameof(SpellEffectEntry.ImplicitTarget2) => e with { ImplicitTarget2 = i },
            nameof(SpellEffectEntry.EffectMechanic) => e with { EffectMechanic = i },
            nameof(SpellEffectEntry.EffectRadiusIndex1) => e with { EffectRadiusIndex1 = i },
            nameof(SpellEffectEntry.EffectRadiusIndex2) => e with { EffectRadiusIndex2 = i },
            nameof(SpellEffectEntry.EffectMiscValue1) => e with { EffectMiscValue1 = (long)value },
            nameof(SpellEffectEntry.EffectMiscValue2) => e with { EffectMiscValue2 = (long)value },
            nameof(SpellEffectEntry.EffectItemType) => e with { EffectItemType = i },
            nameof(SpellEffectEntry.EffectTriggerSpell) => e with { EffectTriggerSpell = i },
            nameof(SpellEffectEntry.EffectChainTargets) => e with { EffectChainTargets = i },
            nameof(SpellEffectEntry.EffectChainAmplitude) => e with { EffectChainAmplitude = f },
            _ => throw new InvalidOperationException($"Unknown effect field '{field}'."),
        };
    }

    private IEditCommand ReplaceEffectsCommand(List<SpellEffectEntry> after) =>
        new SetFieldCommand<List<SpellEffectEntry>>(this, "effects", ReplaceEffects, _effects.ToList(), after);
}
