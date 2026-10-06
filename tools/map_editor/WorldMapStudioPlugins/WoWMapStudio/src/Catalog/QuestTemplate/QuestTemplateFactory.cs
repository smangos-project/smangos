using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`quest_template` as a browsable catalog: id or title search, the core fields editable in place.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class QuestTemplateFactory : ThinDbcFactory<QuestTemplate, QuestTemplateRecord>
{
    public QuestTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "Quest";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(QuestTemplateRecord record) => record.Title ?? string.Empty;

    protected override Expression<Func<QuestTemplateRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.Title, $"%{term}%");

    protected override QuestTemplate ToEntity(QuestTemplateRecord record) => new()
    {
        ID = record.Entry,
        Title = record.Title ?? string.Empty,
        Details = record.Details ?? string.Empty,
        Objectives = record.Objectives ?? string.Empty,
        ZoneOrSort = record.ZoneOrSort,
        MinLevel = record.MinLevel,
        QuestLevel = record.QuestLevel,
        Type = record.Type,
        RequiredClasses = record.RequiredClasses,
        RequiredRaces = record.RequiredRaces,
        PrevQuestId = record.PrevQuestId,
        NextQuestId = record.NextQuestId,
        NextQuestInChain = record.NextQuestInChain,
        QuestFlags = record.QuestFlags,
        SrcItemId = record.SrcItemId,
        IsSaved = true,
    };

    protected override void Write(QuestTemplate entity, QuestTemplateRecord record)
    {
        record.Entry = entity.ID;
        record.Title = entity.Title;
        record.Details = entity.Details;
        record.Objectives = entity.Objectives;
        record.ZoneOrSort = entity.ZoneOrSort;
        record.MinLevel = entity.MinLevel;
        record.QuestLevel = entity.QuestLevel;
        record.Type = entity.Type;
        record.RequiredClasses = entity.RequiredClasses;
        record.RequiredRaces = entity.RequiredRaces;
        record.PrevQuestId = entity.PrevQuestId;
        record.NextQuestId = entity.NextQuestId;
        record.NextQuestInChain = entity.NextQuestInChain;
        record.QuestFlags = entity.QuestFlags;
        record.SrcItemId = entity.SrcItemId;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(QuestTemplate e) =>
    [
        F.Text("Title", () => e.Title, v => e.Title = v, 255),
        F.TextMultiline("Details", () => e.Details, v => e.Details = v),
        F.TextMultiline("Objectives", () => e.Objectives, v => e.Objectives = v),
        F.Int("Zone Or Sort", () => e.ZoneOrSort, v => e.ZoneOrSort = v),
        F.Int("Min Level", () => e.MinLevel, v => e.MinLevel = v),
        F.Int("Quest Level", () => e.QuestLevel, v => e.QuestLevel = v),
        F.Int("Type", () => e.Type, v => e.Type = v),
        F.Int("Required Classes", () => e.RequiredClasses, v => e.RequiredClasses = v),
        F.Int("Required Races", () => e.RequiredRaces, v => e.RequiredRaces = v),
        F.Link("Previous Quest", () => e.PrevQuestId, v => e.PrevQuestId = v, "Quest"),
        F.Link("Next Quest", () => e.NextQuestId, v => e.NextQuestId = v, "Quest"),
        F.Link("Next In Chain", () => e.NextQuestInChain, v => e.NextQuestInChain = v, "Quest"),
        F.Int("Quest Flags", () => e.QuestFlags, v => e.QuestFlags = v),
        F.Link("Start Item", () => e.SrcItemId, v => e.SrcItemId = v, "Item"),
    ];

    internal QuestTemplate CallToEntity(QuestTemplateRecord record) => ToEntity(record);

    internal void CallWrite(QuestTemplate entity, QuestTemplateRecord record) => Write(entity, record);
}
