using System;

namespace WorldMapStudio.WoW;

/// <summary>One row of `npc_vendor` / `npc_vendor_template`.</summary>
public readonly record struct VendorRow
{
    public int Item { get; init; }
    public int Slot { get; init; }
    public int MaxCount { get; init; }
    public int IncrTime { get; init; }
    public int ItemFlags { get; init; }
    public int ConditionId { get; init; }
}

/// <summary>All items one vendor entry (or vendor template) sells.</summary>
public sealed class VendorTable : EntryRowTable<VendorRow>
{
    public VendorTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int item) => AppendRow(new VendorRow { Item = item });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    /// <summary>Sets one <see cref="VendorRow"/> property by name.</summary>
    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(VendorRow.Item) => row with { Item = (int)value },
        nameof(VendorRow.Slot) => row with { Slot = (int)value },
        nameof(VendorRow.MaxCount) => row with { MaxCount = (int)value },
        nameof(VendorRow.IncrTime) => row with { IncrTime = (int)value },
        nameof(VendorRow.ItemFlags) => row with { ItemFlags = (int)value },
        nameof(VendorRow.ConditionId) => row with { ConditionId = (int)value },
        _ => throw new InvalidOperationException($"Unknown vendor row field '{field}'."),
    });
}
