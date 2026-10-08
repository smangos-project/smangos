using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using ImGuiNET;

namespace WorldMapStudio.WoW;

/// <summary>The start location of one race/class (`playercreateinfo`).</summary>
public readonly record struct StartPositionRow
{
    public int Map { get; init; }
    public int Zone { get; init; }
    public float X { get; init; }
    public float Y { get; init; }
    public float Z { get; init; }
    public float O { get; init; }
}

/// <summary>Holds the single start location of a race/class (entry = race * 1000 + class).</summary>
public sealed class StartPositionTable : EntryRowTable<StartPositionRow>
{
    public StartPositionTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(StartPositionRow.Map) => row with { Map = (int)value },
        nameof(StartPositionRow.Zone) => row with { Zone = (int)value },
        nameof(StartPositionRow.X) => row with { X = (float)value },
        nameof(StartPositionRow.Y) => row with { Y = (float)value },
        nameof(StartPositionRow.Z) => row with { Z = (float)value },
        nameof(StartPositionRow.O) => row with { O = (float)value },
        _ => throw new InvalidOperationException($"Unknown position field '{field}'."),
    });

    [ScriptFunction]
    public IEditCommand AddRow() => AppendRow(new StartPositionRow());

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);
}

[Subsystem(nameof(WorldStorage))]
public sealed class StartPositionFactory : EntryRowsFactory<PlayercreateinfoRecord, StartPositionTable, StartPositionRow>
{
    public StartPositionFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Start Position";

    protected override Expression<Func<PlayercreateinfoRecord, int>> EntryExpression() => r => r.Race * 1000 + r.Class;

    protected override void SetEntry(PlayercreateinfoRecord r, int entry)
    {
        r.Race = entry / 1000;
        r.Class = entry % 1000;
    }

    protected override StartPositionTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override StartPositionRow ToRow(PlayercreateinfoRecord r) => new()
    {
        Map = r.Map, Zone = r.Zone, X = r.PositionX, Y = r.PositionY, Z = r.PositionZ, O = r.Orientation,
    };

    protected override void WriteRow(StartPositionRow row, PlayercreateinfoRecord r)
    {
        r.Map = row.Map;
        r.Zone = row.Zone;
        r.PositionX = row.X;
        r.PositionY = row.Y;
        r.PositionZ = row.Z;
        r.Orientation = row.O;
    }

    protected override void WriteKey(StartPositionRow row, PlayercreateinfoRecord r)
    {
    }

    protected override object KeyOf(StartPositionRow row) => 0;

    protected override string DuplicateMessage(int entry, StartPositionRow row) => $"{Kind} {entry} has two locations.";

    protected override void DrawRows(EditorContext context, StartPositionTable table)
    {
        if (table.Rows.Count == 0)
        {
            if (ImGui.Button("Add Position"))
            {
                Apply(context, table.AddRow());
            }

            return;
        }

        StartPositionRow row = table.Rows[0];
        RowWidgets.Int(context, "Map", row.Map, v => table.SetRowField(0, nameof(StartPositionRow.Map), v));
        RowWidgets.Int(context, "Zone", row.Zone, v => table.SetRowField(0, nameof(StartPositionRow.Zone), v));
        RowWidgets.Float(context, "X", row.X, v => table.SetRowField(0, nameof(StartPositionRow.X), v));
        RowWidgets.Float(context, "Y", row.Y, v => table.SetRowField(0, nameof(StartPositionRow.Y), v));
        RowWidgets.Float(context, "Z", row.Z, v => table.SetRowField(0, nameof(StartPositionRow.Z), v));
        RowWidgets.Float(context, "Orientation", row.O, v => table.SetRowField(0, nameof(StartPositionRow.O), v));
        if (ImGui.Button("Remove Position"))
        {
            Apply(context, table.RemoveRow(0));
        }
    }
}
