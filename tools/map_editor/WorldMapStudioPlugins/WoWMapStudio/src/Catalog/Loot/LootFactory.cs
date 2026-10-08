using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>A `*_loot_template` table as a catalog keyed by `entry`. All nine loot tables share their
/// columns, so each subclass only names its table.</summary>
public abstract class LootFactory<TRecord> : EntryRowsFactory<TRecord, LootTable, LootRow>
    where TRecord : class, new()
{
    private static readonly PropertyInfo ItemProp = Prop("Item");
    private static readonly PropertyInfo ChanceProp = Prop("ChanceOrQuestChance");
    private static readonly PropertyInfo GroupProp = Prop("Groupid");
    private static readonly PropertyInfo MinProp = Prop("MincountOrRef");
    private static readonly PropertyInfo MaxProp = Prop("Maxcount");
    private static readonly PropertyInfo ConditionProp = Prop("ConditionId");

    private static readonly bool KeyedByGroup = typeof(TRecord).GetCustomAttributes<PrimaryKeyAttribute>()
        .Single().PropertyNames.Contains("Groupid");

    protected LootFactory(WorldStorage storage) : base(storage)
    {
    }

    private static PropertyInfo Prop(string name) => typeof(TRecord).GetProperty(name)!;

    protected override LootTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override LootRow ToRow(TRecord r) => new()
    {
        Item = (int)ItemProp.GetValue(r)!,
        Chance = (float)ChanceProp.GetValue(r)!,
        GroupId = (int)GroupProp.GetValue(r)!,
        MinCountOrRef = (int)MinProp.GetValue(r)!,
        MaxCount = (int)MaxProp.GetValue(r)!,
        ConditionId = (int)ConditionProp.GetValue(r)!,
    };

    protected override void WriteRow(LootRow row, TRecord r)
    {
        ChanceProp.SetValue(r, row.Chance);
        GroupProp.SetValue(r, row.GroupId);
        MinProp.SetValue(r, row.MinCountOrRef);
        MaxProp.SetValue(r, row.MaxCount);
        ConditionProp.SetValue(r, row.ConditionId);
    }

    protected override void WriteKey(LootRow row, TRecord r) => ItemProp.SetValue(r, row.Item);

    protected override object KeyOf(LootRow row) => (row.Item, KeyedByGroup ? row.GroupId : 0);

    protected override IEnumerable<LootRow> Order(IEnumerable<LootRow> rows) =>
        rows.OrderBy(r => r.GroupId).ThenBy(r => r.Item);

    protected override string DuplicateMessage(int entry, LootRow row) =>
        $"{Kind} {entry} lists item {row.Item} twice" + (KeyedByGroup ? $" in group {row.GroupId}." : ".");

    protected override void DrawRows(EditorContext context, LootTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            LootRow row = table.Rows[i];
            int index = i;
            ImGui.PushID(index);
            ImGui.Text($"Row {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            RowWidgets.Int(context, "Item", row.Item, v => table.SetRowField(index, nameof(LootRow.Item), v));
            RowWidgets.Float(context, "Chance", row.Chance, v => table.SetRowField(index, nameof(LootRow.Chance), v));
            RowWidgets.Int(context, "Group", row.GroupId, v => table.SetRowField(index, nameof(LootRow.GroupId), v));
            RowWidgets.Int(context, "Min Count / Ref", row.MinCountOrRef, v => table.SetRowField(index, nameof(LootRow.MinCountOrRef), v));
            RowWidgets.Int(context, "Max Count", row.MaxCount, v => table.SetRowField(index, nameof(LootRow.MaxCount), v));
            RowWidgets.Int(context, "Condition", row.ConditionId, v => table.SetRowField(index, nameof(LootRow.ConditionId), v));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Row"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}
