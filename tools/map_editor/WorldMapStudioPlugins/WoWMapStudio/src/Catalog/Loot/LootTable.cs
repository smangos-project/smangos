using System;
using System.Collections.Generic;
using System.Linq;

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
public sealed class LootTable : CatalogEntity, ISavedCatalogEntity
{
    private readonly List<LootRow> _rows = [];

    public LootTable(string kind)
    {
        Kind = kind;
    }

    public string Kind { get; }

    [ScriptProperty]
    public int ID { get; set; }

    public bool IsSaved { get; set; }

    public IReadOnlyList<LootRow> Rows => _rows;

    public override string DisplayName => $"{Kind} Loot {ID}";

    public void ReplaceRows(IEnumerable<LootRow> rows)
    {
        _rows.Clear();
        _rows.AddRange(rows);
    }

    [ScriptFunction]
    public IEditCommand AddRow(int item) =>
        ReplaceRowsCommand(_rows.Append(new LootRow { Item = item, Chance = 100f, MinCountOrRef = 1, MaxCount = 1 }).ToList());

    [ScriptFunction]
    public IEditCommand RemoveRow(int index)
    {
        CheckIndex(index);
        List<LootRow> after = _rows.ToList();
        after.RemoveAt(index);
        return ReplaceRowsCommand(after);
    }

    /// <summary>Sets one <see cref="LootRow"/> property by name.</summary>
    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value)
    {
        CheckIndex(index);
        List<LootRow> after = _rows.ToList();
        LootRow row = after[index];
        after[index] = field switch
        {
            nameof(LootRow.Item) => row with { Item = (int)value },
            nameof(LootRow.Chance) => row with { Chance = (float)value },
            nameof(LootRow.GroupId) => row with { GroupId = (int)value },
            nameof(LootRow.MinCountOrRef) => row with { MinCountOrRef = (int)value },
            nameof(LootRow.MaxCount) => row with { MaxCount = (int)value },
            nameof(LootRow.ConditionId) => row with { ConditionId = (int)value },
            _ => throw new InvalidOperationException($"Unknown loot row field '{field}'."),
        };
        return ReplaceRowsCommand(after);
    }

    private void CheckIndex(int index)
    {
        if (index < 0 || index >= _rows.Count)
        {
            throw new InvalidOperationException($"Row {index} does not exist on {DisplayName}.");
        }
    }

    private IEditCommand ReplaceRowsCommand(List<LootRow> after) =>
        new SetFieldCommand<List<LootRow>>(this, "rows", ReplaceRows, _rows.ToList(), after);
}
