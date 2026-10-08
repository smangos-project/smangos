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

/// <summary>The script tables share their columns, so one factory covers them all.</summary>
public abstract class ScriptFactory<TRecord> : EntryRowsFactory<TRecord, ScriptTable, ScriptRow>
    where TRecord : class, new()
{
    private static readonly System.Reflection.PropertyInfo[] Props = typeof(ScriptRow).GetProperties()
        .Select(p => typeof(TRecord).GetProperty(p.Name)!).ToArray();

    protected ScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    protected override ScriptTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override ScriptRow ToRow(TRecord r)
    {
        object row = new ScriptRow();
        System.Reflection.PropertyInfo[] rowProps = typeof(ScriptRow).GetProperties();
        for (int i = 0; i < rowProps.Length; i++)
        {
            rowProps[i].SetValue(row, Props[i].GetValue(r));
        }

        return (ScriptRow)row;
    }

    protected override void WriteRow(ScriptRow row, TRecord r)
    {
    }

    protected override void WriteKey(ScriptRow row, TRecord r)
    {
        System.Reflection.PropertyInfo[] rowProps = typeof(ScriptRow).GetProperties();
        for (int i = 0; i < rowProps.Length; i++)
        {
            Props[i].SetValue(r, rowProps[i].GetValue(row));
        }
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

[Subsystem(nameof(WorldStorage))]
public sealed class GenericScriptFactory : ScriptFactory<GenericScriptsRecord>
{
    public GenericScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Generic Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class EventScriptFactory : ScriptFactory<EventScriptsRecord>
{
    public EventScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Event Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class SpellScriptFactory : ScriptFactory<SpellScriptsRecord>
{
    public SpellScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Spell Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class GossipScriptFactory : ScriptFactory<GossipScriptsRecord>
{
    public GossipScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gossip Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameObjectScriptFactory : ScriptFactory<GameobjectScriptsRecord>
{
    public GameObjectScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "GameObject Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class QuestStartScriptFactory : ScriptFactory<QuestStartScriptsRecord>
{
    public QuestStartScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Quest Start Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class QuestEndScriptFactory : ScriptFactory<QuestEndScriptsRecord>
{
    public QuestEndScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Quest End Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureSpellScriptFactory : ScriptFactory<CreatureSpellsScriptsRecord>
{
    public CreatureSpellScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Spell Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureMovementScriptFactory : ScriptFactory<CreatureMovementScriptsRecord>
{
    public CreatureMovementScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Movement Script";
}

[Subsystem(nameof(WorldStorage))]
public sealed class AreatriggerScriptFactory : ScriptFactory<AreatriggerScriptsRecord>
{
    public AreatriggerScriptFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Areatrigger Script";
}

/// <summary>Keys the keyless script tables on all their columns.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class ScriptTableKeys : ITableConfiguration
{
    public ScriptTableKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        Key<GenericScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<EventScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<SpellScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<GossipScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<GameobjectScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<QuestStartScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<QuestEndScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<CreatureSpellsScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<CreatureMovementScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
        Key<AreatriggerScriptsRecord>(model, r => new { r.Id, r.Delay, r.Priority, r.Command, r.Datalong, r.Datalong2, r.Datalong3, r.Datalong4, r.TargetParam1, r.TargetParam2, r.TargetType, r.DataFlags, r.Dataint, r.Dataint2, r.Dataint3, r.Dataint4, r.X, r.Y, r.Z, r.O, r.ConditionId });
    }

    private static void Key<T>(ModelBuilder model, System.Linq.Expressions.Expression<Func<T, object?>> key) where T : class =>
        model.Entity<T>().HasKey(key);
}
