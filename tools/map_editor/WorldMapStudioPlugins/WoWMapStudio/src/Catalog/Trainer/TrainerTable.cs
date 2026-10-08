using System;

namespace WorldMapStudio.WoW;

/// <summary>One row of `npc_trainer` / `npc_trainer_template`.</summary>
public readonly record struct TrainerRow
{
    public int Spell { get; init; }
    public int SpellCost { get; init; }
    public int ReqSkill { get; init; }
    public int ReqSkillValue { get; init; }
    public int ReqLevel { get; init; }
}

/// <summary>All spells one trainer entry (or trainer template) teaches.</summary>
public sealed class TrainerTable : EntryRowTable<TrainerRow>
{
    public TrainerTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int spell) => AppendRow(new TrainerRow { Spell = spell });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    /// <summary>Sets one <see cref="TrainerRow"/> property by name.</summary>
    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(TrainerRow.Spell) => row with { Spell = (int)value },
        nameof(TrainerRow.SpellCost) => row with { SpellCost = (int)value },
        nameof(TrainerRow.ReqSkill) => row with { ReqSkill = (int)value },
        nameof(TrainerRow.ReqSkillValue) => row with { ReqSkillValue = (int)value },
        nameof(TrainerRow.ReqLevel) => row with { ReqLevel = (int)value },
        _ => throw new InvalidOperationException($"Unknown trainer row field '{field}'."),
    });
}
