using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`npc_trainer` / `npc_trainer_template` as catalogs keyed by `entry`. The tables carry no
/// primary key; <see cref="TrainerTableKeys"/> keys them on (entry, spell) so rows can be tracked.</summary>
public abstract class TrainerFactory<TRecord> : EntryRowsFactory<TRecord, TrainerTable, TrainerRow>
    where TRecord : class, new()
{
    private static readonly System.Reflection.PropertyInfo[] Props =
        new[] { "Spell", "Spellcost", "Reqskill", "Reqskillvalue", "Reqlevel" }
            .Select(n => typeof(TRecord).GetProperty(n)!).ToArray();

    protected TrainerFactory(WorldStorage storage) : base(storage)
    {
    }

    private static int Get(TRecord r, int i) => (int)Props[i].GetValue(r)!;

    protected override TrainerTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override TrainerRow ToRow(TRecord r) => new()
    {
        Spell = Get(r, 0),
        SpellCost = Get(r, 1),
        ReqSkill = Get(r, 2),
        ReqSkillValue = Get(r, 3),
        ReqLevel = Get(r, 4),
    };

    protected override void WriteRow(TrainerRow row, TRecord r)
    {
        Props[1].SetValue(r, row.SpellCost);
        Props[2].SetValue(r, row.ReqSkill);
        Props[3].SetValue(r, row.ReqSkillValue);
        Props[4].SetValue(r, row.ReqLevel);
    }

    protected override void WriteKey(TrainerRow row, TRecord r) => Props[0].SetValue(r, row.Spell);

    protected override object KeyOf(TrainerRow row) => row.Spell;

    protected override IEnumerable<TrainerRow> Order(IEnumerable<TrainerRow> rows) =>
        rows.OrderBy(r => r.ReqLevel).ThenBy(r => r.Spell);

    protected override string DuplicateMessage(int entry, TrainerRow row) =>
        $"{Kind} {entry} lists spell {row.Spell} twice.";

    protected override void DrawRows(EditorContext context, TrainerTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            TrainerRow row = table.Rows[i];
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

            RowWidgets.Int(context, "Spell", row.Spell, v => table.SetRowField(index, nameof(TrainerRow.Spell), v));
            RowWidgets.Int(context, "Cost (copper)", row.SpellCost, v => table.SetRowField(index, nameof(TrainerRow.SpellCost), v));
            RowWidgets.Int(context, "Required Skill", row.ReqSkill, v => table.SetRowField(index, nameof(TrainerRow.ReqSkill), v));
            RowWidgets.Int(context, "Required Skill Value", row.ReqSkillValue, v => table.SetRowField(index, nameof(TrainerRow.ReqSkillValue), v));
            RowWidgets.Int(context, "Required Level", row.ReqLevel, v => table.SetRowField(index, nameof(TrainerRow.ReqLevel), v));
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
public sealed class NpcTrainerFactory : TrainerFactory<NpcTrainerRecord>
{
    public NpcTrainerFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Trainer";
}

[Subsystem(nameof(WorldStorage))]
public sealed class NpcTrainerTemplateFactory : TrainerFactory<NpcTrainerTemplateRecord>
{
    public NpcTrainerTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Trainer Template";
}

/// <summary>Keys the keyless trainer tables on (entry, spell).</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class TrainerTableKeys : ITableConfiguration
{
    public TrainerTableKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<NpcTrainerRecord>().HasKey(r => new { r.Entry, r.Spell });
        model.Entity<NpcTrainerTemplateRecord>().HasKey(r => new { r.Entry, r.Spell });
    }
}
