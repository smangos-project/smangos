using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`gossip_menu` as a catalog keyed by `entry`.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GossipMenuFactory : EntryRowsFactory<GossipMenuRecord, GossipTextTable, GossipTextRow>
{
    public GossipMenuFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gossip Menu";

    protected override GossipTextTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override GossipTextRow ToRow(GossipMenuRecord r) => new()
    {
        TextId = r.TextId,
        ScriptId = r.ScriptId,
        ConditionId = r.ConditionId,
    };

    protected override void WriteRow(GossipTextRow row, GossipMenuRecord r)
    {
        r.ScriptId = row.ScriptId;
        r.ConditionId = row.ConditionId;
    }

    protected override void WriteKey(GossipTextRow row, GossipMenuRecord r) => r.TextId = row.TextId;

    protected override object KeyOf(GossipTextRow row) => row.TextId;

    protected override IEnumerable<GossipTextRow> Order(IEnumerable<GossipTextRow> rows) => rows.OrderBy(r => r.TextId);

    protected override string DuplicateMessage(int entry, GossipTextRow row) =>
        $"{Kind} {entry} lists text {row.TextId} twice.";

    protected override void DrawRows(EditorContext context, GossipTextTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            GossipTextRow row = table.Rows[i];
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

            RowWidgets.Int(context, "Text Id", row.TextId, v => table.SetRowField(index, nameof(GossipTextRow.TextId), v));
            RowWidgets.Int(context, "Script Id", row.ScriptId, v => table.SetRowField(index, nameof(GossipTextRow.ScriptId), v));
            RowWidgets.Int(context, "Condition", row.ConditionId, v => table.SetRowField(index, nameof(GossipTextRow.ConditionId), v));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Row"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}

/// <summary>`gossip_menu_option` as a catalog keyed by `menu_id`.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GossipMenuOptionFactory : EntryRowsFactory<GossipMenuOptionRecord, GossipOptionTable, GossipOptionRow>
{
    public GossipMenuOptionFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gossip Option";

    protected override GossipOptionTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override GossipOptionRow ToRow(GossipMenuOptionRecord r) => new()
    {
        Id = r.Id,
        OptionIcon = r.OptionIcon,
        OptionText = r.OptionText ?? string.Empty,
        OptionBroadcastText = r.OptionBroadcastText,
        OptionId = r.OptionId,
        NpcOptionNpcflag = r.NpcOptionNpcflag,
        ActionMenuId = r.ActionMenuId,
        ActionPoiId = r.ActionPoiId,
        ActionScriptId = r.ActionScriptId,
        BoxCoded = r.BoxCoded,
        BoxText = r.BoxText ?? string.Empty,
        BoxBroadcastText = r.BoxBroadcastText,
        ConditionId = r.ConditionId,
    };

    protected override void WriteRow(GossipOptionRow row, GossipMenuOptionRecord r)
    {
        r.OptionIcon = row.OptionIcon;
        r.OptionText = row.OptionText.Length == 0 ? null : row.OptionText;
        r.OptionBroadcastText = row.OptionBroadcastText;
        r.OptionId = row.OptionId;
        r.NpcOptionNpcflag = row.NpcOptionNpcflag;
        r.ActionMenuId = row.ActionMenuId;
        r.ActionPoiId = row.ActionPoiId;
        r.ActionScriptId = row.ActionScriptId;
        r.BoxCoded = row.BoxCoded;
        r.BoxText = row.BoxText.Length == 0 ? null : row.BoxText;
        r.BoxBroadcastText = row.BoxBroadcastText;
        r.ConditionId = row.ConditionId;
    }

    protected override void WriteKey(GossipOptionRow row, GossipMenuOptionRecord r) => r.Id = row.Id;

    protected override object KeyOf(GossipOptionRow row) => row.Id;

    protected override IEnumerable<GossipOptionRow> Order(IEnumerable<GossipOptionRow> rows) => rows.OrderBy(r => r.Id);

    protected override string DuplicateMessage(int entry, GossipOptionRow row) =>
        $"{Kind} {entry} uses option id {row.Id} twice.";

    protected override void DrawRows(EditorContext context, GossipOptionTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            GossipOptionRow row = table.Rows[i];
            int index = i;
            ImGui.PushID(index);
            ImGui.Text($"Option {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            RowWidgets.Int(context, "Id", row.Id, v => table.SetRowField(index, nameof(GossipOptionRow.Id), v));
            RowWidgets.Int(context, "Icon", row.OptionIcon, v => table.SetRowField(index, nameof(GossipOptionRow.OptionIcon), v));
            RowWidgets.Text(context, "Text", row.OptionText, v => table.SetRowText(index, nameof(GossipOptionRow.OptionText), v));
            RowWidgets.Int(context, "Broadcast Text", row.OptionBroadcastText, v => table.SetRowField(index, nameof(GossipOptionRow.OptionBroadcastText), v));
            RowWidgets.Int(context, "Option Type", row.OptionId, v => table.SetRowField(index, nameof(GossipOptionRow.OptionId), v));
            RowWidgets.Int(context, "Npc Flag", row.NpcOptionNpcflag, v => table.SetRowField(index, nameof(GossipOptionRow.NpcOptionNpcflag), v));
            RowWidgets.Int(context, "Action Menu", row.ActionMenuId, v => table.SetRowField(index, nameof(GossipOptionRow.ActionMenuId), v));
            RowWidgets.Int(context, "Action Poi", row.ActionPoiId, v => table.SetRowField(index, nameof(GossipOptionRow.ActionPoiId), v));
            RowWidgets.Int(context, "Action Script", row.ActionScriptId, v => table.SetRowField(index, nameof(GossipOptionRow.ActionScriptId), v));
            RowWidgets.Int(context, "Box Coded", row.BoxCoded, v => table.SetRowField(index, nameof(GossipOptionRow.BoxCoded), v));
            RowWidgets.Text(context, "Box Text", row.BoxText, v => table.SetRowText(index, nameof(GossipOptionRow.BoxText), v));
            RowWidgets.Int(context, "Box Broadcast Text", row.BoxBroadcastText, v => table.SetRowField(index, nameof(GossipOptionRow.BoxBroadcastText), v));
            RowWidgets.Int(context, "Condition", row.ConditionId, v => table.SetRowField(index, nameof(GossipOptionRow.ConditionId), v));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Option"))
        {
            Apply(context, table.AddRow(table.Rows.Count == 0 ? 0 : table.Rows.Max(r => r.Id) + 1));
        }
    }
}

/// <summary>Keys the keyless gossip tables.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GossipTableKeys : ITableConfiguration
{
    public GossipTableKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<GossipMenuRecord>().HasKey(r => new { r.Entry, r.TextId });
        model.Entity<GossipMenuOptionRecord>().HasKey(r => new { r.MenuId, r.Id });
    }
}
