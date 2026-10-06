using System;
using System.Collections.Generic;
using System.Linq.Expressions;
using Microsoft.EntityFrameworkCore;

namespace WorldMapStudio.WoW;

/// <summary>`gameobject_template` as a browsable catalog: id or name search, the common fields editable in place.</summary>
[Subsystem(nameof(WorldStorage))]
public sealed class GameObjectTemplateFactory : ThinDbcFactory<GameObjectTemplate, GameobjectTemplateRecord>
{
    public GameObjectTemplateFactory(WorldStorage storage) : base(storage)
    {
    }

    public override string CatalogName => "GameObject Template";

    protected override string KeyPropertyName => "Entry";

    protected override string DisplayText(GameobjectTemplateRecord record) => record.Name;

    protected override Expression<Func<GameobjectTemplateRecord, bool>> TextFilter(string term) =>
        record => EF.Functions.Like(record.Name, $"%{term}%");

    protected override GameObjectTemplate ToEntity(GameobjectTemplateRecord record) => new()
    {
        ID = record.Entry,
        Name = record.Name,
        Type = record.Type,
        DisplayId = record.DisplayId,
        Faction = record.Faction,
        Flags = record.Flags,
        Size = record.Size,
        ScriptName = record.ScriptName,
        IsSaved = true,
    };

    protected override void Write(GameObjectTemplate entity, GameobjectTemplateRecord record)
    {
        record.Entry = entity.ID;
        record.Name = entity.Name;
        record.Type = entity.Type;
        record.DisplayId = entity.DisplayId;
        record.Faction = entity.Faction;
        record.Flags = entity.Flags;
        record.Size = entity.Size;
        record.ScriptName = entity.ScriptName;
    }

    protected override IEnumerable<CatalogFieldSpec> Fields(GameObjectTemplate e) =>
    [
        F.Text("Name", () => e.Name, v => e.Name = v, 100),
        F.Int("Type", () => e.Type, v => e.Type = v),
        F.Int("Display Id", () => e.DisplayId, v => e.DisplayId = v),
        F.Int("Faction", () => e.Faction, v => e.Faction = v),
        F.Int("Flags", () => e.Flags, v => e.Flags = v),
        F.Float("Size", () => e.Size, v => e.Size = v),
        F.Text("Script Name", () => e.ScriptName, v => e.ScriptName = v, 64),
    ];

    internal GameObjectTemplate CallToEntity(GameobjectTemplateRecord record) => ToEntity(record);

    internal void CallWrite(GameObjectTemplate entity, GameobjectTemplateRecord record) => Write(entity, record);
}
