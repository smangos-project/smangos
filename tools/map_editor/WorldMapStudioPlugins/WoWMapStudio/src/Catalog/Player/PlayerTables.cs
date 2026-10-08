using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>One row of `playercreateinfo_spell`.</summary>
public readonly record struct StartSpellRow
{
    public int Spell { get; init; }
}

/// <summary>One row of `playercreateinfo_item`.</summary>
public readonly record struct StartItemRow
{
    public int Item { get; init; }
    public int Amount { get; init; }
}

/// <summary>One row of `playercreateinfo_action`.</summary>
public readonly record struct StartActionRow
{
    public int Button { get; init; }
    public int Action { get; init; }
    public int Type { get; init; }
}

/// <summary>One row of `player_levelstats`.</summary>
public readonly record struct LevelStatsRow
{
    public int Level { get; init; }
    public int Str { get; init; }
    public int Agi { get; init; }
    public int Sta { get; init; }
    public int Int { get; init; }
    public int Spi { get; init; }
}

/// <summary>One row of `player_classlevelstats`.</summary>
public readonly record struct ClassLevelStatsRow
{
    public int Level { get; init; }
    public int BaseHp { get; init; }
    public int BaseMana { get; init; }
}

/// <summary>Generic list of rows of one race/class (entry = race * 1000 + class) or class.</summary>
public sealed class PlayerRowTable<TRow> : EntryRowTable<TRow>
{
    public PlayerRowTable(string kind) : base(kind)
    {
    }

    public int Race => ID / 1000;

    public int Class => ID % 1000;

    [ScriptFunction]
    public IEditCommand AddRow(int key) => AppendRow(Create(key));

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    /// <summary>Sets one row property by name.</summary>
    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => With(row, field, (int)value));

    private static readonly System.Reflection.PropertyInfo[] Props = typeof(TRow).GetProperties();

    private static TRow Create(int key)
    {
        object row = Activator.CreateInstance<TRow>()!;
        Props[0].SetValue(row, key);
        return (TRow)row;
    }

    private static TRow With(TRow row, string field, int value)
    {
        System.Reflection.PropertyInfo prop = Props.FirstOrDefault(p => p.Name == field)
            ?? throw new InvalidOperationException($"Unknown field '{field}'.");
        object boxed = row!;
        prop.SetValue(boxed, value);
        return (TRow)boxed;
    }
}

/// <summary>Shared base for the player tables keyed by race/class (or class) and one row key.</summary>
public abstract class PlayerRowsFactory<TRecord, TRow> : EntryRowsFactory<TRecord, PlayerRowTable<TRow>, TRow>
    where TRecord : class, new()
    where TRow : struct
{
    private readonly string[] _labels;

    protected PlayerRowsFactory(WorldStorage storage, string[] labels) : base(storage)
    {
        _labels = labels;
    }

    protected override PlayerRowTable<TRow> NewTable(int entry) => new(Kind) { ID = entry };

    protected override object KeyOf(TRow row) => typeof(TRow).GetProperties()[0].GetValue(row)!;

    protected override string DuplicateMessage(int entry, TRow row) => $"{Kind} {entry} repeats key {KeyOf(row)}.";

    protected override void DrawRows(EditorContext context, PlayerRowTable<TRow> table)
    {
        System.Reflection.PropertyInfo[] props = typeof(TRow).GetProperties();
        for (int i = 0; i < table.Rows.Count; i++)
        {
            TRow row = table.Rows[i];
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

            for (int p = 0; p < props.Length; p++)
            {
                string name = props[p].Name;
                RowWidgets.Int(context, _labels[p], (int)props[p].GetValue(row)!, v => table.SetRowField(index, name, v));
            }

            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Row"))
        {
            Apply(context, table.AddRow(0));
        }
    }

    protected static Expression<Func<TRecord, int>> RaceClass(string race, string cls)
    {
        ParameterExpression p = Expression.Parameter(typeof(TRecord), "r");
        Expression body = Expression.Add(
            Expression.Multiply(Expression.Property(p, race), Expression.Constant(1000)),
            Expression.Property(p, cls));
        return Expression.Lambda<Func<TRecord, int>>(body, p);
    }

    protected static void SetRaceClass(TRecord r, int entry)
    {
        typeof(TRecord).GetProperty("Race")!.SetValue(r, entry / 1000);
        typeof(TRecord).GetProperty("Class")!.SetValue(r, entry % 1000);
    }
}

[Subsystem(nameof(WorldStorage))]
public sealed class StartSpellFactory : PlayerRowsFactory<PlayercreateinfoSpellRecord, StartSpellRow>
{
    public StartSpellFactory(WorldStorage storage) : base(storage, ["Spell"])
    {
    }

    public override string CatalogName => "Start Spells";

    protected override Expression<Func<PlayercreateinfoSpellRecord, int>> EntryExpression() => RaceClass("Race", "Class");

    protected override void SetEntry(PlayercreateinfoSpellRecord r, int entry) => SetRaceClass(r, entry);

    protected override StartSpellRow ToRow(PlayercreateinfoSpellRecord r) => new() { Spell = r.Spell };

    protected override void WriteRow(StartSpellRow row, PlayercreateinfoSpellRecord r)
    {
    }

    protected override void WriteKey(StartSpellRow row, PlayercreateinfoSpellRecord r) => r.Spell = row.Spell;

    protected override IEnumerable<StartSpellRow> Order(IEnumerable<StartSpellRow> rows) => rows.OrderBy(r => r.Spell);
}

[Subsystem(nameof(WorldStorage))]
public sealed class StartItemFactory : PlayerRowsFactory<PlayercreateinfoItemRecord, StartItemRow>
{
    public StartItemFactory(WorldStorage storage) : base(storage, ["Item", "Amount"])
    {
    }

    public override string CatalogName => "Start Items";

    protected override Expression<Func<PlayercreateinfoItemRecord, int>> EntryExpression() => RaceClass("Race", "Class");

    protected override void SetEntry(PlayercreateinfoItemRecord r, int entry) => SetRaceClass(r, entry);

    protected override StartItemRow ToRow(PlayercreateinfoItemRecord r) => new() { Item = r.Itemid, Amount = r.Amount };

    protected override void WriteRow(StartItemRow row, PlayercreateinfoItemRecord r) => r.Amount = row.Amount;

    protected override void WriteKey(StartItemRow row, PlayercreateinfoItemRecord r) => r.Itemid = row.Item;

    protected override IEnumerable<StartItemRow> Order(IEnumerable<StartItemRow> rows) => rows.OrderBy(r => r.Item);
}

[Subsystem(nameof(WorldStorage))]
public sealed class StartActionFactory : PlayerRowsFactory<PlayercreateinfoActionRecord, StartActionRow>
{
    public StartActionFactory(WorldStorage storage) : base(storage, ["Button", "Action", "Type"])
    {
    }

    public override string CatalogName => "Start Actions";

    protected override Expression<Func<PlayercreateinfoActionRecord, int>> EntryExpression() => RaceClass("Race", "Class");

    protected override void SetEntry(PlayercreateinfoActionRecord r, int entry) => SetRaceClass(r, entry);

    protected override StartActionRow ToRow(PlayercreateinfoActionRecord r) => new() { Button = r.Button, Action = r.Action, Type = r.Type };

    protected override void WriteRow(StartActionRow row, PlayercreateinfoActionRecord r)
    {
        r.Action = row.Action;
        r.Type = row.Type;
    }

    protected override void WriteKey(StartActionRow row, PlayercreateinfoActionRecord r) => r.Button = row.Button;

    protected override IEnumerable<StartActionRow> Order(IEnumerable<StartActionRow> rows) => rows.OrderBy(r => r.Button);
}

[Subsystem(nameof(WorldStorage))]
public sealed class LevelStatsFactory : PlayerRowsFactory<PlayerLevelstatsRecord, LevelStatsRow>
{
    public LevelStatsFactory(WorldStorage storage) : base(storage, ["Level", "Strength", "Agility", "Stamina", "Intellect", "Spirit"])
    {
    }

    public override string CatalogName => "Level Stats";

    protected override Expression<Func<PlayerLevelstatsRecord, int>> EntryExpression() => RaceClass("Race", "Class");

    protected override void SetEntry(PlayerLevelstatsRecord r, int entry) => SetRaceClass(r, entry);

    protected override LevelStatsRow ToRow(PlayerLevelstatsRecord r) =>
        new() { Level = r.Level, Str = r.Str, Agi = r.Agi, Sta = r.Sta, Int = r.Inte, Spi = r.Spi };

    protected override void WriteRow(LevelStatsRow row, PlayerLevelstatsRecord r)
    {
        r.Str = row.Str;
        r.Agi = row.Agi;
        r.Sta = row.Sta;
        r.Inte = row.Int;
        r.Spi = row.Spi;
    }

    protected override void WriteKey(LevelStatsRow row, PlayerLevelstatsRecord r) => r.Level = row.Level;

    protected override IEnumerable<LevelStatsRow> Order(IEnumerable<LevelStatsRow> rows) => rows.OrderBy(r => r.Level);
}

[Subsystem(nameof(WorldStorage))]
public sealed class ClassLevelStatsFactory : PlayerRowsFactory<PlayerClasslevelstatsRecord, ClassLevelStatsRow>
{
    public ClassLevelStatsFactory(WorldStorage storage) : base(storage, ["Level", "Base Health", "Base Mana"])
    {
    }

    public override string CatalogName => "Class Level Stats";

    protected override Expression<Func<PlayerClasslevelstatsRecord, int>> EntryExpression()
    {
        ParameterExpression p = Expression.Parameter(typeof(PlayerClasslevelstatsRecord), "r");
        return Expression.Lambda<Func<PlayerClasslevelstatsRecord, int>>(Expression.Property(p, nameof(PlayerClasslevelstatsRecord.Class)), p);
    }

    protected override void SetEntry(PlayerClasslevelstatsRecord r, int entry) => r.Class = entry;

    protected override ClassLevelStatsRow ToRow(PlayerClasslevelstatsRecord r) =>
        new() { Level = r.Level, BaseHp = r.Basehp, BaseMana = r.Basemana };

    protected override void WriteRow(ClassLevelStatsRow row, PlayerClasslevelstatsRecord r)
    {
        r.Basehp = row.BaseHp;
        r.Basemana = row.BaseMana;
    }

    protected override void WriteKey(ClassLevelStatsRow row, PlayerClasslevelstatsRecord r) => r.Level = row.Level;

    protected override IEnumerable<ClassLevelStatsRow> Order(IEnumerable<ClassLevelStatsRow> rows) => rows.OrderBy(r => r.Level);
}

/// <summary>Keys the player tables on their natural columns.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class PlayerTableKeys : ITableConfiguration
{
    public PlayerTableKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<PlayercreateinfoSpellRecord>().HasKey(r => new { r.Race, r.Class, r.Spell });
        model.Entity<PlayercreateinfoItemRecord>().HasKey(r => new { r.Race, r.Class, r.Itemid });
        model.Entity<PlayercreateinfoActionRecord>().HasKey(r => new { r.Race, r.Class, r.Button });
        model.Entity<PlayerLevelstatsRecord>().HasKey(r => new { r.Race, r.Class, r.Level });
        model.Entity<PlayerClasslevelstatsRecord>().HasKey(r => new { r.Class, r.Level });
    }
}
