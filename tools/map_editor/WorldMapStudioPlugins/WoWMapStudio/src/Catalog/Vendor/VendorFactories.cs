using System.Collections.Generic;
using System.Linq;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>`npc_vendor` / `npc_vendor_template` as catalogs keyed by `entry`.</summary>
public abstract class VendorFactory<TRecord> : EntryRowsFactory<TRecord, VendorTable, VendorRow>
    where TRecord : class, new()
{
    private static readonly System.Reflection.PropertyInfo[] Props =
        new[] { "Item", "Slot", "Maxcount", "Incrtime", "Itemflags", "ConditionId" }
            .Select(n => typeof(TRecord).GetProperty(n)!).ToArray();

    protected VendorFactory(WorldStorage storage) : base(storage)
    {
    }

    private static int Get(TRecord r, int i) => (int)Props[i].GetValue(r)!;

    protected override VendorTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override VendorRow ToRow(TRecord r) => new()
    {
        Item = Get(r, 0),
        Slot = Get(r, 1),
        MaxCount = Get(r, 2),
        IncrTime = Get(r, 3),
        ItemFlags = Get(r, 4),
        ConditionId = Get(r, 5),
    };

    protected override void WriteRow(VendorRow row, TRecord r)
    {
        Props[1].SetValue(r, row.Slot);
        Props[2].SetValue(r, row.MaxCount);
        Props[3].SetValue(r, row.IncrTime);
        Props[4].SetValue(r, row.ItemFlags);
        Props[5].SetValue(r, row.ConditionId);
    }

    protected override void WriteKey(VendorRow row, TRecord r) => Props[0].SetValue(r, row.Item);

    protected override object KeyOf(VendorRow row) => row.Item;

    protected override IEnumerable<VendorRow> Order(IEnumerable<VendorRow> rows) =>
        rows.OrderBy(r => r.Slot).ThenBy(r => r.Item);

    protected override string DuplicateMessage(int entry, VendorRow row) =>
        $"{Kind} {entry} lists item {row.Item} twice.";

    protected override void DrawRows(EditorContext context, VendorTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            VendorRow row = table.Rows[i];
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

            RowWidgets.Int(context, "Item", row.Item, v => table.SetRowField(index, nameof(VendorRow.Item), v));
            RowWidgets.Int(context, "Slot", row.Slot, v => table.SetRowField(index, nameof(VendorRow.Slot), v));
            RowWidgets.Int(context, "Max Count", row.MaxCount, v => table.SetRowField(index, nameof(VendorRow.MaxCount), v));
            RowWidgets.Int(context, "Restock Time (s)", row.IncrTime, v => table.SetRowField(index, nameof(VendorRow.IncrTime), v));
            RowWidgets.Int(context, "Item Flags", row.ItemFlags, v => table.SetRowField(index, nameof(VendorRow.ItemFlags), v));
            RowWidgets.Int(context, "Condition", row.ConditionId, v => table.SetRowField(index, nameof(VendorRow.ConditionId), v));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Row"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}

[Subsystem(nameof(WorldStorage))]
public sealed class NpcVendorFactory : VendorFactory<NpcVendorRecord>
{
    public NpcVendorFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Vendor";
}

[Subsystem(nameof(WorldStorage))]
public sealed class NpcVendorTemplateFactory : VendorFactory<NpcVendorTemplateRecord>
{
    public NpcVendorTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Vendor Template";
}
