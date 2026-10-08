using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>A `pool_template` row. `ID` is the pool entry.</summary>
public sealed class PoolTemplate : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MaxLimit { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Description { get; set; } = string.Empty;

    [ScriptProperty(Mutable = true)]
    public int Flags { get; set; }

    [ScriptProperty(Mutable = true)]
    public int Instance { get; set; }

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Pool {ID}: {Description}";
}

/// <summary>A `creature_addon` row. `ID` is the creature spawn guid.</summary>
public sealed class CreatureAddon : CatalogEntity, IDbcCatalogEntity
{
    [ScriptProperty]
    public int ID { get; set; }

    [ScriptProperty(Mutable = true)]
    public int DisplayId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int MountDisplayId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EquipmentId { get; set; }

    [ScriptProperty(Mutable = true)]
    public int StandState { get; set; }

    [ScriptProperty(Mutable = true)]
    public int SheathState { get; set; }

    [ScriptProperty(Mutable = true)]
    public int EmoteState { get; set; }

    [ScriptProperty(Mutable = true)]
    public string Auras { get; set; } = string.Empty;

    public bool IsSaved { get; set; }

    public override string DisplayName => $"Creature Addon {ID}";
}

[Subsystem(nameof(WorldStorage))]
public sealed class PoolTemplateFactory : ThinDbcFactory<PoolTemplate, PoolTemplateRecord>
{
    public PoolTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Pool";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(PoolTemplateRecord record) => record.Description;

    protected override Expression<Func<PoolTemplateRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.Description, $"%{term}%");

    protected override PoolTemplate ToEntity(PoolTemplateRecord r) => new()
    {
        ID = r.Entry,
        MaxLimit = r.MaxLimit,
        Description = r.Description,
        Flags = r.Flags,
        Instance = r.Instance,
        IsSaved = true,
    };

    protected override void Write(PoolTemplate e, PoolTemplateRecord r)
    {
        r.Entry = e.ID;
        r.MaxLimit = e.MaxLimit;
        r.Description = e.Description;
        r.Flags = e.Flags;
        r.Instance = e.Instance;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(PoolTemplate e) =>
    [
        F.Text("Description", () => e.Description, v => e.Description = v, 255),
        F.Int("Max Limit", () => e.MaxLimit, v => e.MaxLimit = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
        F.Int("Instance", () => e.Instance, v => e.Instance = v),
    ];

    internal PoolTemplate CallToEntity(PoolTemplateRecord record) => ToEntity(record);

    internal void CallWrite(PoolTemplate entity, PoolTemplateRecord record) => Write(entity, record);
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureAddonFactory : ThinDbcFactory<CreatureAddon, CreatureAddonRecord>
{
    public CreatureAddonFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Addon";

    protected override string KeyPropertyName => "Guid";

    protected override CreatureAddon ToEntity(CreatureAddonRecord r) => new()
    {
        ID = r.Guid,
        DisplayId = r.DisplayId,
        MountDisplayId = r.MountDisplayId,
        EquipmentId = r.EquipmentId,
        StandState = r.StandState,
        SheathState = r.SheathState,
        EmoteState = r.EmoteState,
        Auras = r.Auras ?? string.Empty,
        IsSaved = true,
    };

    protected override void Write(CreatureAddon e, CreatureAddonRecord r)
    {
        r.Guid = e.ID;
        r.DisplayId = e.DisplayId;
        r.MountDisplayId = e.MountDisplayId;
        r.EquipmentId = e.EquipmentId;
        r.StandState = e.StandState;
        r.SheathState = e.SheathState;
        r.EmoteState = e.EmoteState;
        r.Auras = e.Auras.Length == 0 ? null : e.Auras;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(CreatureAddon e) =>
    [
        F.Int("Display Id", () => e.DisplayId, v => e.DisplayId = v),
        F.Int("Mount Display Id", () => e.MountDisplayId, v => e.MountDisplayId = v),
        F.Link("Equipment", () => e.EquipmentId, v => e.EquipmentId = v, "Creature Equipment"),
        F.Int("Stand State", () => e.StandState, v => e.StandState = v),
        F.Int("Sheath State", () => e.SheathState, v => e.SheathState = v),
        F.Int("Emote State", () => e.EmoteState, v => e.EmoteState = v),
        F.Text("Auras", () => e.Auras, v => e.Auras = v, 1024),
    ];

    internal CreatureAddon CallToEntity(CreatureAddonRecord record) => ToEntity(record);

    internal void CallWrite(CreatureAddon entity, CreatureAddonRecord record) => Write(entity, record);
}

/// <summary>One weighted equipment set of `creature_equip_template`.</summary>
public readonly record struct EquipRow
{
    public int Item1 { get; init; }
    public int Item2 { get; init; }
    public int Item3 { get; init; }
    public int Probability { get; init; }
}

/// <summary>The equipment sets of one `creature_equip_template` entry.</summary>
public sealed class EquipTable : EntryRowTable<EquipRow>
{
    public EquipTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int item1) => AppendRow(new EquipRow { Item1 = item1, Probability = 100 });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    [ScriptFunction]
    public IEditCommand SetRowField(int index, string field, double value) => UpdateRow(index, row => field switch
    {
        nameof(EquipRow.Item1) => row with { Item1 = (int)value },
        nameof(EquipRow.Item2) => row with { Item2 = (int)value },
        nameof(EquipRow.Item3) => row with { Item3 = (int)value },
        nameof(EquipRow.Probability) => row with { Probability = (int)value },
        _ => throw new InvalidOperationException($"Unknown equipment field '{field}'."),
    });
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureEquipFactory : EntryRowsFactory<CreatureEquipTemplateRecord, EquipTable, EquipRow>
{
    public CreatureEquipFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Equipment";

    protected override EquipTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override EquipRow ToRow(CreatureEquipTemplateRecord r) => new()
    {
        Item1 = r.Item1,
        Item2 = r.Item2,
        Item3 = r.Item3,
        Probability = r.Probability,
    };

    protected override void WriteRow(EquipRow row, CreatureEquipTemplateRecord r) => r.Probability = row.Probability;

    protected override void WriteKey(EquipRow row, CreatureEquipTemplateRecord r)
    {
        r.Item1 = row.Item1;
        r.Item2 = row.Item2;
        r.Item3 = row.Item3;
    }

    protected override object KeyOf(EquipRow row) => (row.Item1, row.Item2, row.Item3);

    protected override string DuplicateMessage(int entry, EquipRow row) =>
        $"{Kind} {entry} lists the same items twice.";

    protected override void DrawRows(EditorContext context, EquipTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            EquipRow row = table.Rows[i];
            int index = i;
            ImGui.PushID(index);
            ImGui.Text($"Set {index}");
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            RowWidgets.Int(context, "Main Hand", row.Item1, v => table.SetRowField(index, nameof(EquipRow.Item1), v));
            RowWidgets.Int(context, "Off Hand", row.Item2, v => table.SetRowField(index, nameof(EquipRow.Item2), v));
            RowWidgets.Int(context, "Ranged", row.Item3, v => table.SetRowField(index, nameof(EquipRow.Item3), v));
            RowWidgets.Int(context, "Probability", row.Probability, v => table.SetRowField(index, nameof(EquipRow.Probability), v));
            ImGui.Separator();
            ImGui.PopID();
        }

        if (ImGui.Button("Add Set"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}

/// <summary>Keys `creature_equip_template` on (entry, item1, item2, item3).</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class EquipTemplateKeys : ITableConfiguration
{
    public EquipTemplateKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model) =>
        model.Entity<CreatureEquipTemplateRecord>().HasKey(r => new { r.Entry, r.Item1, r.Item2, r.Item3 });
}
