using System;
using System.Collections.Generic;
using System.Linq;
using ImGuiNET;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>One quest a creature or object starts or ends.</summary>
public readonly record struct QuestRelationRow
{
    public int Quest { get; init; }
}

/// <summary>The quests one creature or gameobject starts or ends.</summary>
public sealed class QuestRelationTable : EntryRowTable<QuestRelationRow>
{
    public QuestRelationTable(string kind) : base(kind)
    {
    }

    [ScriptFunction]
    public IEditCommand AddRow(int quest) => AppendRow(new QuestRelationRow { Quest = quest });

    [ScriptFunction]
    public IEditCommand RemoveRow(int index) => RemoveRowAt(index);

    [ScriptFunction]
    public IEditCommand SetQuest(int index, int quest) => UpdateRow(index, row => row with { Quest = quest });
}

/// <summary>`creature_questrelation` and friends as catalogs keyed by the creature or object entry.</summary>
public abstract class QuestRelationFactory<TRecord> : EntryRowsFactory<TRecord, QuestRelationTable, QuestRelationRow>
    where TRecord : class, new()
{
    private static readonly System.Reflection.PropertyInfo QuestProp = typeof(TRecord).GetProperty("Quest")!;

    protected QuestRelationFactory(WorldStorage storage) : base(storage)
    {
    }

    protected override QuestRelationTable NewTable(int entry) => new(Kind) { ID = entry };

    protected override QuestRelationRow ToRow(TRecord r) => new() { Quest = (int)QuestProp.GetValue(r)! };

    protected override void WriteRow(QuestRelationRow row, TRecord r)
    {
    }

    protected override void WriteKey(QuestRelationRow row, TRecord r) => QuestProp.SetValue(r, row.Quest);

    protected override object KeyOf(QuestRelationRow row) => row.Quest;

    protected override IEnumerable<QuestRelationRow> Order(IEnumerable<QuestRelationRow> rows) => rows.OrderBy(r => r.Quest);

    protected override string DuplicateMessage(int entry, QuestRelationRow row) =>
        $"{Kind} {entry} lists quest {row.Quest} twice.";

    protected override void DrawRows(EditorContext context, QuestRelationTable table)
    {
        for (int i = 0; i < table.Rows.Count; i++)
        {
            int index = i;
            ImGui.PushID(index);
            RowWidgets.Int(context, "Quest", table.Rows[i].Quest, v => table.SetQuest(index, (int)v));
            ImGui.SameLine();
            if (ImGui.SmallButton("Remove"))
            {
                Apply(context, table.RemoveRow(index));
                ImGui.PopID();
                break;
            }

            ImGui.PopID();
        }

        if (ImGui.Button("Add Quest"))
        {
            Apply(context, table.AddRow(0));
        }
    }
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureQuestStarterFactory : QuestRelationFactory<CreatureQuestrelationRecord>
{
    public CreatureQuestStarterFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Quest Starts";
}

[Subsystem(nameof(WorldStorage))]
public sealed class CreatureQuestEnderFactory : QuestRelationFactory<CreatureInvolvedrelationRecord>
{
    public CreatureQuestEnderFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Creature Quest Ends";
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameObjectQuestStarterFactory : QuestRelationFactory<GameobjectQuestrelationRecord>
{
    public GameObjectQuestStarterFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gameobject Quest Starts";
}

[Subsystem(nameof(WorldStorage))]
public sealed class GameObjectQuestEnderFactory : QuestRelationFactory<GameobjectInvolvedrelationRecord>
{
    public GameObjectQuestEnderFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Gameobject Quest Ends";
}

/// <summary>Keys the relation tables on (id, quest).</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class QuestRelationTableKeys : ITableConfiguration
{
    public QuestRelationTableKeys(WorldStorage storage)
    {
    }

    public void Configure(ModelBuilder model)
    {
        model.Entity<CreatureQuestrelationRecord>().HasKey(r => new { r.Id, r.Quest });
        model.Entity<CreatureInvolvedrelationRecord>().HasKey(r => new { r.Id, r.Quest });
        model.Entity<GameobjectQuestrelationRecord>().HasKey(r => new { r.Id, r.Quest });
        model.Entity<GameobjectInvolvedrelationRecord>().HasKey(r => new { r.Id, r.Quest });
    }
}
