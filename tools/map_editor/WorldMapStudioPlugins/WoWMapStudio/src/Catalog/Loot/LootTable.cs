using System;

namespace WorldMapStudio.WoW;

/// <summary>One row of a `*_loot_template` table. `MinCountOrRef` &lt; 0 makes the row a reference to
/// a `reference_loot_template` entry.</summary>
public readonly record struct LootRow
{
    public int Item { get; init; }
    public float Chance { get; init; }
    public int GroupId { get; init; }
    public int MinCountOrRef { get; init; }
    public int MaxCount { get; init; }
    public int ConditionId { get; init; }
}

/// <summary>All rows of one `*_loot_template` table sharing an `entry`.</summary>
public sealed class LootTable : EntryRowTable<LootRow>
{
    public LootTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int item) =>
        AppendRow(new LootRow { Item = item, Chance = 100f, MinCountOrRef = 1, MaxCount = 1 });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    /// <summary>Sets one <see cref="LootRow"/> property by name.</summary>
    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(LootRow.Item) => row with { Item = (int)value },
        nameof(LootRow.Chance) => row with { Chance = (float)value },
        nameof(LootRow.GroupId) => row with { GroupId = (int)value },
        nameof(LootRow.MinCountOrRef) => row with { MinCountOrRef = (int)value },
        nameof(LootRow.MaxCount) => row with { MaxCount = (int)value },
        nameof(LootRow.ConditionId) => row with { ConditionId = (int)value },
        _ => throw new InvalidOperationException($"Unknown loot row field '{field}'."),
    });
}
