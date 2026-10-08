using System;
using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>One command of a `generic_scripts` script.</summary>
public readonly record struct ScriptRow
{
    public int Delay { get; init; }
    public int Priority { get; init; }
    public int Command { get; init; }
    public int Datalong { get; init; }
    public int Datalong2 { get; init; }
    public int Datalong3 { get; init; }
    public int Datalong4 { get; init; }
    public int TargetParam1 { get; init; }
    public int TargetParam2 { get; init; }
    public int TargetType { get; init; }
    public int DataFlags { get; init; }
    public int Dataint { get; init; }
    public int Dataint2 { get; init; }
    public int Dataint3 { get; init; }
    public int Dataint4 { get; init; }
    public float X { get; init; }
    public float Y { get; init; }
    public float Z { get; init; }
    public float O { get; init; }
    public int ConditionId { get; init; }
}

/// <summary>The commands of one `generic_scripts` id.</summary>
public sealed class ScriptTable : EntryRowTable<ScriptRow>
{
    private static readonly System.Reflection.PropertyInfo[] Props = typeof(ScriptRow).GetProperties();

    public ScriptTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int command) => AppendRow(new ScriptRow { Command = command });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row =>
    {
        System.Reflection.PropertyInfo prop = Props.FirstOrDefault(p => p.Name == field)
            ?? throw new InvalidOperationException($"Unknown script field '{field}'.");
        object boxed = row;
        prop.SetValue(boxed, prop.PropertyType == typeof(float) ? (float)value : (int)value);
        return (ScriptRow)boxed;
    });
}

[Subsystem(nameof(WorldStorage))]
public sealed class GenericScriptFactory : EntryRowsFactory<GenericScriptsRecord, ScriptTable, ScriptRow>
{
    public GenericScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Generic Script";

    protected override ScriptTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override ScriptRow ToRow(GenericScriptsRecord r) => new()
    {
        Delay = r.Delay, Priority = r.Priority, Command = r.Command,
        Datalong = r.Datalong, Datalong2 = r.Datalong2, Datalong3 = r.Datalong3, Datalong4 = r.Datalong4,
        TargetParam1 = r.TargetParam1, TargetParam2 = r.TargetParam2, TargetType = r.TargetType,
        DataFlags = r.DataFlags, Dataint = r.Dataint, Dataint2 = r.Dataint2, Dataint3 = r.Dataint3, Dataint4 = r.Dataint4,
        X = r.X, Y = r.Y, Z = r.Z, O = r.O, ConditionId = r.ConditionId,
    };

    protected override void WriteRow(ScriptRow row, GenericScriptsRecord r)
    {
    }

    protected override void WriteKey(ScriptRow row, GenericScriptsRecord r)
    {
        r.Delay = row.Delay; r.Priority = row.Priority; r.Command = row.Command;
        r.Datalong = row.Datalong; r.Datalong2 = row.Datalong2; r.Datalong3 = row.Datalong3; r.Datalong4 = row.Datalong4;
        r.TargetParam1 = row.TargetParam1; r.TargetParam2 = row.TargetParam2; r.TargetType = row.TargetType;
        r.DataFlags = row.DataFlags; r.Dataint = row.Dataint; r.Dataint2 = row.Dataint2; r.Dataint3 = row.Dataint3; r.Dataint4 = row.Dataint4;
        r.X = row.X; r.Y = row.Y; r.Z = row.Z; r.O = row.O; r.ConditionId = row.ConditionId;
    }

    protected override object KeyOf(ScriptRow row) => row;

    protected override IEnumerable<ScriptRow> Order(IEnumerable<ScriptRow> rows) =>
        rows.OrderBy(r => r.Delay).ThenBy(r => r.Priority);

    protected override string DuplicateMessage(int entry, ScriptRow row) =>
        $"{Kind} {entry} has two identical commands.";

    protected override void DrawRows(EditorContext context, ScriptTable table)
    {
        System.Reflection.PropertyInfo[] props = typeof(ScriptRow).GetProperties();
        for (int i = 0; i < table.Rows.Count; i++)
        {
            ScriptRow row = table.Rows[i];
            int index = i;
            ImGui.PushID(index);
            ImGui.Text($"Command {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            foreach (System.Reflection.PropertyInfo prop in props)
            {
                string name = prop.Name;
                if (prop.PropertyType == typeof(float))
                {
                    RowWidgets.Float(context, name, (float)prop.GetValue(row)!, v => table.SetRowField(index, name, v));
                }
                else
                {
                    RowWidgets.Int(context, name, (int)prop.GetValue(row)!, v => table.SetRowField(index, name, v));
                }
            }

            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Command"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}

/// <summary>Keys the keyless `generic_scripts` table on all its columns.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GenericScriptKeys : ITableConfiguration
{
    public GenericScriptKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model) =>
        model.Entity<GenericScriptsRecord>().HasKey(r => new
        {
            r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4,
            r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4,
            r.X, r.Y, r.Z, r.O, r.ConditionId,
        });
}
